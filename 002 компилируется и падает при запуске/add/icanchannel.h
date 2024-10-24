#ifndef CANCHANNEL_H
#define CANCHANNEL_H

#include <QString>

/////\brief Структура CAN-сообщения
typedef struct
{
	unsigned int ID;		///< Идентификатор
	unsigned int data[8];	///< Поля данных
	unsigned int length;	///< фактическая длина поля данных
	unsigned int regime;	///< формат сообщения
	unsigned int timeStamp; ///< Отметка времени получения
} CANMsg_t;

/////\brief определение типов указателей на функции для обработчиков событий
typedef void (*CBFunctionExt)(int, int, void*);
//typedef void (*CB_Read_Msg_t)(CANMsg_t*);


class ICANChannel
{
public:
		///\brief Получение номера канала
	virtual int GetNumber() const = 0;

		///\brief Получение имени канала
	virtual QString GetName() const = 0;

		///\brief Текущий статус канала (определяется перечислением STATUS_CODE)
	virtual int GetStatus() const = 0;

		///\brief Установка режима передачи
		/// после установки режима канал выставляется в тот же статус, что был до
		///\return результат выполнения функции
		///\param[in] regime значение режима передачи (определяется перечислением REGIME_CODE)/	virtual int SetRegime(unsigned int regime) = 0;
	virtual int SetRegime(unsigned int regime_) = 0;

		///\brief Текущая скорость (определяется перечислением REGIME_CODE)
	virtual int GetRegime() const = 0;

		///\brief Установка скорости передачи
		/// после установки скорости канал выставляется в тот же статус, что был до
		///\return результат выполнения функции (определяется перечислением ERROR_CODE)
		///\param[in] baud скорость передачи (определяется перечислением BAUD_CODE)
	virtual int SetBaud(unsigned int baud_) = 0;

		///\brief Текущая скорость (определяется перечислением BAUD_CODE)
	virtual int GetBaud() const = 0;

		///\brief Установка аппаратного фильтра
		/// после установки фильтра канал выставляется в тот же статус, что был до
		///\return результат выполнения функции
		///\param[in] filter значение фильтра
		///\param[in] mask значение маски
	virtual int SetAppFilter(unsigned long filter, unsigned long mask) = 0;

		///\brief Текущий аппаратный фильтр
		///\param[out] filter значение фильтра
		///\param[out] mask значение маски
	virtual void GetAppFilter(unsigned long &filter, unsigned long &mask) const = 0;

		///\brief Установка программного фильтра
		/// после установки фильтра канал выставляется в тот же статус, что был до
		///\return результат выполнения функции
		///\param[in] filter значение фильтра
		///\param[in] mask значение маски
	virtual int SetProgFilter(unsigned long filter, unsigned long mask) = 0;

		///\brief Текущий программный фильтр
		///\param[out] filter значение фильтра
		///\param[out] mask значение маски
	virtual void GetProgFilter(unsigned long &filter, unsigned long &mask) const = 0;

		///\brief Открытие канала для приема-передачи
		///\return результат выполнения функции
	virtual int Start() = 0;

		///\brief Закрытие канала для приема-передачи
		///\return результат выполнения функции
	virtual int Stop() = 0;

		///\brief Отправка кадра в сеть
		///\return результат выполнения функции
		///\param[out] *Msg указатель на отправляемый кадр
	virtual int WriteMsg(CANMsg_t *Msg) = 0;

		///\brief Запрос количества принятых кадров в очереди
		///\return количество сообщений в очереди принятых сообщений
	virtual int GetRecCnt(unsigned int channel) const = 0;

		///\brief Чтение сообщений из очереди принятых
		///\return результат выполнения функции
		///\param[out] *Msg указатель на буфер для приема кадров
		///\param[in] cnt количество считываемых сообщений
	virtual int ReadMsg(CANMsg_t *Msg, unsigned int cnt = 1) = 0;

		///\brief Регистрирует функцию обратного вызова при возникновении ошибки
		///\return результат выполнения функции
		///\param [in] CBFext указатель на функцию обратного вызова
		///\param [out] udata буфер для сообщения
	virtual int SetCBFunctionForError(CBFunctionExt CBFext, void *udata) = 0;

		///\brief Регистрирует функцию обратного вызова при получении сообщения
		///\return результат выполнения функции
		///\param [in] CBFext указатель на функцию обратного вызова
		///\param [out] udata буфер для сообщения
	virtual int SetCBFunctionForMessage(CBFunctionExt CBFext, void *udata) = 0;

		///\brief Регистрирует функцию записи лога
		///\return результат выполнения функции
		///\param [in] writeLogFunction указатель на функцию обратного вызова
	virtual int WriteLog(QString log) const = 0;


	///\brief Набор возможных статусов канала
	enum STATUS_CODE
	{
		UNAVIABLE = 0,		///< Недоступен для нас
		AVIABLE = 1,		///< Доступен, но не запущен, можно запустить
		STARTED = 2,		///< Запущен
	};

	///\brief Набор возможных ошибок
	enum ERROR_CODE
	{
		MOCK_ERROR = -14,		///< Вызов нереализованной функции
		STATUS_ERROR = -13,		///< Ошибка определения статуса
		TYMEOUT = -12,			///< Прервано по таймауту
		INTERRUPT = -11,		///< Отмена по прерыванию от события
		NO_DEVICE = -10,		///< Устройство не обнаружено
		IO_ERROR = -9,			///< Ошибка ввода-вывода
		NOT_IMPLEMENT = -8,		///< Ошибка выполнения функции
		NO_RESOURCES = -7,		///< Отсутствует запрашиваемый ресурс
		ACCES_ERROR = -7,		///< Ошибка доступа к ресурсу
		INVALID_PARAMETR = -6,	///< Неверное значение параметра
		CALL_ERROR = -5,		///< Ошибка вызова объекта
		STATE_ERROR = -4,		///< Действие не может быть выполнено в данном состоянии устройства
		MEMORY_ERROR = -3,		///< Ошибка памяти
		BUSY = -2,				///< Устройство занято
		NO_SPECIFIED = -1,		///< Тип ошибки не определен
		NO_ERROR = 0			///< Ошибка отсутствует
	};

	///\brief Набор кодов возможных скоростей приема-передачи
	enum BAUD_CODE
	{
		BR_125K = 125,	///< 125 кБ/с
		BR_250K = 250,	///< 250 кБ/с
		BR_500K = 500,	///< 500 кБ/с
		BR_1M = 1000	///< 1 МБ/с
	};

	///\brief Режим работы канала
	enum REGIME_CODE
	{
		REG_CAN11 = 0x2,    ///<Стандартный режим (идентификатор 11 бит)
		REG_CAN29 = 0x4,    ///<Расширенный режим (идентификатор 29 бит)
		REG_CAN11_29 = 0x6  ///<Работа в любом из режимов (идентификатор 11 или 29 бит)
	};

};

#endif // CANCHANNEL_H
