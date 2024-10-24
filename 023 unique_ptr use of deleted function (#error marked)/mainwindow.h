#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>

class MainWindow : public QMainWindow
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
};
#endif // MAINWINDOW_H
