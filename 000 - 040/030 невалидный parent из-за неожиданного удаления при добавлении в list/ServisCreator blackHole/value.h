#ifndef VALUE_H
#define VALUE_H

#include <memory>

#include <QDebug>
#include <QString>

#include "MyQShortings.h"

#include "icanchannel.h"

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
	//const QString ullong  {"ullong"}; // нет необходимости и не реализвано. Реализация потребует изменения типов в структуре Range

	const QString vectorSByte	{"sbyte[]"};
	const QString vectorShort	{"sshort[]"};
	const QString vectorSLong	{"slong[]"};
	const QString vectorSLLong	{"sllong[]"};

	const QString vectorBool	{"bool[]"};
	const QString vectorUByte	{"ubyte[]"};
	const QString vectorUShort	{"ushort[]"};
	const QString vectorULong	{"ulong[]"};
	//const QString vectorUllong	{"ullong[]"};  // аналогично ullong

	const QString text {"text"};
	const QString textConstant {"textConstant"};
	const QString vectorText {"vectorText"};
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
				text, textConstant, vectorText, binCode};

	const std::vector<QStringPair> VectorsElementsTypes {
		{vectorSByte,sByte}, {vectorShort,sShort}, {vectorSLong,sLong}, {vectorSLLong,sLLong},
		{vectorBool,ubool}, {vectorUByte,uByte}, {vectorUShort,uShort}, {vectorULong,uLong},
		{vectorText,text}
	};

	const QString undefined {"undefined"};
	const QString unknown {"unknown"};
	const QString any {"any"};
	const QString autodetect {"autodetect"};

	const char vectorSplitter = ',';
	/// vectorSplitter должен быть ','
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
	static const int sizeBytes {8};
	static const int sizeBits {sizePriv};

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
};

namespace Encodings
{
	const QString bin {"bin"};
	const QString dec {"dec"};
	const QString hex {"hex"};

	const QStringList all {bin, dec, hex};
}

class EncondingValue: public LogedClass
{
	QString v;
	QString e;
	uint bitLenth {0};
public:
	EncondingValue() = default;
	EncondingValue(QString v_, QString enc_, uint len): v{v_}, e{enc_}, bitLenth {len} {}

	bool Set(QString v_, QString enc_, uint bitLenth_);
	bool Set(const EncondingValue &v) { return Set(v.GetVal(),v.GetEnc(),GetBitLength()); }
	void SetEncoding(const QString &encoding_) { e = encoding_; }
	void SetValue(const QString &value_) { v = value_; }
	void SetBitSize(uint bitLenth_) { bitLenth = bitLenth_; }

	QString GetVal() const { return v; }
	QString GetEnc() const { return e; }
	uint GetBitLength() const { return bitLenth; }

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

	Value(const Value & src);
	Value(Value && src);
	Value& operator= (const Value & src);
	Value& operator= (Value && src);

	void Init(QString name_, QString value_, QString type_);

	void Set(QString newValue, QString type_);
	void Set(const Value &newValue);
	void SetName(const QString &name_);

	typedef void (*cbValueChanged_t)(const Value &newValue, HaveClassName *cbHandler);
private:
	cbValueChanged_t cbValueChanged = nullptr;
	HaveClassName *cbValueChangedHandler = nullptr;
public:
	void SetCBValueChanged(cbValueChanged_t cbValueChanged_, HaveClassName *cbValueChangedHandler_);

	const QString& GetName() const { return name; }
	const QString& GetValue() const { return value; }
	const QString& GetType() const { return type; }
	QStringPair GetValueAndType() const { return {value,type}; }

	std::pair<Value,QString> ConvertToType(QString newType) const;
	qint64 ToNumber() const;
	std::vector<bool> ToVectBits() const;
	std::vector<Byte> ToVectBytes(int size) const;
	QString ToStr() const;

	void ToDataUniversal(CANMsg_t &msg, QString dataOperand) const;
	Value& FromDataUniversal(const CANMsg_t &msg, QString dataOperand);

