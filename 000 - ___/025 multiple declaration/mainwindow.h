#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>

class MainWindow : public QDialog
{
	Q_OBJECT

public:

	bool showMainWindow = false;

	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

    void CreateErrorLabel(QString text)
    {
        auto lo = new QVBoxLayout(this);
        lo->addWidget(new QLabel("Ошибка" + text,this));
        //auto label = ;
    }

	void ReadExcel();

private slots:
	void SlotReadExcelFinished();

private: signals:
	void SignalReadExcelFinished();

};
#endif // MAINWINDOW_H
