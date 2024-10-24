#ifndef CLIENTEMULATOR_H
#define CLIENTEMULATOR_H

#include <QMainWindow>
#include <QTcpSocket>

#include "icanchannel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientEmulator; }
QT_END_NAMESPACE

class ClientEmulator : public QMainWindow
{
	Q_OBJECT

public:
	ClientEmulator(QWidget *parent = nullptr);
	~ClientEmulator();

private slots:
	void on_pushButton_clicked();

	void SlotReadyRead();
	void SlotError(QAbstractSocket::SocketError);
	void SlotSend();
	void SlotConnected();
	void SlotDisconnected();

	void on_pushButton_2_clicked();

	void on_pushButton_3_clicked();

	QString FormatHex(int value, int digits);
	QString ConvertCANMsgForServer(CANMsg_t &msg);
	void ConvertFromServerToCANMsg(QString msg_str, CANMsg_t &msg);

	void on_pushButton_4_clicked();

private:
	Ui::ClientEmulator *ui;
};
#endif // CLIENTEMULATOR_H
