#ifndef TRAFFICVIEW_H
#define TRAFFICVIEW_H

#include <QWidget>

#include "traffic.h"

namespace Ui {
	class TrafficView;
}

class TrafficView : public QWidget
{
	Q_OBJECT
	Ui::TrafficView *ui;

	nsTraffic::Traffic *traffic;
	bool getTraffic;

	void CreateChannelCheckBoxes();
	bool CheckFilters(const nsTraffic::TrafficMsg &msg);

public:
	explicit TrafficView(QWidget *parent = nullptr);
	~TrafficView();

	void SetupViewMsgs(nsTraffic::Traffic *traffic_, unsigned int maxRowsCount = 1000);
	void StopView();

	void SuspendGetTraffic();
	void ContinueGetTraffic();

	void ReadMsgsFromFiles(std::vector<QString> files_str);

private: signals:
	void SignalMessage(const QString &msg_str);

private slots:
	void SlotMessage(const QString &msg_str);

	void SlotChannelFilterSet();

	void SlotIDFilterSet();
	void SlotIDFilterEditsSet();
	void SlotIDFilterConvert();

	void SlotDataFilterSet();
	void SlotDataFilterEditsSet();
	void SlotDataFilterConvert();

	void on_checkBoxGetTraffic_clicked();
	void on_pushButtonThreshold_clicked();
	void on_checkBoxAutolean_clicked();
	void on_pushButtonOpen_clicked();
};

#endif // TRAFFICVIEW_H
