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
#include "parametr.h"

namespace CrWidgCmnds
{
	const QString obrabotchik {"Обработчик"};
};

class Object
{
	int windowsCount {0};
public:
	QString name;
	const int objectTip;  // Parametr::Types
	Constants *constants;
	std::vector<Parametr> params;

	std::vector<QWidget*> windows;
	std::vector<QTableWidget*> windowTables;

	int channel;
	std::queue<CANMsg_t> msgs;
	std::vector<Object*> server;
	std::vector<Object*> synchrone;
	QTimer *timerMsgsWorker;

	Object(const std::vector<QStringList> &definedParams, const std::vector<QStringList> &definedWidgets,
			 QString Version, QString name_, int objTip_, Constants *constants, int channel_);
	~Object() { Log("~Object()"); }

	void InitFilters();
	void InitPeremennie();

	QString ToStr() { QString str; for(auto &c:params) str += c.ToStr() +"\n"; return str; }

	int GetNumberFromNumberOperand(QString numberOperand, Parametr *param, const CANMsg_t &msg);

	std::vector<Parametr*> FindParamsByName(const TextConstant &nameToFind);
	Parametr* FindParamById(const QString &id);
	Parametr* FindParamByName(const TextConstant & nameParametr);
	Parametr* FindParamVModuleNomer(int nomerModule, const TextConstant &nameParametr);
	Value* FindPeremennayaVParametre(QString namePeremenn, const TextConstant &nameParametr);

	void SendFrame(const CANMsg_t &msg);
	void MsgsWorker();
	Parametr* CheckFilters(const CANMsg_t &msg, QString &tipFrame);

	CANMsg_t EmulatorConstructFrame(Parametr *param, int windowIndex);
	CANMsg_t ServisConstructFrame(Parametr *param, int windowIndex);
	void ServisWorkIncommingFrame(CANMsg_t &frame, Parametr *parametrInicialiser);
	void EmulatorWorkIncommingFrame(CANMsg_t &frame, Parametr *parametrInicialiser);

	void CreateViewWidg(Parametr *param, int windowIndex);
	void CreateCtrlWidg(Parametr *param, int windowIndex, std::vector<WidgPart> &doConnect);
	void CreateWidgets();
	void PlaceWidgets(int startX, int startY, QString PrefixName);
	void PlaceWindows(int x, int y);
	void FillTables();
	void MinResize();

	void ServisConnectWidg(std::vector<WidgPart> &doConnect, Parametr *param, int windowIndex);
	void EmulatorConnectWidg(Parametr *param, int windowIndex);

	// WCount - words count. Including func name
	const QString funcFindParamInModule		{"ПоискПараметраВМодулеНомер"};
	const int funcFindParamInModuleWCount	{3};
	const QString funcFindParams			{"ПоискПараметров"};
	const int funcFindParamsWCount			{2};
	const QString funcSendFrame				{"ОтправитьКадр"};
	const int funcSendFrameWCount			{2};
	const QString funcLog					{"Лог"};
	const int funcLogWCount					{2};
	const QString funcIf					{"if"};
	const int funcIfWCount					{6};

	const QStringList functions{
		funcFindParamInModule,
		funcFindParams,
		funcSendFrame,
		funcLog,
		funcIf
	};

	const std::vector<int> functionsWCounts{
		funcFindParamInModuleWCount,
		funcFindParamsWCount,
		funcSendFrameWCount,
		funcLogWCount,
		funcIfWCount
	};

	class DEReturn
	{
	public:
		IOperand* ptr {nullptr};
		bool needDelete {false};

		DEReturn() = default;
		//DEReturn(DEReturn &deRet) = delete;
		DEReturn(DEReturn &&deRet)
		{
			ptr = deRet.ptr;
			needDelete = deRet.needDelete;
			Log("DEReturn &&deRet");
		}
		~DEReturn()
		{
			Log("~DEReturn()");
			if(needDelete)
			{
				if(ptr)
				{
					//delete ptr;
					ptr = nullptr;
					needDelete = false;
					Log("~DEReturn() MOCK!!!! did delete ptr");
				}
				else { Error("Flag needDelete set, but ptr is nullptr"); }
			}
		}

		void Log(const QString &logStr)	  {	qdbg << "Log:   DEReturn: " + logStr; }
		void Error(const QString &errStr) { qdbg << "Error: DEReturn: " + errStr; }
	};

	DEReturn DoEverything(QString command, Parametr *param, int windowIndex, CANMsg_t &frame, QString retType);
	static QString DEretNullptr() {return "DEretNullptr";}
	static QString DEretPtr() {return "DEretPtr";}

	void Log(const QString &logStr)	  {	qdbg << "Log:   Object " + name + ": " + logStr; }
	void Error(const QString &errStr) { qdbg << "Error: Object " + name + ": " + errStr; }

	enum Different { noWindow = -1 };
};

#endif // PROTOCOL_H
