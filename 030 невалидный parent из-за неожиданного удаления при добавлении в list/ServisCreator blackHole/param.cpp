#include "param.h"

#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

#include "MyQDom.h"
#include "MyCppDifferent.h"

Param::Param(const QDomElement &paramElement, void *objectParent_, int type_)
{
	SetCellValuesFromDomElement(paramElement);

	type = type_;
	objectParent = objectParent_;
	addProperties.Init(addPropsCell);

	InitParamValue();
}

void Param::InitParamValue()
{
	auto commands {Code::TextToCommands(typeCell)};

	Code::RemoveEmulatorServis(commands,type);

	if(commands.size() == 1)
	{
		auto words {Code::CommandToWords(commands[0])};
		if(words.empty())
		{
			Error("InitParamValue wrong typeCell words.empty() " + typeCell);
			return;
		}

		Operator::CorrectVectorDefinition(words);
		typeDefined = words[0];

		if(ValuesTypes::all.contains(typeDefined))
		{
			QString initStr = Code::GetInitialisationStr(commands[0], false);

			if(ValuesTypes::vectorsNumeric.contains(typeDefined))
			{
				initStr.replace(QString(" ")+ValuesTypes::vectorSplitter, QString(ValuesTypes::vectorSplitter));
				initStr.replace(QString(ValuesTypes::vectorSplitter)+" ", QString(ValuesTypes::vectorSplitter));
			}

			if(QString checkRes = Value::CheckCorrectionAndRange(initStr,typeDefined); checkRes.isEmpty())
				value.Init(nameCell,initStr,typeDefined);
			else
			{
				Logs::WarningSt("Ошибка инициализации значения параметра " + ToStrForLogShort() + " ("+typeCell+") " + checkRes);
				value.Init(nameCell,"",typeDefined);
			}

			Value::cbValueChanged_t cbForValue = [](const Value &newValue, HaveClassName *cbHandler)
			{
				Param *thisParam = dynamic_cast<Param*>(cbHandler);
				if(thisParam)
				{
					if(Settings::logParamChanges && !thisParam->flagParamChangedInVectorParams)
					{
						QString log = "параметр " + thisParam->ToStrForParamChangedLog();
						log += " установлен в [" + thisParam->value.ToStr()+ "]";
						thisParam->Log(log);
					}

					for(auto cbParamChanged:thisParam->cbParamChangedVector)
					{
						cbParamChanged.cbFunction(newValue, cbParamChanged.cbHandler);
					}

					for(auto cbParamChanged:thisParam->cbParamChangedVectorForExternal)
					{
						cbParamChanged.cbFunction(newValue, cbParamChanged.cbWidgetHandler);
					}
				}
				else Logs::ErrorSt("Param::cbForValue error dynamic_cast cbHandler to param");
			};

			value.SetCBValueChanged(cbForValue, this);
		}
		else Error("InitParamValue wrong defined type "+typeDefined+" in param " + CellValuesToStrForLog());
	}
	else Error("InitParamValue: wrong typeCell: commands.size() != 1\n\tParam:\n" + ToStrForLog());
}

QString Param::CellValuesToStrForLog() const
{
	QString ret = "<Param";
	auto attrs = GetAttributes();
	for(auto &attr:attrs) ret += "[" + attr.first + "=" + attr.second + "]";
	return ret + ">";
}

QStringList Param::CellValuesToStringListForEditor() const
{
	QStringList ret;
	for(int i=0; i<Param_ns::count; i++) ret.append("");

	ret[Param_ns::nameI] = nameCell;
	ret[Param_ns::categoryI] = categoryCell;
	ret[Param_ns::id_I] = idCell;
	ret[Param_ns::typeI] = typeCell;
	ret[Param_ns::addProtertiesI] = addPropsCell;

	return ret;
}

void Param::SetCellValuesFromEditor(const QStringList & values)
{
	if(values.size() == Param_ns::count)
	{
		categoryCell		= values[Param_ns::categoryI];
		nameCell			= values[Param_ns::nameI];
		idCell				= values[Param_ns::id_I];
		typeCell			= values[Param_ns::typeI];
		addPropsCell		= values[Param_ns::addProtertiesI];
	}
	else Error("Param::SetCellValuesFromEditor values.size() != Param_ns::count ("+QSn(values.size())+" != "+QSn(Param_ns::count)+")");
}

