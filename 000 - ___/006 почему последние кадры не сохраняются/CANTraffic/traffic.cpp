#include <thread>
#include <mutex>
#include <chrono>
#include <fstream>

#include <QDir>
#include <QFile>
#include <QTime>
#include <QCoreApplication>
#include <QMessageBox>

#include "MyQShortings.h"

#include "traffic.h"

using namespace nsTraffic; 

QDir pathTraffic;
QString filePrefix;
mutex mapMutex;
unsigned int TrafficChannel::msgNumber = 0;
map<int, TrafficChannel*> TrafficChannel::mapChannels;
unsigned int TrafficChannel::thresholdWriting {2000};  // sizeof(CANMsgTr) = 52 байта; 2000 это примерно 100 КБайт
//=================================================================================================================================================
TrafficMsg::TrafficMsg(int chNumAbs, int direction_, unsigned int msgNumber_, const QDateTime &dt, const CANMsg_t &msgSrc)
	: chNumberAbsolut{chNumAbs}, direction {direction_}, msgNumber{msgNumber_}, dateTime{dt}
{
	msgSrc.CopyCanMsg(this);
}

QString TrafficMsg::DirectionToStr(int code)
{
	switch (code)
	{
	case UNKNOWN : return "UNKNOWN";
	case READED : return "READED";
	case WRITED : return "WRITED";
	default: return "Wrong status code!";
	}
}

QString TrafficMsg::DirectionToStr() const
{
	switch (direction)
	{
	case UNKNOWN : return "UNKNOWN";
	case READED : return "READED";
	case WRITED : return "WRITED";
	default: return "Wrong status code!";
	}
}

int TrafficMsg::DirectionFromStr(QString direction)
{
	if(direction == "UNKNOWN") return UNKNOWN;
	else if(direction == "READED") return READED;
	else if(direction ==  "WRITED") return WRITED;
	else return UNKNOWN;
}

QString TrafficMsg::ToStrTraffic() const
{
	QString msg = QSn(msgNumber);
	while(msg.length() < 10) msg = "0" + msg;
	msg += "  ch_" + QSn(chNumberAbsolut) + "  " + DirectionToStr(direction) + "  ";
	msg += ToStrEx2(16) + "  " + dateTime.toString("yyyy.MM.dd HH.mm.ss.zzz");
	return msg;
}

void TrafficMsg::FromStrTraffic(QString str_traffic)
{
	msgNumber =                    str_traffic.left(10).toUInt();
	chNumberAbsolut =              str_traffic.mid(10+2+3,1).toInt();
	direction = DirectionFromStr(  str_traffic.mid(10+2+3+1+2,6));
	FromStrEx2_16(                 str_traffic.mid(10+2+3+1+2+6+2,71));
	dateTime = dateTime.fromString(str_traffic.mid(10+2+3+1+2+6+2+71+2,23), "yyyy.MM.dd HH.mm.ss.zzz");
}

//=================================================================================================================================================
TrafficChannel::~TrafficChannel()
{
	StopWriting();

	if(msgsToTraffic && !msgsToTraffic->empty())
	{
		msgsToFile = msgsToTraffic;
		WriteMessagesToFile();
		//this_thread::sleep_for(chrono::milliseconds(3000));
	}

	int i = 0;
	while(writingNow && i<30)
	{
		this_thread::sleep_for(chrono::milliseconds(100));
		i++;
	}
}

int TrafficChannel::StartWriting()
{
	msgsToTraffic = &messages1;
	msgsToTraffic->clear();
	msgsToTraffic->reserve(thresholdWriting);

	int result = iCANChannel->SetFunctionWriteTraffic(
	[](int chNumberAbsolut, bool read, const CANMsg_t &msg)
	{
		int direction = (read ? TrafficMsg::READED : TrafficMsg::WRITED);

		mapMutex.lock();
		TrafficChannel* currentChannel = mapChannels[chNumberAbsolut];
		mapMutex.unlock();

		if(currentChannel->msgsToTraffic->size() >= thresholdWriting) // если вектор наполнен
		{
			if(currentChannel->msgsToTraffic == &currentChannel->messages1) // если наполняли messages1
			{
				currentChannel->msgsToTraffic = &currentChannel->messages2;
				currentChannel->msgsToFile = &currentChannel->messages1;
			}
			else // если наполняли messages2
			{
				currentChannel->msgsToTraffic = &currentChannel->messages1;
				currentChannel->msgsToFile = &currentChannel->messages2;
			}

			currentChannel->msgsToTraffic->clear();
			currentChannel->msgsToTraffic->reserve(thresholdWriting);

			currentChannel->WriteMessagesToFile();
		}

		msgNumber++;
		currentChannel->msgsToTraffic->push_back(
					TrafficMsg(currentChannel->iCANChannel->GetNumberAbsolut(), direction, msgNumber, QDateTime::currentDateTime(), msg));

		// вызов обработчика траффика Вьювера
		if(currentChannel->VieverCB) currentChannel->VieverCB(currentChannel->msgsToTraffic->back());
	});
	return result;
}

