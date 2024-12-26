#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "frameworker.h"
#include "category.h"
#include "object.h"

class Protocol: public LogedClass
{
	public:
	QString name;
	Object *connectedObject = nullptr;
	Constants *constants = nullptr;
	QStringList constantsNames;
	std::vector<std::unique_ptr<FrameWorker>> frameWorkers;
	Category category;

	int channel = -1;
	std::queue<CANMsg_t> msgs;
	std::vector<Protocol*> network;
	std::vector<Protocol*> synchrone;
	QTimer *timerMsgsWorker = nullptr;

	const int type = undefined;
	enum Types { undefined, emulator, servis };
	QString TypeToStr();

	Protocol(const std::vector<QStringList> &frameWorkersStrs, Object *objectToConnect_,
		   QString version, QString name_, int objTip_, Constants *constants_, int channel_);

	void InitFilters();
	void InitVariables();

	FrameWorker* FindFrameWorkerById(const QString &id);
	std::vector<FrameWorker*> FindFrameWorkersInCategory(const QString &category, const QString &nameFW, bool includeSubCats);

	///\brief создаёт сообщение для отправки,
	/// создаёт сообщения для отправки от параметров с которыми синхронизировано,
	/// возвращает вектор указателей на параметры и сообщений
	std::vector<FrameWorkerAndFrame> ConstructFrames(FrameWorker *frameWorker);
	void SetDataFilterInFrame(CANMsg_t &frame, CANMsgDataFilter_t filter);

	void SendFrames(std::vector<FrameWorkerAndFrame> &toSend);
	void SendFrame(const CANMsg_t &msg);

	FrameWorker* CheckFilters(const CANMsg_t &msg);
	void WorkerIncommingFrames();
	void WorkerFramePassedFilter(CANMsg_t &frame, FrameWorker *frameWorker);

	/// в некоторых случаях DoEverything возвращает указатель на объекты которые сам создаёт в куче,
	/// при этом они создаются через make_shared и записываются в createdIOperands
	/// при вызове уничтожении Object-а в котором они хранятся они тоже будут уничтожены
	IOperand* DoEverything(QString command, FrameWorker *frameWorker, CANMsg_t &frame, QString retType);
	std::vector<std::shared_ptr<IOperand>> createdIOperands;
	static QString DEretNullptr() {return "DEretNullptr";}
	static QString DEretPtr() {return "DEretPtr";}
	IOperand* DoFunction(QString command, Function* function, std::vector<IOperand*> params, FrameWorker *frameWorker);
	IOperand* DoExpression(QString command, Expression &expression, FrameWorker *frameWorker, QString retType);
};

#endif // PROTOCOL_H