void Param::SetCellValuesFromDomElement(const QDomElement & paramElement)
{
	auto attrs = MyQDom::Attributes(paramElement);
	if((int)attrs.size() == Param_ns::count)
	{
		for(auto &attr:attrs)
		{
			if(attr.first == Param_ns::category) categoryCell = attr.second;
			else if(attr.first == Param_ns::name) nameCell = attr.second;
			else if(attr.first == Param_ns::id_) idCell = attr.second;
			else if(attr.first == Param_ns::type) typeCell = attr.second;
			else if(attr.first == Param_ns::addProterties) addPropsCell = attr.second;
			else Error("Param::SetCellValuesFromDomElement wrong param attribute ["+attr.first+"]");
		}
	}
	else Error("Param::SetCellValuesFromDomElement wrong attrs size ("+QSn(attrs.size())+")");
}

void Param::SetCellValue(QString fieldName, QString newValue)
{
	if(fieldName == Param_ns::category) categoryCell = newValue;
	else if(fieldName == Param_ns::name) nameCell = newValue;
	else if(fieldName == Param_ns::id_) idCell = newValue;
	else if(fieldName == Param_ns::type) typeCell = newValue;
	else if(fieldName == Param_ns::addProterties) addPropsCell = newValue;
	else Error("Param::SetCellValue wrong param attribute ["+fieldName+"]");
}

QStringPairVector Param::GetAttributes() const
{
	QStringPairVector atts;
	atts.push_back({Param_ns::category,categoryCell});
	atts.push_back({Param_ns::name,nameCell});
	atts.push_back({Param_ns::id_,idCell});
	atts.push_back({Param_ns::type,typeCell});
	atts.push_back({Param_ns::addProterties,addPropsCell});
	return atts;
}

void Param::AddCBParamChanged(CBParamChanged cbParamChanged)
{
	if(cbParamChanged.cbFunction && cbParamChanged.cbHandler)
	{
		cbParamChangedVector.push_back(cbParamChanged);
	}
	else Error("AddCBParamChanged: wrong cbParamChanged. cbFunction and cbHandler must not be nullptr");
}

void Param::AddCBParamChangedForExternal(CBParamChangedForExternal cbParamChanged)
{
	if(cbParamChanged.cbFunction && cbParamChanged.cbWidgetHandler)
	{
		cbParamChangedVectorForExternal.push_back(cbParamChanged);
	}
	else Error("AddCBParamChangedForExternal: wrong cbParamChanged. cbFunction and cbHandler must not be nullptr");
}

QString Param::GetClassName() const
{
	return IOperand::classParam();
}

QString Param::ToStrForLog() const
{
	return ToStrForLogShort();
}

QString Param::ToStrForParamChangedLog() const
{
	return categoryCell+"::"+nameCell;
}

QString Param::ToStrForLogShort() const
{
	return categoryCell + "::" + nameCell + " ("+typeDefined+")";
}

void Param::Assign(const IOperand *operand2)
{
	QString curValueType { value.GetType() };
	Value newValue(operand2->IOGetValue(curValueType),curValueType);
	value.Set(newValue.GetValue(),newValue.GetType());
}

std::shared_ptr<IOperand> Param::Minus(const IOperand * operand2)
{
	return GetParamValue().Minus(operand2);
}

void Param::PlusAssign(const IOperand *operand2)
{
	Value curValue(value.IOGetValueAndType());
	curValue.PlusAssign(operand2);
	value.Set(curValue.GetValue(),curValue.GetType());
}

bool Param::CmpOperation(const IOperand *operand2, QString operationCode) const
{
	Error(GetClassName()+"::CmpOperation "+ operand2->ToStrForLog()+ " " + CodeMarkers::mock + " " + operationCode);
	return false;
}

std::shared_ptr<IOperand> Param::Index(int index)
{
	return std::make_shared<SubValue>(value,index);
}

