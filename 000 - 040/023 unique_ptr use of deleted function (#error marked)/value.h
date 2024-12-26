#ifndef VALUE_H
#define VALUE_H

#include <memory>

#include <QDebug>
#include <QString>

#include "MyQShortings.h"

#include "icanchannel.h"

#include "code.h"
#include "ioperand.h"

namespace ValuesTypes
{
	const QString sByte		{"sbyte"};
	const QString sShort	{"sshort"};
	const QString sLong		{"slong"};
	const QString sLLong	{"sllong"};

	const QString ubool  {"bool"};
	const QString uByte	 {"ubyte"};
	const QString uShort {"ushort"};
	const QString uLong  {"ulong"};
	//const QString ullong  {"ullong"}; // запрещено

	const QString vectorSByte	{"sbyte[]"};
	const QString vectorShort	{"sshort[]"};
	const QString vectorSLong	{"slong[]"};
	const QString vectorSLLong	{"sllong[]"};

	const QString vectorBool	{"bool[]"};
	const QString vectorUByte	{"ubyte[]"};
	const QString vectorUShort	{"ushort[]"};
	const QString vectorULong	{"ulong[]"};
	//const QString vectorUllong	{"ullong[]"};  // запрещено

	const QString text {"text"};
	const QString binCode {"binCode"};

	const QStringList simpleNumeric {
				sByte, sShort, sLong, sLLong,
				ubool, uByte, uShort, uLong/*, ullong*/};

	const QStringList vectorsNumeric {
				vectorShort, vectorSByte, vectorSLong, vectorSLLong,
				vectorBool, vectorUByte, vectorUShort, vectorULong, /*vectorUllong*/};

	const QStringList all {
				sByte, sShort, sLong, sLLong,
				ubool, uByte, uShort, uLong, /*ullong,*/
				vectorShort, vectorSByte, vectorSLong, vectorSLLong,
				vectorBool, vectorUByte, vectorUShort, vectorULong, /*vectorUllong,*/
				text, binCode};

	const std::vector<QStringPair> VectorsElementsTypes {
		{vectorSByte,sByte}, {vectorShort,sShort}, {vectorSLong,sLong}, {vectorSLLong,sLLong},
		{vectorBool,ubool}, {vectorUByte,uByte}, {vectorUShort,uShort}, {vectorULong,uLong}
	};

	const QString undefined {"undefined"};
	const QString unknown {"unknown"};
	const QString any {"any"};
	const QString autodetect {"autodetect"};

	const QString vectorSplitter {","};
	/// vectorSplitter должен быть ,
	/// ; - нельзя, потому что в конфигурационном файле это разделитель команд
	/// [;] [,] - неудобно писать при задавании значений
	/// если нужно будет реализовать хранение вектора текстов -
	///		нужно будет для них и только них особый разделитель в конфигурации
	///		и в программном коде сделать разделение на разделитель из конфигурации и разделитель при хранении

	struct SizeBits { int size {0}; QStringList types; };

	const SizeBits sizeBits1		{1,	{ubool,			vectorBool}};
	const SizeBits sizeBits8		{8,	{uByte,sByte,	vectorUByte,vectorSByte}};
	const SizeBits sizeBits16		{16,{sShort,uShort,	vectorShort, vectorUShort}};
	const SizeBits sizeBits32		{32,{sLong,uLong,	vectorSLong, vectorULong}};
	const SizeBits sizeBits64		{64,{sLLong, vectorSLLong}};
	const std::vector<SizeBits> allSizesBits {sizeBits1,sizeBits8,sizeBits16,sizeBits32,sizeBits64};

	struct Range { qint64 min {0}; qint64 max {0}; QStringList types; };

	const Range	rangeBool		{0,						1,						{ubool,vectorBool}};
	const Range	rangeByte		{0,						255,					{uByte,vectorUByte}};
	const Range	rangeUshort		{0,						65535,					{uShort,vectorUShort}};
	const Range	rangeUlong		{0,						4294967295,				{uLong,vectorULong}};

	const Range rangeSByte		{-128,					127,					{sByte,vectorSByte}};
	const Range	rangeShort		{-32768,				32767,					{sShort,vectorShort}};
	const Range	rangeLong		{-2147483648,			2147483647,				{sLong,vectorSLong}};
	const Range rangeLLong		{-9223372036854775807,	9223372036854775807,	{sLLong,vectorSLLong}};
	// в Range::types первым должен быть простой тип, а вторым вектор, поскольку при автоопределении типа вовзвращает range.types[0]

	// диапазоны должны идти сначала беззнаковые, и от меньшего к большему, чтобы при переборе выдавал минимальный диапазон куда попадает число
	const std::vector<Range> allRanges {rangeBool,rangeByte,rangeUshort,rangeUlong,
				rangeSByte, rangeShort, rangeLong, rangeLLong};
}

class Byte
{
public:
	std::bitset<8> data {0};

	Byte() = default;
	bool SetByte(long long value);
	ulong ToULong() { return data.to_ulong(); }
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

	bool GetBit(int byteIndex, int bitInByteIndex);

	const bool* Data() {return data;}
	int Len() {return len;}

	void FromBinCode(QString strBinCode);

	void FromMsg(const CANMsg_t &msg);
	void ToMsg(CANMsg_t &msg);

	QString ToStr();

	template<typename T>
	void FromVect(const std::vector<T> &vectVals);
};

namespace Encodings
{
	const QString bin {"bin"};
	const QString dec {"dec"};
	const QString hex {"hex"};

	const QString unknown {"unknown"};
	const QString undefined {"undefined"};

	const QStringList all {bin, dec, hex};
}

class EncondingValue: public LogedClass
{
	QString v {Encodings::undefined};
	QString e {Encodings::undefined};
	uint bitLenth {0};
public:
	EncondingValue() = default;
	EncondingValue(QString v_, QString enc_, uint len): v{v_}, e{enc_}, bitLenth {len} {}

	QString GetVal() const { return v; }
	QString GetEnc() const { return e; }
	uint GetBitLength() const { return bitLenth; }

	bool Set(const EncondingValue &v) { return Set(v.GetVal(),v.GetEnc(),GetBitLength()); }
	bool Set(QString v_, QString enc_, uint bitLenth_);

	QString ToBinStr() const;
};

class Value: public IOperand
{
	QString name {ValuesTypes::undefined};
	QString value {ValuesTypes::undefined};
	QString type {ValuesTypes::undefined};

public:
	Value() = default;
	Value(QString value_, QString type_) { Init("", value_, type_); }
	Value(const QStringPair &valueAndType) { Init("", valueAndType.first, valueAndType.second); }
	Value(QString name_, QString value_, QString type_) { Init(name_, value_, type_); }

	void Init(QString name_, QString value_, QString type_);

	void Set(QString newValue, QString type_);
	void Set(const Value &newValue);

	typedef void (*cbValueChanged_t)(const Value &newValue, HaveClassName *cbHandler);
private:
	cbValueChanged_t cbValueChanged = nullptr;
	HaveClassName *cbValueChangedHandler = nullptr;
public:
	void SetCBValueChanged(cbValueChanged_t cbValueChanged_, HaveClassName *cbValueChangedHandler_)
	{
		if(cbValueChanged_ && cbValueChangedHandler_)
		{
			cbValueChanged = cbValueChanged_;
			cbValueChangedHandler = cbValueChangedHandler_;
		}
		else Error("Value::SetCBValueChanged: params must not be nullptr");
	}

	const QString& GetName() const { return name; }
	const QString& GetValue() const { return value; }
	const QString& GetType() const { return type; }
	QStringPair GetValueAndType() const { return {value,type}; }

	std::pair<Value,QString> ConvertToType(QString newType) const;
	qint64 ToNumber() const;
	std::vector<bool> ToVectBits() const;
	std::vector<Byte> ToVectBytes(int size) const;
	std::vector<qint64> ToVectLLong() const;
	QString ToStr() const;

	void ToDataUniversal(CANMsg_t &msg, QString dataOperand) const;
	Value& FromDataUniversal(const CANMsg_t &msg, QString dataOperand);

	ValuesTypes::Range GetRange() const;
	QString CheckCorrectionAndRange() const { return CheckCorrectionAndRange(this); }
	static QString CheckCorrectionAndRange(const Value *value_);
	static QString CheckVectorCorrectionAndRange(const Value *value_, const ValuesTypes::Range &range);

	/// для простого типа возвращает размер типа
	/// для вектора возвращает размер типа одного элемента вектора
	static int GetSizeBitsOfType(const QString &type);
	static QString GetTypeByStrValue(const QString &value);
	static QStringList GetStringListFromValueVector(const QString &value);
	static QStringPair BinCodeToVectorUByte(QString binCode);
	static bool IsVectorNumeric(const QString &value);

	// override IOperand group
	QString GetClassName() const override { return IOperand::classValue(); }
	QString ToStrForLog() const override {return "["+value+"]["+type+"]";}
	void Assign(const IOperand *operand2) override;
	void PlusAssign(const IOperand *operand2) override;
	bool IsEqual(const IOperand *operand2) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override;
	virtual IOperand* PointAccess(QString nameToAccess) override
	{
		if(PointAcsAll().contains(nameToAccess))
		{
			if(nameToAccess == PointAcsClear())
			{
				if(ValuesTypes::vectorsNumeric.contains(type))
				{
					value.clear();
				}
				else Error(GetClassName() + "::PointAccess unsupported nameToAccess ["+nameToAccess+"] for type ["+type+"]");
			}
			else Error(GetClassName() + "::PointAccess unrealesed nameToAccess ["+nameToAccess+"]");
		}
		else Error(GetClassName() + "::PointAccess unknown nameToAccess ["+nameToAccess+"]");
		return nullptr;
	}
	QString IOGetName() const override { return name; }
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputType) const override;
	QString IOGetType() const override { return type; }

	static QString PointAcsClear() { return "clear"; }
	static QStringList PointAcsAll() { return { PointAcsClear() }; }
};

