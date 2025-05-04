#ifndef MARATHONINTERFACE_H
#define MARATHONINTERFACE_H

#include <memory>

#include "chai.h"
#include "icanchannel.h"
#include "icaninterface.h"

#include "MarathonInterface_global.h"

class MARATHONINTERFACE_EXPORT MarathonInterface : public ICANInterface
{
	static bool inited;

	int number;
	QString name;

	///\brief Указатель на функцию записи лога
	CBFunctionWriteLog CBFLog;

	///\brief Перечень каналов
	vector<ICANChannel*> channels;

public:
	///\brief задаются начальные значения свойствам
	MarathonInterface();
	///\brief вызов ClearChannels();
	~MarathonInterface();

	///\brief Остановка всех каналов, освобождение выделенной памяти под каналы.
	void ClearChannels();

	///\brief инициализация библиотеки (однократно),
	/// вызов ClearChannels()
	/// заполнение vector<ICANChannel*> channels
	int ScanInterface() override;
	int	GetNumber() const override;
	QString GetName() const override;
	int GetChannelsCount() const override;
	void GetChannels(vector<ICANChannel*> &channels) override;
	void GetChannels(vector<ICANChannel*> &channels, int status) override;
	void GetChannels_if(vector<ICANChannel*> &channels, bool(*condition)(ICANChannel*)) override;
	void SetCBFunctionForLog(CBFunctionWriteLog CBFLog_) override;
	void SetCBFunctionForToAllChannels(CBFunctionWriteLog CBFLog_) override;

	void WriteLog(QString log) const;
};

#endif // MARATHONINTERFACE_H
