#include <cmath>
#include <type_traits>

#include "value.h"

bool EncondingValue::Set(QString v_, QString enc_, uint bitLenth_)
{
	if(Encodings::all.contains(enc_))
	{
		v = v_;
		e = enc_;
		bitLenth = bitLenth_;
		return true;
	}
	else Error("Set wrong encoding" + enc_);
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

void Value::PlusAssign(const IOperand *operand2)
{
	if(ValuesTypes::simpleNumeric.contains(type))
	{
		Error("PlusAssign: unrealesed ValuesTypes::simpleNumeric.contains(type)");
	}
	else if(ValuesTypes::vectorsNumeric.contains(type) && ValuesTypes::simpleNumeric.contains(operand2->IOGetType()))
	{
		auto addingVal = operand2->IOGetValueAndType();
		auto valsList = Value::GetStringListFromValueVector(value);
		valsList += addingVal.first;
		Set(valsList.join(ValuesTypes::vectorSplitter),type);
	}
	else Error("PlusAssign unrealesed PlusAssign. Tried to PlusAssign " + type + " to " + operand2->IOGetType());
}

bool Value::IsEqual(const IOperand *operand2) const
{
	Value valueOp2(operand2->IOGetValue(type),type);
	if(value == valueOp2.value) return true;
	else return false;
}

std::shared_ptr<IOperand> Value::Index(int index)
{
	return std::make_shared<SubValue>(*this,index);
}

QStringPair Value::IOGetValueAndType() const
{
	return {value, type };
}

QString Value::IOGetValue(QString outputType) const
{
	auto convertRes = ConvertToType(outputType);
	if(convertRes.second != "") Error("Value::IOGetValue ConvertToType error: " + convertRes.second);
	return convertRes.first.GetValue();
}

QString Value::CheckCorrectionAndRange(const Value *value_)
{
	QString ret("");
	if(value_->type == ValuesTypes::binCode)
	{
		ret = "";
		for(auto c:value_->value)
			if(!(c=='0' || c=='1'))
			{
				ret = "Wrong bincode: " + value_->ToStrForLog();
				break;
			}
	}
	else if(ValuesTypes::simpleNumeric.contains(value_->type))
	{
		if(value_->value == "") { /* ret(""); - по-умолчанию */ }
		else
		{
			bool ok;
			long long check = value_->value.toLongLong(&ok);
			if(!ok) ret = "RangeCheck: error toLongLong conversion " + value_->ToStrForLog();
			ValuesTypes::Range curRange = value_->GetRange();
			if(check >= curRange.min && check <= curRange.max) { /* ret(true, ""); - по-умолчанию */ }
			else ret = "RangeCheck: error range " + value_->ToStrForLog();
		}
	}
	else if(ValuesTypes::vectorsNumeric.contains(value_->type))
	{
		if(value_->value == "") { /* ret(""); - по-умолчанию */ }
		else
		{
			ValuesTypes::Range curRange = value_->GetRange();
			ret = CheckVectorCorrectionAndRange(value_, curRange);
		}
	}
	else if(value_->type == ValuesTypes::text) { /* ret(""); - по-умолчанию */ }
	else if(value_->type == ValuesTypes::any) { /* ret(""); - по-умолчанию */ }
	else
	{
		ret = "RangeCheck: unrealesed type " + value_->ToStrForLog();
	}

	return ret;
}

QString Value::CheckVectorCorrectionAndRange(const Value *value_, const ValuesTypes::Range &range)
{
	QString result("");
	auto vals { Value::GetStringListFromValueVector(value_->value) };
	for(auto &v:vals)
	{
		bool ok;
		auto tmpVal {v.toLongLong(&ok)};
		if(!ok) result = "RangeCheck: error toLongLong conversion " + value_->ToStrForLog();
		if (!(tmpVal>=range.min && tmpVal<=range.max)) result = "RangeCheckVector error " + value_->ToStrForLog();
	}
	return result;
}

void Value::Init(QString name_, QString value_, QString type_)
{
	if(value != ValuesTypes::undefined) Error("Init value not undefined " + name + " " + value + " " + type);
	name=name_, value=value_, type=type_;

	if(CheckCorrectionAndRange() != "")
	{
		Error("Value::Init error. Range check error " + ToStrForLog());
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

ValuesTypes::Range Value::GetRange() const
{
	ValuesTypes::Range ret;
	for(auto &range:ValuesTypes::allRanges)
		if(range.types.contains(type)) ret = range;
	if(!ret.min && !ret.max) Error("GetRange: can't get range " + type);
	return ret;
}

void Value::Set(const Value &newValue)
{
	auto new_vt {newValue.IOGetValueAndType()};
	Set(new_vt.first, new_vt.second);
}

void Value::Set(QString newValue, QString type_)
{
	value = "";
	if(type_ == ValuesTypes::undefined || type_ == ValuesTypes::unknown)
		Error("Set: wrong setting type ("+type_+")");
	else if(type == type_) value = newValue;
	else if(type == ValuesTypes::undefined) { value = newValue; type = type_; }
	else
	{
		auto convertRes = Value(newValue,type_).ConvertToType(type);
		if(convertRes.second != "") Error("Value::Set error ConvertToType ConvertToType error: " + convertRes.second);
		else value = convertRes.first.GetValue();
	}

	if(cbValueChanged) cbValueChanged(*this,cbValueChangedHandler);
}

std::pair<Value,QString> Value::ConvertToType(QString newType) const
{
	Value retValue("",newType);
	QString retErrorStr;

	if(newType == type || newType == ValuesTypes::any)
		retValue = *this;
	else if(ValuesTypes::simpleNumeric.contains(type) && ValuesTypes::simpleNumeric.contains(newType))
		retValue.value = value;
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
		else retErrorStr = "code 0 (tryed convert " + ToStrForLog() + " to " + newType + ")";
	}
	else if(newType == ValuesTypes::text)
	{
		retValue.value = value;
	}
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
	else if(ValuesTypes::simpleNumeric.contains(type))
	{
		if(ValuesTypes::simpleNumeric.contains(newType))
		{
			retValue.value = value;
		}
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
	else if(ValuesTypes::vectorsNumeric.contains(type))
	{
		if(ValuesTypes::simpleNumeric.contains(newType))
		{
			auto valsList = Value::GetStringListFromValueVector(value);
			if(valsList.size() == 1) retValue.value = valsList[0];
			else if(type == ValuesTypes::vectorBool ||
					type == ValuesTypes::vectorUByte)
			{
				retValue.value = QSn(ToNumber());
			}
			else retErrorStr = "code 3.1 (tryed convert " + ToStrForLog() + " to " + newType + ")";
		}
		else retErrorStr = "code 3 (tryed convert " + ToStrForLog() + " to " + newType + ")";
	}
	else retErrorStr = "code 4 (tryed convert " + ToStrForLog() + " to " + newType + ")";

	if(retErrorStr == "")
	{
		if(retValue.CheckCorrectionAndRange() != "")
		retErrorStr = "RangeCheck error " + retValue.CheckCorrectionAndRange() + " (tryed convert " + ToStrForLog() + " to " + newType + ")";
	}

	return {retValue,retErrorStr};
}

QString Value::ToStr() const
{
	QString ret = ValuesTypes::undefined;

	if(ValuesTypes::simpleNumeric.contains(type)) ret = value;
	else if(ValuesTypes::vectorsNumeric.contains(type)) ret = value;
	else if(type == ValuesTypes::text) ret = value;
	else Error("ToStr unrealesed type " + name + " " + value + " " + type);

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
		if(!ok) Error("ToNumber error conversion 1 Value " + ToStrForLog());
	}
	else if(type == ValuesTypes::binCode)
	{
		bool ok = false;
		ret = value.toLongLong(&ok,2);
		if(!ok) Error("ToNumber error conversion 2 Value " + ToStrForLog());
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
			if(!ok) Error("ToNumber error conversion 1.1.1 Value " + ToStrForLog());
		}
		else if(valsListSize > 1)
		{
			if(type == ValuesTypes::vectorBool)
			{
				bool ok;
				ret = valsList.join("").toLongLong(&ok,2);
				if(!ok) Error("ToNumber error conversion 1.1 Value " + ToStrForLog());
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
				if(!ok) Error("ToNumber error conversion 1.1.1 Value " + ToStrForLog());
			}
			else Error("ToNumber wrong type output 1.2 Value " + ToStrForLog());
		}
		else Error("ToNumber impossible vsize "+QSn(valsListSize)+" Value " + ToStrForLog());
	}
	else Error("ToNumber unrealesed output final. Value " + ToStrForLog());

	return ret;
}

std::vector<qint64> Value::ToVectLLong() const
{
	if(ValuesTypes::simpleNumeric.contains(type)) return {value.toLongLong()};
	if(ValuesTypes::vectorsNumeric.contains(type))
	{
		std::vector<qint64> ret;
		auto valList {Value::GetStringListFromValueVector(value)};
		ret.resize(valList.size());
		for(int i=0; i<valList.size(); i++) ret[i] = valList[i].toULongLong();
		return ret;
	}
	Error("Value::ToVectInt unrealesed type" + name + " " + value + " " + type);
	return {};
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
		binCode = value;
		binCode.remove(ValuesTypes::vectorSplitter);
	}
	else Error("ToVectBits unrealesed type "+type + " " + ToStrForLog());

	for(int i=binCode.size()-1; i>=0; i--)
	{
		if(binCode[i] == '0') ret.push_back(0);
		else if(binCode[i] == '1') ret.push_back(1);
		else Error("ToVectBits wrong bin code [" + binCode +"] from Value " +ToStrForLog());
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
	else Error("ToVectBytes: unrealesed type " + ToStrForLog());

	return bytes;
}

QString Value::GetTypeByStrValue(const QString &value)
{
	QString ret {ValuesTypes::unknown};
	if(Code::IsInteger(value))
	{
		bool ok;
		auto intVal { value.toLongLong(&ok) };
		if(!ok) intVal = value.toULongLong(&ok);
		if(ok)
		{
			// перебираем все диапазоны, если нашли - возвращаем тип
			for(auto &range:ValuesTypes::allRanges)
				if(intVal >= range.min && intVal <= range.max) ret = range.types[0];
		}
	}

	if(ret == ValuesTypes::unknown)
		Logs::ErrorSt("GetTypeByStrValue: can't get type of " +value);

	return ret;
}

QStringList Value::GetStringListFromValueVector(const QString &value)
{
	return value.split(ValuesTypes::vectorSplitter,QString::SkipEmptyParts);
}

void Value::ToDataUniversal(CANMsg_t &msg, QString dataOperand) const
{
	if(dataOperand.left(CommandsKeyWords::dataOperand.length()) == CommandsKeyWords::dataOperand)
	{
		auto indexes { Code::GetAllIndexes(dataOperand) };

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
				auto vals { ToVectLLong() };
				int valsSize = vals.size();
				if(valsSize > 8) Error("ToDataUniversal error 1 to long value " + ToStrForLog());
				msg.length = valsSize;
				for(int i=0; i<valsSize; i++) msg.data[i] = vals[i];
			}
			else if(type == ValuesTypes::vectorBool)
			{
				EasyData ed;
				ed.FromVect<qint64>(ToVectLLong());
				ed.ToMsg(msg);
			}
			else Error("ToDataUniversal error 2 unrealesed type " + type + " dataOperand ["+dataOperand+"]");
		}
		// data[x]	data[a,b,c...]	data[x-y]
		else if(indexes.first.size() && indexes.secnd.empty())
		{
			auto convertValueRes = ConvertToType(ValuesTypes::vectorUByte);
			if(convertValueRes.second != "") Error("Value::ToDataUniversal error 3 ConvertToType error: " + convertValueRes.second
												   + " dataOperand ["+dataOperand+"]");
			else
			{
				auto vectBytesStrList = Value::GetStringListFromValueVector(convertValueRes.first.value);
				std::vector<Byte> vectBytes(vectBytesStrList.size());
				for(int i=0; i<vectBytesStrList.size(); i++)
				{
					if(!vectBytes[i].SetByte(vectBytesStrList[i].toLongLong()))
						Error("ToDataUniversal: error 3.1; can't set byte " + ToStrForLog());
				}

				while(vectBytes.size() < indexes.first.size())
					vectBytes.insert(vectBytes.begin(),Byte());

				if(vectBytes.size() == indexes.first.size())
				{
					EasyData ed(msg);
					for(uint i=0; i<vectBytes.size(); i++)
					{
						ed.SetByte(indexes.first[i], vectBytes[i]);
					}
					ed.ToMsg(msg);
				}
				else Error("ToDataUniversal: error 4; wrong size value: " + ToStrForLog() + " for dataOperand ["+dataOperand+"]");
			}
		}
		// data[x][n-m]
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
			else Error("ToDataUniversal error 7 wrong bits count in value " + ToStrForLog() + " for dataOperand ["+dataOperand+"]");
		}
		// data[][x]	data[][a,b,c...]	data[][x-y]
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
		else Error("ToDataUniversal error 8 unrealesed indexes dataOperand ["+dataOperand+"]");
	}
	else
		Error("ToDataUniversal error 9 wrong dataOperand ["+dataOperand+"]");
}

