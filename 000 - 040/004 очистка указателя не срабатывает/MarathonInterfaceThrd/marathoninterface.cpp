#include <algorithm>

#include <QDebug>

#include "marathonchannel.h"
#include "marathoninterface.h"

bool MarathonInterface::inited = false;

MarathonInterface::MarathonInterface()
{
	number = 0;
	name = "MarathonInterface";

	CBFLog = nullptr;
}
MarathonInterface::~MarathonInterface()
{
	WriteLog("~MarathonInterface() begin");
	ClearChannels();
	WriteLog("~MarathonInterface() end");
}

void MarathonInterface::ClearChannels()
{
	if(channels.size())
	{
		for(auto ch:channels)
		{
			ch->Stop();
			delete ch;
		}
		channels.clear();
	}
}

int MarathonInterface::ScanInterface()
{
	if(!inited)
	{
		if(CiInit() == ECIOK) inited = true;
		else return INIT_ERROR;
	}

	if(inited)
	{
		vector<int> started_us_indexes;
		for(auto ch:channels)
			if(ch->GetStatus() == ICANChannel::STARTED)
				started_us_indexes.push_back(ch->GetNumber());

		ClearChannels();

		for(int i=0;i<CI_BRD_NUMS;i++)
		{
			canboard_t board;
			board.brdnum = (_u8)i;
			_s16 resultBoardInfo = CiBoardInfo(&board);
			if(resultBoardInfo >= 0)
			{
				for(int j=0; j<4; j++)
				{
					if(board.chip[j]>=0)
					{
						int ChannelNumber = board.chip[j];
						QString boardName = "";
						for(int k=0; (k < 64) && (board.name[k] != '\0'); k++)
						{
							boardName.append(board.name[k]);
						}
						QString ChannelName=QString("Channel %1 on %2").arg(ChannelNumber).arg(boardName);

						ICANChannel *channel = new MarathonChannel(ChannelNumber, ChannelName);
						channels.push_back(channel);
					}
				}
			}
		}
	}
	return NO_ERROR;
}

int	MarathonInterface::GetNumber() const
{
	return number;
}
QString MarathonInterface::GetName() const
{
	return name;
}
int MarathonInterface::GetChannelsCount() const
{
	return channels.size();
}
void MarathonInterface::GetChannels(vector<ICANChannel*> &channels_)
{
	channels_.resize(channels.size());
	copy(channels.begin(), channels.end(), channels_.begin());
}
void MarathonInterface::GetChannels(vector<ICANChannel*> &channels_, int status)
{
	channels_.clear();
	copy_if(channels.begin(),channels.end(), back_inserter(channels_),
			[status](ICANChannel *ch)
			{
				return ch->GetStatus() == status;
			});
}
void MarathonInterface::GetChannels_if(vector<ICANChannel*> &channels_, bool(*condition)(ICANChannel*))
{
	channels_.clear();
	copy_if(channels.begin(),channels.end(), back_inserter(channels_), condition);
}

void MarathonInterface::SetCBFunctionForLog(CBFunctionWriteLog CBFLog_)
{
	CBFLog = CBFLog_;
}
void MarathonInterface::SetCBFunctionForToAllChannels(CBFunctionWriteLog CBFLog_)
{
	for(auto ch:channels) ch->SetCBFunctionForLog(CBFLog_);
}

void MarathonInterface::WriteLog(QString log) const
{
	if(CBFLog) CBFLog("interface " + name + ": " + log);
}
