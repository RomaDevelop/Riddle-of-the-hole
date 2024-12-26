#ifndef MARATHONCHANNEL_H
#define MARATHONCHANNEL_H

#include "chai.h"
#include "icanchannel.h"

class MarathonChannel : public ICANChannel
{
	int number;
	QString name;
	bool started_us;
	int regime;
	int baud;
public:
		///\brief Конструктор
	MarathonChannel(int number_, QString name_);

		///\brief Получение номера канала
	int	GetNumber() const override;

		///\brief Получение имени канала
	QString GetName() const override;

		///\brief Текущий статус канала (определяется перечислением STATUS_CODE)
	int GetStatus() const override;

		///\brief Установка режима передачи
		/// после установки режима канал выставляется в тот же статус, что был до
		///\return результат выполнения функции
		///\param[in] regime значение режима передачи (определяется перечислением REGIME_CODE)
	int SetRegime(unsigned int regime) override;

		///\brief Текущая скорость (определяется перечислением REGIME_CODE)
	int GetRegime() const override;

		///\brief Установка скорости передачи
		/// после установки скорости канал выставляется в тот же статус, что был до
		///\return результат выполнения функции (определяется перечислением ERROR_CODE)
		///\param[in] baud скорость передачи (определяется перечислением BAUD_CODE)
	int SetBaud(unsigned int baud) override;

		///\brief Текущая скорость (определяется перечислением BAUD_CODE)
	int GetBaud() const override;

		///\brief Установка аппаратного фильтра
		/// после установки фильтра канал выставляется в тот же статус, что был до
		///\return результат выполнения функции
		///\param[in] filter значение фильтра
		///\param[in] mask значение маски
	int SetAppFilter(unsigned long filter, unsigned long mask) override;

		///\brief Текущий аппаратный фильтр
		///\param[out] filter значение фильтра
		///\param[out] mask значение маски
	void GetAppFilter(unsigned long &filter, unsigned long &mask) const override;

		///\brief Установка программного фильтра
		/// после установки фильтра канал выставляется в тот же статус, что был до
		///\return результат выполнения функции
		///\param[in] filter значение фильтра
		///\param[in] mask значение маски
	int SetProgFilter(unsigned long filter, unsigned long mask) override;

		///\brief Текущий программный фильтр
		///\param[out] filter значение фильтра
		///\param[out] mask значение маски
	void GetProgFilter(unsigned long &filter, unsigned long &mask) const override;

		///\brief Открытие канала для приема-передачи
		///\return результат выполнения функции
	int Start() override;

		///\brief Закрытие канала для приема-передачи
		///\return результат выполнения функции
	int Stop() override;

		///\brief Отправка кадра в сеть
		///\return результат выполнения функции
		///\param[out] *Msg указатель на отправляемый кадр
	int WriteMsg(CANMsg_t *Msg) override;

		///\brief Запрос количества принятых кадров в очереди
		///\return количество сообщений в очереди принятых сообщений
	int GetRecCnt(unsigned int channel) const override;

		///\brief Чтение сообщений из очереди принятых
		///\return результат выполнения функции
		///\param[out] *Msg указатель на буфер для приема кадров
		///\param[in] cnt количество считываемых сообщений
	int ReadMsg(CANMsg_t *Msg, unsigned int cnt = 1) override;

		///\brief Регистрирует функцию обратного вызова при возникновении ошибки
		///\return результат выполнения функции
		///\param [in] CBFext указатель на функцию обратного вызова
		///\param [out] udata буфер для сообщения
	int SetCBFunctionForError(CBFunctionExt CBFext, void *udata) override;

		///\brief Регистрирует функцию обратного вызова при получении сообщения
		///\return результат выполнения функции
		///\param [in] CBFext указатель на функцию обратного вызова
		///\param [out] udata буфер для сообщения
	int SetCBFunctionForMessage(CBFunctionExt CBFext, void *udata) override;

		///\brief Регистрирует функцию записи лога
		///\return результат выполнения функции
		///\param [in] writeLogFunction указатель на функцию обратного вызова
	int WriteLog(QString log) const override;
};

#endif // MARATHONCHANNEL_H