Value& Value::FromDataUniversal(const CANMsg_t &msg, QString dataOperand)
{
	auto indexes {Code::GetAllIndexes(dataOperand)};

	// no indexes
	if(indexes.first.empty() && indexes.secnd.empty())
	{
		Error("FromDataUniversal 1 no index dataOperand ["+dataOperand+"] is not allowed in predicate");
	}
	// data[x]	data[a,b,c...]	data[x-y]
	else if(indexes.first.size() && indexes.secnd.empty())
	{
		QString vals;
		for(auto i:indexes.first) vals += QSn(msg.data[i]) + ValuesTypes::vectorSplitter;

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
			if(!ok)  Error("FromDataUniversal 3 impossible error 1");

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
		else Error("FromDataUniversal 4 unrealesed type " + type +" and dataOperand ["+dataOperand+"]");
	}
	// data[][x]	data[][a,b,c...]	data[][x-y]
	else if(indexes.first.empty() && indexes.secnd.size())
	{
		if(type == ValuesTypes::text)
		{
			Error("FromDataUniversal 5.2 conversion from dataOperand [" + dataOperand + "] to type " + type + " forbidden");
		}
		else if(type == ValuesTypes::vectorBool)
		{
			QString newValue="";
			EasyData ed(msg);
			for(uint i=0; i<indexes.secnd.size(); i++)
			{
				int index = indexes.secnd[i];
				if(index < EasyData::size)
					newValue += QSn(ed.Data()[index]) + ValuesTypes::vectorSplitter;
				else Error("FromDataUniversal 5.1 to big index ("+QSn(index)+") in dataOperand ["+dataOperand+"]");
			}
			Set(newValue, ValuesTypes::vectorBool);
		}
		else Error("FromDataUniversal 5 unrealesed type " + type +" and dataOperand ["+dataOperand+"]");
	}
	else Error("FromDataUniversal 6 unrealesed dataOperand ["+dataOperand+"]");

	if(cbValueChanged) cbValueChanged(*this,cbValueChangedHandler);

	return *this;
}

