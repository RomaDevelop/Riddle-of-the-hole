#ifndef VALUE_H
#define VALUE_H

#include <QDebug>
#include <QString>

#include "icanchannel.h"

#include "compiler.h"

#include "MyQShortings.h"

class IOperand
{
public:
	typedef std::pair<QString, QString> QStringPair;

	virtual ~IOperand() = default;

	// virtual pure group
	virtual void Assign(IOperand *operand2) = 0;
	virtual bool IsEqual(IOperand *operand2) = 0;
	virtual QString IOClassName() const = 0;
	virtual QStringPair IOGetValueAndType() const = 0;
	virtual QString IOGetValue(QString outputType) const = 0;
	virtual QString IOGetType() const = 0;

	// pair group
	QString IOGetValueAndTypeAndJoinPair() { return IOperand::IOPairJoin(IOGetValueAndType()); }
	static QString IOPairJoin(QStringPair tmp) { return "<["+tmp.first+"]["+tmp.second+"]>"; }

	// class names group
	static QString classValue() { return "IOperand:Value"; }
	static QString classWidjet() { return "IOperand:Wdget"; }
	static QString classWidjetPt() { return "IOperand:WdgetPt"; }
	static QString classParametr() { return "IOperand:Parametr"; }
	static QString classVectorParams() { return "IOperand:classVectorParams"; }
	static QString classDataOperand() { return "IOperand:DataOperand"; }
};

namespace ValuesTypes
{
	const QString vectorWord0	{"vector"};

	const QString vectorBool	{"vector<bool>"};
	const QString vectorByte	{"vector<byte>"};
	const QString vectorUshort	{"vector<ushort>"};
	const QString vectorUlong	{"vector<ulong>"};
	const QString vectorInt		{"vector<int>"};

	const QString int_  {"int"};
	const QString llong {"llong"};

	const QString bool_  {"bool"};
	const QString byte	 {"byte"};
	const QString ushort {"USHORT"};
	const QString ulong  {"ULONG"};

	const QString text {"text"};
	const QString binCode {"binCode"};

	const QString paramPt {"Параметр*"};

	const QStringList simpleNumeric {int_, llong, bool_, byte, ushort, ulong};
	const QStringList vectorsNumeric {vectorBool, vectorByte, vectorUshort, vectorUlong, vectorInt};

	const QStringList all { vectorBool, vectorByte, vectorUshort, vectorUlong, vectorInt,
				int_, llong,
				bool_, byte, ushort, ulong,
				text, binCode,
				paramPt};

	const QString undefined {"undefined"};
	const QString unknown {"unknown"};
	const QString any {"any"};
	const QString autodetect {"autodetect"};

	const int sizeBitsShort {16};
	const int sizeBitsLong	{32};

	struct Range { qint64 min; qint64 max; QString type; };

	const Range	rangeBool		{0,						1,						bool_};
	const Range	rangeByte		{0,						255,					byte};
	const Range	rangeUshort		{0,						65535,					ushort};
	const Range	rangeUlong		{0,						4294967295,				ulong};

	const Range	rangeInt		{-2147483648,			2147483647,				int_};
	const Range rangeLLong		{-9223372036854775807,	9223372036854775807,	llong};

	// диапазоны должны идти сначала беззнаковые, и от меньшего к большему, чтобы при переборе выдавал минимальный диапазон куда попадает число
	const std::vector<Range> allRanges {rangeBool,rangeByte,rangeUshort,rangeUlong,
				rangeInt, rangeLLong};
}

class Byte
{
public:
	std::bitset<8> data {0};

	Byte() = default;
	Byte(long long value) {Set(value);}

	void Set(long long value);

	ulong ToULong() { return data.to_ulong(); }

	void Log(const QString &logStr) const { qdbg << "Log: Byte::" + logStr; }
	void Error(const QString &errStr) const { qdbg << "ERRROR: Byte::" + errStr; }
};

class EasyData
{
	static const int sizePriv {64};
	int len {0};
	bool data[sizePriv] {};
public:
	static const int size {sizePriv};

	EasyData() = default;
	EasyData(const CANMsg_t &msg) { FromMsg(msg); }

	void SetBitBi(int bitIndex, bool val);
	void SetBitBiBi(int byteIndex, int bitInByteIndex, bool val);
	void SetByte(int byteIndex, Byte byte);
	void Clear();

	const bool* Data() {return data;}
	int Len() {return len;}

	void FromBinCode(QString strBinCode);

	void FromMsg(const CANMsg_t &msg);
	void ToMsg(CANMsg_t &msg);

	QString ToStr();

	template<typename T>
	void FromVect(const std::vector<T> &vectVals);

	void Error(const QString &errStr) const { qdbg << "ERRROR: EasyData::" + errStr; }
};

namespace Encodings
{
	const QString bin {"0b"};
	const QString dec {"0d"};
	const QString text {"text"};
	const QString unknown {"unknown"};

	const QStringList all {bin, dec, text};
}

class EncondingValue
{
	QString v {"undefined"};
	QString e {Encodings::text};
public:
	EncondingValue() = default;
	EncondingValue(QString v_, QString enc_): v{v_}, e{enc_} {}

	QString GetVal() const { return v; }
	QString GetEnc() const { return e; }

	bool Set(const EncondingValue &v) { return Set(v.GetVal(),v.GetEnc()); }
	bool Set(QString v_, QString enc_);

	int ToInt() const;

	QString ToStr() const { return GetVal(); }
	QString ToDecStr() const;
	QString ToBinStr() const;

};

