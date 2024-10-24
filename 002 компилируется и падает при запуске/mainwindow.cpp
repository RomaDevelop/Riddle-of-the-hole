#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "marathonchannel.h"


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	if(CiInit() < 0) ui->textBrowser0->append("err CiInit");

	ICANChannel *ch0 = new MarathonChannel(0,"test0");
	QString print0;
	print0 += "channelNumber: " + QString::number(ch0->GetNumber());
	print0 += "\nchannelName: " + ch0->GetName();
	print0 += "\nGetStatus: " + QString::number(ch0->GetStatus());
	print0 += "\nStart: " + QString::number(ch0->Start());
	print0 += "\nGetStatus: " + QString::number(ch0->GetStatus());
	print0 += "\nStop: " + QString::number(ch0->Stop());
	print0 += "\nGetStatus: " + QString::number(ch0->GetStatus());

	ui->textBrowser0->append(print0);


	ICANChannel *ch1 = new MarathonChannel(1,"test1");
	print0 = "";
	print0 += "channelNumber: " + QString::number(ch1->GetNumber());
	print0 += "\nchannelName: " + ch1->GetName();
	print0 += "\nGetStatus: " + QString::number(ch1->GetStatus());
	print0 += "\nStart: " + QString::number(ch1->Start());
	print0 += "\nGetStatus: " + QString::number(ch1->GetStatus());
	print0 += "\nStop: " + QString::number(ch1->Stop());
	print0 += "\nGetStatus: " + QString::number(ch1->GetStatus());

	ui->textBrowser1->append(print0);

	ui->setupUi(this);
}

MainWindow::~MainWindow()
{
	delete ui;
}