IOperand *Param::PointAccess(QString nameToAccess)
{
	if(PointAccessAll().contains(nameToAccess))
	{
		if(nameToAccess == PointAccessClear())
		{
			value.PointAccess(nameToAccess);
		}
		else Error(GetClassName() + "::PointAccess unrealesed nameToAccess ["+nameToAccess+"]");
	}
	else Error(GetClassName() + "::PointAccess unknown nameToAccess ["+nameToAccess+"]");
	return nullptr;
}

QString Param::IOGetName() const
{
	return nameCell;
}

QStringPair Param::IOGetValueAndType() const
{
	return value.IOGetValueAndType();
}

QString Param::IOGetValue(QString outputValueType) const
{
	auto convertRes = value.ConvertToType(outputValueType);
	if(convertRes.second != "") Error("Param::IOGetValue ConvertToType error: " + convertRes.second);
	return convertRes.first.GetValue();
}

QString Param::IOGetType() const
{
	return value.GetType();
}

void VectorParams::Set(const std::vector<Param *> &params_, QString name_)
{
	if(params.size()) Logs::WarningSt("VectorParams::Set not empty VectorParams");
	name = name_;
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
		else if(types.size() > 1) Logs::ErrorSt("VectorParams::Set: different values types in &params_: " + types.join(';'));
		else Logs::ErrorSt("VectorParams::Set: impossible behavior");
	}
	else Logs::ErrorSt("VectorParams::Set: empty &params_");
}

QString VectorParams::ToStrForParamChangedLog()
{
	if(!params.empty()) return params[0]->NameCell()+" "+QSn(params.size())+" шт.";
	else return "empty " + inCofigCode;
}

void VectorParams::Assign(const IOperand *operand2)
{
	int thisSize = params.size();
	if(thisSize)
	{
		QString thisType {IOGetType()};
		QString type1paramVal { params[0]->IOGetType() };

		QString newVals { operand2->IOGetValue(thisType) };
		QStringList newValsList = Value::GetStringListFromValueVector(newVals);

		if(thisSize == newValsList.size())
		{
			for(int i=0; i<thisSize; i++)
			{
				Value tmpVal {newValsList[i],type1paramVal};
				params[i]->flagParamChangedInVectorParams = true;
				params[i]->Assign(&tmpVal);
				params[i]->flagParamChangedInVectorParams = false;
			}
			if(Settings::logParamChanges)
			{
				QString log = "параметры " + ToStrForParamChangedLog();
				log += " установлены в [" + newVals+ "]";
				Log(log);
			}
		}
		else Logs::ErrorSt(GetClassName() + "::Assign: sizes different (thisSize="+QSn(thisSize)+" and newValsListSize="+QSn(newValsList.size())+")");
	}
	else Logs::ErrorSt(GetClassName() + "::Assign: empty VectorParams");
}

std::shared_ptr<IOperand> VectorParams::Minus(const IOperand * operand2)
{
	Logs::LogSt(GetClassName() + " PlusAssign forbidden" + operand2->IOGetType());
	return nullptr;
}

void VectorParams::PlusAssign(const IOperand *operand2)
{
	Logs::LogSt(GetClassName() + " PlusAssign " + operand2->IOGetType() + CodeMarkers::mock);
}

bool VectorParams::CmpOperation(const IOperand *operand2, QString operationCode) const
{
	Error(GetClassName()+"::CmpOperation "+ operand2->ToStrForLog()+ " " + CodeMarkers::mock + " " + operationCode);
	return false;
}

std::shared_ptr<IOperand> VectorParams::Index(int index)
{
	if(index < (int)params.size())
		return std::make_shared<ParamPtr>(params[index],"");
	else Error(GetClassName()+"::Index bad index. VectorParams size = " + QSn(params.size()) + " and index = " + QSn(index));
	return nullptr;
}

IOperand * VectorParams::PointAccess(QString nameToAccess)
{
	if(0) {}
	else if(nameToAccess == VectorParams::pointAccessSize)
	{
		Param::createdIOperands.push_back(std::make_shared<Value>(QSn(params.size()),ValuesTypes::uLong));
		return Param::createdIOperands.back().get();
	}
	else Error(GetClassName()+"::PointAccess unrealesed name to access [" + nameToAccess + "]");
	return nullptr;
}

