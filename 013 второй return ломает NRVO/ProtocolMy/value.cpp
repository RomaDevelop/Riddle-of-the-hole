#include "value.h"

#include <cmath>
#include <type_traits>

bool EncondingValue::Set(QString v_, QString enc_)
{
	if(Encodings::all.contains(enc_))
	{
		v = v_;
		e = enc_;
		return true;
	}
	else qdbg << "EncondingValue::Set wrong encoding" << enc_;
	return false;
}

int EncondingValue::ToInt() const
{
	if(e == Encodings::bin) return v.toInt(nullptr, 2);
	if(e == Encodings::dec) return v.toInt(nullptr, 10);
	if(e == Encodings::text) { qdbg << "can't EncondingValue::ToInt text value"; return 0; }
	qdbg << "EncondingValue::ToInt unrealesed encoding";
	return 0;
}

QString EncondingValue::ToDecStr() const
{
	if(Encodings::all.contains(e))
	{
		if(e == Encodings::bin) return QSn(v.toInt(nullptr, 2)) ;
		if(e == Encodings::dec) return v;
		if(e == Encodings::text) return "Encodings::text";
	}
	else qdbg << "EncondingValue::ToDecStr2 wrong encoding" << e;
	return "Encodings::unknown";
}
QString EncondingValue::ToBinStr() const
{
	if(Encodings::all.contains(e))
	{
		if(e == Encodings::bin) return v;
		if(e == Encodings::dec) return QSn(v.toInt(nullptr),2);
		if(e == Encodings::text) return "Encodings::text";
	}
	else qdbg << "EncondingValue::ToBinStr2 wrong encoding" << e;
	return "Encodings::unknown";
}

Value::Value(QString name_, const std::vector<Value> &values)
{
	name = name_;
	value = "";
	type = ValuesTypes::vectorInt;
	for(uint i=0; i<values.size(); i++)
	{
		auto val_i_vt {IOGetValueAndType()};
		if(val_i_vt.second == ValuesTypes::int_)
		{
			value += val_i_vt.first + ";";
		}
		else if(values[i].IOGetValueAndType().second == ValuesTypes::vectorInt) value = val_i_vt.first;
		else qdbg << "Value::Value(QString name_, const std::vector<Value> &values) unrealesed" << values[i].name << values[i].value << values[i].type;
	}
	if(!RangeCheck()) value = "";
}



bool Value::RangeCheck(const Value &value_)
{
	long long check = value_.value.toLongLong();

//	if(name == "Заводской номер")
//	{
//		int a=5;
//		qdbg << t << ValuesTypes::ulong;
//		qdbg << check << ValuesTypes::rangeUlong.first;
//		qdbg << check << ValuesTypes::rangeUlong.second;
//	}

	if(value_.type == ValuesTypes::binCode) return true;
	if(value_.type == ValuesTypes::bool_ && (check==0 || check==1)) return true;
	if(value_.type == ValuesTypes::int_ && (check>=ValuesTypes::rangeInt.min && check<=ValuesTypes::rangeInt.max)) return true;
	if(value_.type == ValuesTypes::ulong && (check>=ValuesTypes::rangeUlong.min && check<=ValuesTypes::rangeUlong.max)) return true;
	if(value_.type == ValuesTypes::ushort && (check>=ValuesTypes::rangeUshort.min && check<=ValuesTypes::rangeUshort.max)) return true;
	if(value_.type == ValuesTypes::ulong && (check>=ValuesTypes::rangeUlong.min && check<=ValuesTypes::rangeUlong.max)) return true;
	if(value_.type == ValuesTypes::llong && (check>=ValuesTypes::rangeLLong.min && check<=ValuesTypes::rangeLLong.max)) return true;
	if(value_.type == ValuesTypes::vectorInt)
	{
		if(RangeCheckVector(value_, ValuesTypes::rangeInt))
			return true;
	}
	if(value_.type == ValuesTypes::vectorByte)
	{
		if(RangeCheckVector(value_, ValuesTypes::rangeByte))
			return true;
	}
	if(value_.type == ValuesTypes::vectorBool)
	{
		if(RangeCheckVector(value_, ValuesTypes::rangeBool))
			return true;
	}

	qdbg << "Value::RangeCheck error or unrealesed type = " << value_.type << "value = " << value_.value << "name = " << value_.name;
	return false;
}