class SubValue: public IOperand
{
	Value &value;
	uint index;
	QString elementType;

	QStringPair GetSubValue() const
	{
		Value ret;
		auto elementsList = Value::GetStringListFromValueVector(value.GetValue());
		if((int)index < elementsList.size())
			ret.Set(elementsList[index],elementType);
		else Error("SubValue:GetSubValue: value " + value.ToStrForLog() + " has no element " + QSn(index));
		return ret.GetValueAndType();
	}
	void SetSubValue(QString newElementValue)
	{
		auto elementsList = Value::GetStringListFromValueVector(value.GetValue());
		if((int)index < elementsList.size())
		{
			elementsList[index] = newElementValue;
			value.Set(elementsList.join(ValuesTypes::vectorSplitter),value.GetType());
		}
		else Error("SubValue:GetSubValue: value " + value.ToStrForLog() + " has no element " + QSn(index));
	}
public:
	SubValue(Value &valueSrc, uint index_): value{valueSrc}, index {index_}, elementType {""}
	{
		for(auto &pair:ValuesTypes::VectorsElementsTypes)
			if(pair.first == value.GetType()) elementType = pair.second;

		if(elementType == "")
			Error("SubValue creation: can't define element type of " + valueSrc.GetType());
	}

	// override IOperand group
	QString GetClassName() const override { return IOperand::classSubValue(); }
	QString ToStrForLog() const override {return "["+value.GetValue()+"]["+value.GetType()+"]";}
	void Assign(const IOperand *operand2) override { SetSubValue(operand2->IOGetValue(elementType)); }
	void PlusAssign(const IOperand *operand2) override { Error(GetClassName()+"::PlusAssign "+ operand2->IOGetType() + " " + CodeMarkers::mock); }
	bool IsEqual(const IOperand *operand2) const override  { Error(GetClassName()+"::IsEqual "+ operand2->IOGetType() + " " + CodeMarkers::mock); return {}; }
	virtual std::shared_ptr<IOperand> Index(int index) override
	{
		Error("SubValue::Index Can't take Index from SubValue " + QSn(index));
		return nullptr;
	}
	virtual IOperand* PointAccess(QString nameToAccess) override
	{
		Error(GetClassName()+"::PointAccess " + nameToAccess + " " + CodeMarkers::mock);
		return nullptr;
	}
	QString IOGetName() const override { Error("SubValue:IOGetName forbidden"); return ""; }
	QStringPair IOGetValueAndType() const override { return GetSubValue(); }
	QString IOGetValue(QString outputType) const override { Error(GetClassName() + "::IOGetValue "+ outputType + " " + CodeMarkers::mock); return {}; }
	QString IOGetType() const override { Error(GetClassName()+"::IOGetType " + CodeMarkers::mock); return {}; }
};

