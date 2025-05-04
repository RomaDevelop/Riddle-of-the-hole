#include "marathonchannel.h"

#include <thread>
using namespace std;

#include <QDebug>

thread *thReadMsgCnt = nullptr;
bool thReadMsgCntStop = false;
int thReadMsgCntPrevCont = 0;
thread *thReadErrCnt = nullptr;
bool thReadErrCntStop = false;

MarathonChannel::MarathonChannel(int number_, QString name_)
{
	number = number_;
	name = name_;
	started_us = false;
	regime = REG_CAN11_29;
	baud = 500;
	appFilter = 0;
	appMask = 0;
	progFilter = 0;
	progMask = 0;

	dataCBError = nullptr;
	dataCBMsg = nullptr;
	CBFError = nullptr;
	CBFMessage = nullptr;
	CBFLog = nullptr;
}

MarathonChannel::~MarathonChannel()
{
	WriteLog("~MarathonChannel()");
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

void MarathonChannel::GetAppFilter(unsigned long &filter_, unsigned long &mask_) const
{
	filter_ = appFilter;
	mask_ = appMask;
}

void MarathonChannel::GetProgFilter(unsigned long &filter_, unsigned long &mask_) const
{
	filter_ = progFilter;
	mask_ = progMask;
}

int MarathonChannel::GetStatus() const
{
	WriteLog("GetStatus begin");
	int result = NO_SPECIFIED;
	if(started_us) // если стоит отметка, что запущен нами
	{
		if(CiStart(number) == ECIOK)
			result = STARTED;
		// chai допускает повторное CiStart и не выдаёт ошибок
		// если он был ранее стартед - ничего страшного,
		// если он не стартед и его невозможно стартовать - мы получим ошибку
		else result = NO_SPECIFIED;
	}
	else
	{
		if(CiOpen(number,REG_CAN11_29) != ECIOK) result = UNAVIABLE;  // если не удалось открыть
		else  // если удалось открыть - останавливаем
		{
			int res_close = CiClose(number);
			if(res_close != ECIOK)
			{
				WriteLog("Ошибка при выполнении GetStatus(), не удалось закрыть канал!" + QString::number(res_close));
				result = STATUS_ERROR;
			}
			else result = AVIABLE;
		}
	}
	WriteLog("GetStatus end");
	return result;
}

int MarathonChannel::SetRegime(unsigned int regime_)
// методы SetRegime, SetBaud, SetAppFilter, SetProgFilter, имеют идентичный принцип работы
// в случае изменения логики работы, изменения нужно вносить везде
{
	WriteLog("SetRegime begin");
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
	WriteLog("SetRegime end");
	return result;
}

int MarathonChannel::SetBaud(unsigned int baud_)
// методы SetRegime, SetBaud, SetAppFilter, SetProgFilter, имеют идентичный принцип работы
// в случае изменения логики работы, изменения нужно вносить везде
{
	WriteLog("SetBaud begin");
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
	WriteLog("SetBaud end");
	return result;
}

int MarathonChannel::SetAppFilter(unsigned long filter, unsigned long mask)
// методы SetRegime, SetBaud, SetAppFilter, SetProgFilter, имеют идентичный принцип работы
// в случае изменения логики работы, изменения нужно вносить везде
{
	WriteLog("SetAppFilter begin");
	int cur_status = GetStatus();  // смотрим текущий статус
	int result = cur_status;
	if(cur_status != STATUS_ERROR)  // если статус определился корректно - устанавливаем указанную скорость
	{
		result = NO_ERROR;
		if(cur_status == UNAVIABLE) result = BUSY;
		if(cur_status == AVIABLE)
		{
			appFilter = filter;
			appMask = mask;
		}
		if(cur_status == STARTED)
		{
			Stop();		// останавливаем для замены фильтра
			started_us = false;
			appFilter = filter;
			appMask = mask;
			result = Start();	// запускаем обратно
			if(result == NO_ERROR) started_us = true;
		}
	}
	WriteLog("SetAppFilter end");
	return result;
}

int MarathonChannel::SetProgFilter(unsigned long filter, unsigned long mask)
// методы SetRegime, SetBaud, SetAppFilter, SetProgFilter, имеют идентичный принцип работы
// в случае изменения логики работы, изменения нужно вносить везде
{
	WriteLog("SetProgFilter begin");
	int cur_status = GetStatus();  // смотрим текущий статус
	int result = cur_status;
	if(cur_status != STATUS_ERROR)  // если статус определился корректно - устанавливаем указанную скорость
	{
		result = NO_ERROR;
		if(cur_status == UNAVIABLE) result = BUSY;
		if(cur_status == AVIABLE)
		{
			progFilter = filter;
			progMask = mask;
		}
		if(cur_status == STARTED)
		{
			Stop();		// останавливаем для замены фильтра
			started_us = false;
			progFilter = filter;
			progMask = mask;
			result = Start();	// запускаем обратно
			if(result == NO_ERROR) started_us = true;
		}
	}
	WriteLog("SetProgFilter end");
	return result;
}

int MarathonChannel::SetBaudInt(int channel, int baud_)
{
	int result = MarathonChannel::NO_SPECIFIED;
	switch (baud_)
	{
	case MarathonChannel::BR_125K: result = CiSetBaud(channel,BCI_125K);
		break;
	case MarathonChannel::BR_250K: result = CiSetBaud(channel,BCI_250K);
		break;
	case MarathonChannel::BR_500K: result = CiSetBaud(channel,BCI_500K);
		break;
	case MarathonChannel::BR_1M: result = CiSetBaud(channel,BCI_1M);
		break;
	default: result = MarathonChannel::INVALID_PARAMETR;
	}
	return result;
}

int MarathonChannel::Start()
{
	WriteLog("Start begin");
	int cur_status = GetStatus();
	int result = cur_status;
	if(cur_status != STATUS_ERROR)  // если статус определился корректно
	{
		if(cur_status == UNAVIABLE) result = BUSY; // если канал недоступен - возвращаем ошибку
		else if(cur_status == AVIABLE) // если канал доступен - делаем опен, скорость, фильтр и старт
		{
			result = CiOpen(number,regime);
			if(result < 0) result = NO_SPECIFIED; // если не удалось открыть - возвращаем ошибку
			else // если удалось открыть  - устанавливаем скорость и фильтр
			{
				int resultBaud = SetBaudInt(number,baud);
				int resultAppF = 0;
				if(appMask) resultAppF = CiSetFilter(number, appFilter, appMask);
				// логика подсказывает тут активацию программного фильтра, но его не нужно никак активировать

				if(resultBaud == ECIOK && resultAppF == ECIOK)  // если удалось установить скорость и фильтр - стартуем
				{
					result = CiStart(number);
					if(result < 0) result = NO_SPECIFIED; // если не удалось стартовать - возвращаем ошибку
					else
					{
						started_us = true;   // если удалось стартовать - ставим пометку, что запущен нами
						result = NO_ERROR;
					}
				}
				else result = NO_SPECIFIED;
			}
		}
		else if(cur_status == STARTED)  // если канал стартован - возвращаем ошибку
			result = STATE_ERROR;
	}
	WriteLog("Start end");
	return result;
}

int MarathonChannel::Stop()
{
	WriteLog("Stop begin");
	started_us = false;  // даже если CiClose возращает ошибку, это означает, что канал итак не открыт.
	int res_close = CiClose(number);
	if(res_close < 0)
	{
		WriteLog("При выполнении Stop канала " + QString::number(number) + " CiClose вернуло значение " + QString::number(res_close));
	}
	WriteLog("Stop end");
	return NO_ERROR;
}

void MarathonChannel::ConvertMsgChaiToCAN(const canmsg_t *chaiMSG, CANMsg_t *CANmsg)
{
	CANmsg->ID = chaiMSG->id;
	CANmsg->length = chaiMSG->len;
	for(int i=0; i < chaiMSG->len; i++)
	{
		CANmsg->data[i] = chaiMSG->data[i];
	}
	CANmsg->regime = chaiMSG->flags;
	CANmsg->timeStamp = chaiMSG->ts;
}

void MarathonChannel::ConvertMsgCANToChai(const CANMsg_t *CANmsg, canmsg_t *chaiMSG)
{
	chaiMSG->id = CANmsg->ID;
	chaiMSG->len = CANmsg->length;
	for(unsigned int i=0; i < CANmsg->length; i++)
	{
		chaiMSG->data[i] = CANmsg->data[i];
	}
	chaiMSG->flags = CANmsg->regime;
	chaiMSG->ts = CANmsg->timeStamp;
}

int MarathonChannel::WriteMsg(CANMsg_t *Msg)
{
	canmsg_t chaiMSG;
	ConvertMsgCANToChai(Msg, &chaiMSG);
	int result = CiWrite(number,&chaiMSG,1);
	if(result == 1) result = ECIOK;  // CiWrite возвращает 1 при успешной отправке, поэтому присваиваем ECIOK
	result=ConvertErrorCode(result);
	return result;
}

int MarathonChannel::GetRecCnt() const
{
	int result = CiRcGetCnt(number);
	if(result < 0) result = ConvertErrorCode(result);
	return result;
}

int MarathonChannel::ReadMsg(CANMsg_t *Msg, unsigned int cnt)
{
	canmsg_t currentMsg[cnt];
	int result = CiRead(number, currentMsg, cnt);
	if(result >= 0)
	{
		for(int i=0; i < result; i++) ConvertMsgChaiToCAN(&currentMsg[i],&Msg[i]);
	}
	return result;
}

int MarathonChannel::SetCBFunctionForError(CBFunctionExt CBFext, void *udata)
{
	int result = CiSetCBex((_u8)number, CIEV_CANERR, (ci_cb_ex)CBFext, udata);
	return result;
}

void StopTread(thread *th, bool &stopFlag)
{
	stopFlag = true;
	th->join();
	stopFlag = false;
	delete th;
	//qDebug() << th << thReadMsgCnt;
	th = nullptr;
	//qDebug() << th << thReadMsgCnt;
}

int MarathonChannel::SetCBFunctionForMessage(CBFunctionExt CBFext, void *udata)
{
    if(CBFext) // если передан указатель
    {
        if(thReadMsgCnt) // если поток существует, останавливаем и удаляем
            StopTread(thReadMsgCnt, thReadMsgCntStop);

        // создание потока
        thReadMsgCnt = new thread([CBFext, this, udata]{
			WriteLog("CBFunctionForMessage start");
            while(!thReadMsgCntStop)
            {
                int cnt = CiRcGetCnt(this->number);
                if(cnt == 0) thReadMsgCntPrevCont = cnt;
                if(cnt > 0 && cnt > thReadMsgCntPrevCont)
                {
                    thReadMsgCntPrevCont = cnt;
                    CBFext(this->number,0,udata);;
                }
            }
			WriteLog("CBFunctionForMessage stop");
        });
    }
    else // если передано nullptr
    {
        WriteLog("В SetCBFunctionForMessage передано nullptr");
        if(thReadMsgCnt) // если поток существует, останавливаем и удаляем
			StopTread(thReadMsgCnt, thReadMsgCntStop);
        // else поток не существует, ничего не делать надо
    }
    return 0;
}
int MarathonChannel::SetCBFunctionForLog(CBFunctionWriteLog CBFLog_)
{
	CBFLog = CBFLog_;
	return NO_ERROR;
}

void MarathonChannel::WriteLog(QString log) const
{
	if(CBFLog) CBFLog("channel " + name + ": " + log);
}

int MarathonChannel::ConvertErrorCode(int sourceCode)
{
	int systemCode;
	// по факту ошибки приходят как в документации: -1...-13, но в chai.h: #define ECIGEN 1...#define ECITOUT 13
	// поэтому switch ( - sourceCode)
	switch ( - sourceCode)
	{
	case ECIOK: systemCode = NO_ERROR; ///< Ошибка отсутствует (0)
		break;
	case ECIGEN: systemCode = NO_SPECIFIED; ///< Тип ошибки не определен (-1)
		break;
	case ECIBUSY: systemCode = BUSY; ///< Устройство занято (-2)
		break;
	case ECIMFAULT: systemCode = MEMORY_ERROR; ///< Ошибка памяти (-3)
		break;
	case ECISTATE: systemCode = STATE_ERROR; ///< Действие не может быть выполнено в данном состоянии устройства (-4)
		break;
	case ECIINCALL: systemCode = CALL_ERROR; ///< Ошибка вызова объекта (-5)
		break;
	case ECIINVAL: systemCode = INVALID_PARAMETR; ///< Неверное значение параметра (-6)
		break;
	case ECIACCES: systemCode = ACCES_ERROR; ///< Ошибка доступа к ресурсу (-7)
		break;
	case ECINOSYS: systemCode = NOT_IMPLEMENT; ///< Ошибка выполнения функции (-8)
		break;
	case ECIIO: systemCode = IO_ERROR; ///< Ошибка ввода-вывода (-9)
		break;
	case ECINODEV: systemCode = NO_DEVICE; ///< Устройство не обнаружено (-10)
		break;
	case ECIINTR: systemCode = INTERRUPT; ///< Отмена по прерыванию от события
		break;
	case ECINORES: systemCode = RESOURCE_ERROR; ///< Отмена по прерыванию от события (-12)
		break;
	case ECITOUT: systemCode = TYMEOUT; ///< Прервано по таймауту (-13)
		break;
	default:systemCode = NO_SPECIFIED;
	}
	return systemCode;
}
