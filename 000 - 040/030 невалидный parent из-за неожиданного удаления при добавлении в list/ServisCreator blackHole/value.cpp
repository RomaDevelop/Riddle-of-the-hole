#include "value.h"

#include <cmath>

bool EncondingValue::Set(QString v_, QString enc_, uint bitLenth_)
{
	if(Encodings::all.contains(enc_))
	{
		v = v_;
		e = enc_;
		bitLenth = bitLenth_;
		return true;
	}
	else
		Error("EncondingValue::Set wrong encoding [" + enc_ + "]");
	return false;
}

QString EncondingValue::ToBinStr() const
{
	QString ret;
	if(e == Encodings::bin) ret = v;
	else if(e == Encodings::hex) ret = QSn(v.toUInt(nullptr,16),2);
	else if(e == Encodings::dec) ret = QSn(v.toUInt(nullptr,10),2);
	else Error("ToBinStr unrealesed encoding " + e);

	while(ret.length() < (int)bitLenth) ret = '0' + ret;

	if(ret.length() > (int)bitLenth) Error("ToBinStr to big value " + v);

	return ret;
}

void Value::Assign(const IOperand *operand2)
{
	Set(operand2->IOGetValue(type),type);
}

std::shared_ptr<IOperand> Value::Minus(const IOperand * operand2)
{
	if(ValuesTypes::simpleNumeric.contains(type))
	{
		QString typeOp2 = operand2->IOGetType();
		if(ValuesTypes::simpleNumeric.contains(typeOp2))
		{
			QString newValue = QSn(value.toULongLong() - operand2->IOGetValue(type).toLongLong());
			return std::make_shared<Value>(newValue,type);
		}
		else Error(GetClassName() + "::Minus unrealesed PlusAssign 1. Tried to PlusAssign " + type + " to " + operand2->IOGetType());
	}
	else Error(GetClassName() + "::Minus unrealesed PlusAssign final. Tried to PlusAssign " + type + " to " + operand2->IOGetType());
	return nullptr;
}

void Value::PlusAssign(const IOperand *operand2)
{
	if(ValuesTypes::simpleNumeric.contains(type))
	{
		QString typeOp2 = operand2->IOGetType();
		if(ValuesTypes::simpleNumeric.contains(typeOp2))
		{
			QString newValue = QSn(value.toULongLong() + operand2->IOGetValue(type).toLongLong());
			Set(newValue,type);
		}
		else Error(GetClassName() + "::PlusAssign: simpleNumeric.contains, unrealesed type operand2 ["+typeOp2+"]");
	}
	else if(ValuesTypes::vectorsNumeric.contains(type) && ValuesTypes::simpleNumeric.contains(operand2->IOGetType()))
	{
		auto addingVal = operand2->IOGetValueAndType();
		auto valsList = Value::GetStringListFromValueVector(value);
		valsList += addingVal.first;
		Set(valsList.join(ValuesTypes::vectorSplitter),type);
	}
	else if(type == ValuesTypes::text)
	{
		if(operand2->IOGetType() == ValuesTypes::text)
			value += operand2->IOGetValueAndType().first;
		else if(operand2->IOGetType() == ValuesTypes::textConstant)
			value += TextConstant::GetTextConstVal(operand2->IOGetValueAndType().first);
		else Error(GetClassName() + "::PlusAssign unrealesed text PlusAssign. Tried to PlusAssign " + operand2->IOGetType() + " to " + type);
	}
	else Error(GetClassName() + "::PlusAssign unrealesed PlusAssign. Tried to PlusAssign " + operand2->IOGetType() + " to " + type);
}

bool Value::CmpOperation(const IOperand *operand2, QString operationCode) const
{
	Value valueOp2(operand2->IOGetValue(type),type);
	if(type == ValuesTypes::binCode) // переводим в число чтобы сравнение 0011 == 11 выдавало true
	{
		if(operationCode == Operators::cmpEqual)
			return value.toULongLong() == valueOp2.value.toULongLong();
		else if(operationCode == Operators::cmpNotEqual)
			return value.toULongLong() != valueOp2.value.toULongLong();
	}
	else
	{
		if(operationCode == Operators::cmpEqual)
			return value == valueOp2.value;
		else if(operationCode == Operators::cmpNotEqual)
			return value != valueOp2.value;
	}
	Error(GetClassName() + "::CmpOperation unrealesed operation ["+ operationCode +"]");
	return false;
}

std::shared_ptr<IOperand> Value::Index(int index)
{
	return std::make_shared<SubValue>(*this,index);
}

