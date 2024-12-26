#ifndef BKZ27PSI_H
#define BKZ27PSI_H

#include <thread>

#include <QTextBrowser>
#include <QPushButton>
#include <QCloseEvent>
#include <QLabel>
#include <QComboBox>

#include "IExternalWindow.h"

class BKZ27_psi: public QWidget, public LogedClass, public IExternalWindow
{
	Q_OBJECT

	QString windowGeo;

	QString name;
	QString connectString;
	std::vector<Object*> objects;
	std::vector<Protocol*> protocols;
	IConnectExternalWidgets *programm = nullptr;

	QLabel *labelNomerBloka;
	QComboBox *comboBoxNomerBloka;
	QPushButton *btnStartCheckWorking;
	QPushButton *btnStartCheckLoading;
	QPushButton *btnStartCheckPassport;
	QPushButton *btnStartCheckBlNumberAndChRegims;
	QPushButton *btnStartCheck8Hours;
	QPushButton *btnStopCheck;

	QTextBrowser *textBrowser = nullptr;
	std::vector<std::pair<QWidget*,ConnectData>> ctrlElements;
	std::vector<std::pair<QWidget*,ConnectData>> viewElements;

	const uint chansCountMustBe = 48;
	Param *paramSostYstrCh1;
	Param *paramSostYstrCh2;
	Param *paramFabricNumber1;
	Param *paramFabricNumber2;
	Param *paramSoftVersion1;
	Param *paramSoftVersion2;
	std::vector<Param*> channelsVkls1;
	std::vector<Param*> channelsVkls2;
	std::vector<Param*> channelsIspr1;
	std::vector<Param*> channelsIspr2;
	std::vector<Param*> channelsErrorLoading1;
	std::vector<Param*> channelsErrorLoading2;

	Worker *frWorkerUprChans1;
	Worker *frWorkerUprChans2;
	Worker *frWorkerZaprosDannixObIsdelii1;
	Worker *frWorkerZaprosDannixObIsdelii2;

	std::thread *treadPtr = nullptr;
	bool stopThreadFlag = false;
	bool threadGoing = false;
	bool StopTreadIfGoing();
	bool DeleteThreadIfExists();
	void Delay(int milliSec, int count, bool printPoint);

public:
	explicit BKZ27_psi(QString connectString_, QString name_, QWidget *parent = nullptr);
	~BKZ27_psi();

	bool DefineParamPtrs();
	bool ClearParamsValues(std::vector<Param*> params);

	virtual QString GetClassName() const override { return classBKZ27_psi; }
	virtual QString ToStrForLog() const override { return GetClassName() + " ToStrForLog " + CodeMarkers::mock; }

	virtual QString Name() const override { return name; }
	virtual QWidget* Window() override { return this; }

	virtual void ConnectToObjects(IConnectExternalWidgets *programm_,
								  std::vector<Object*> allObjects,
								  std::vector<Protocol*> allProtocols) override;

	void closeEvent (QCloseEvent *event) override;
	virtual QString SaveSettings() override;
	virtual void LoadSettings(const QString &str) override;

	inline static const QString classBKZ27_psi { "BKZ27_psi" };

	void DisableStartBtnsEnableStopBtn();
	void EnableStartBtnsDisableStopBtn();

	private: signals: void SignalPrint(QString text, int type);
	private slots: void SlotPrint(QString text, int type);
	private: enum printTypes { psiTextBrowser, psiTextBrowserAddInLastRow, log, warning, error};

	private slots: void SlotChangeBlockNumber();

	private slots: void SlotBtnStopCheck();

	private: signals: void SignalAnyCheckFinished();
	private slots: void SlotAnyCheckFinished();

	private slots: void SlotCheckWorking();
	private slots: void SlotCheckWorking8Hours();
	private: signals: void SignalVklChan(int channelIndex);
	private slots: void SlotVklChan(int channelIndex);
	private: signals: void SignalCheckVklChan(int channelIndexMustBeVkl);
	private slots: void SlotCheckVklChan(int channelIndexMustBeVkl);

	private slots: void SlotCheckLoading();

	private slots: void SlotCheckPassport();

	private slots: void SlotCheckBlNumberAndChRegims();
};

#endif // BKZ27PSI_H
