
#include <QDebug>

#include "marathonchannel.h"

MarathonChannel::MarathonChannel(int number_, QString name_)
{
	number = number_;
	name = name_;
	started_us = false;
	regime = REG_CAN11_29;
	baud = 500;
}

int	MarathonChannel::GetNumber() const
{
	return number;
}

QString MarathonChannel::GetName() const
{
	return name;
}

int MarathonChannel::GetRegime() const
{
	return regime;
}

int MarathonChannel::GetBaud() const
{
	return baud;
}

int MarathonChannel::GetStatus() const
{
	int result = -1;
	if(started_us) // если стоит отметка, что запущен нами
		result = STARTED;
	else
	{
		if(CiOpen(number,REG_CAN11_29) < 0) result = UNAVIABLE;  // если не удалось открыть
		else  // если удалось открыть - останавливаем
		{
			int res_close = CiClose(number);
			if(res_close < 0)
			{
				QString log = "Ошибка при выполнении GetStatus(), не удалось закрыть канал!" + QString::number(res_close);
				WriteLog(log);
				qDebug() << log;
				result = STATUS_ERROR;
			}
			else result = AVIABLE;
		}
	}
	return result;
}

int MarathonChannel::SetRegime(unsigned int regime_)
{
	int cur_status = GetStatus();  // смотрим текущий статус
	int result = cur_status;
	if(cur_status != STATUS_ERROR)  // если статус определился корректно - устанавливаем указанный режим режим
	{
		result = NO_ERROR;
		if(cur_status == UNAVIABLE) result = BUSY;
		if(cur_status == AVIABLE) regime = regime_;
		if(cur_status == STARTED)
		{
			Stop();		// останавливаем для замены режима
			started_us = false;
			regime = regime_;
			result = Start();	// запускаем обратно
			if(result == NO_ERROR) started_us = true;
		}
	}
	return result;
}

int MarathonChannel::SetBaud(unsigned int baud_)
{
	int cur_status = GetStatus();  // смотрим текущий статус
	int result = cur_status;
	if(cur_status != STATUS_ERROR)  // если статус определился корректно - устанавливаем указанную скорость
	{
		result = NO_ERROR;
		if(cur_status == UNAVIABLE) result = BUSY;
		if(cur_status == AVIABLE) baud = baud_;
		if(cur_status == STARTED)
		{
			Stop();		// останавливаем для замены скорости
			started_us = false;
			baud = baud_;
			result = Start();	// запускаем обратно
			if(result == NO_ERROR) started_us = true;
		}
	}
	return result;
}

int MarathonChannel::SetAppFilter(unsigned long filter, unsigned long mask)
{
	filter = mask = 0;
	return MOCK_ERROR + filter;
}

void MarathonChannel::GetAppFilter(unsigned long &filter, unsigned long &mask) const
{
	filter = MOCK_ERROR;
	mask = MOCK_ERROR;
}

int MarathonChannel::SetProgFilter(unsigned long filter, unsigned long mask)
{
	filter = mask = 0;
	return MOCK_ERROR + filter;
}

void MarathonChannel::GetProgFilter(unsigned long &filter, unsigned long &mask) const
{
	filter = MOCK_ERROR;
	mask = MOCK_ERROR;
}

int MarathonChannel::Start()
{
	int cur_status = GetStatus();
	int result = cur_status;
	if(cur_status != STATUS_ERROR)  // если статус определился корректно
	{
		if(cur_status == UNAVIABLE) result = BUSY; // если канал недоступен - возвращаем ошибку
		else if(cur_status == AVIABLE) // если канал доступен - делаем опен и старт
		{
			result = CiOpen(number,regime);
			if(result < 0) result = NO_SPECIFIED; // если не удалось открыть - возвращаем ошибку
			else // если удалось открыть  - стартуем
			{
				switch (baud)
				{
				case BR_125K: result = CiSetBaud(number,BCI_125K);
					break;
				case BR_250K: result = CiSetBaud(number,BCI_250K);
					break;
				case BR_500K: result = CiSetBaud(number,BCI_500K);
					break;
				case BR_1M: result = CiSetBaud(number,BCI_1M);
					break;
				default:
					result = NO_SPECIFIED;
					WriteLog("в Start канала " + QString::number(number) + " switch выдал default");
					qDebug() << "в Start канала " + QString::number(number) + " switch выдал default";
				}
				if(result < 0) result = NO_SPECIFIED; // если не удалось стартовать - возвращаем ошибку
				else
				{
					started_us = true;   // если удалось стартовать - ставим пометку, что запущен нами
					result = NO_ERROR;
				}
			}
		}
		else if(cur_status == STARTED)  // если канал стартован - возвращаем ошибку
			result = INVALID_PARAMETR;
	}
	return result;
}

int MarathonChannel::Stop()
{
	started_us = false;
	int res_close = CiClose(number);
	if(res_close < 0)
	{
		QString log = "При выполнении Stop канала " + QString::number(number) + " CiClose вернуло значение " + QString::number(res_close);
		WriteLog(log);
		qDebug() << log;
	}
	return NO_ERROR;
}

int MarathonChannel::WriteMsg(CANMsg_t *Msg)
{
	return Msg->ID;
}

int MarathonChannel::GetRecCnt(unsigned int channel) const
{
	return channel;
}

int MarathonChannel::ReadMsg(CANMsg_t *Msg, unsigned int cnt)
{
	Msg->ID = cnt;
	return Msg->ID;
}

int MarathonChannel::SetCBFunctionForError(CBFunctionExt CBFext, void *udata)
{
	typedef void (*CBF_CI_t)(_u8, _s16, void*) ;
	int result=0;
	result=CiSetCBex((_u8)number, CIEV_CANERR, (CBF_CI_t)CBFext,udata);
	return result;
}

int MarathonChannel::SetCBFunctionForMessage(CBFunctionExt CBFext, void *udata)
{
	typedef void (*CBF_CI_t)(_u8, _s16, void*) ;
	_s16 result=0;
	result=CiSetCBex((_u8)number, CIEV_CANERR, (CBF_CI_t) CBFext,udata);
	return result;
}

int MarathonChannel::WriteLog(QString log) const
{
	return log.toInt();
}
