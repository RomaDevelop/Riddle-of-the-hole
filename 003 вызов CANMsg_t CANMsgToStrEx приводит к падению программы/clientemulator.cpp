#include "clientemulator.h"
#include "ui_clientemulator.h"

#include <memory>
using namespace std;

#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>

shared_ptr<QTcpSocket> sock_cmd;
shared_ptr<QTcpSocket> sock_data;

ClientEmulator::ClientEmulator(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::ClientEmulator)
{
	ui->setupUi(this);
	this->move(550,20);

	sock_cmd = make_shared<QTcpSocket>();
	sock_cmd->setObjectName("sock_cmd");
	sock_data = make_shared<QTcpSocket>();
	sock_data->setObjectName("sock_data");

	connect(sock_cmd.get(),SIGNAL(connected()),SLOT(SlotConnected()));
	connect(sock_cmd.get(),SIGNAL(disconnected()),SLOT(SlotDisconnected()));
	connect(sock_cmd.get(),SIGNAL(readyRead()),SLOT(SlotReadyRead()));
	connect(sock_cmd.get(),SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(SlotError(QAbstractSocket::SocketError)));

	connect(sock_data.get(),SIGNAL(connected()),SLOT(SlotConnected()));
	connect(sock_data.get(),SIGNAL(disconnected()),SLOT(SlotDisconnected()));
	connect(sock_data.get(),SIGNAL(readyRead()),SLOT(SlotReadyRead()));
	connect(sock_data.get(),SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(SlotError(QAbstractSocket::SocketError)));
}

ClientEmulator::~ClientEmulator()
{
	delete ui;
}


void ClientEmulator::on_pushButton_clicked()
{
	int port_cmd = 11010;
	int port_data = 11013;
	sock_cmd->connectToHost("127.0.0.1", port_cmd);
	sock_data->connectToHost("127.0.0.1", port_data);
}


void ClientEmulator::on_pushButton_2_clicked()
{
	sock_cmd->disconnectFromHost();
	sock_data->disconnectFromHost();
}

void ClientEmulator::ConvertFromServerToCANMsg(QString msg_str, CANMsg_t &msg)
{
	QString regime = msg_str.left(1);
	int data_start_index;
	if(regime == "t")
	{
		msg.ID = msg_str.mid(1, 3).toInt(nullptr,16);
		msg.regime = 0b000;
		msg.length = msg_str.mid(4, 1).toInt(nullptr, 16);
		data_start_index = 5;
	}
	else if(regime == "T")
	{
		msg.ID = msg_str.mid(1, 3).toInt(nullptr,16);
		msg.regime = 0b001;
		msg.length = msg_str.mid(4, 1).toInt(nullptr, 16);
		data_start_index = 0;
	}
	else if(regime == "e")
	{
		msg.ID = msg_str.mid(1, 8).toInt(nullptr,16);
		msg.regime = 0b100;
		msg.length = msg_str.mid(9, 1).toInt(nullptr, 16);
		data_start_index = 10;
	}
	else if(regime == "E")
	{
		msg.ID = msg_str.mid(1, 8).toInt(nullptr,16);
		msg.regime = 0b101;
		msg.length = msg_str.mid(9, 1).toInt(nullptr, 16);
		data_start_index = 0;
	}
	else
	{
		QMessageBox::critical(this, "Ошибка", "Ошибка выполнения ConvertCANMsgForServer! Неверно указан режим кадра!");

		msg.ID = msg_str.mid(1, 3).toInt(nullptr,16);
		msg.regime = 0b001;
		msg.length = msg_str.mid(4, 1).toInt(nullptr, 16);
		data_start_index = 0;
	}

	if(data_start_index)
		for(unsigned int i=0; i<msg.length; i++)
			msg.data[i] = msg_str.mid(i*2 + data_start_index, 2).toInt(nullptr, 16);
}

