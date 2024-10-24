#ifndef PROGRAMM_H
#define PROGRAMM_H

#include <QMessageBox>
#include <QTextBrowser>

#include "object.h"

struct Monitor
{
	QString name;
	QWidget *windwowPtQWidget = nullptr;
	QVBoxLayout *vLayout = nullptr;
	QTextBrowser *tBrowser = nullptr;

	Monitor(QString name_):
		name {name_}
	{
		windwowPtQWidget = new QWidget();
		windwowPtQWidget->setWindowTitle(name);
		vLayout = new QVBoxLayout(windwowPtQWidget);
		tBrowser = new QTextBrowser;
		QFont font = tBrowser->font();
		font.setFamily("Courier new");
		font.setPointSize(12);
		tBrowser->setFont(font);
		vLayout->addWidget(tBrowser);
		QPushButton *btnClear = new QPushButton("Очистить");

		QTextBrowser *tBrowserPtrForConnect = tBrowser;
		QObject::connect(btnClear, &QPushButton::clicked, [tBrowserPtrForConnect](){ tBrowserPtrForConnect->clear(); });
		vLayout->addWidget(btnClear);

		windwowPtQWidget->show();
	}
};

class Programm: public LogedClass
{
	QString version;
	QString programmName;
	std::vector<QStringList> objectsRows;
	std::vector<QStringList> windowsRows;
	std::vector<QStringList> contantsRows;
	std::vector<QStringList> paramsRows;
	std::vector<QStringList> copyParamsRows;
	std::vector<QStringList> widgetsRows;
	std::vector<QStringList> copyWidgetsRows;
	std::vector<QStringList> synchRows;
	std::vector<QStringList> monitorRows;

	std::vector<Constants*> constants;
	std::vector<Object*> objects;
	std::vector<Window> windows;
	std::vector<Monitor> monitors;

	void CreateCopyParamRows();
	void CreateCopyWidgetRows();

	int FindIndexParamRow(QString idSrcParam, uint startIndex = 0);
	int FindIndexWidgetRow(QString windowClass, QString idSrcWidget, uint startIndex = 0);
	enum {notFound = -1};

	void CreateConstsAndObjects();
	void CreateWindows();
	void CreateWidgets();

	void PlaceWidgets();
	void PlaceWindows(int startX, int startY);

	void TestWindow();
	void GiveParamsPtrOfWidgets();
	void GiveObjectsPtrOfWindows();

	Object* FindObject(QString name);

	void SaveSettings();
	void LoadSettings();

public:
	Programm(IExcelWorkbook *wb);
	~Programm();
	void CreateSynchConnections();
};

#endif // PROGRAMM_H
