#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <QDateTime>

#include "icanchannel.h"

namespace nsTraffic
{
	using namespace std;

//=================================================================================================================================================
	struct TrafficMsg: public CANMsg_t
	{
		int chNumberAbsolut;
		int direction;
		unsigned int msgNumber;
		QDateTime dateTime;

		TrafficMsg() = default;
		TrafficMsg(int chNumAbs, int direction_, unsigned int msgNumber_, const QDateTime &dt, const CANMsg_t &msgSrc);

		enum DIRECTIONS
		{
			UNKNOWN = 0,
			READED = 1,
			WRITED = 2
		};
		static QString DirectionToStr(int code);
		static int DirectionFromStr(QString direction);

		QString DirectionToStr() const;
		QString ToStrTraffic() const;
		void FromStrTraffic(QString str_traffic);
	};
//=================================================================================================================================================
	typedef void (*TrafficMsgWorker_t)(const TrafficMsg &msg);
//=================================================================================================================================================
	class TrafficChannel
	{
		///\brief порог сообщений наполнения вектора
		static unsigned int thresholdWriting;

		static unsigned int msgNumber;

		typedef std::vector<TrafficMsg> vectorMessages;
		vectorMessages messages1;
		vectorMessages messages2;
		vectorMessages *msgsToFile = nullptr;
		vectorMessages *msgsToTraffic = nullptr;

		bool versionPrinted = false;
		bool writingNow = false;

		TrafficMsgWorker_t VieverCB = nullptr;

		///\brief создаёт поток, в котором vectorMessages *msgsToFile записывается в файл
		void WriteMessagesToFile();

	public:
		///\brief зписывает не записанные сообщения, дожидается окончания их записи (максимум 3 секунды)
		~TrafficChannel();

		ICANChannel *iCANChannel;
		static map<int, TrafficChannel*> mapChannels; // int - абсолютный номер канала

		///\brief Регистрирует обработчик траффика.
		/// Развязка из статики на конекретный TrafficChannel осуществляется через mapChannels
		/// поиск в mapChannels защищён mapMutex
		/// (можно уйти от мапы с мьютексом, заменить на динамический массив каналов с индексами абсолютного номера)
		/// при наполнении переключает векторы регистрации и записи в файл траффика
		/// вызывает WriteMessagesToFile();
		int StartWriting();
		void StopWriting();

		void SetViewerCB(TrafficMsgWorker_t function);

		static void SetThresholdWriting(unsigned int thresholdWriting_);
		static unsigned int GetThresholdWriting();
	};
//=================================================================================================================================================
	class Traffic
	{
		QString name;
		vector<TrafficChannel> channels;

		ICANChannel::CBFunctionWriteLog_t CBFLog = nullptr;

		void WriteLog(const QString &log) const;

	public:
		Traffic();
		Traffic(vector<ICANChannel *> &channels_, unsigned int thresholdWriting = 2000, bool strartWriting = true);
		~Traffic();

		void SetChannels(vector<ICANChannel *> &channels_, unsigned int thresholdWriting = 2000, bool strartWriting = true);
		std::vector<int> GetChNumbers();

		//  запись траффика осуществляется исходя из принципа:
		/// один канал не может отправлять и принимать сообщения в один момент времени
		//  если это изменится, потребуеются мьютекс в TrafficChannel::StartWriting()
		void StartWriting();
		void StopWriting();

		void SetCBFunctionForLog(ICANChannel::CBFunctionWriteLog_t CBFLog_);
		void SetViewerCB(TrafficMsgWorker_t function);

		static void SetThresholdWriting(unsigned int thresholdWriting_);
		static unsigned int GetThresholdWriting();
	};
};
#endif // TRAFFIC_H