void EasyData::SetBitBi(int bitIndex, bool val)
{
	if(bitIndex > size)
	{
		Logs::ErrorSt("SetBit bitIndex ["+ QSn(size) +"] bigger size");
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
	for(int i=0; i<size; i++)
		data[i] = 0;
}

bool EasyData::GetBit(int byteIndex, int bitInByteIndex)
{
	int bitInd = byteIndex*8 + bitInByteIndex;
	if(bitInd < size) return data[byteIndex*8 + bitInByteIndex];
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
			if(eDataIndex >= size) { Logs::ErrorSt("FromMsg eDataIndex >= " + QSn(size)); return; }
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
			if(eDataIndex >= size) { Logs::ErrorSt("ToMsg eDataIndex >= " + QSn(size)); return; }
			dataBitset[bit] = data[eDataIndex];
			eDataIndex++;
		}
		msg.data[i] = dataBitset.to_ulong();
	}
}

QString EasyData::ToStr()
{
	QString ret;
	for(int i=0; i<size; i++)
	{
		ret += QSn(data[i]);
		if((i+1)%8==0) ret += " ";
	}
	return ret;
}

template<typename T>
void EasyData::FromVect(const std::vector<T> &vectVals)
{
	static_assert(std::is_same<T,bool>::value || std::is_same<T,int>::value || std::is_same<T,qint64>::value, "EasyData::FromVect wrong type (not int or bool)");

	len = vectVals.size()/8 + 1;
	if(len > 8 || vectVals.size() > size) Logs::ErrorSt("FromVect: to big size value" + QSn(vectVals.size()));
	for(uint i=0; i<vectVals.size(); i++)
	{
		if(vectVals[i] > 1 || vectVals[i] < 0) {
			data[i] = 0;
			Logs::ErrorSt("FromVect: to big value " + QSn(vectVals[i]));
		}
		else data[i] = vectVals[i];
	}
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

void DataOperand::SetDataFilterMaskFromOperand(QString strDataOperand, CANMsgDataFilter_t &filter)
{
	auto indexes = Code::GetAllIndexes(strDataOperand);
	if(indexes.first.size() && indexes.secnd.empty())
	{
		for(uint i=0; i<indexes.first.size(); i++)
		{
			if(indexes.first[i] < 8) filter.mask[indexes.first[i]] = 0b11111111;
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

		if(indexes.first[0] < 8) filter.mask[indexes.first[0]] = byte.to_ulong();
		else Logs::ErrorSt("SetDataFilterMaskFromOperand: err 3: to big index " + QSn(indexes.first[0]) + " in operand " + strDataOperand);
	}
	else Logs::ErrorSt("DataOperand::SetDataFilterMaskFromOperand: unrealesed indexes [" + strDataOperand + "]");
}

void DataOperand::Assign(const IOperand *operand2)
{
	auto vt {operand2->IOGetValueAndType()};
	Value tmpVal(vt.first, vt.second);
	tmpVal.ToDataUniversal(*frame,strDataOperand);
	//Log("DataOperand Assign " + strDataOperand + " = " + IOperand::PairJoin(operand2GetVal) + " res: " +frame->ToStrEx2());
}

void DataOperand::PlusAssign(const IOperand *operand2)
{
	Error("PlusAssign DataOperand " + operand2->IOGetType());
}

bool DataOperand::IsEqual(const IOperand *operand2) const
{
	bool retVal = false;
	QString type;
	if(operand2->GetClassName() == IOperand::classDataOperand()) // if compare DataOperand and DataOperand
		type = ValuesTypes::vectorBool;
	else type = operand2->IOGetType();

	QString thisVal = this->IOGetValue(type);
	QString op2Val = operand2->IOGetValue(type);
	retVal = (thisVal == op2Val);

	return retVal;
}

QStringPair DataOperand::IOGetValueAndType() const
{
	Error("try IOGetValueAndType() from DataOperand! DataOperand can only IOGetValue(outputType)");
	return {ValuesTypes::undefined, ValuesTypes::undefined};
}

QString DataOperand::IOGetValue(QString outputType) const
{
	Value ret("", outputType);
	ret.FromDataUniversal(*frame,strDataOperand);
	return ret.IOGetValueAndType().first;
}