bool Value::RangeCheckVector(const Value &value_, const ValuesTypes::Range &range)
{
	bool check = true;
	auto vals {value_.value.split(';',QString::SkipEmptyParts)};
	for(auto &v:vals)
	{
		auto tmpVal {v.toLongLong()};
		if (!(tmpVal>=range.min && tmpVal<=range.max)) check = false;
	}
	return check;
}

void Value::Init(QString name_, QString value_, QString type_)
{
	if(value != ValuesTypes::undefined) qdbg << "Value::Init value not undefined" << name << value << type;
	name=name_, value=value_, type=type_;
	if(!RangeCheck()) value = "";
}

void Value::InitPt(QString name_, void *pt_, QString type_)
{
	if(pt != nullptr) Error("InitPt not nullptr pt " + name);
	name=name_, pt=pt_, type=type_;
}

int Value::GetSizeBits() const
{
	if(type == ValuesTypes::ushort) return ValuesTypes::sizeBitsShort;
	if(type == ValuesTypes::ulong) return ValuesTypes::sizeBitsLong;

	Error("GetSizeBits: ubrealesed type "+type);
	return 0;
}

int Value::GetSizeBytes() const
{
	if(type == ValuesTypes::ushort) return ValuesTypes::sizeBitsShort/8;
	if(type == ValuesTypes::ulong) return ValuesTypes::sizeBitsLong/8;
	if(type == ValuesTypes::binCode)
	{
		float size = (float)value.length()/(float)8;
		size = ceil (size);
		return size;
	}

	Error("GetSizeBytes: ubrealesed type "+type);
	return 0;
}

void Value::Set(QString newValue, QString type_)
{
	value = "";
	if(type == type_) value = newValue;
	else if(type == ValuesTypes::undefined) { value = newValue; type = type_; }
	else if(ValuesTypes::simpleNumeric.contains(type))
	{
		if(ValuesTypes::simpleNumeric.contains(type_))
		{
			value = newValue;
		}
		else if(type_ == ValuesTypes::vectorByte)
		{
			auto newValList {newValue.split(';', QString::SkipEmptyParts)};
			if(type == ValuesTypes::bool_)
			{
				if(newValList.size() == 1 && newValList[0].toInt() <= 1)
					value = QSn(newValList[0].toInt());
				else qdbg << "Assign error conversion. Tried to Assign " << type_ << "in" << type << "new value =" << newValue;
			}
			else if(type == ValuesTypes::ushort)
			{
				if(newValList.size() == (ValuesTypes::sizeBitsShort/8))
				{
					for(int i=0; i<newValList.size(); i++)
					{
						newValList[i] = QSn(newValList[i].toUInt(nullptr,10),2);
						while(newValList[i].length() < 8) newValList[i] = '0' + newValList[i];
					}
					value = QSn(newValList.join(';').remove(';').toUInt(nullptr,2));
				}
				else qdbg << "Assign error conversion. Tried to Assign " << type_ << "in" << type << "new value =" << newValue << "different size";
			}
			else qdbg << "Assign unrealesed conversion. Tried to Assign " << type_ << "in" << type << "new value =" << newValue;
		}
		else qdbg << "Assign unrealesed conversion. Tried to Assign " << type_ << "in" << type << "new value =" << newValue;
	}
	else if(ValuesTypes::vectorsNumeric.contains(type) && ValuesTypes::vectorsNumeric.contains(type_)) value = newValue;
	else qdbg << "Assign unrealesed conversion. Tried to Assign " << type_ << "in" << type << "new value =" << newValue;

	if(!RangeCheck()) value = "";
}

void Value::Set(const Value &newValue)
{
	auto new_vt {newValue.IOGetValueAndType()};
	Set(new_vt.first, new_vt.second);
}

void Value::PlusAssign(QString newValue, QString type_)
{
	if(type == ValuesTypes::vectorBool || type == ValuesTypes::vectorByte || type == ValuesTypes::vectorInt)
	{
		if(type_ == ValuesTypes::int_) value += newValue + ";";
		else if(type_ == ValuesTypes::llong) value += newValue + ";";
		else if(type_ == ValuesTypes::bool_) value += newValue + ";";
		else if(type_ == ValuesTypes::ushort) value += newValue + ";";
		else if(type_ == ValuesTypes::ulong) value += newValue + ";";
		else qdbg << "Value::PlusAssign unrealesed PlusAssign. Tried to PlusAssign " << type << "to" << type_;
	}
	else qdbg << "Value::PlusAssign unrealesed PlusAssign. Tried to PlusAssign " << type << "to" << type_;

	if(!RangeCheck()) value = "";
}

