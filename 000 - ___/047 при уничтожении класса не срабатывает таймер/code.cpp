Загадка
	не выводится сообщение о завершении работы потока, уже в деструкторе подождал целую секунду!
Разгадка
	сообщение о завершении потока выводится таймером, таймер уничтожается не тикнув и не разобрав очередь
	ожидание sleep_for в деструкторе не помогает, поскольку оно замораживает и сам таймер не даёт ему тикнуть
	нужно руками в деструкторе вызвать функцию обработчик тика таймера

в классе: 
std::thread *thReadMsgCnt;
bool thReadMsgCntStop;

mutable std::queue<QString> logsFromThread;
mutable std::mutex mtxLogFromThread;
std::unique_ptr<QTimer> logsFromThreadTimer;



MarathonChannel::MarathonChannel(int numberOnInterface_, int numberAbsolut_, QString name_, MarathonInterface *parentInterface_):
	logsFromThreadTimer(new QTimer)
{
	QObject::connect(logsFromThreadTimer.get(), &QTimer::timeout, [this](){
		std::lock_guard<std::mutex> lock(mtxLogFromThread);
		while(!logsFromThread.empty())
		{
			WriteLog(logsFromThread.front());
			logsFromThread.pop();
		}
	});
	logsFromThreadTimer->start(10);
}

MarathonChannel::~MarathonChannel()
{
	WriteLog("~MarathonChannel()");
	if(thReadMsgCnt)
		StopThread(&thReadMsgCnt, thReadMsgCntStop);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int MarathonChannel::SetCBFunctionForMessage(CBFunction_t CBFunction)
{
	int resCmp = VersionCmp(GetVersion(parentInterface->GetVersionLib()), {2,10,0});
	_s16 result = NO_ERRORS;
	if(1/*resCmp == LEFT_NEWER*/)  // установка CBFext через поток
	{
		WriteLog("SetCBFunctionForMessage: current CHAI doesn't support CiSetCBex. Thread will be made.");
		if(CBFunction) // если передан указатель
		{
			if(thReadMsgCnt) // если поток существует, останавливаем и удаляем
				StopThread(&thReadMsgCnt, thReadMsgCntStop);

			// создание потока опрашивающего chai на наличие сообщений
			thReadMsgCnt = new thread([CBFunction, this]{
				WriteLogFromThread("CBFunctionForMessage thread started");
				canwait_t wait;
				wait.chan = numberOnInterface;
				wait.wflags = CI_WAIT_RC;
				while(!thReadMsgCntStop)
				{
					_s16 result = CiWaitEvent(&wait, 1, 50);
					if(result > 0) // > 0 - успешное выполнение: произошло одно из указанных событий;
					{
						CBFunction(numberAbsolut);
					}
					else if(result < 0)
					{
						WriteLogFromThread("Ошибка при выполнении SetCBFunctionForMessage, CHAI::CiSetCBex вернуло значение " + QString::number(result));
					}
					// else не нужно, result == 0, значит просто истёк таймаут


					//int cnt = CiRcGetCnt(numberOnInterface);
					//if(cnt > 0) CBFunction(numberAbsolut);
				}
				WriteLogFromThread("CBFunctionForMessage thread stoped");
			});
		}
		else // если передано nullptr
		{
			WriteLog("В SetCBFunctionForMessage передано nullptr");
			if(thReadMsgCnt) // если поток существует, останавливаем и удаляем
				StopThread(&thReadMsgCnt, thReadMsgCntStop);
			// else если поток не существует, ничего не делать надо
		}
	}
	else // установка CBFext через CHAI
	{
		WriteLog("SetCBFunctionForMessage: current CHAI support CiSetCBex.");
		_s16 result = CiSetCB((_u8)numberOnInterface, CIEV_RC, ci_cb(CBFunction));
		if(result != ECIOK) WriteLog("Ошибка при выполнении SetCBFunctionForMessage, CHAI::CiSetCBex вернуло значение " + QString::number(result));
		result = ConvertErrorCode(result);
	}

	return result;
}