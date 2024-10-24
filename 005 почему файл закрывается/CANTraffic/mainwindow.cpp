#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <vector>
#include <mutex>
using namespace std;

#include <QDebug>

#include "chai.h"
#include "caninterfaces.h"

#include "traffic.h"
#include "trafficview.h"
#include "cleanerthread.h"

CANInterfaces *ints;
vector<ICANChannel*> chs;

MainWindow *mainWindowObj;

nsTraffic::Traffic *traffic;
TrafficView *trafficView;

CleanerThread *cleanerThread;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	mainWindowObj = this;
	move(30,30);

	ints = new CANInterfaces;
	ints->GetChannels(chs);
	for(auto ch:chs) ui->listWidget->addItem(ch->GetName());

	traffic = new nsTraffic::Traffic;
	traffic->SetCBFunctionForLog([](const QString &log){qDebug()<<log;});\

	trafficView = new TrafficView();
	cleanerThread = new CleanerThread(ui->textBrowser, 1000);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButtonStart_clicked()
{
	int index = ui->listWidget->currentRow();
	QString log = "ch " + QString::number(index) + " Start";
	if(chs[index]->SetBaud(ICANChannel::BR_1M)  == ICANChannel::NO_ERROR) log += "(BR_1M)";
	if(chs[index]->Start() == ICANChannel::NO_ERROR) log += " success";
	else log += " error";
	ui->textBrowser->append(log);
}

void MainWindow::on_pushButtonStop_clicked()
{
	int index = ui->listWidget->currentRow();
	QString log = "ch " + QString::number(index) + " Stop";
	if(chs[index]->Stop() == ICANChannel::NO_ERROR) log += " success";
	else log += " error";
	ui->textBrowser->append(log);
}

void MainWindow::on_pushButtonTransmit_clicked()
{
	int index = ui->listWidget->currentRow();
	QString log = "ch " + QString::number(index) + " WriteMsg";
	CANMsg_t msg { 0x1010101, {8,7,6,5,4,3,2,1}, 8, 0b100, 0 };
	if(chs[index]->WriteMsg(&msg) == ICANChannel::NO_ERROR) log += " success";
	else log += " error";
	ui->textBrowser->append(log);
}

void MainWindow::on_pushButtonReceive_clicked()
{
	int index = ui->listWidget->currentRow();
	QString log = "ch " + QString::number(index) + " receive";
	CANMsg_t msg;
	int count;
	count = chs[index]->GetRecCnt();
	log += " " + QString::number(count) + " messages";
	ui->textBrowser->append(log);
	for(int i=0; i<count; i++)
	{
		chs[index]->ReadMsg(&msg);
		ui->textBrowser->append(msg.ToStr());
	}
}

void MainWindow::on_pushButtonTrafficOn_clicked()
{
	traffic->SetChannels(chs, 2000, true);
}

void MainWindow::on_pushButtonStartAll_clicked()
{
	for(auto c:chs)
	{
		QString log = "ch " + QString::number(c->GetNumberAbsolut()) + " Start";
		if(c->Start() == ICANChannel::NO_ERROR) log += " success";
		else log += " error";
		ui->textBrowser->append(log);
	}
}

void MainWindow::on_pushButtonTrafficOff_clicked()
{
	traffic->StopWriting();
}

void MainWindow::on_pushButtonTrafficView_clicked()
{
	trafficView->SetupViewMsgs(traffic, 1000);
	trafficView->show();
}

void MainWindow::on_pushButtonReceiveCB_clicked()
{
	connect(this,&MainWindow::SignalMessage,this,&MainWindow::SlotMessage);
	for(auto c:chs)
	{
		c->SetCBFunctionForMessage([](int channel){
			CANMsg_t msg;
			chs[channel]->ReadMsg(&msg);
			emit mainWindowObj->SignalMessage(msg.ToStr());
		});
	}
}

int mainWindowMsgNumber = 0;
void MainWindow::SlotMessage(const QString &str)
{
	mainWindowMsgNumber++;
	//ui->textBrowser->append(QSn(mainWindowMsgNumber)+" "+str);
	ui->lineEdit->setText(QSn(mainWindowMsgNumber)+" "+str);
}

void MainWindow::on_pushButtonDoAll_clicked()
{
	ui->pushButtonStart->click();
	ui->pushButtonReceiveCB->click();
	ui->pushButtonTrafficOn->click();
	ui->pushButtonTrafficView->click();
}
