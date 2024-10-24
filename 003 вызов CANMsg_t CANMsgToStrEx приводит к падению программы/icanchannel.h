#ifndef ICANCHANNEL_H
#define ICANCHANNEL_H

#include <QString>

///\brief Структура CAN-сообщения
struct CANMsg_t
{
    unsigned int ID;        ///< Идентификатор
    unsigned int data[8];   ///< Поля данных
    unsigned int length;    ///< Фактическая длина поля данных
    unsigned int regime;    ///< Формат сообщения (bit 0 - RTR, bit 2 – EFF)
    unsigned int timeStamp; ///< Отметка времени получения

    static QString CANMsgToStr(CANMsg_t &msg, int base = 16)
    {
        QString str = QString::number(msg.ID, base);
        if(msg.length)
        {
            str += " { ";
            for(unsigned int i = 0; i < msg.length; i++)
                str += QString::number(msg.data[i], base) + " ";
            str += "}";
        }
        return str;
    }

    static QString CANMsgToStrEx(CANMsg_t &msg, int base = 16)
    {
        QString str = QString::number(msg.ID, base);
        QString r = QString::number(msg.regime, 2);
        if(r == "0") r += "00";
        str += " R:" + r;
        str += " L:" + QString::number(msg.length, 1) + " ";
        if(msg.length)
        {
            str += "{ ";
            for(unsigned int i = 0; i < msg.length; i++)
                str += QString::number(msg.data[i], base) + " ";
            str += "}";
        }
        return str;
    }
};

///\brief определение типов указателей на функции для обработчиков событий
typedef void (*CBFunctionExt)(int, int, void*);
typedef void (*CBFunctionWriteLog)(QString log);
//typedef void (*CB_Read_Msg_t)(CANMsg_t*);

class ICANChannel
{
public:
    virtual ~ICANChannel() = default;

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
    virtual int GetRecCnt() const = 0;

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
    ///\param [in] указатель на функцию  записи лога
    virtual int SetCBFunctionForLog(CBFunctionWriteLog CBFLog) = 0;

    ///\brief Набор возможных статусов канала
    enum STATUS_CODE
    {
        STATUS_ERROR = 404,	///< Ошибка определения статуса
        UNAVIABLE = 1,		///< Недоступен для нас
        AVIABLE = 2,		///< Доступен, но не запущен, можно запустить
        STARTED = 3			///< Запущен
    };

    ///\brief Набор возможных ошибок
    enum ERROR_CODE
    {
        NO_ERROR = 0,			///< Ошибка отсутствует
        NO_SPECIFIED = -1,		///< Тип ошибки не определен
        BUSY = -2,				///< Устройство занято
        MEMORY_ERROR = -3,		///< Ошибка памяти
        STATE_ERROR = -4,		///< Действие не может быть выполнено в данном состоянии устройства
        CALL_ERROR = -5,		///< Ошибка вызова объекта
        INVALID_PARAMETR = -6,	///< Неверное значение параметра
        ACCES_ERROR = -7,		///< Ошибка доступа к ресурсу
        NO_RESOURCES = -7,		///< Отсутствует запрашиваемый ресурс
        NOT_IMPLEMENT = -8,		///< Ошибка выполнения функции
        IO_ERROR = -9,			///< Ошибка ввода-вывода
        NO_DEVICE = -10,		///< Устройство не обнаружено
        INTERRUPT = -11,		///< Отмена по прерыванию от события
        RESOURCE_ERROR = -12,	///< Нет ресурсов
        TYMEOUT = -13,			///< Прервано по таймауту
        MOCK_ERROR = -14		///< Вызов нереализованной функции
    };

    ///\brief Набор кодов возможных скоростей приема-передачи
    enum BAUD_CODE
    {
        BR_ERROR = 404,	///< Ошибка определения скорости
        BR_125K = 125,	///< 125 кБ/с
        BR_250K = 250,	///< 250 кБ/с
        BR_500K = 500,	///< 500 кБ/с
        BR_1M = 1000	///< 1 МБ/с
    };

    ///\brief Режим работы канала
    enum REGIME_CODE
    {
        REG_ERROR = 404,	///< Ошибка определения режима
        REG_CAN11 = 0x2,    ///<Стандартный режим (идентификатор 11 бит)
        REG_CAN29 = 0x4,    ///<Расширенный режим (идентификатор 29 бит)
        REG_CAN11_29 = 0x6  ///<Работа в любом из режимов (идентификатор 11 или 29 бит)
    };

};

#endif // ICANCHANNEL_H
