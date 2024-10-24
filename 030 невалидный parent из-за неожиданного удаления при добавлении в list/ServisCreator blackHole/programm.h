#ifndef PROGRAMM_H
#define PROGRAMM_H

#include <QStatusBar>
#include <QMessageBox>
#include <QTextBrowser>

#include "widget.h"

#include "IConnectExternalWidgets.h"
#include "IExternalWindow.h"

class Monitor
{
	QString name;

	QVBoxLayout *vLayout = nullptr;
	QTextBrowser *tBrowser = nullptr;
	std::vector<LogedClass*> connectedObjects;

	const int undefined = -1;
	int logsCbMsgGetId = -1;

public:

	QWidget *windowPtQWidget = nullptr;

	~Monitor()
	{
		if(logsCbMsgGetId != undefined)
			Logs::RemoveCbfMsgGet(logsCbMsgGetId);
	}

	void Create();
	void Delete();

	void SetProgrammName(QString name_);
};

struct ControlPanel
{
	QWidget *window = nullptr;

	QCheckBox *chLogDoEvrythingCommands = nullptr;
	QCheckBox *chWarningNotPassedFilterFrames = nullptr;
	QCheckBox *chLogParamChanges = nullptr;
	QCheckBox *chLogFrameSent = nullptr;
	QCheckBox *chLogFrameGet = nullptr;

	void SetCheckBoxesFromSettings();
};

class Programm: public LogedClass, public IConnectExternalWidgets
{
public:
	typedef void(*NoParamsFunction)();

	Programm(const QDomElement &bodyElement, QString programmName_, QString aprojectFile,
			 NoParamsFunction deleter_, NoParamsFunction onDestructorEnds_);
	~Programm();

	void PrintGetedConfig();
	void PrintStructureObjects();
	void CloseAll();
	bool IsProgrammShowsNothing();

private:

	QString version;
	QString programmName;
	QString projectFile;
	QString getedConfig;
	bool getedConfigWritten = false;
	std::unique_ptr<CANInterfaces> interfaces;

	std::vector<std::unique_ptr<InternalChannel>> channels;

	std::vector<Constants*> constants;
	std::vector<Object*> objects;
	std::vector<Protocol*> protocols;
	std::vector<Window> windows;
	std::vector<std::shared_ptr<IExternalWindow>> externalWindows;
	Monitor monitor;

	ControlPanel controlPanel;

	NoParamsFunction deleter = nullptr;
	NoParamsFunction onDestructorEnds = nullptr;

	int FindIndexWidgetRow(QString windowClass, QString idSrcWidget, int startIndex = 0);
	enum {notFound = -1};

	void SynchConnectionsCreate(const std::vector<QStringList> &synchRows);

	void SetLogsMsgWorkers();

	int ConnectExternalWidget(const ConnectData &connectData) override;
	bool CheckConnectData(const ConnectData &connectData);

	void ConnectMake();

	void PlaceAndShowWindows(int startX, int startY);

	void CreateControlPanel(QString creationStr);
	void DeleteControlPanel();

	void GiveWindowsPtrs();
	void GiveWidgetsPtrs();

	Window* FindWindow(QString objectName, QString windowName, bool printError);
	IExternalWindow* FindExternalWindow(QString name);

	void SaveSettings();
	void LoadSettings();

	bool CheckProgrammName(bool printError);

	QString GetClassName() { return "Programm"; }
};

#endif // PROGRAMM_H
