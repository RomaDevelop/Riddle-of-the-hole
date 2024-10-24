#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

#include <vector>
using namespace std;

#include "marathoninterface.h"

ICANInterface *marathonInterface;
vector<ICANChannel*> chs;
CANMsg_t udata_msg;

Ui::MainWindow *objectUI;
int chForEv;
int evForEv;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	objectUI = ui;

	marathonInterface = new MarathonInterface;
	marathonInterface->ScanInterface();
	marathonInterface->GetChannels(chs);
	for(auto ch:chs)
	{
		ui->listWidget->addItem(ch->GetName());
		ch->SetCBFunctionForLog([](QString log){qDebug() << log;});
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_pushButtonStart_clicked()
{
	int index = ui->listWidget->currentRow();
	QString log = "ch " + QString::number(index) + " Start";
	if(chs[index]->SetBaud(ICANChannel::BR_1M) == ICANChannel::NO_ERROR) log += "(BR_1M)";
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

QString msgToStr(CANMsg_t *msg, int base = 16)
{
	QString ret = QString::number(msg->ID, base) + "{";
	int byts_count = msg->length;
	for(int i=0; i<byts_count; i++)
	{
		ret += QString::number(msg->data[i], base);
		if(i != byts_count-1) ret += ", ";
		else ret += "}";
	}
	return ret;
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
		ui->textBrowser->append(QString::number(i) + " " + msgToStr(&msg));
	}
}

int cbcnt = 0;
void CBFunctionMsg(int ch, int ev, void* data)
{
	cbcnt++;
	qDebug() << "begin CBFunctionMsg" << cbcnt << "msg cnt = " << chs[ch]->GetRecCnt();

	// делаем что-то
	for(int i=0; i<100000; i++)
	{
		QString s = "sdvsdvxc xvxcfdbdfbdfbdfbdfb";
		s += s;
	}

	qDebug() << "end CBFunctionMsg";
	//objectUI->textBrowser->append(QString("CB ") + ch + " " + ev);
}

//void ci_cb_ex(_u8 ch, _s16 ev, void* data)
//{
//	qDebug() << "cb";
//	objectUI->textBrowser->append(QString("CB ") + ch + " " + ev);
//}

//void ci_cb(_s16 ev)
//{
//	qDebug() << "cb";
//	objectUI->textBrowser->append(QString("CB ") + " " + ev);
//}

void MainWindow::on_pushButtonRegCBMsg_clicked()
{
	//qDebug() << "cb";
	int index = ui->listWidget->currentRow();
	QString log = "ch " + QString::number(index) + " SetCBFunctionForMessage ";

	int result = chs[index]->SetCBFunctionForMessage(CBFunctionMsg, &udata_msg);
	//int result = chs[index]->SetCBFunctionForMessage(CBFunctionExt(ci_cb_ex), &udata_msg);
	if(result == ICANChannel::NO_ERROR) log += "success";
	else log += "error " + QString::number(result);

	//int result = CiSetCBex((_u8)chs[index]->GetNumber(), CIEV_RC, ci_cb_ex, &udata_msg);
	//int result = CiSetCB((_u8)chs[index]->GetNumber(), CIEV_RC, ci_cb);
	//if(result == ECIOK) log += "success";
	//else log += "error " + QString::number(result);

	objectUI->textBrowser->append(log);
}

void MainWindow::on_pushButtonDisReg_clicked()
{
	int index = ui->listWidget->currentRow();
	int result = chs[index]->SetCBFunctionForMessage(nullptr, nullptr);
}