void Value::PlusAssign(const Value &newValue)
{
	auto new_vt {newValue.IOGetValueAndType()};
	PlusAssign(new_vt.first, new_vt.second);
}

QString Value::ToBinStr() const
{
	if(type == ValuesTypes::binCode) return value;
	if(type == ValuesTypes::ushort)
	{
		bool ok;
		QString ret {QSn(value.toInt(&ok),2)};
		if(!ok) qdbg << "Value::ToBinStr() can't convert; type" << type << "but value" << value;
		while (ret.length()< ValuesTypes::sizeBitsShort) ret = '0' + ret;
		return ret;
	}
	qdbg << "Value::ToBinStr unrealesed type" << name <<value<<type;
	return "";
}

QString Value::ToStr() const
{
	if(type == ValuesTypes::bool_) return value;
	if(type == ValuesTypes::int_) return value;
	if(type == ValuesTypes::ulong) return value;
	if(type == ValuesTypes::ushort) return value;
	if(type == ValuesTypes::text) return value;
	if(type == ValuesTypes::undefined) return value;
	qdbg << "Value::ToDecStr unrealesed type" << name <<value<<type;
	return "";
}

QString Value::ToDecStr() const
{
	if(type == ValuesTypes::bool_) return value;
	if(type == ValuesTypes::int_) return value;
	if(type == ValuesTypes::ulong) return value;
	if(type == ValuesTypes::ushort) return value;
	qdbg << "Value::ToDecStr unrealesed type" << name <<value<<type;
	return "";
}

int Value::ToInt(QString operand) const
{
	AllIndexes indexes {Compiler::GetAllIndexes(operand)};
	bool noIndexes = (indexes.first.empty() && indexes.secnd.empty());

	if(type == ValuesTypes::bool_ && noIndexes) return value.toInt();
	if(type == ValuesTypes::int_ && noIndexes) return value.toInt();
	if(type == ValuesTypes::ushort && noIndexes) return value.toInt();
	if(type == ValuesTypes::vectorBool || type == ValuesTypes::vectorByte || type == ValuesTypes::vectorInt)
	{
		int vSize = value.count(';');
		if(vSize == 1 && noIndexes) return value.left(value.indexOf(';')).toInt();
		else
		{
			if(indexes.first.size() && !indexes.secnd.size())
			{
				if(indexes.first.size() == 1)
				{
					int getElemIndex = indexes.first[0];
					if(getElemIndex < vSize)
					{
						auto valList {value.split(';',QString::SkipEmptyParts)};
						return valList[getElemIndex].toInt();
					}
				}
			}
		}
	}
	Error("ToInt unrealesed or wrong output. Type = " + type + "; value = "+value+"; operand = "+operand);
	return 0;
}

std::vector<int> Value::ToVectInt() const
{
	if(type == ValuesTypes::bool_) return {value.toInt()};
	if(type == ValuesTypes::int_) return {value.toInt()};
	if(type == ValuesTypes::ulong) return {value.toInt()};
	if(type == ValuesTypes::ushort) return {value.toInt()};
	if(type == ValuesTypes::vectorInt || type == ValuesTypes::vectorByte || type == ValuesTypes::vectorBool)
	{
		std::vector<int> ret;
		auto valList {value.split(';',QString::SkipEmptyParts)};
		ret.resize(valList.size());
		for(int i=0; i<valList.size(); i++) ret[i] = valList[i].toInt();
		return ret;
	}
	qdbg << "Value::ToVectInt unrealesed type" << name <<value<<type;
	return {};
}

