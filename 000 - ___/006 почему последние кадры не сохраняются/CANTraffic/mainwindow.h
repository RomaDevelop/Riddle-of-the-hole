#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT
	Ui::MainWindow *ui;

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_pushButtonStart_clicked();
	void on_pushButtonStop_clicked();
	void on_pushButtonTransmit_clicked();
	void on_pushButtonReceive_clicked();
	void on_pushButtonReceiveCB_clicked();
	void on_pushButtonTrafficOn_clicked();
	void on_pushButtonStartAll_clicked();
	void on_pushButtonTrafficOff_clicked();
	void on_pushButtonTrafficView_clicked();
	void on_pushButtonDoAll_clicked();

	void SlotMessage(const QString &str);

signals:
	void SignalMessage(const QString &str);
};
#endif // MAINWINDOW_H