void TrafficChannel::StopWriting()
{
	iCANChannel->SetFunctionWriteTraffic(nullptr);
}

void TrafficChannel::WriteMessagesToFile()
{
	this->writingNow = true;
	thread write([this]{
		QFile file(pathTraffic.path() + "/" + filePrefix + " ch_" + QSn(this->iCANChannel->GetNumberAbsolut()) + ".txt");
		if(file.open(QIODevice::Append))
		{
			if(!versionPrinted)
			{
				file.write(QString("TrafficChannel::WriteMessagesToFile v001\r\n").toUtf8());
				versionPrinted = true;
			}

			for(auto &msg:*this->msgsToFile)
			{
				file.write((msg.ToStrTraffic()).toUtf8());
				file.write(QString("\r\n").toUtf8());
			}
			file.close();
		}
		else
		{
			qDbg << "TrafficChannel::WriteMessagesToFile():\nОшибка! Не удалось открыть файл для записи траффика!";
			QMbw(nullptr, "Ошибка", "TrafficChannel::WriteMessagesToFile():\nНе удалось открыть файл для записи траффика!");
		}
		this->writingNow = false;
	});
	write.detach();
}

void TrafficChannel::SetViewerCB(nsTraffic::TrafficMsgWorker_t function)
{
	VieverCB = function;
}

void TrafficChannel::SetThresholdWriting(unsigned int thresholdWriting_)
{
	TrafficChannel::thresholdWriting = thresholdWriting_;
}

unsigned int TrafficChannel::GetThresholdWriting()
{
	return TrafficChannel::thresholdWriting;
}

//=================================================================================================================================================
Traffic::Traffic()
{
	QDir pathExe(QFileInfo(QCoreApplication::applicationFilePath()).path());
	QDir pathFiles(pathExe.path()+"/files");
	pathTraffic.setPath(pathFiles.path()+"/traffic");
	if(!pathFiles.exists())
	{
		pathFiles.mkdir(pathFiles.path());
		pathTraffic.mkdir(pathTraffic.path());
	}
	if(!pathTraffic.exists())
	{
		WriteLog("Ошибка создания директории для сохранения траффика");
		qDbg << "Ошибка создания директории для файла траффика";
		QMbw(nullptr, "Ошибка", "Ошибка создания директории для файла траффика");

		return;
	}

	filePrefix = QDateTime::currentDateTime().toString("yyyy.MM.dd HH.mm.ss");
}

Traffic::Traffic(vector<ICANChannel *> &channels_, unsigned int thresholdWriting, bool strartWriting)
{
	Traffic();
	SetChannels(channels_, thresholdWriting, strartWriting);
}

Traffic::~Traffic()
{
	StopWriting();
}

void Traffic::SetChannels(vector<ICANChannel *> &channels_, unsigned int thresholdWriting_, bool strartWriting)
{
	if(!channels.empty())
	{
		StopWriting();
		channels.clear();
		TrafficChannel::mapChannels.clear();
	}

	channels.resize(channels_.size());

	name = "Traffic{ ";
	for(unsigned int i=0; i<channels_.size(); i++)
	{
		ICANChannel *ch = channels_[i];
		name += QString::number(ch->GetNumberAbsolut()) + " ";

		channels[i].iCANChannel = ch;

		TrafficChannel::mapChannels[ch->GetNumberAbsolut()] = &channels[i];
	}
	name += "}";

	TrafficChannel::SetThresholdWriting(thresholdWriting_);

	if(strartWriting) StartWriting();
}

vector<int> Traffic::GetChNumbers()
{
	vector<int> retV;
	retV.resize(channels.size());
	for(unsigned int i=0; i<channels.size(); i++)
		retV[i] = channels[i].iCANChannel->GetNumberAbsolut();
	return retV;
}

void Traffic::StartWriting()
{
	for(auto &channel:channels)
	{
		if(channel.StartWriting() == ICANChannel::NO_ERROR)
		{
			WriteLog(channel.iCANChannel->GetName()+": регистрация траффика включена!");
		}
	}
}

void Traffic::StopWriting()
{
	for(auto &channel:channels)
		channel.StopWriting();
}

void Traffic::SetCBFunctionForLog(ICANChannel::CBFunctionWriteLog_t CBFLog_)
{
	CBFLog = CBFLog_;
}

void Traffic::WriteLog(const QString &log) const
{
	if(CBFLog) CBFLog(name + ": " + log);
}

void Traffic::SetViewerCB(nsTraffic::TrafficMsgWorker_t function)
{
	for(auto &channel:channels)
		channel.SetViewerCB(function);
}

void Traffic::SetThresholdWriting(unsigned int thresholdWriting_)
{
	TrafficChannel::SetThresholdWriting(thresholdWriting_);
}

unsigned int Traffic::GetThresholdWriting()
{
	return TrafficChannel::GetThresholdWriting();
}