std::vector<bool> Value::ToVectBits(int size) const
{
	std::vector<bool> ret;
	if(type == ValuesTypes::bool_ || type == ValuesTypes::ushort)
	{
		QString bitsVal { QSn(ToInt(""),2) };
		if(bitsVal.length() <= size)
		{
			while(bitsVal.length() < size) bitsVal = "0" + bitsVal;
			for(int i=size-1; i>=0; i--)
			{
				if(bitsVal[i] == '0') ret.push_back(0);
				else if(bitsVal[i] == '1') ret.push_back(1);
				else Error("ToVectBits wrong converted to bits [" + bitsVal +"] value [" +value+ "]");
			}
		}
		else Error("ToVectBits to big value [" +value+ "] to write in vect bits size " + QSn(size));
	}
	else Error("ToVectBits unrealesed type "+type + " (name" + name + ")");

	return ret;
}

std::vector<Byte> Value::ToVectBytes(int size) const
{
	std::vector<Byte> bytes(size);
	if(type == ValuesTypes::ushort)
	{
		QString tmpVal = QSn(value.toInt(),2);
		while(tmpVal.length()<size*8) { tmpVal = '0' + tmpVal; }
		for(int i=0; i<size; i++)
		{
			QString byte = tmpVal.left(8);
			tmpVal.remove(0,8);
			bytes[i].Set(byte.toInt(nullptr,2));
		}
	}

//	QString logStr = "ToVectBytes: value " + value + "converted to ";
//	for(auto &b:bytes) logStr += QSn(b.data.to_ulong()) + "; ";
//	Log(logStr);
	return bytes;
}

QString Value::ToBinStrByBytes() const
{
	if(type == ValuesTypes::vectorInt)
	{
		QString ret;
		QString cur;
		QStringList value_ = value.split(";",QString::SkipEmptyParts);
		for(auto &v:value_)
		{
			bool ok;
			cur = v;
			int curI = cur.toInt(&ok);
			if(!ok) qdbg << "Value::ToBinStrByBytes, wrong value" << name <<v<<type<<cur;
			cur = QSn(curI,2);
			while(cur.length()<8) cur = "0" + cur;
			if(cur.length()>8) qdbg << "Value::ToBinStrByBytes, too large values" << name <<v<<type<<cur;
			ret += cur;
		}
		return ret;
	}
	if(type == ValuesTypes::binCode) return value;
	qdbg << "Value2::ToBinStrAddZeros unrealesed type" << name <<value<<type;
	return "";
}

void Value::BinCodesListToDataPobit(const QStringList &values, CANMsg_t &msg)
{
	if(values.size() > 8) { qdbg << "Value::ToDataPobit to long value" << values.join(";"); }
	else
	{
		if(msg.length < (uint)values.size()) msg.length = values.size();

		for(int l=0; l<values.size() && l<8; l++)
		{

			std::bitset<8> data {msg.data[l]};
			for(int i=0; i<values[l].length(); i++)
			{
				if(values[l][i] == '0') data[i] = 0;
				if(values[l][i] == '1') data[i] = 1;
			}
			msg.data[l] = data.to_ulong();
		}
	}
}

QString Value::GetTypeByStrValue(const QString &value)
{
	QString ret;
	if(Compiler::IsInteger(value))
	{
		bool ok;
		auto intVal { value.toLongLong(&ok) };
		if(!ok) intVal = value.toULongLong(&ok);
		if(ok)
		{
			// перебираем все диапазоны, если нашли - возвращаем тип
			for(auto &range:ValuesTypes::allRanges)
				if(intVal>=range.min && intVal>=range.max) return range.type;
		}
		else Error("GetTypeByStrValue: can't get type of " +value);
	}
	else Error("GetTypeByStrValue: can't get type of " +value);

	return ValuesTypes::unknown;
}

Value Value::ConvertToType(QString newType) const
{
	Value retV;
	if(RangeCheck())
	{
		if(newType == type || newType == ValuesTypes::any) retV = *this;
		else if(newType == ValuesTypes::binCode)
		{
			if(type == ValuesTypes::vectorByte)
			{
				QString binCode;
				auto vect { ToVectInt() };
				for(uint i=0; i<vect.size(); i++)
				{
					QString strByte = QSn(vect[i],2);
					while (strByte.length() < 8) strByte = '0' + strByte;
					binCode += strByte;
				}
				retV.Set(binCode,newType);
			}
			else if(ValuesTypes::simpleNumeric.contains(type))
			{
				int needLenth = 0;
				if(type == ValuesTypes::ushort) needLenth = ValuesTypes::sizeBitsShort;
				else Error("ConvertToType: can't define lenth  " + type);

				QString binCode { QSn(value.toULong(),2) };
				while (binCode.length() < needLenth) binCode = '0' + binCode;
			}
			else Error("ConvertToType: unrealesed to bincode conversion  " + type + " to " + newType);
		}
		else Error("ConvertToType: unrealesed conversion " + type + " to " + newType);
	}
	else Error("ConvertToType: error RangeCheck " + type + " " + value);

	return retV;
}

