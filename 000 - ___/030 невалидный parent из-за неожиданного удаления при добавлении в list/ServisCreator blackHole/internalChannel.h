#ifndef INTERNALCHANNEL_H
#define INTERNALCHANNEL_H

#include <memory>

#include <QTimer>

#include "caninterfaces.h"

class InternalClient
{
public:
	virtual ~InternalClient() = default;

	virtual void GiveClientMsg(const CANMsg_t &msg) = 0;
};

class InternalChannel
{
	CANInterfaces *interfaces = nullptr;
	ICANChannel* StartChannel(int chanNumber, QString regimeStr, QString baudStr);
	std::vector<InternalClient*> clients;
	std::shared_ptr<QTimer> timerReader;
	bool forwardOnly = true;

public:
	QString name;
	ICANChannel *externalChannel = nullptr;
	bool externalStarted = false;

	InternalChannel();
	~InternalChannel();

	bool Init(const QString & name_, CANInterfaces *interfaces_, const QString &params);
	bool AddClient(InternalClient *client);
	int WriteMsg(const CANMsg_t &msg, InternalClient *sender);

	static int GetICANRegime(QString regimeStr);
	static int GetICANBaud(QString baudStr);
};

#endif // INTERNALCHANNEL_H