IOperand * Value::PointAccess(QString nameToAccess)
{
	if(PointAcsAll().contains(nameToAccess))
	{
		if(nameToAccess == PointAcsClear())
		{
			if(ValuesTypes::vectorsNumeric.contains(type) || type == ValuesTypes::text)
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

QStringPair Value::IOGetValueAndType() const
{
	return {value, type };
}

QString Value::IOGetValue(QString outputType) const
{
	auto convertRes = ConvertToType(outputType);
	if(convertRes.second != "") Error(GetClassName() + "::IOGetValue ConvertToType error: " + convertRes.second);
	return convertRes.first.GetValue();
}

Value::Value(const Value & src):
	name{src.name}, value{src.value}, type{src.type},
	cbValueChanged {nullptr}, cbValueChangedHandler{nullptr}
{

}

Value::Value(Value && src):
	name{std::move(src.name)}, value{std::move(src.value)}, type{src.type},
	cbValueChanged {src.cbValueChanged}, cbValueChangedHandler{src.cbValueChangedHandler}
{
	src.cbValueChanged = nullptr;
	src.cbValueChangedHandler = nullptr;
}

Value & Value::operator= (Value && src)
{
	name = std::move(src.name);
	value = std::move(src.value);
	type = src.type;
	cbValueChanged = src.cbValueChanged;
	cbValueChangedHandler = src.cbValueChangedHandler;
	src.cbValueChanged = nullptr;
	src.cbValueChangedHandler = nullptr;

	return *this;
}

Value & Value::operator= (const Value & src)
{
	name = src.name;
	value = src.value;
	type = src.type;
	cbValueChanged  = nullptr;
	cbValueChangedHandler = nullptr;

	return *this;
}

void Value::Init(QString name_, QString value_, QString type_)
{
	if(value != ValuesTypes::undefined) Error("Init value not undefined " + name + " " + value + " " + type);
	name=name_, value=value_, type=type_;

	if(QString checkRes = CheckCorrectionAndRange(value_,type_); checkRes != "")
	{
		Error("Value::Init error. Range check error " + ToStrForLog() + " " + checkRes);
		value = "";
	}

	if(cbValueChanged) cbValueChanged(*this,cbValueChangedHandler);
}

int Value::GetSizeBitsOfType(const QString &type)
{
	int sizeType = 0;
	for(auto &size:ValuesTypes::allSizesBits)
		if(size.types.contains(type)) sizeType = size.size;
	if(!sizeType) Logs::ErrorSt("GetSizeBitsOfType: can't get size of type ["+type+"]");

	return sizeType;
}

ValuesTypes::Range Value::GetRange(QString type)
{
	ValuesTypes::Range ret;
	for(auto &range:ValuesTypes::allRanges)
		if(range.types.contains(type)) ret = range;
	if(!ret.min && !ret.max) Logs::ErrorSt("Can't get range of type " + type);
	return ret;
}

QString Value::CheckCorrectionAndRange(QString value, QString type)
{
	QString ret;
	QString addStr = " [" + value + "] to type [" + type + "]";

	if(type == ValuesTypes::binCode)
	{
		ret = "";
		for(auto c:value)
			if(!(c=='0' || c=='1'))
			{
				ret = "Wrong bincode: " + value;
				break;
			}
	}
	else if(ValuesTypes::simpleNumeric.contains(type))
	{
		if(value == "") { /* ret(""); - по-умолчанию */ }
		else
		{
			bool ok;
			long long check = value.toLongLong(&ok);
			if(!ok) ret = "RangeCheck: error toLongLong conversion" + addStr;
			ValuesTypes::Range curRange = GetRange(type);
			if(check >= curRange.min && check <= curRange.max) { /* ret(true, ""); - по-умолчанию */ }
			else ret = "RangeCheck: error check range" + addStr;
		}
	}
	else if(ValuesTypes::vectorsNumeric.contains(type))
	{
		ret = CheckVectorCorrectionAndRange(value,type);
	}
	else if(type == ValuesTypes::textConstant)
	{
		if(TextConstant::IsItTextConstant(value,false)) { /* ret(""); - по-умолчанию */ }
		else ret = "RangeCheck: textConstant ["+value+"] is not textConstant";
	}
	else if(type == ValuesTypes::text) { /* ret(""); - по-умолчанию */ }
	else if(type == ValuesTypes::any) { /* ret(""); - по-умолчанию */ }
	else
	{
		ret = "RangeCheck: unrealesed type [" + type + "]";
	}

	return ret;
}

QString Value::CheckVectorCorrectionAndRange(QString value, QString type)
{
	if(value == "")
		return "";

	QString addStr = " [" + value + "] to type [" + type + "]";
	ValuesTypes::Range range = GetRange(type);
	QString result("");
	auto vals { Value::GetStringListFromValueVector(value) };
	for(auto &v:vals)
	{
		if(v.isEmpty())
			continue;

		bool ok;
		auto tmpVal {v.toLongLong(&ok)};
		if(!ok) result = "RangeCheck: error toLongLong conversion" + addStr;
		if (!(tmpVal>=range.min && tmpVal<=range.max)) result = "RangeCheckVector error" + addStr;
	}
	return result;
}

void Value::SetCBValueChanged(Value::cbValueChanged_t cbValueChanged_, HaveClassName * cbValueChangedHandler_)
{
	if(cbValueChanged_ && cbValueChangedHandler_)
	{
		cbValueChanged = cbValueChanged_;
		cbValueChangedHandler = cbValueChangedHandler_;
	}
	else Error("Value::SetCBValueChanged: params must not be nullptr");
}

void Value::SetName(const QString & name_)
{
	if(name != ValuesTypes::undefined)
		Warning(GetClassName() + "::SetName: name not undefined, name was ["+name+"]");
	name = name_;
}

void Value::Set(QString newValue, QString type_)
{
	value = "";
	if(type_ == ValuesTypes::undefined || type_ == ValuesTypes::unknown)
		Error(GetClassName()+"::Set: wrong setting type ("+type_+")");
	else if(type == type_)
	{
		value = newValue;
		if(QString checkRes = CheckCorrectionAndRange(newValue,type_); !checkRes.isEmpty())
		{
			Error(GetClassName()+":: CheckCorrectionAndRange error ["+checkRes+"]");
		}
	}
	else if(type == ValuesTypes::undefined) { value = newValue; type = type_; }
	else
	{
		auto convertRes = Value(newValue,type_).ConvertToType(type);
		if(convertRes.second != "") Error(GetClassName()+"::Set error ConvertToType ConvertToType error: " + convertRes.second);
		else value = convertRes.first.GetValue();
	}

	if(cbValueChanged) cbValueChanged(*this,cbValueChangedHandler);
}

void Value::Set(const Value &newValue)
{
	auto new_vt {newValue.IOGetValueAndType()};
	Set(new_vt.first, new_vt.second);
}

std::pair<Value,QString> Value::ConvertToType(QString newType) const
{
	Value retValue("",newType);
	QString retErrorStr;

	// типы совпадают

	// выходной тип - any
	if(newType == type || newType == ValuesTypes::any)
		retValue = *this;

	// исходный тип - simpleNumeric
	else if(ValuesTypes::simpleNumeric.contains(type))
	{
		// выходной тип - тоже simpleNumeric
		if(ValuesTypes::simpleNumeric.contains(newType))
		{
			retValue.value = value;
		}
		// выходной тип - тоже vectorsNumeric
		else if(ValuesTypes::vectorsNumeric.contains(newType))
		{
			if(Value::GetSizeBitsOfType(type) <= Value::GetSizeBitsOfType(newType))
				retValue.value = value;
			else if(newType == ValuesTypes::vectorUByte)
			{
				QString binCode = QSn(value.toLongLong(),2);
				while(binCode.length() < Value::GetSizeBitsOfType(type))
					binCode = "0" + binCode;
				retValue = BinCodeToVectorUByte(binCode);
			}
			else retErrorStr = "code 2.1 (tryed convert " + ToStrForLog() + " to " + newType + ")";
		}
		else retErrorStr = "code 2 (tryed convert " + ToStrForLog() + " to " + newType + ")";
	}

	// исходный тип - текст
	else if(type == ValuesTypes::text)
	{
		if(ValuesTypes::simpleNumeric.contains(newType) && Code::IsInteger(value))
		{
			retValue.value = value;
		}
		else if(ValuesTypes::vectorsNumeric.contains(newType) && Value::IsVectorNumeric(value))
		{
			retValue.value = value;
		}
		else if(newType == ValuesTypes::vectorUByte && Code::IsInteger(value))
		{
			QString binCode = QSn(value.toLongLong(),2);
			retValue = BinCodeToVectorUByte(binCode);
		}
		else if(newType == ValuesTypes::binCode)
		{
			bool ok;
			value.toULongLong(&ok,2);
			if(ok) retValue.value = value;
			else retErrorStr = "value ("+ToStrForLog()+") conversion to bincode error";
		}
		else retErrorStr = "code 0 (tryed convert " + ToStrForLog() + " to " + newType + ")";
	}

	// выходной тип - текст
	else if(newType == ValuesTypes::text)
	{
		if(type == ValuesTypes::textConstant) retValue.value = TextConstant::GetTextConstVal(value);
		else retValue.value = value;
	}

	// исходный тип - binCode
	else if(type == ValuesTypes::binCode)
	{
		if(ValuesTypes::simpleNumeric.contains(newType))
			retValue.value = QSn(value.toLongLong(nullptr,2));
		else if(newType == ValuesTypes::vectorUByte)
		{
			retValue = BinCodeToVectorUByte(value);
		}
		else retErrorStr = "code 1 (tryed convert " + ToStrForLog() + " to " + newType + ")";
	}

	// исходный тип - vectorsNumeric
	else if(ValuesTypes::vectorsNumeric.contains(type))
	{
		if(ValuesTypes::simpleNumeric.contains(newType))
		{
			auto valsList = Value::GetStringListFromValueVector(value);
			if(valsList.size() == 1)
				retValue.value = valsList[0];
			else if(type == ValuesTypes::vectorBool || type == ValuesTypes::vectorUByte)
				retValue.value = QSn(ToNumber());
			else retErrorStr = "code 3.1 (tryed convert " + ToStrForLog() + " to " + newType + ")";
		}
		else retErrorStr = "code 3 (tryed convert " + ToStrForLog() + " to " + newType + ")";
	}
	else retErrorStr = "code 4 (tryed convert " + ToStrForLog() + " to " + newType + ")";

	if(newType == ValuesTypes::ubool && retErrorStr == "")  // для того, чтобы bool можно было сравнивать с 2, 10 и тд
	{
		if(retValue.value != "0")
		{
			bool ok;
			if(retValue.value.toLongLong(&ok,10) > 0 && ok)
				retValue.value = "1";
			else retErrorStr = "code 5, wrong bool result ["+retValue.value+"]; (tryed convert " + ToStrForLog() + " to " + newType + ")";
		}
	}

	if(retErrorStr == "")
	{
		if(QString checkRes = CheckCorrectionAndRange(retValue.value,retValue.type); !checkRes.isEmpty())
		{
			retValue.value = "";
			retErrorStr = "RangeCheck error " + checkRes + " (tryed convert " + ToStrForLog() + " to " + newType + ")";
		}
	}

	return {retValue,retErrorStr};
}

QString Value::ToStr() const
{
	QString ret = ValuesTypes::undefined;

	if(ValuesTypes::simpleNumeric.contains(type)) ret = value;
	else if(ValuesTypes::vectorsNumeric.contains(type)) ret = value;
	else if(type == ValuesTypes::text) ret = value;
	else Error(GetClassName() + "::ToStr unrealesed type " + name + " " + value + " " + type);

	return ret;
}

QStringPair Value::BinCodeToVectorUByte(QString binCode)
{
	Value ret("",ValuesTypes::vectorUByte);
	while (binCode.length() %8 != 0) binCode = "0" + binCode;
	for(int i=0; i<binCode.length(); i+=8)
	{
		QString byteBinCode = binCode.mid(i,8);
		bool ok;
		ret.value += QSn(byteBinCode.toUShort(&ok,2)) + ValuesTypes::vectorSplitter;
		if(!ok) Logs::ErrorSt("Value::BinCodeToVectorUByte error. BinCode: " + binCode);
	}
	return ret.GetValueAndType();
}

bool Value::IsVectorNumeric(const QString &value)
{
	auto strList = GetStringListFromValueVector(value);
	for(auto &str:strList) if(!Code::IsNumber(str))
		return false;
	return true;
}

qint64 Value::ToNumber() const
{
	qint64 ret = 0;

	if(ValuesTypes::simpleNumeric.contains(type))
	{
		bool ok = false;
		ret = value.toLongLong(&ok);
		if(!ok) Error(GetClassName() + "::ToNumber error conversion 1 Value " + ToStrForLog());
	}
	else if(type == ValuesTypes::binCode)
	{
		bool ok = false;
		ret = value.toLongLong(&ok,2);
		if(!ok) Error(GetClassName() + "::ToNumber error conversion 2 Value " + ToStrForLog());
	}
	else if(ValuesTypes::vectorsNumeric.contains(type))
	{
		auto valsList { Value::GetStringListFromValueVector(value) };
		int valsListSize = valsList.size();
		if(valsListSize == 0) ret = 0;
		else if(valsListSize == 1)
		{
			bool ok;
			ret = valsList[0].toLongLong(&ok,2);
			if(!ok) Error(GetClassName() + "::ToNumber error conversion 1.1.1 Value " + ToStrForLog());
		}
		else if(valsListSize > 1)
		{
			if(type == ValuesTypes::vectorBool)
			{
				bool ok;
				ret = valsList.join("").toLongLong(&ok,2);
				if(!ok) Error(GetClassName() + "::ToNumber error conversion 1.1 Value " + ToStrForLog());
			}
			else if(type == ValuesTypes::vectorUByte)
			{
				auto valsList { Value::GetStringListFromValueVector(value) };
				for(auto &val:valsList)
				{
					val = QSn(val.toUShort(),2);
					while(val.size() < 8) val = "0" + val;
				}
				bool ok;
				ret = valsList.join("").toLongLong(&ok,2);
				if(!ok) Error(GetClassName() + "::ToNumber error conversion 1.1.1 Value " + ToStrForLog());
			}
			else Error(GetClassName() + "::ToNumber wrong type output 1.2 Value " + ToStrForLog());
		}
		else Error(GetClassName() + "::ToNumber impossible vsize "+QSn(valsListSize)+" Value " + ToStrForLog());
	}
	else Error(GetClassName() + "::ToNumber unrealesed output final. Value " + ToStrForLog());

	return ret;
}

std::vector<bool> Value::ToVectBits() const
{
	std::vector<bool> ret;
	QString binCode;
	if(ValuesTypes::simpleNumeric.contains(type))
	{
		binCode = QSn(ToNumber(),2);
	}
	else if(type == ValuesTypes::binCode)
	{
		binCode = value;
	}
	else if(type == ValuesTypes::vectorBool)
	{
		auto tmpStr = GetStringListFromValueVector(value).join("");
		for(auto c:tmpStr) binCode = c + binCode;
	}
	else Error(GetClassName() + "::ToVectBits unrealesed type "+type + " " + ToStrForLog());

	QString wrongSymbols;
	for(int i=binCode.size()-1; i>=0; i--)
	{

		if(binCode[i] == '0') ret.push_back(0);
		else if(binCode[i] == '1') ret.push_back(1);
		else
		{
			ret.push_back(0);
			wrongSymbols += "'" + binCode[i] + "', ";
		}
	}
	if(!wrongSymbols.isEmpty())
	{
		wrongSymbols.chop(2);
		Error(GetClassName() + "::ToVectBits wrong symbols [" + wrongSymbols +"] found in bin code string [" + binCode +"]. Conversion from Value " +ToStrForLog());
	}

	return ret;
}

std::vector<Byte> Value::ToVectBytes(int size) const
{
	std::vector<Byte> bytes(size);
	if(type == ValuesTypes::uShort)
	{
		QString tmpVal = QSn(value.toInt(),2);
		while(tmpVal.length()<size*8) { tmpVal = '0' + tmpVal; }
		for(int i=0; i<size; i++)
		{
			QString byte = tmpVal.left(8);
			tmpVal.remove(0,8);
			bytes[i].SetByte(byte.toInt(nullptr,2));
		}
	}
	else if(ValuesTypes::simpleNumeric.contains(type))
	{
		QString tmpVal = QSn(value.toULongLong(),2);
		while(tmpVal.length()<size*8) { tmpVal = '0' + tmpVal; }
		for(int i=0; i<size; i++)
		{
			QString byte = tmpVal.left(8);
			tmpVal.remove(0,8);
			bytes[i].SetByte(byte.toInt(nullptr,2));
		}
	}
	else Error(GetClassName() + "::ToVectBytes: unrealesed type " + ToStrForLog());

	return bytes;
}

QString Value::GetTypeByStrValue(const QString &str)
{
	QString ret {ValuesTypes::unknown};
	if(Code::IsInteger(str))
	{
		bool ok;
		auto intVal { str.toLongLong(&ok) };
		if(!ok) intVal = str.toULongLong(&ok);
		if(ok)
		{
			// перебираем все диапазоны, если нашли - возвращаем тип
			for(auto &range:ValuesTypes::allRanges)
				if(intVal >= range.min && intVal <= range.max)
					return ret = range.types[0];
		}
	}

	if(ret == ValuesTypes::unknown)
		Logs::ErrorSt("GetTypeByStrValue: can't get type of " +str);

	return ret;
}

QStringList Value::GetStringListFromValueVector(const QString &str)
{
	QStringList ret;
	if(!str.isEmpty())
	{
		ret.push_back(QString());
		for(int i=0; i<str.size(); i++)
		{
			if(str[i] == ValuesTypes::vectorSplitter)
			{
				if(ret.last().startsWith(' ') || ret.last().endsWith(' '))
				{
					while(ret.last().startsWith(' ')) ret.last().remove(0,1);
					while(ret.last().endsWith(' ')) ret.last().chop(1);
				}

				if(str.size()-1) ret.push_back(QString());
				continue;
			}

			ret.last() += str[i];
		}
		if(str.endsWith(ValuesTypes::vectorSplitter)) ret.removeLast();

		if(ret.last().startsWith(' ') || ret.last().endsWith(' '))
		{
			while(ret.last().startsWith(' ')) ret.last().remove(0,1);
			while(ret.last().endsWith(' ')) ret.last().chop(1);
		}
	}
	return ret;
}

void Value::ToDataUniversal(CANMsg_t &msg, QString dataOperand) const
{
	if(dataOperand.left(CodeKeyWords::dataOperand.length()) == CodeKeyWords::dataOperand)
	{
		if(!DataOperand::CheckDataOperand(dataOperand))
		{
			Error("ToDataUniversal error CheckDataOperand ["+dataOperand+"]");
			return;
		}

		auto allIndexes { Code::GetAllIndexes(dataOperand) };
		if(allIndexes.indexes.size() > 2)
		{
			Logs::ErrorSt("ToDataUniversal: GetAllIndexes("+dataOperand+") size("+QSn(allIndexes.indexes.size())+") > 2");
			return;
		}
		AllIndexesOld indexes;
		if(allIndexes.indexes.size() >= 1) indexes.first = allIndexes.indexes[0];
		if(allIndexes.indexes.size() >= 2) indexes.secnd = allIndexes.indexes[1];

		// no indexes
		if(indexes.first.empty() && indexes.secnd.empty())
		{
			if(type == ValuesTypes::binCode)
			{
				EasyData ed(msg);
				ed.FromBinCode(value);
				ed.ToMsg(msg);
			}
			else if(type == ValuesTypes::vectorUByte)
			{
				auto vals { GetStringListFromValueVector(this->value) };
				int valsSize = vals.size();
				if(valsSize > 8) Error(GetClassName() + "::ToDataUniversal error 1 to long value " + ToStrForLog());
				msg.length = valsSize;
				for(int i=0; i<valsSize; i++) msg.data[i] = vals[i].toUInt();
			}
			else if(type == ValuesTypes::vectorBool)
			{
				EasyData ed;
				auto vectBits = ToVectBits();
				if(vectBits.size() <= EasyData::sizeBits)
					for(uint i=0; i<vectBits.size(); i++)
						ed.SetBitBi(i,vectBits[i]);
				ed.ToMsg(msg);
			}
			else Error(GetClassName() + "::ToDataUniversal error 2 unrealesed type " + type + " dataOperand ["+dataOperand+"]");
		}
		// data[...]
		else if(indexes.first.size() && indexes.secnd.empty())
		{
			auto convertValueRes = ConvertToType(ValuesTypes::vectorUByte);
			if(convertValueRes.second != "") Error(GetClassName() + "::ToDataUniversal error 3 ConvertToType error: " + convertValueRes.second
												   + " dataOperand ["+dataOperand+"]");
			else
			{
				auto vectBytesStrList = Value::GetStringListFromValueVector(convertValueRes.first.value);
				std::vector<Byte> vectBytes(vectBytesStrList.size());
				for(int i=0; i<vectBytesStrList.size(); i++)
				{
					if(!vectBytes[i].SetByte(vectBytesStrList[i].toLongLong()))
						Error(GetClassName() + "::ToDataUniversal: error 3.1; can't set byte " + ToStrForLog());
				}

				while(vectBytes.size() < indexes.first.size())
					vectBytes.insert(vectBytes.begin(),Byte());		// достраивание нулями

				if(vectBytes.size() == indexes.first.size())
				{
					EasyData ed(msg);
					for(uint i=0; i<vectBytes.size(); i++)
					{
						ed.SetByte(indexes.first[i], vectBytes[i]);
					}
					ed.ToMsg(msg);
				}
				else Error(GetClassName() + "::ToDataUniversal: error 4; wrong size value: " + ToStrForLog() + " for dataOperand ["+dataOperand+"]");
			}
		}
		// data[...][...]
		else if(indexes.first.size() == 1 && indexes.secnd.size())
		{
			EasyData ed(msg);
			auto valueVectBits { ToVectBits() };
			while(valueVectBits.size() < indexes.secnd.size()) valueVectBits.push_back(0);

			if(valueVectBits.size() == indexes.secnd.size())
			{
				for(uint i=0; i<valueVectBits.size(); i++)
					ed.SetBitBiBi(indexes.first[0],indexes.secnd[i],valueVectBits[i]);
				ed.ToMsg(msg);
			}
			else Error(GetClassName() + "::ToDataUniversal error 7 wrong bits count in value " + ToStrForLog() + " for dataOperand ["+dataOperand+"]");
		}
		// data[][...]
		else if(indexes.first.empty() && indexes.secnd.size())
		{
			auto thisValueVectBool = ToVectBits();
			EasyData ed(msg);
			for(uint i=0; i<indexes.secnd.size(); i++)
			{
				bool tmpBool = false;
				if(thisValueVectBool.size())
				{
					tmpBool = thisValueVectBool[0];
					thisValueVectBool.erase(thisValueVectBool.begin());
				}
				ed.SetBitBi(indexes.secnd[i], tmpBool);
			}
			ed.ToMsg(msg);
		}
		else Error(GetClassName() + "::ToDataUniversal error 8 unrealesed indexes dataOperand ["+dataOperand+"]");
	}
	else
		Error(GetClassName() + "::ToDataUniversal error 9 wrong dataOperand ["+dataOperand+"]");
}

Value& Value::FromDataUniversal(const CANMsg_t &msg, QString dataOperand)
{
	if(!DataOperand::CheckDataOperand(dataOperand))
	{
		Error("ToDataUniversal error CheckDataOperand ["+dataOperand+"]");
		return *this;
	}

	auto allIndexes { Code::GetAllIndexes(dataOperand) };
	if(allIndexes.indexes.size() > 2)
	{
		Logs::ErrorSt("FromDataUniversal: GetAllIndexes("+dataOperand+") size("+QSn(allIndexes.indexes.size())+") > 2");
		return *this;
	}
	AllIndexesOld indexes;
	if(allIndexes.indexes.size() >= 1) indexes.first = allIndexes.indexes[0];
	if(allIndexes.indexes.size() >= 2) indexes.secnd = allIndexes.indexes[1];

	// no indexes
	if(indexes.first.empty() && indexes.secnd.empty())
	{
		QString newValue="";
		EasyData ed(msg);
		for(int byte_i=0; byte_i<msg.length; byte_i++)
		{
			for(int bit_i=0; bit_i<8; bit_i++)
				newValue += QSn(ed.GetBit(byte_i,bit_i)) + ValuesTypes::vectorSplitter;
		}
		Set(newValue, ValuesTypes::vectorBool);
	}
	// data[x]	data[a,b,c...]	data[x-y]
	else if(indexes.first.size() && indexes.secnd.empty())
	{
		QString vals;
		for(auto i:indexes.first)
			vals += QSn(msg.data[i]) + ValuesTypes::vectorSplitter;

		Set(vals, ValuesTypes::vectorUByte);
	}
	// data[x][n-m]
	else if(indexes.first.size() == 1 && indexes.secnd.size())
	{
		if(ValuesTypes::simpleNumeric.contains(type))
		{
			QString chisloFromData;
			Byte byte;
			if(!byte.SetByte(msg.data[indexes.first[0]])) Error("FromDataUniversal 2: Error set Byte " + QSn(msg.data[indexes.first[0]]));

			for(uint i=0; i<indexes.secnd.size(); i++)
			{
				if(byte.data[indexes.secnd[i]]) chisloFromData = '1' + chisloFromData;
				else chisloFromData = '0' + chisloFromData;
			}

			bool ok;
			chisloFromData = QSn(chisloFromData.toUShort(&ok,2));
			if(!ok)  Error(GetClassName() + "::FromDataUniversal 3 impossible error 1");

			Set(chisloFromData, ValuesTypes::sLLong);
		}
		else if(type == ValuesTypes::binCode)
		{
			QString binCode;
			EasyData ed(msg);
			for(uint i=0; i<indexes.secnd.size(); i++)
				binCode = QSn(ed.GetBit(indexes.first[0],indexes.secnd[i])) + binCode;
			Set(binCode, ValuesTypes::binCode);
		}
		else Error(GetClassName() + "::FromDataUniversal 4 unrealesed type " + type +" and dataOperand ["+dataOperand+"]");
	}
	// data[][x]	data[][a,b,c...]	data[][x-y]
	else if(indexes.first.empty() && indexes.secnd.size())
	{
		if(type == ValuesTypes::text)
		{
			Error(GetClassName() + "::FromDataUniversal 5.2 conversion from dataOperand [" + dataOperand + "] to type " + type + " forbidden");
		}
		else if(type == ValuesTypes::vectorBool)
		{
			QString newValue="";
			EasyData ed(msg);
			for(uint i=0; i<indexes.secnd.size(); i++)
			{
				int index = indexes.secnd[i];
				if(index < EasyData::sizeBits)
					newValue += QSn(ed.Data()[index]) + ValuesTypes::vectorSplitter;
				else Error(GetClassName() + "::FromDataUniversal 5.1 to big index ("+QSn(index)+") in dataOperand ["+dataOperand+"]");
			}
			Set(newValue, ValuesTypes::vectorBool);
		}
		else Error(GetClassName() + "::FromDataUniversal 5 unrealesed type " + type +" and dataOperand ["+dataOperand+"]");
	}
	else Error(GetClassName() + "::FromDataUniversal 6 unrealesed dataOperand ["+dataOperand+"]");

	if(cbValueChanged) cbValueChanged(*this,cbValueChangedHandler);

	return *this;
}

void EasyData::SetBitBi(int bitIndex, bool val)
{
	if(bitIndex > sizeBits)
	{
		Logs::ErrorSt("SetBit bitIndex ["+ QSn(sizeBits) +"] bigger size");
		return;
	}
	int newLen = bitIndex/8 + 1;
	if(newLen > len) len = newLen;
	data[bitIndex] = val;
}

void EasyData::SetBitBiBi(int byteIndex, int bitInByteIndex, bool val)
{
	int bitIndex = byteIndex*8 + bitInByteIndex;
	SetBitBi(bitIndex, val);
}

void EasyData::SetByte(int byteIndex, Byte byte)
{
	if(len < byteIndex+1) len = byteIndex+1;

	for(int i=0; i<8; i++)
	{
		data[byteIndex*8+i] = byte.data[i];
	}
}

void EasyData::Clear()
{
	len=0;
	for(int i=0; i<sizeBits; i++)
		data[i] = 0;
}

bool EasyData::GetBit(int byteIndex, int bitInByteIndex)
{
	int bitInd = byteIndex*8 + bitInByteIndex;
	if(bitInd < sizeBits) return data[byteIndex*8 + bitInByteIndex];
	else
	{
		Logs::ErrorSt("EasyData::GetBit wrong index byteIndex = " + QSn(byteIndex) + " or bitInByteIndex = " + QSn(bitInByteIndex));
		return 0;
	}
}

void EasyData::FromBinCode(QString strBinCode)
{
	Logs::ErrorSt("FromBinCode unrealesed. Commented realisation contains errors." + strBinCode);
	//if(strBinCode.length() > size) Error("FromBinCode to long strBinCode " + strBinCode);
	//int newLen = strBinCode.length()/8;
	//if(newLen > len) len = newLen;
	//int edIndex = 0;
	//for(int i=strBinCode.length()-1; i>=0; i--)
	//{
	//	if(strBinCode[i]=='0') this->SetBitBi(edIndex,0);
	//	else if(strBinCode[i]=='1') this->SetBitBi(edIndex,1);
	//	else {Error("FromBinCode wrong symbol in strBinCode " + strBinCode); this->SetBitBi(edIndex,0); }
	//	edIndex++;
	//}
}

void EasyData::FromMsg(const CANMsg_t &msg)
{
	len = msg.length;
	if(len > 8) { Logs::ErrorSt("FromMsg len > "+QSn(8)); return; }
	int eDataIndex = 0;
	for(int i=0; i<len; i++)
	{
		std::bitset<8> dataBitset {msg.data[i]};
		for(int bit=0; bit<8; bit++)
		{
			if(eDataIndex >= sizeBits) { Logs::ErrorSt("FromMsg eDataIndex >= " + QSn(sizeBits)); return; }
			data[eDataIndex] = dataBitset[bit];
			eDataIndex++;
		}
	}
}

void EasyData::ToMsg(CANMsg_t &msg)
{
	msg.length = len;
	if(len > 8) { Logs::ErrorSt("ToMsg len > " + QSn(8)); return; }
	int eDataIndex = 0;
	for(int i=0; i<len; i++)
	{
		std::bitset<8> dataBitset;
		for(int bit=0; bit<8; bit++)
		{
			if(eDataIndex >= sizeBits) { Logs::ErrorSt("ToMsg eDataIndex >= " + QSn(sizeBits)); return; }
			dataBitset[bit] = data[eDataIndex];
			eDataIndex++;
		}
		msg.data[i] = dataBitset.to_ulong();
	}
}

QString EasyData::ToStr()
{
	QString ret;
	for(int i=0; i<sizeBits; i++)
	{
		ret += QSn(data[i]);
		if((i+1)%8==0) ret += " ";
	}
	return ret;
}

bool Byte::SetByte(long long value)
{
	bool ret = true;
	if(value > ValuesTypes::rangeByte.max || value < ValuesTypes::rangeByte.min)
	{
		value = 0;
		ret=false;
	}
	data = value;
	return ret;
}

DataOperand::DataOperand(CANMsg_t & frame_, QString strDataOperand_):
	frame {frame_}
{
	if(CheckDataOperand(strDataOperand_))
		strDataOperand = strDataOperand_;
	else
	{
		Logs::ErrorSt(QSn(frame_.length));
		Logs::ErrorSt(GetClassName() + "::DataOperand(...) wrong strDataOperand_ ["+strDataOperand_+"]");
	}
}

void DataOperand::SetDataFilterMaskFromOperand(QString strDataOperand, std::array<u8, 8> &mask)
{
	auto allIndexes { Code::GetAllIndexes(strDataOperand) };
	if(allIndexes.indexes.size() > 2)
	{
		Logs::ErrorSt("ToDataUniversal: GetAllIndexes("+strDataOperand+") size("+QSn(allIndexes.indexes.size())+") > 2");
		return;
	}
	AllIndexesOld indexes;
	if(allIndexes.indexes.size() >= 1) indexes.first = allIndexes.indexes[0];
	if(allIndexes.indexes.size() >= 2) indexes.secnd = allIndexes.indexes[1];

	if(indexes.first.size() && indexes.secnd.empty())
	{
		for(uint i=0; i<indexes.first.size(); i++)
		{
			if(indexes.first[i] < 8) mask[indexes.first[i]] = 0b11111111;
			else Logs::ErrorSt("DataOperand::SetDataFilterMaskFromOperand: err 1: to big index " + QSn(indexes.first[i]) + " in operand " + strDataOperand);
		}
	}
	else if(indexes.first.size() == 1 && indexes.secnd.size())
	{
		std::bitset<8> byte = 0;
		for(uint i=0; i<indexes.secnd.size(); i++)
		{
			if(indexes.secnd[i] < 8) byte[indexes.secnd[i]] = 1;
			else Logs::ErrorSt("DataOperand::SetDataFilterMaskFromOperand: err 2: to big index " + QSn(indexes.secnd[i]) + " in operand " + strDataOperand);
		}

		if(indexes.first[0] < 8) mask[indexes.first[0]] = byte.to_ulong();
		else Logs::ErrorSt("SetDataFilterMaskFromOperand: err 3: to big index " + QSn(indexes.first[0]) + " in operand " + strDataOperand);
	}
	else Logs::ErrorSt("DataOperand::SetDataFilterMaskFromOperand: unrealesed indexes [" + strDataOperand + "]");
}



bool DataOperand::CheckDataOperand(const QString & strDataOperand)
{
	auto allIndexes { Code::GetAllIndexes(strDataOperand) };
	if(allIndexes.indexes.size() > 2)
	{
		Logs::ErrorSt("ToDataUniversal: GetAllIndexes("+strDataOperand+") size("+QSn(allIndexes.indexes.size())+") > 2");
		return false;
	}
	AllIndexesOld indexes;
	if(allIndexes.indexes.size() >= 1) indexes.first = allIndexes.indexes[0];
	if(allIndexes.indexes.size() >= 2) indexes.secnd = allIndexes.indexes[1];

	if(indexes.first.empty() && indexes.secnd.empty()) return true;

	if(!indexes.first.empty())  // проверка индекса байта
	{
		for(auto index:indexes.secnd)
			if(index < 0 || index >= EasyData::sizeBits)
				return false;
	}

	if(!indexes.secnd.empty()) // проверка индекса бита
	{
		for(auto index:indexes.first)
			if(index < 0 || index >= EasyData::sizeBytes)
				return false;
	}

	return true;
}

void DataOperand::Assign(const IOperand *operand2)
{
	auto vt {operand2->IOGetValueAndType()};
	Value tmpVal(vt.first, vt.second);
	tmpVal.ToDataUniversal(frame,strDataOperand);
}

std::shared_ptr<IOperand> DataOperand::Minus(const IOperand * operand2)
{
	Error(GetClassName() + "::PlusAssign DataOperand " + operand2->IOGetType());
	return nullptr;
}

void DataOperand::PlusAssign(const IOperand *operand2)
{
	Error(GetClassName() + "::PlusAssign DataOperand " + operand2->IOGetType());
}

bool DataOperand::CmpOperation(const IOperand *operand2, QString operationCode) const
{
	QString type;
	if(operand2->GetClassName() == IOperand::classDataOperand()) // if compare DataOperand and DataOperand
		type = ValuesTypes::vectorBool;
	else type = operand2->IOGetType();

	QString thisVal = this->IOGetValue(type);
	Value op2Val(operand2->IOGetValue(type), type);

	return (Value(thisVal,type).CmpOperation(&op2Val, operationCode));
}

std::shared_ptr<IOperand> DataOperand::Index(int index)
{
	Error(CodeMarkers::mock + GetClassName() + "::Index; index = " + QSn(index));
	return nullptr;
}

IOperand * DataOperand::PointAccess(QString nameToAccess)
{
	Error(GetClassName()+"::PointAccess " + nameToAccess + " " + CodeMarkers::mock);
	return nullptr;
}

QStringPair DataOperand::IOGetValueAndType() const
{
	Error("try IOGetValueAndType() from DataOperand! DataOperand can only IOGetValue(outputType)");
	return {ValuesTypes::undefined, ValuesTypes::undefined};
}

QString DataOperand::IOGetValue(QString outputType) const
{
	Value ret("", outputType);
	ret.FromDataUniversal(frame,strDataOperand);
	return ret.IOGetValueAndType().first;
}

QStringPair SubValue::GetSubValue() const
{
	Value ret;
	auto elementsList = Value::GetStringListFromValueVector(value.GetValue());
	if((int)index < elementsList.size())
		ret.Set(elementsList[index],elementType);
	else Error("SubValue:GetSubValue: value " + value.ToStrForLog() + " has no element " + QSn(index));
	return ret.GetValueAndType();
}

void SubValue::SetSubValue(QString newElementValue)
{
	auto elementsList = Value::GetStringListFromValueVector(value.GetValue());
	if((int)index < elementsList.size())
	{
		elementsList[index] = newElementValue;
		value.Set(elementsList.join(ValuesTypes::vectorSplitter),value.GetType());
	}
	else Error("SubValue:GetSubValue: value " + value.ToStrForLog() + " has no element " + QSn(index));
}

SubValue::SubValue(Value & valueSrc, uint index_): value{valueSrc}, index {index_}, elementType {""}
{
	for(auto &pair:ValuesTypes::VectorsElementsTypes)
		if(pair.first == value.GetType()) elementType = pair.second;

	if(elementType == "")
		Error("SubValue creation: can't define element type of " + valueSrc.GetType());
}

std::shared_ptr<IOperand> SubValue::Minus(const IOperand * operand2)
{
	Value subValue(GetSubValue());
	auto ret = subValue.Minus(operand2);
	if(!ret) Error(GetClassName()+"::Minus subValue.Minus(operand2) returned nullptr (tryed "+ToStrForLog()+"->Minus("+operand2->ToStrForLog()+")");
	return ret;
}

bool SubValue::CmpOperation(const IOperand * operand2, QString operationCode) const
{
	Error(GetClassName()+"::CmpOperation "+ operand2->IOGetType() + " " + CodeMarkers::mock + " " + operationCode); return {};
}

std::shared_ptr<IOperand> SubValue::Index(int index)
{
	Error("SubValue::Index Can't take Index from SubValue " + QSn(index));
	return nullptr;
}

IOperand * SubValue::PointAccess(QString nameToAccess)
{
	Error(GetClassName()+"::PointAccess " + nameToAccess + " " + CodeMarkers::mock);
	return nullptr;
}

QString SubValue::IOGetValue(QString outputType) const
{
	auto tmpPair = GetSubValue();
	Value tmpVal(tmpPair.first,tmpPair.second);
	auto convertRes = tmpVal.ConvertToType(outputType);
	if(convertRes.second != "") Error(GetClassName() + "::IOGetValue ConvertToType error: " + convertRes.second);
	return convertRes.first.GetValue();
}