void Value::ToDataPobit(CANMsg_t &msg, QString dataOperand) const
{
	if(Compiler::GetOperandName(dataOperand) == "Data")
	{
		auto indexesBytes { Compiler::GetAllIndexes(dataOperand) };
		if(indexesBytes.first.empty() && indexesBytes.secnd.empty())
		{
			ToDataPobit(msg);
		}
		else if(indexesBytes.secnd.empty())
		{
			QString valueBinCode {ToBinStr()};
			if((uint)valueBinCode.length() != indexesBytes.first.size() * 8)
				qdbg << "Value::ToDataPobit size value differ size dataOperand";
			else
			{
				for(auto i:indexesBytes.first)
				{
					if(i<8)
					{
						if((int)msg.length < i+1) msg.length = i+1;
						msg.data[i] = valueBinCode.left(8).toUInt(nullptr,2);
						valueBinCode.remove(0,8);
					}
					else { qdbg << "Value::ToDataPobit wrong dataOperand"; return; }
				}
			}
		}
	}
	else qdbg << "Value::ToDataPobit wrong dataOperand";
}

void Value::ToDataUniversal(CANMsg_t &msg, QString dataOperand) const
{
	if(Compiler::GetOperandName(dataOperand) == "Data")
	{
		auto indexesBytes { Compiler::GetAllIndexes(dataOperand) };

		// If indexes are not set
		if(indexesBytes.first.empty() && indexesBytes.secnd.empty())
		{
			if(type == ValuesTypes::binCode)
			{
				EasyData ed(msg);
				ed.FromBinCode(value);
				ed.ToMsg(msg);
			}
			else if(type == ValuesTypes::vectorByte)
			{
				auto vals { ToVectInt() };
				int valsSize = vals.size();
				if(valsSize > 8) Error("ToDataUniversal to long value " + ToStrForLog());
				msg.length = valsSize;
				for(int i=0; i<valsSize; i++) msg.data[i] = vals[i];
			}
			else if(type == ValuesTypes::vectorBool)
			{
				EasyData ed;
				ed.FromVect<int>(ToVectInt());
				ed.ToMsg(msg);
			}
			else Error("ToDataUniversal unrealesed type " + type + " dataOperand: "+dataOperand);
		}
		// If seted only first indexes
		else if(indexesBytes.first.size() && indexesBytes.secnd.empty())
		{
			if(indexesBytes.first.size() == 1)
			{
				int index = indexesBytes.first[0];
				if((int)msg.length < index+1) msg.length = index+1;
				if(type == ValuesTypes::int_)
				{
					int newVal = ToInt("");
					if(newVal>=ValuesTypes::rangeByte.min && newVal<=ValuesTypes::rangeByte.max)
						msg.data[index] = newVal;
					else Error("To big value. Try write "+ToStrForLog()+" to "+dataOperand);
				}
			}
			else
			{
				if((int)indexesBytes.first.size() == GetSizeBytes())
				{
					EasyData ed(msg);
					auto vectBytes { ToVectBytes(indexesBytes.first.size()) };
					for(uint i=0; i<vectBytes.size(); i++)
					{
						ed.SetByte(indexesBytes.first[i], vectBytes[i]);
					}
					ed.ToMsg(msg);
				}
				else Error("ToDataUniversal unrealesed dataOperand (indexesBytes.first.size() != GetSizeBytes()): "+dataOperand);
			}
		}
		// If seted first and secnd indexes
		else if(indexesBytes.first.size() && indexesBytes.secnd.size())
		{
			if(indexesBytes.first.size() == 1 && indexesBytes.secnd.size() == 1)
			{
				if(type == ValuesTypes::bool_)
				{
					EasyData ed(msg);
					ed.SetBitBiBi(indexesBytes.first[0], indexesBytes.secnd[0], value.toInt());
					ed.ToMsg(msg);
				}
			}
			else if(indexesBytes.first.size() == 1 && indexesBytes.secnd.size() > 1)
			{
				EasyData ed(msg);
				auto valueVectBits { ToVectBits(indexesBytes.secnd.size()) };
				for(uint i=0; i<valueVectBits.size(); i++)
					ed.SetBitBiBi(indexesBytes.first[0],indexesBytes.secnd[i],valueVectBits[i]);
				ed.ToMsg(msg);
			}
			else Error("ToDataUniversal unrealesed dataOperand: "+dataOperand);
		}
		else Error("ToDataUniversal dataOperand ["+dataOperand+"] can't be with 3 indexes");
	}
	else Error("ToDataUniversal wrong dataOperand " + dataOperand);
}

