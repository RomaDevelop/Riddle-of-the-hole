#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <memory>
#include <bitset>
#include <vector>
#include <queue>

#include <QObject>
#include <QDebug>
#include <QString>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>

#include "MyQShortings.h"
#include "MyQExcel.h"

#include "icanchannel.h"

#include "value.h"
#include "widget.h"
#include "compiler.h"
#include "constants.h"
#include "param.h"

class Object: public LogedClass
{
public:
	QString name;
	const int objectTip;  // Param::Types
	Constants *constants;
	QStringList constantsNames;
	std::vector<Param> params;

	std::vector<Window*> windows;

	int channel;
	std::queue<CANMsg_t> msgs;
	std::vector<Object*> server;
	std::vector<Object*> synchrone;
	QTimer *timerMsgsWorker;
	QTimer *timerBlinking;
	std::vector<Widget*> blinkingWidgs;
	void Blink() { for(auto bw:blinkingWidgs) bw->Blink(); }

	Object(const std::vector<QStringList> &paramsStrs,
			 QString Version, QString name_, int objTip_, Constants *constants, int channel_);
	~Object() { Log("desroyed"); }

	QString ToStr() { QString str; for(auto &c:params) str += c.CellValuesToStr() +"\n"; return str; }

	void InitFilters();
	void InitPeremennie();

	void ConnectMake();

	std::vector<Param*> FindParamsByName(const TextConstant &nameToFind);
	Param* FindParamById(const QString &id);
	Param* FindParamByName(const TextConstant & nameParam);
	Param* FindParamInCategory(QString category, QString nameParam);
	Param* FindParamInModuleNomer(int nomerModule, const TextConstant &nameParam);
	Param* FindParamInCategoryAndModule(int nomerModule, const TextConstant &nameParam, const TextConstant &nameCategory);
	Value* FindVariableVParametre(QString namePeremenn, const TextConstant &nameParame);
	Window* FindWindow(QString windowName);

	///\brief создаёт сообщение для отправки,
	/// создаёт сообщения для отправки от параметров с которыми синхронизировано,
	/// возвращает вектор указателей на параметры и сообщений
	std::vector<ParamAndFrame> ConstructFrames(Param *param);
	void SetDataFilterInFrame(CANMsg_t &frame, CANMsgDataFilter_t filter);

	void SendFrames(std::vector<ParamAndFrame> &paramsAndFrames);
	void SendFrame(const CANMsg_t &msg);

	void MsgsWorker();
	Param* CheckFilters(const CANMsg_t &msg, QString &tipFrame);
	void WorkIncommingFrame(CANMsg_t &frame, Param *parametrInicialiser);

	IOperand* DoEverything(QString command, Param *param, CANMsg_t &frame, QString retType);
	/// в некоторых случаях DoEverything возвращает указатель на объекты которые сам создаёт в куче,
	/// при этом они создаются через make_shared и записываются в createdIOperands
	/// при вызове уничтожении Object-а в котором они хранятся они тоже будут уничтожены
	std::vector<std::shared_ptr<IOperand>> createdIOperands;
	static QString DEretNullptr() {return "DEretNullptr";}
	static QString DEretPtr() {return "DEretPtr";}

	void DoObrabotchik(QString obrabotchik, QString senderName, Param *senderParam);
	void DoObrabotchiki(int windowIndex, QString senderName, Param *senderParam);

	enum Different { noWindow = -1 };

};

#endif // PROTOCOL_H
