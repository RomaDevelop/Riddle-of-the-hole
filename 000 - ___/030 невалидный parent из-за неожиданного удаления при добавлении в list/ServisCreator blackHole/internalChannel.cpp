
#include "logs.h"
#include "code.h"
#include "allConstants.h"
#include "internalChannel.h"

InternalChannel::InternalChannel()
{
	timerReader = std::make_shared<QTimer>();
	QObject::connect(timerReader.get(), &QTimer::timeout, [this](){
		if(externalStarted)
		{
			auto getCntRes = externalChannel->GetMsgCnt();
			if(getCntRes.first == ICANChannel::NO_ERRORS)
			{
				for(int i=0; i<getCntRes.second; i++)
				{
					CANMsg_t msg;
					int countReading = 1;
					auto resReadMsg = externalChannel->ReadMsg(&msg, countReading);
					if(resReadMsg.first == ICANChannel::NO_ERRORS)
					{
						if(resReadMsg.second == countReading)
						{
							for(auto &client:clients)
							{
								client->GiveClientMsg(msg);
							}
						}
						else Logs::ErrorSt("InternalChannel::Init lambdaReader can't read msg from channel");
					}
					else Logs::ErrorSt("InternalChannel::Init lambdaReader ReadMsg error " + ICANChannel::ErrorCodeToStr(resReadMsg.first));
				}
			}
			else Logs::ErrorSt("InternalChannel::Init lambdaReader GetMsgCnt error " + ICANChannel::ErrorCodeToStr(getCntRes.first));
		}
	});
	timerReader->start(50);
}

InternalChannel::~InternalChannel()
{
	externalChannel->Stop();
	externalStarted = false;
}

bool InternalChannel::Init(const QString & name_, CANInterfaces * interfaces_, const QString & params)
{
	name = name_;
	interfaces = interfaces_;
	externalChannel = nullptr;
	int undefinedNumber = -1;
	int number = undefinedNumber;
	QString regime;
	QString baud;

	auto commands = Code::TextToCommands(params);
	for(auto &command:commands)
	{
		auto words = Code::CommandToWords(command);
		if(words.size() == 3 && words[1] == "=")
		{
			if(words[0] == "number") number = words[2].toInt();
			else if(words[0] == "regime") regime = words[2];
			else if(words[0] == "baud") baud = words[2];
			else Logs::ErrorSt("InternalServer::Init wrong word ["+words[0]+"] in params ["+params+"]");
		}
	}

	if(number != undefinedNumber && regime.size() && baud.size())
	{
		externalChannel = StartChannel(number,regime,baud);
		if(externalChannel)
		{
			return true;
		}
		else Logs::ErrorSt("InternalServer::Init StartChannel returned nullptr");
	}
	else Logs::ErrorSt("InternalServer::Init bad params ["+params+"]");

	return false;
}

bool InternalChannel::AddClient(InternalClient * client)
{
	if(client)
	{
		clients.push_back(client);
		return true;
	}
	return false;
}

int InternalChannel::WriteMsg(const CANMsg_t & msg, InternalClient * sender)
{
	if(!externalChannel) return ICANChannel::NO_DEVICE;
	if(!externalStarted) return ICANChannel::NO_ERRORS;

	int res = externalChannel->WriteMsg(&msg);
	if(res != ICANChannel::NO_ERRORS) return res;

	if(!forwardOnly)
	{
		for(auto &client:clients)
		{
			if(client != sender)
				client->GiveClientMsg(msg);
		}
	}

	return ICANChannel::NO_ERRORS;
}

ICANChannel * InternalChannel::StartChannel(int chanNumber, QString regimeStr, QString baudStr)
{
	ICANChannel* channel = nullptr;
	int regime = GetICANRegime(regimeStr);
	int baud = GetICANBaud(baudStr);

	if(interfaces)
	{

		channel = interfaces->GetChannel(chanNumber);
		if(channel)
		{
			int res = channel->SetRegime(regime);
			if(res == ICANChannel::NO_ERRORS)
			{
				res = channel->SetBaud(baud);
				if(res == ICANChannel::NO_ERRORS)
				{
					res = channel->Start();
					if(res == ICANChannel::NO_ERRORS)
					{
						externalStarted = true;
						Logs::LogSt("InternalServer::StartChannel Channel "+QSn(chanNumber)
									+" started whith regime " + ICANChannel::RegimeToStr(channel->GetRegime())
									+ " and baud " + ICANChannel::BaudToStr(channel->GetBaud()));
					}
					else Logs::ErrorSt("InternalServer::StartChannel Start error ("+ICANChannel::ErrorCodeToStr(res)+")");
				}
				else Logs::ErrorSt("InternalServer::StartChannel SetBaud error ("+ICANChannel::ErrorCodeToStr(res)+")");
			}
			else Logs::ErrorSt("InternalServer::StartChannel SetRegime error ("+ICANChannel::ErrorCodeToStr(res)+")");
		}
		else Logs::ErrorSt("InternalServer::StartChannel channels.size() != 1");
	}
	else Logs::ErrorSt("InternalServer::StartChannel interfaces nullptr");

	if(!channel) Logs::ErrorSt("InternalServer::StartChannel returnes nullptr");
	return channel;
}

int InternalChannel::GetICANRegime(QString regimeStr)
{
	if(regimeStr == ChannelConfigItem_ns::Settings::reg11) return ICANChannel::REG_CAN11;
	if(regimeStr == ChannelConfigItem_ns::Settings::reg29) return ICANChannel::REG_CAN29;
	if(regimeStr == ChannelConfigItem_ns::Settings::reg1129) return ICANChannel::REG_CAN11_29;
	Logs::ErrorSt("Convert::GetICANRegime wrong regimeStr ["+regimeStr+"]");
	return ICANChannel::REG_ERROR;
}

int InternalChannel::GetICANBaud(QString baudStr)
{
	if(baudStr == ChannelConfigItem_ns::Settings::baud250) return ICANChannel::BR_250K;
	if(baudStr == ChannelConfigItem_ns::Settings::baud500) return ICANChannel::BR_500K;
	if(baudStr == ChannelConfigItem_ns::Settings::baud1000) return ICANChannel::BR_1M;
	Logs::ErrorSt("Convert::GetICANBaud wrong baudStr ["+baudStr+"]");
	return ICANChannel::BR_ERROR;
}