void Value::ToDataPobit(CANMsg_t &msg) const
{
	if(type == ValuesTypes::binCode)
	{
		QStringList dataList;
		dataList.push_back("");
		int curIndex = 0;
		for(int i=value.length()-1; i>=0; i--)
		{
			if(dataList[curIndex].length()==8)
			{
				curIndex++;
				dataList.push_back("");
			}

			dataList[curIndex] = value[i] + dataList[curIndex];
		}

		BinCodesListToDataPobit(dataList, msg);
	}
	if(type == ValuesTypes::vectorBool)
	{
		QStringList values {value.split(";",QString::SkipEmptyParts)};
		int sizeVals = values.size();
		if(sizeVals > 8*8) qdbg << "Value::ToDataPobit Types::vectorBool to long value" << value;
		int valuesInd = 0;
		for(int d=0; d<8 && valuesInd<values.size(); d++)
		{
			std::bitset<8> data {msg.data[d]};
			for(int i=0; i<8 && valuesInd<values.size(); i++)
			{
				msg.length = d+1;

				bool ok;
				int res = values[valuesInd].toInt(&ok);
				valuesInd++;
				if(!ok || res < 0 || res > 1) qdbg << "Value::ToDataPobit wrong value " <<  values[valuesInd] <<"in values"<<value;
				data[i] = (bool)res;
			}
			msg.data[d] = data.to_ulong();
		}
	}
	else qdbg << "Value::ToDataPobit unrealesed type" << type;
}

void Value::ToDataPobyte(CANMsg_t &msg) const
{
	if(type == ValuesTypes::vectorInt)
	{
		auto vals {value.split(";",QString::SkipEmptyParts)};

		if(vals.empty()) qdbg << "Value::ToDataPobyte dannie.empty()==true";
		if(vals.size() > 8) qdbg << "Value::ToDataPobyte dannie.size() > 8 (to big size)";
		for(int i=0; i<8 && i<vals.size(); i++)
		{
			if((int)msg.length < i+1) msg.length = i+1;
			bool ok;
			uint val = vals[i].toUInt(&ok);
			msg.data[i] = val;
			if(!ok || msg.data[i] != val) qdbg << "Value::ToDataPobyte, wrong value" << name << value << type;
		}
	}
	else qdbg << "Value2::ToData unrealesed type" << type;
}

void Value::FromDataPobit(const CANMsg_t &msg, std::vector<Value> &values)
{
	uint valIndex = 0;

	for(uint byte=0; byte<msg.length; byte++)
	{
		std::bitset<8> data = msg.data[byte];
		for(uint bit=0; bit<8; bit++)
		{
		values[valIndex].Set(QSn(data[bit],2), ValuesTypes::bool_);
		valIndex++;
		if(valIndex == values.size()) {  break; }
		}
	}
	if(valIndex < values.size()) Error("FromDataPobitovo: not enougth data " + msg.ToStrEx());
}

void Value::FromDataPobyteOld(const CANMsg_t &msg, QString dataOperand, std::vector<Value> &values)
{
	auto indexes {Compiler::GetAllIndexes(dataOperand)};

	values.clear();
	if(indexes.secnd.empty())
	{
		values.reserve(indexes.first.size());
		for(auto i:indexes.first)
		{
			values.push_back(Value("", QSn(msg.data[i]), ValuesTypes::int_));
		}
	}
	else qdbg << "FromDataPobyte(const CANMsg_t &msg, QString dataOperand, std::vector<Value> &values) unrealesed dataOperand" << dataOperand;
}

