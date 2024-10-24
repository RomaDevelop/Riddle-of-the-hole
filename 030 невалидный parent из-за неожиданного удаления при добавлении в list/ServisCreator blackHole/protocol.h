#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <map>

#include "internalChannel.h"

#include "worker.h"
#include "object.h"

struct Timer
{
	Worker* frameWorkerTimer;
	std::unique_ptr<QTimer> qTimer;
	Timer(Worker* frameWorkerTimer_): frameWorkerTimer{frameWorkerTimer_} {}
};

class Protocol: public LogedClass, public HaveClassName, public InternalClient
{
	ItemsListForFillCats GetItemsListForFillCats();

	public:
	QString name;
	Object *connectedObject = nullptr;
	Constants *constants = nullptr;
	std::vector<std::unique_ptr<Worker>> workers;
	std::vector<u8> defaultIdFilterFragmentsLengths;
	std::vector<Timer> timers;
	Category category;

	QString channelStr;
	InternalChannel *internalChanPtr = nullptr;
	bool ignoreIncoming = false;
	bool muteOutgoing = false;
	std::queue<CANMsg_t> msgs;
	std::vector<Protocol*> network;
	std::vector<Protocol*> synchrone;
	QTimer* timerMsgsWorker = nullptr;

	int type = CodeKeyWords::codeUndefined;

	Protocol(const QDomElement & domElement, Object *objectToConnect_, Constants *constants_);
	~Protocol();

	void InitFilters();
	void ClearFilters();
	void InitVariables();
	void InitTimers();

	Worker* FindWorkerById(const QString &id);
	std::vector<Worker*> FindWorkersInCategory(const QString &category, const QString &nameW, bool includeSubCats);
	std::vector<Worker*> FindWorkers(const QString &nameW);

	///\brief создаёт сообщение для отправки,
	/// создаёт сообщения для отправки от параметров с которыми синхронизировано,
	/// возвращает вектор указателей на параметры и сообщений
	std::vector<WorkerAndFrame> ConstructFrames(Worker *frameWorker, std::vector<Value> *vidgetValues);
	void SetDataFilterInFrame(CANMsg_t &frame, CANMsgDataFilter_t filter);

	void SendFrames(std::vector<WorkerAndFrame> &toSend);
	void SendFrame(const CANMsg_t &msg, const Worker &frameWorker);

	Worker* CheckFilters(const CANMsg_t &msg);
	void GiveClientMsg(const CANMsg_t &msg) override;
	void WorkerIncommingFrames();
	void WorkerFramePassedFilter(CANMsg_t &frame, Worker *frameWorker);

	/// в некоторых случаях DoEverything возвращает указатель на объекты которые сам создаёт в куче,
	/// при этом они создаются через make_shared и записываются в createdIOperands
	/// при вызове уничтожении Object-а в котором они хранятся они тоже будут уничтожены
	IOperand* DoEverything(QString command, Param *thisParam, Worker *worker, CANMsg_t &frame,
						   std::vector<Value> *WidgetValues, QString retType);
	std::deque<std::shared_ptr<IOperand>> createdIOperands;
	std::deque<std::shared_ptr<HaveClassName>> createdHaveClassNames;
	void ClearOldCreatedIOperands();
	static QString DEretNullptr() {return "DEretNullptr";}
	static QString DEretPtr() {return "DEretPtr";}
	IOperand* DoFunction(QString command, Function* function, std::vector<IOperand*> params, Worker *frameWorker);
	IOperand* DoExpression(QString command, Expression &expression, Worker *frameWorker, QString retType);
	QString DataOperandFromWords(QStringList words, int DataOperandIndex, int & countWordsInDataOperand, Worker *worker);

	virtual QString GetClassName() const override { return classProtocol; }
	virtual QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+name+"]"; }

	inline static const QString classProtocol { "Protocol" };
};

#endif // PROTOCOL_H
