#ifndef BKZ27_H
#define BKZ27_H

#include <QTabWidget>
#include <QGridLayout>

#include "IExternalWindow.h"

class BKZ27: public LogedClass, public IExternalWindow
{
	QString name;
	QString connectString;
	std::vector<Object*> objects;
	std::vector<Protocol*> protocols;

	std::unique_ptr<QWidget> window;
	std::unique_ptr<QTabWidget> tabWidget;
	std::vector<std::unique_ptr<QWidget>> tabs;

	std::vector<std::unique_ptr<QWidget>> ctrlElements;
	std::vector<std::unique_ptr<QWidget>> viewElements;

public:
	BKZ27(QString connectString_, QString name_);
	~BKZ27() = default;

	virtual QString GetClassName() const override { return classBKZ27; }
	virtual QString ToStrForLog() const override { return GetClassName() + " ToStrForLog " + CodeMarkers::mock; }

	virtual QString Name() const override { return name; }
	virtual QWidget* Window() override { return window.get(); }

	virtual void ConnectToObjects(IConnectExternalWidgets *programm, std::vector<Object*> allObjects, std::vector<Protocol*> allProtocols) override;

	virtual QString SaveSettings() override;
	virtual void LoadSettings(const QString &str) override;

	inline static const QString classBKZ27 { "BKZ27" };

private:

	void CreateRow(QGridLayout *layOutPlace, QString captionRow,
				   QStringList btnCaptions, QStringList btnWorkers, QString labelsHasVals,
				   QString category, QString nameFrWorker, QString nameParam);

	QString CreateStrConnectData(QString objectOrProtocol, int index, QString cat, QString name, QString obrabotch);
	ConnectData GetConnectDataFromStr(const QString & widgetObjectName, int widgetCat, QWidget *widgetPtr);
};

#endif // BKZ27_H