namespace FrameTypes
{
	const QString infoKadr{"Информационный_кадр"};
	const QString ctrlKadr{"Управляющий_кадр"};
	const QString rqstKadr{"Запрашивающий_кадр"};
	const QString rspnsKadr{"Ответный_кадр"};

	const QStringList all {infoKadr, ctrlKadr, rqstKadr, rspnsKadr};
};

class Value: public IOperand
{
	QString name {ValuesTypes::undefined};
	QString value {ValuesTypes::undefined};
	QString type {ValuesTypes::undefined};
	void *pt {nullptr};

public:
	Value() = default;
	Value(QString value_, QString type_) { Init("", value_, type_); }
	Value(QString name_, QString value_, QString type_) { Init(name_, value_, type_); }
	Value(QString name_, const std::vector<Value> &values);
	Value(QString name_, void *pt_) { name = name_; pt = pt_; type = ValuesTypes::paramPt; }

	// override group
	void Assign(IOperand *operand2) override
	{
		Set(operand2->IOGetValue(type),type);
		//Log("Assign this = " + IOperand::PairJoin(op2Val));
	}
	virtual bool IsEqual(IOperand *operand2) override
	{
		Log("mock IsEqual " + operand2->IOGetValueAndTypeAndJoinPair());
		return false;
	}
	QString IOClassName() const override { return IOperand::classValue(); }
	QStringPair IOGetValueAndType() const override
	{
		std::pair<QString, QString> ret {ValuesTypes::undefined,ValuesTypes::undefined};
		if(type == ValuesTypes::paramPt)
		{
			Error("IOGetValueType override IOperand: can't get value from ValuesTypes::paramPt");
		}
		else {ret.first = value; ret.second = type; }

		return ret;
	}
	QString IOGetValue(QString outputType)  const override
	{
		Value ret(ConvertToType(outputType));
		return ret.IOGetValueAndType().first;
	}
	QString IOGetType() const override { return type; }

	const QString& GetName() const { return name; }

	bool RangeCheck() const { return RangeCheck(*this); }
	static bool RangeCheck(const Value &value_);
	static bool RangeCheckVector(const Value &value_, const ValuesTypes::Range &range);

	void Init(QString name_, QString value_, QString type_);
	void InitPt(QString name_, void *pt_, QString type_);

	int GetSizeBits() const;
	int GetSizeBytes() const;

	void* GetPt() const { return pt; }

	void Set(QString newValue, QString type_);
	void Set(const Value &newValue);
	void PlusAssign(QString newValue, QString type_);
	void PlusAssign(const Value &newValue);

	static void BinCodesListToDataPobit(const QStringList &values, CANMsg_t &msg);

	static QString GetTypeByStrValue(const QString &value);

	Value ConvertToType(QString newType) const;

	int ToInt(QString operand) const;
	std::vector<int> ToVectInt() const;
	std::vector<bool> ToVectBits(int size) const;
	std::vector<Byte> ToVectBytes(int size) const;
	QString ToStr() const;
	QString ToStrForLog() const {return "<["+value+"]["+type+"]>";}
	QString ToDecStr() const;
	QString ToBinStr() const;
	QString ToBinStrByBytes() const;

	void ToDataPobit(CANMsg_t &msg) const;
	void ToDataPobit(CANMsg_t &msg, QString dataOperand) const;
	void ToDataPobyte(CANMsg_t &msg) const;

	void ToDataUniversal(CANMsg_t &msg, QString dataOperand) const;
	Value& FromDataUniversal(const CANMsg_t &msg, QString dataOperand);

	static void FromDataPobit(const CANMsg_t &msg, std::vector<Value> &values);
	static void FromDataPobyteOld(const CANMsg_t &msg, QString dataOperand, std::vector<Value> &values);

	static void Log(const QString &logStr)   { qdbg << "Log:    Value::" + logStr; }
	static void Error(const QString &errStr) { qdbg << "ERRROR: Value::" + errStr; }
};

class DataOperand: public IOperand
{
public:
	CANMsg_t *frame {nullptr};
	QString strDataOperand;

	void Assign(IOperand *operand2) override
	{
		auto vt {operand2->IOGetValueAndType()};
		Value tmpVal(vt.first, vt.second);
		tmpVal.ToDataUniversal(*frame,strDataOperand);
		//Log("DataOperand Assign " + strDataOperand + " = " + IOperand::PairJoin(operand2GetVal) + " res: " +frame->ToStrEx2());
	}
	virtual bool IsEqual(IOperand *operand2) override
	{
		Log("mock IsEqual " + operand2->IOGetValueAndTypeAndJoinPair());
		return false;
	}

	QString IOClassName() const override { return IOperand::classDataOperand(); }
	QStringPair IOGetValueAndType() const override
	{
		Error("try IOGetValueAndType() from DataOperand! DataOperand can only IOGetValue(outputType)");
		return {ValuesTypes::undefined, ValuesTypes::undefined};
	}
	QString IOGetValue(QString outputType)  const override
	{
		Value ret("", outputType);
		ret.FromDataUniversal(*frame,strDataOperand);
		return ret.IOGetValueAndType().first;
	}
	QString IOGetType() const override { return IOClassName(); }

	void Log(const QString &logStr) const   { qdbg << "Log:    DataOperand::" + logStr; }
	void Error(const QString &errStr) const { qdbg << "ERRROR: DataOperand::" + errStr; }
};

#endif // VALUE_H