	static ValuesTypes::Range GetRange(QString type);
	static QString CheckCorrectionAndRange(QString value, QString type);
	static QString CheckVectorCorrectionAndRange(QString value, QString type);

	/// для простого типа возвращает размер типа
	/// для вектора возвращает размер типа одного элемента вектора
	static int GetSizeBitsOfType(const QString &type);

	static QString GetTypeByStrValue(const QString &str);
	static QStringList GetStringListFromValueVector(const QString &str);
	static QStringPair BinCodeToVectorUByte(QString binCode);
	static bool IsVectorNumeric(const QString &value);

	// override IOperand group
	virtual QString GetClassName() const override { return IOperand::classValue(); }
	virtual QString ToStrForLog() const override {return "["+name+"]["+value+"]["+type+"]";}
	virtual void Assign(const IOperand *operand2) override;
	virtual std::shared_ptr<IOperand> Minus(const IOperand *operand2) override;
	virtual void PlusAssign(const IOperand *operand2) override;
	virtual bool CmpOperation(const IOperand *operand2, QString operationCode) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override;
	virtual IOperand* PointAccess(QString nameToAccess) override;
	virtual QString IOGetName() const override { return name; }
	virtual QStringPair IOGetValueAndType() const override;
	virtual QString IOGetValue(QString outputType) const override;
	virtual QString IOGetType() const override { return type; }

	static QString PointAcsClear() { return "clear"; }
	static QStringList PointAcsAll() { return { PointAcsClear() }; }
};

class SubValue: public IOperand
{
	Value &value;
	uint index;
	QString elementType;

	QStringPair GetSubValue() const;
	void SetSubValue(QString newElementValue);
public:
	SubValue(Value &valueSrc, uint index_);

	// override IOperand group
	virtual QString GetClassName() const override { return IOperand::classSubValue(); }
	virtual QString ToStrForLog() const override {return "["+value.GetValue()+"]["+value.GetType()+"]";}
	virtual void Assign(const IOperand *operand2) override { SetSubValue(operand2->IOGetValue(elementType)); }
	virtual std::shared_ptr<IOperand> Minus(const IOperand *operand2) override;
	virtual void PlusAssign(const IOperand *operand2) override { Error(GetClassName()+"::PlusAssign "+ operand2->IOGetType() + " " + CodeMarkers::mock); }
	virtual bool CmpOperation(const IOperand *operand2, QString operationCode) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override;
	virtual IOperand* PointAccess(QString nameToAccess) override;
	virtual QString IOGetName() const override { Error("SubValue:IOGetName forbidden"); return ""; }
	virtual QStringPair IOGetValueAndType() const override { return GetSubValue(); }
	virtual QString IOGetValue(QString outputType) const override;
	virtual QString IOGetType() const override { Error(GetClassName()+"::IOGetType " + CodeMarkers::mock); return {}; }
};

class DataOperand: public IOperand
{
public:
	CANMsg_t &frame;
	QString strDataOperand;

	DataOperand() = delete;
	DataOperand(CANMsg_t &frame_, QString strDataOperand_);

	static void SetDataFilterMaskFromOperand(QString strDataOperand, std::array<u8, 8> &mask);
	static bool CheckDataOperand(const QString &strDataOperand);

	virtual QString GetClassName() const override { return IOperand::classDataOperand(); }
	virtual QString ToStrForLog() const override {return "["+strDataOperand+"]["+frame.ToStrExt2(2)+"]";}
	virtual void Assign(const IOperand *operand2) override;
	virtual std::shared_ptr<IOperand> Minus(const IOperand *operand2) override;
	virtual void PlusAssign(const IOperand *operand2) override;
	virtual bool CmpOperation(const IOperand *operand2, QString operationCode) const override;
	virtual std::shared_ptr<IOperand> Index(int index) override;
	virtual IOperand* PointAccess(QString nameToAccess) override;
	virtual QString IOGetName() const override { Error("VectorParams:IOGetName forbidden"); return ""; }
	virtual QStringPair IOGetValueAndType() const override;
	virtual QString IOGetValue(QString outputType)  const override;
	virtual QString IOGetType() const override { return GetClassName(); }
};

#endif // VALUE_H
