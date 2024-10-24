#ifndef MARATHONCHANNEL_H
#define MARATHONCHANNEL_H

#include "chai.h"
#include "icanchannel.h"

#include "MarathonInterface_global.h"

/* typedef void (*ci_cb)(_s16);
ограничится либо только вызовами CiSetCB(), либо только
вызовами CiSetCBex. Поведение библиотеки при
одновременном использовании функций CiSetCB() и
CiSetCBex() не определено. */
typedef void (*ci_cb_ex)(_u8, _s16, void*);

class MarathonChannel : public ICANChannel
{
	int number;
	QString name;
	bool started_us;
	int regime;
	int baud;
	unsigned long appFilter;
	unsigned long appMask;
	unsigned long progFilter;
	unsigned long progMask;

	QString *dataCBError;
	CANMsg_t *dataCBMsg;
	CBFunctionExt CBFError;
	CBFunctionExt CBFMessage;
	CBFunctionWriteLog CBFLog;

public:
	MarathonChannel(int number_, QString name_);
	~MarathonChannel() override;

	QString GetName() const override;
	int	GetNumber() const override;
	int GetRegime() const override;
	int GetBaud() const override;
	void GetAppFilter(unsigned long &filter, unsigned long &mask) const override;
	void GetProgFilter(unsigned long &filter, unsigned long &mask) const override;

	// если started_us - STARTED
		// если нет, если CiOpen выдаёт ошибку - UNAVIABLE
			// если CiOpen, CiClose успешно - AVIABLE
				// если CiClose выдло ошибку - STATUS_ERROR
	int GetStatus() const override;

	// если канал доступен - выставляет параметр
	// если канал запущен - останавливает, выставляет параметр, запускает
	int SetRegime(unsigned int regime) override;
	int SetBaud(unsigned int baud) override;
	int SetAppFilter(unsigned long filter, unsigned long mask) override;
	int SetProgFilter(unsigned long filter, unsigned long mask) override;

	// если статус определился корректно
		// если канал недоступен - возвращаем ошибку
			// если канал доступен - делаем опен(с режимом), скорость, фильтр и старт
				// если не удалось открыть - возвращаем ошибку
				// если удалось открыть  - устанавливаем SetBaudInt и CiSetFilter
				// если удалось установить скорость и фильтр - CiStart
					// если не удалось стартовать - возвращаем ошибку
					// если удалось стартовать - started_us, NO_ERROR
			// если статус == STARTED - возвращаем ошибку STATE_ERROR
	int Start() override;
	int Stop() override;

	int WriteMsg(CANMsg_t *Msg) override;

	int GetRecCnt() const override;
	int ReadMsg(CANMsg_t *Msg, unsigned int cnt = 1) override;

	int SetCBFunctionForError(CBFunctionExt CBFext, void *udata) override;
	int SetCBFunctionForMessage(CBFunctionExt CBFext, void *udata) override;
	int SetCBFunctionForLog(CBFunctionWriteLog CBFLog) override;

	///\brief вызывает функцию записи лога если она зарегистрирована
	void WriteLog(QString log) const;
	///\brief вызывает CiSetBaud с указанной скоростью
	static int SetBaudInt(int channel, int baud_);
	///\brief преобразует canmsg_t в CANMsg_t
	static void ConvertMsgChaiToCAN(const canmsg_t *chaiMSG, CANMsg_t *CANmsg);
	///\brief преобразует CANMsg_t в canmsg_t
	static void ConvertMsgCANToChai(const CANMsg_t *CANmsg, canmsg_t *chaiMSG);
	///\brief преобразует код ошибки chai в ICANChannel::ERROR_CODE
	static int ConvertErrorCode(int sourceCode);
};

#endif // MARATHONCHANNEL_H