class DataOperand: public IOperand
{
public:
	CANMsg_t *frame {nullptr};
	QString strDataOperand;

	static void SetDataFilterMaskFromOperand(QString strDataOperand, CANMsgDataFilter_t &filter);
	static QString DataOperandFromWords(QStringList words, int DataOperandIndex, int &countWordsInDataOperand)
	{
		countWordsInDataOperand = 0;
		QString dataOperand = "Data";
		int startFrom = DataOperandIndex+1;
		for(int twice=0; twice<2; twice++)
		{
			int countNestedBracers = 0;
			if(startFrom < words.size() && words[startFrom] == Operators::index1)
				for(int i=startFrom; i<words.size(); i++)
				{
					if(words[i] == Operators::index1) countNestedBracers++;
					if(words[i] == Operators::index2) countNestedBracers--;

					dataOperand += words[i];
					countWordsInDataOperand++;

					if(countNestedBracers == 0 && words[i] == Operators::index2)
					{
						startFrom = i+1;
						break;
					}
				}
		}
		return dataOperand;
	}

	QString GetClassName() const override { return IOperand::classDataOperand(); }
	QString ToStrForLog() const override {return "["+strDataOperand+"]["+frame->ToStrEx2(2)+"]";}
	void Assign(const IOperand *operand2) override;
	void PlusAssign(const IOperand *operand2) override;
	bool IsEqual(const IOperand *operand2) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override
	{
		Error(CodeMarkers::mock + GetClassName() + "::Index; index = " + QSn(index));
		return nullptr;
	}
	virtual IOperand* PointAccess(QString nameToAccess) override
	{
		Error(GetClassName()+"::PointAccess " + nameToAccess + " " + CodeMarkers::mock);
		return nullptr;
	}
	QString IOGetName() const override { Error("VectorParams:IOGetName forbidden"); return ""; }
	QStringPair IOGetValueAndType() const override;
	QString IOGetValue(QString outputType)  const override;
	QString IOGetType() const override { return GetClassName(); }
};

#endif // VALUE_H
