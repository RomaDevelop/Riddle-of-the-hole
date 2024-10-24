#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_pushButtonStart_clicked();
	void on_pushButtonStop_clicked();
	void on_pushButtonTransmit_clicked();
	void on_pushButtonReceive_clicked();
	void on_pushButtonRegCBMsg_clicked();

	void on_pushButtonDisReg_clicked();

private:
	Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