void ClientEmulator::SlotReadyRead()
{
	QTcpSocket *sock = (QTcpSocket*)sender();
	int aviable = sock->bytesAvailable();
	char *buf = new char[aviable + 1];
	sock->read(buf, aviable);
	buf[aviable] = '\0';
	ui->textEdit->append(sock->objectName() + " received from server {" + buf + "}");

	if(buf[aviable-1] != '\r')
	{
		QMessageBox::critical(this, "Ошибка", "Ошибка обработки входящих данных, нет завершающего символа!");
		return;
	}

	QString msg_str;
	for(int i=0; i<aviable; i++)
	{
		if(buf[i] != '\r')
			msg_str += buf[i];
		else
		{
			CANMsg_t msg;
			ConvertFromServerToCANMsg(msg_str, msg);
			ui->textEdit->append(CANMsg_t::CANMsgToStr(msg));
			ui->textEdit->append(CANMsg_t::CANMsgToStrEx(msg));
			msg_str = "";
		}
	}
}

void ClientEmulator::SlotError(QAbstractSocket::SocketError err)
{
	QTcpSocket *sock = (QTcpSocket*)sender();
	QString str ="";
	if(err == QAbstractSocket::HostNotFoundError) str += "Host not foud. ";
	if(err == QAbstractSocket::RemoteHostClosedError) str += "Remote host is closed. ";
	if(err == QAbstractSocket::ConnectionRefusedError) str += "Connection was refused. ";
	str += sock->errorString();
	ui->textEdit->append(sock->objectName() + ": " + str);
}

QString ClientEmulator::FormatHex(int value, int digits)
{
	QString ret = QString::number(value, 16);
	ret.reserve(digits);
	int count = digits - ret.length();
	if(count < 0)
	{
		QMessageBox::critical(this, "Ошибка", "Ошибка выполнения FormatXex! Недостаточно разрядов!");
		return ret.right(digits);
	}
	else
	{
	for(int i=0; i < count; i++)
		ret.insert(0,'0');
	}
	return ret;
}
QString ClientEmulator::ConvertCANMsgForServer(CANMsg_t &msg)
{
	QString msg_str;
	msg_str.reserve(27);

	QString regime = QString::number(msg.regime, 2).right(3);  // берём биты 2 1 0 (bit 0 - RTR, bit 2 – EFF)
	if(regime == "000")
		msg_str = "t" + FormatHex(msg.ID, 3);
	else if(regime == "001")
		msg_str = "T" + FormatHex(msg.ID, 3);
	else if(regime == "100")
		msg_str = "e" + FormatHex(msg.ID, 8);
	else if(regime == "101")
		msg_str = "E" + FormatHex(msg.ID, 8);
	else
	{
		QMessageBox::critical(this, "Ошибка", "Ошибка выполнения ConvertCANMsgForServer! Проблема с режимом кадра!");
		msg_str = "e" + FormatHex(msg.ID, 8);
	}

	msg_str += FormatHex(msg.length, 1);
	for(unsigned int i=0; i<msg.length; i++) msg_str += FormatHex(msg.data[i], 2);
	msg_str += "\r";

	return msg_str;
}

void ClientEmulator::SlotSend()
{
	CANMsg_t msg1 { 0x10100101, {8,7,25,58,4,3,2,1}, 8, 0b100, 0 };
	sock_data->write(ConvertCANMsgForServer(msg1).toStdString().data());

	CANMsg_t msg2 { 0x131F0003, {0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00}, 8, 0b100, 0 };
	sock_data->write(ConvertCANMsgForServer(msg2).toStdString().data());

	CANMsg_t msg3 { 0x00000003, {0x05, 0x25, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00}, 8, 0b101, 0 };
	sock_data->write(ConvertCANMsgForServer(msg3).toStdString().data());
}

void ClientEmulator::SlotConnected()
{
	QTcpSocket *sock = (QTcpSocket*)sender();
	ui->textEdit->append(sock->objectName() + " connected");
}

void ClientEmulator::SlotDisconnected()
{
	QTcpSocket *sock = (QTcpSocket*)sender();
	ui->textEdit->append(sock->objectName() + " disconnected");
}

void ClientEmulator::on_pushButton_3_clicked()
{
	SlotSend();
}

void ClientEmulator::on_pushButton_4_clicked()
{
	ui->textEdit->clear();
}