QString VectorParams::IOGetName() const
{
	if(!name.size()) Warning("VectorParams:IOGetName, but name empty");
	return name;
}

QStringPair VectorParams::IOGetValueAndType() const
{
	QStringPair retPair {"",ValuesTypes::undefined};
	int size = params.size();
	if(size)
	{
		QString type1paramVal { params[0]->IOGetType() };
		QString retVal;
		for(int i=0; i<size; i++)
			retPair.first += params[i]->IOGetValueAndType().first + ValuesTypes::vectorSplitter;

		if(type1paramVal == ValuesTypes::sByte)			retPair.second = ValuesTypes::vectorSByte;
		else if(type1paramVal == ValuesTypes::sShort)	retPair.second = ValuesTypes::vectorShort;
		else if(type1paramVal == ValuesTypes::sLong)	retPair.second = ValuesTypes::vectorSLong;
		else if(type1paramVal == ValuesTypes::sLLong)	retPair.second = ValuesTypes::vectorSLLong;
		else if(type1paramVal == ValuesTypes::ubool)	retPair.second = ValuesTypes::vectorBool;
		else if(type1paramVal == ValuesTypes::uByte)	retPair.second = ValuesTypes::vectorUByte;
		else if(type1paramVal == ValuesTypes::uShort)	retPair.second = ValuesTypes::vectorUShort;
		else if(type1paramVal == ValuesTypes::uLong)	retPair.second = ValuesTypes::vectorULong;
		else Logs::ErrorSt("VectorParams::IOGetValueAndType: unrealesed param type " + type1paramVal);
	}
	else Logs::ErrorSt("VectorParams::IOGetValueAndType: empty VectorParams");

	return retPair;
}

QString VectorParams::IOGetValue(QString outputValueType) const
{
	auto curVT {IOGetValueAndType()};
	auto convertRes = Value(curVT.first,curVT.second).ConvertToType(outputValueType);
	if(convertRes.second != "") Error("VectorParams::IOGetValue ConvertToType error: " + convertRes.second);
	return convertRes.first.GetValue();
}

QString VectorParams::IOGetType() const
{
	QString ret {ValuesTypes::undefined};
	int size = params.size();
	if(size)
	{
		QString t0 {params[0]->IOGetType()};

		if(t0 == ValuesTypes::sByte)		ret = ValuesTypes::vectorSByte;
		else if(t0 == ValuesTypes::sShort)	ret = ValuesTypes::vectorShort;
		else if(t0 == ValuesTypes::sLong)	ret = ValuesTypes::vectorSLong;
		else if(t0 == ValuesTypes::sLLong)	ret = ValuesTypes::vectorSLLong;
		else if(t0 == ValuesTypes::ubool)	ret = ValuesTypes::vectorBool;
		else if(t0 == ValuesTypes::uByte)	ret = ValuesTypes::vectorUByte;
		else if(t0 == ValuesTypes::uShort)	ret = ValuesTypes::vectorUShort;
		else if(t0 == ValuesTypes::uLong)	ret = ValuesTypes::vectorULong;
		else Logs::ErrorSt("VectorParams::IOGetType: unrealesed param type " + ret);
	}
	else Logs::ErrorSt("VectorParams::IOGetType: empty VectorParams");
	return ret;
}



ParamPtr::ParamPtr(Param *param_, QString name_)
{
	if(!param_) Logs::WarningSt("ParamPtr() get nullptr paramPtr");
	name = name_;
	param = param_;
}

QString ParamPtr::IOGetName() const
{
	if(!name.size()) Warning("ParamPtr:IOGetName, but name empty");
	return name;
}

QStringPair ParamPtr::IOGetValueAndType() const
{
	return param->IOGetValueAndType();
}

QString ParamPtr::IOGetValue(QString outputValueType) const
{
	return param->IOGetValue(outputValueType);
}

QString ParamPtr::IOGetType() const
{
	return param->IOGetType();
}
