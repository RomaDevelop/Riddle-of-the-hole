#ifndef PARAMETR_H
#define PARAMETR_H

#include <vector>

#include "value.h"
#include "widget.h"
#include "compiler.h"
#include "constants.h"
//#include "protocol.h"


namespace ParamsDefines
{
	const QString param{"Параметр"};
};

namespace ParametrProperties
{
	const QString name{"Имя"};
};

class Parametr: public IOperand
{
	Value value;
	Constants *constants {nullptr};

public:
	const int objectTip; // Parametr::Types

	QString system;
	QString module;
	QString category;
	QString name;
	QString id;
	QString strType;
	QString strPeremennie;
	QString frame;
	std::vector <QString> strViewWidgs;
	std::vector <QString> strCtrlWidgs;

	std::vector<Widget> viewWidgets;
	std::vector<Widget> ctrlWidgs;
	QStringList			ctrlWidgsNames;

	unsigned int	outgoingID {0};
	QString			outgoingIDStr {};
	std::vector<CANMsgIDFilter_t>	incomingFilters;
	QStringList						incomingFiltersStrs;

	std::vector<Value>  peremennie;
	QStringList			peremennieNames;

	static const int maxIndexDataForParametr;

	Parametr(const QStringList &data, int protocolTip_, Constants *constants_);
	QString ToStr();

	// override group
	void Assign(IOperand *operand2) override
	{
		QString curValueType {GetParamValue().IOGetValueAndType().second};
		Value newValue(operand2->IOGetValue(curValueType),curValueType);
		SetValue(newValue);
	}
	virtual bool IsEqual(IOperand *operand2) override
	{
		Log("mock IsEqual " + operand2->IOGetValueAndTypeAndJoinPair());
		return false;
	}
	QString IOClassName() const override { return IOperand::classParametr(); }
	QStringPair IOGetValueAndType() const override
	{
		return GetParamValue().IOGetValueAndType();
	}
	QString IOGetValue(QString outputValueType) const override
	{
		Value ret(GetParamValue().ConvertToType(outputValueType));
		return ret.IOGetValueAndType().first;
	}
	QString IOGetType() const override { return GetParamValue().IOGetType(); }

	const Value& GetParamValue() const { return value; }
	void SetValue(QString newValue, QString valueType);
	void SetValue(const Value& value_);

	void AddWidgetStrs(const QStringList &widgRow);
	std::pair<void*,QString> FindWidget(QString name_, int windowIndex);

	Value* FindPeremennaya(QString name, bool giveErrorIfNotFind);

	static void InitFilter(CANMsgIDFilter_t &filter, QString IDcommand, const Constants *constants);
	void InitFilters();
	void InitParamValue();

	static QStringList ConstructIDStrList(QString idCommand);
	static unsigned int ConstuctID(QString idCommand, const Constants *constants);

	void DoObrabotchik(QString obrabotchik, QString senderName, int windowIndex);
	void DoObrabotchiki(int windowIndex, const QString senderName);

	enum Types { emulator, servis };
	QString TypeToStr();

	void Log(const QString &logStr) const   { qdbg << "Log:   Parametr " + name + ": " + logStr; }
	void Error(const QString &errStr) const { qdbg << "Error: Parametr " + name + ": " + errStr; }
	static void LogSt(const QString &logStr)    { qdbg << "Log:   Parametr: " + logStr; }
	static void ErrorSt(const QString &errStr)  { qdbg << "Error: Parametr: " + errStr; }
};

class VectorParams: public IOperand
{
	std::vector<Parametr*> params;
public:
	void Set(const std::vector<Parametr*> &params_)
	{
		int size = params_.size();
		if(size)
		{
			QStringList types;
			for(int i=0; i<size; i++)
			{
				QString paramT = params_[i]->IOGetType();
				if(!types.contains(paramT))
					types += paramT;
			}

			int typesSize {types.size()};
			if(typesSize == 1)
				params = params_;
			else if(types.size() > 1) Error("Set: different values types in params_: " + types.join(';'));
			else Error("Set: impossible behavior");
		}
		else Error("Set: empty params_");
	}

	// override group
	void Assign(IOperand *operand2) override
	{
		int thisSize = params.size();
		if(thisSize)
		{
			QString thisType {IOGetType()};
			QString type1paramVal { params[0]->IOGetType() };

			QString newVals { operand2->IOGetValue(thisType) };

			QStringList newValsList {newVals.split(';',QString::SkipEmptyParts)};

			if(thisSize == newValsList.size())
			{
				for(int i=0; i<thisSize; i++)
				{
					Value tmpVal {newValsList[i],type1paramVal};
					params[i]->Assign(&tmpVal);
				}
			}
			else Error("Assign: sizes different ("+QSn(thisSize)+" "+QSn(newValsList.size())+")");
		}
		else Error("Assign: empty VectorParams");
	}
	virtual bool IsEqual(IOperand *operand2) override
	{
		Log("mock IsEqual " + operand2->IOGetValueAndTypeAndJoinPair());
		return false;
	}
	QString IOClassName() const override { return IOperand::classVectorParams(); }
	QStringPair IOGetValueAndType() const override
	{
		QStringPair retPair {"",ValuesTypes::undefined};
		int size = params.size();
		if(size)
		{
			QString type1paramVal { params[0]->IOGetType() };
			QString retVal;
			for(int i=0; i<size; i++)
				retPair.first += params[i]->IOGetValueAndType().first + ';';

			if(type1paramVal == ValuesTypes::bool_) retPair.second = ValuesTypes::vectorBool;
			else if(type1paramVal == ValuesTypes::byte) retPair.second = ValuesTypes::vectorByte;
			else if(type1paramVal == ValuesTypes::ushort) retPair.second = ValuesTypes::vectorUshort;
			else if(type1paramVal == ValuesTypes::ulong) retPair.second = ValuesTypes::vectorUlong;
			else if(type1paramVal == ValuesTypes::int_) retPair.second = ValuesTypes::vectorInt;
			else Error("IOGetValueAndType: unrealesed param type " + type1paramVal);
		}
		else Error("IOGetValueAndType: empty VectorParams");

		return retPair;
	}
	QString IOGetValue(QString outputValueType) const override
	{
		auto curVT {IOGetValueAndType()};
		Value cur(curVT.first,curVT.second);
		return cur.ConvertToType(outputValueType).IOGetValueAndType().first;
	}
	QString IOGetType() const override
	{
		QString ret {ValuesTypes::undefined};
		int size = params.size();
		if(size)
		{
			QString t0 {params[0]->IOGetType()};
			if(t0 == ValuesTypes::bool_) ret = ValuesTypes::vectorBool;
			else if(t0 == ValuesTypes::byte) ret = ValuesTypes::vectorByte;
			else if(t0 == ValuesTypes::ushort) ret = ValuesTypes::vectorUshort;
			else if(t0 == ValuesTypes::ulong) ret = ValuesTypes::vectorUlong;
			else if(t0 == ValuesTypes::int_) ret = ValuesTypes::vectorInt;
			else Error("IOGetType: unrealesed param type " + ret);
		}
		else Error("IOGetType: empty VectorParams");
		return ret;
	}

	void Log(const QString &logStr) const   { qdbg << "Log:    VectorParams::" + logStr; }
	void Error(const QString &errStr) const { qdbg << "ERRROR: VectorParams::" + errStr; }
};

#endif // PARAMETR_H
