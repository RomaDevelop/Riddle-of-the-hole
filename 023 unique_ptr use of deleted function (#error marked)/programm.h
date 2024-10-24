#ifndef PROGRAMM_H
#define PROGRAMM_H

#include <QStatusBar>
#include <QMessageBox>
#include <QTextBrowser>

#include "MyQExcel.h"

#include "widget.h"
#include "object.h"

struct Monitor
{
	QString name;
	QWidget *windowPtQWidget = nullptr;
	QVBoxLayout *vLayout = nullptr;
	QTextBrowser *tBrowser = nullptr;

	void Create()
	{
		windowPtQWidget = new QWidget();
		windowPtQWidget->setWindowTitle("Monitor");
		vLayout = new QVBoxLayout(windowPtQWidget);
		tBrowser = new QTextBrowser;
		QFont font = tBrowser->font();
		font.setFamily("Courier new");
		font.setPointSize(12);
		tBrowser->setFont(font);
		vLayout->addWidget(tBrowser);
		QPushButton *btnClear = new QPushButton("Очистить");
		vLayout->addWidget(btnClear);
		QStatusBar *statusBar = new QStatusBar;
		QLabel *labeStBar = new QLabel;
		statusBar->addWidget(labeStBar);
		vLayout->addWidget(statusBar);

		QTextBrowser *tBrowserPtrForConnect = tBrowser;
		QObject::connect(btnClear, &QPushButton::clicked, [tBrowserPtrForConnect](){ tBrowserPtrForConnect->clear(); });
		QObject::connect(tBrowserPtrForConnect,&QTextBrowser::textChanged, [labeStBar](){
			labeStBar->setText("Предупреждений: " + QSn(Logs::WarningsCount()) + "\t\tОшибок: " + QSn(Logs::ErrorsCount()));
		});

		windowPtQWidget->show();
	}

	void SetProgrammName(QString name_)
	{
		name = name_;
		if(name != "")
		windowPtQWidget->setWindowTitle("Monitor " + name);
	}
};

class Programm: public LogedClass
{
public:
	Programm(IExcelWorkbook *wb);
	~Programm();

	bool IsProgrammShowsNothing();

private:

	QString version;
	QString programmName;
	std::vector<QStringList> objectsRows;
	std::vector<QStringList> windowsRows;

	std::vector<QStringList> contantsRows;

	std::vector<QStringList> paramsRows;
	std::vector<QStringList> copyParamsRows;
	std::vector<QStringList> frameWorkersRows;

	std::vector<QStringList> widgetsRows;
	std::vector<QStringList> copyWidgetsRows;

	std::vector<QStringList> synchRows;
	std::vector<QStringList> monitorRows;

	std::vector<Constants*> constants;
	std::vector<Object*> objects;
	std::vector<Protocol*> protocols;
	std::vector<Window> windows;
	Monitor monitor;

	QWidget *controlPanel = nullptr;

	void CreateCopyParamRows();
	QString CopyParamStrToStrForLog(QStringList strList);
	void CreateCopyWidgetRows();
	void ModificateCopyRow(QStringList &copyRow, QString modificationCellValue);

	int FindIndexParamRow(QString idSrcParam, int startIndex = 0);
	int FindIndexWidgetRow(QString windowClass, QString idSrcWidget, int startIndex = 0);
	enum {notFound = -1};

	void CreateConstsObjectsProtocols();
	void CreateSynchConnections();

	void SetLogsMsgWorkers();

	void CreateWindows();
	void CreateWidgets();

	void ConnectMake();

	void PlaceWidgets();
	void PlaceWindows(int startX, int startY);

	void CreateControlPanel();

	void GiveWindowsPtrs();
	void GiveWidgetsPtrs();

	Window* FindWindow(QString objectName, QString windowName);

	void SaveSettings();
	void LoadSettings();
};

#endif // PROGRAMM_H