Value& Value::FromDataUniversal(const CANMsg_t &msg, QString dataOperand)
{
	auto indexes {Compiler::GetAllIndexes(dataOperand)};

//	if(dataOperand.contains("Data[]"))
//		int a = 5;

	if(indexes.first.empty() && indexes.secnd.empty())
	{
		Error("FromDataUniversal no index dataOperand ("+dataOperand+") is not allowed in predicate");
	}
	else if(indexes.first.size() && indexes.secnd.empty())
	{
		QString vals;
		for(auto i:indexes.first) vals += QSn(msg.data[i]) + ";";

		Set(vals, ValuesTypes::vectorByte);
	}
	else if(indexes.first.size() == 1 && indexes.secnd.size())
	{
		if(ValuesTypes::simpleNumeric.contains(type))
		{
			QString chisloFromData;
			Byte byte(msg.data[indexes.first[0]]);

			for(uint i=0; i<indexes.secnd.size(); i++)
			{
				if(byte.data[indexes.secnd[i]]) chisloFromData = '1' + chisloFromData;
				else chisloFromData = '0' + chisloFromData;
			}

			bool ok;
			chisloFromData = QSn(chisloFromData.toUShort(&ok,2));
			if(!ok)  Error("FromDataUniversal impossible error 1");

			value = chisloFromData;
			if(!RangeCheck()) Error("FromDataUniversal error range check. Type " + type +" and dataOperand " + dataOperand + ". Try write " + chisloFromData);
		}
		else Error("FromDataUniversal unrealesed type " + type +" and dataOperand " + dataOperand);
	}
	else if(indexes.first.empty() && indexes.secnd.size())
	{
		if(type == ValuesTypes::vectorBool)
		{
			value="";
			EasyData ed(msg);
			for(uint i=0; i<indexes.secnd.size(); i++)
			{
				int index = indexes.secnd[i];
				if(index < EasyData::size)
					value += QSn(ed.Data()[index]) + ';';
				else Error("FromDataUniversal to big index ("+QSn(index)+") in dataOperand " + dataOperand);
			}
		}
		else Error("FromDataUniversal unrealesed type " + type +" and dataOperand " + dataOperand);
	}
	else Error("FromDataUniversal unrealesed dataOperand " + dataOperand);

	return *this;
}

void EasyData::SetBitBi(int bitIndex, bool val)
{
	if(bitIndex > size)
	{
		Error("SetBit bitIndex ["+ QSn(size) +"] bigger size");
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

void EasyData::FromBinCode(QString strBinCode)
{
	Error("FromBinCode unrealesed. Commented realisation contains errors." + strBinCode);
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
	if(len > 8) { Error("FromMsg len > "+QSn(8)); return; }
	int eDataIndex = 0;
	for(int i=0; i<len; i++)
	{
		std::bitset<8> dataBitset {msg.data[i]};
		for(int bit=0; bit<8; bit++)
		{
			if(eDataIndex >= size) { Error("FromMsg eDataIndex >= " + QSn(size)); return; }
			data[eDataIndex] = dataBitset[bit];
			eDataIndex++;
		}
	}
}

void EasyData::ToMsg(CANMsg_t &msg)
{
	msg.length = len;
	if(len > 8) { Error("ToMsg len > " + QSn(8)); return; }
	int eDataIndex = 0;
	for(int i=0; i<len; i++)
	{
		std::bitset<8> dataBitset;
		for(int bit=0; bit<8; bit++)
		{
			if(eDataIndex >= size) { Error("ToMsg eDataIndex >= " + QSn(size)); return; }
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
	static_assert(std::is_same<T,int>::value || std::is_same<T,bool>::value, "EasyData::FromVect wrong type (not int or bool)");

	len = vectVals.size()/8 + 1;
	if(len > 8 || vectVals.size() > size) Error("FromVect: to big size value" + QSn(vectVals.size()));
	for(uint i=0; i<vectVals.size(); i++)
	{
		if(vectVals[i] > 1 || vectVals[i] < 0) {
			data[i] = 0;
			Error("FromVect: to big value " + QSn(vectVals[i]));
		}
		else data[i] = vectVals[i];
	}
}

void Byte::Set(long long value)
{
	if(value > ValuesTypes::rangeByte.max || value < ValuesTypes::rangeByte.min)
	{
		Error("To value going out of range to convert to byte: " + QSn(value));
		value = 0;
	}
	data = value;
}
