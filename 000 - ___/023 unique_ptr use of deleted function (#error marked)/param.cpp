#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

#include "configconstants.h"
#include "category.h"
#include "param.h"

void Param::InitParam(const QStringList &config, void *objectParent_)

{
	categoryCell		= config[ParamsColIndexes::category];
	nameCell			= config[ParamsColIndexes::name];
	idCell				= config[ParamsColIndexes::id_];
	typeCell			= config[ParamsColIndexes::type];
	addPropsCell		= config[ParamsColIndexes::addProterties];

	objectParent = objectParent_;
	addProperties.Init(addPropsCell);
}

void Param::InitParamValue()
{
	auto commands {Code::TextToCommands(typeCell)};
	if(commands.size() == 1)
	{
		for(auto &cmd:commands)
		{
			auto words {Code::CommandToWords(cmd)};
			if(words.empty()) Error("InitParamValue wrong typeCell words.empty() " + typeCell);
			else
			{
				Operator::CorrectVectorDefinition(words);
				typeDefined = words[0];

				if(ValuesTypes::all.contains(typeDefined))
				{
					QString initStr = Code::GetInitialisationStr(cmd, false);

					if(initStr == ParamsColNames::moduleNumber)
						initStr = CategoryPath::PathFromQString(categoryCell).back().number;

					if(ValuesTypes::vectorsNumeric.contains(typeDefined))
					{
						initStr.replace(" "+ValuesTypes::vectorSplitter, ValuesTypes::vectorSplitter);
						initStr.replace(ValuesTypes::vectorSplitter+" ", ValuesTypes::vectorSplitter);
					}
					value.Init(nameCell,initStr,typeDefined);

					Value::cbValueChanged_t cbForValue = [](const Value &newValue, HaveClassName *cbHandler)
					{
						Param *thisParam = dynamic_cast<Param*>(cbHandler);
						if(thisParam)
						{
							QString log = "параметр " + thisParam->CellValuesToStrForLog();
							log += " установлен в [" + thisParam->value.ToStr()+ "]";
							thisParam->Log(log);

							for(auto cbParamChanged:thisParam->cbParamChangedVector)
							{
								cbParamChanged.cbFunction(newValue, cbParamChanged.cbHandler);
							}
						}
						else Logs::ErrorSt("Param::cbForValue error dynamic_cast cbHandler to param");
					};

					value.SetCBValueChanged(cbForValue, this);
				}
				else Error("InitParamValue wrong defined type "+typeDefined+" in param " + CellValuesToStrForLog());
			}
		}
	}
	else Error("InitParamValue: commands.size() != 1");
}

QString Param::CellValuesToStrForLog() const
{
	QString ret;
	if(categoryCell != "") ret += categoryCell + " ";
	if(nameCell != "") ret += nameCell + " ";
	if(idCell != "") ret += "(id=" + idCell + ") ";
	if(typeCell != "") ret += typeCell + " ";
	if(addPropsCell != "") ret += addPropsCell + " ";
	while(ret.right(1) == " ") ret.chop(1);
	return ret;
}

void Param::AddCBParamChanged(CBParamChanged cbParamChanged)
{
	if(cbParamChanged.cbFunction && cbParamChanged.cbHandler)
	{
		cbParamChangedVector.push_back(cbParamChanged);
	}
	else Error("AddCBParamChanged: wrong cbParamChanged. cbFunction and cbHandler must not be nullptr");
}

QString Param::GetClassName() const
{
	return IOperand::classParam();
}

QString Param::ToStrForLog() const
{
	return "["+CellValuesToStrForLog()+"]";
}

void Param::Assign(const IOperand *operand2)
{
	QString curValueType { value.GetType() };
	Value newValue(operand2->IOGetValue(curValueType),curValueType);
	value.Set(newValue.GetValue(),newValue.GetType());
}

void Param::PlusAssign(const IOperand *operand2)
{
	Value curValue(value.IOGetValueAndType());
	curValue.PlusAssign(operand2);
	value.Set(curValue.GetValue(),curValue.GetType());
}

bool Param::IsEqual(const IOperand *operand2) const
{
	Error(GetClassName()+"::IsEqual "+ operand2->ToStrForLog()+ " " + CodeMarkers::mock);
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

void Param::SetAllLogsToTextBrowser(QTextBrowser *tBrowser_, const QString &msgPrefix)
{
	this->LogedClass::SetAllLogsToTextBrowser(tBrowser_, msgPrefix);
	value.SetAllLogsToTextBrowser(tBrowser_, msgPrefix);
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
				params[i]->Assign(&tmpVal);
			}
		}
		else Logs::ErrorSt("VectorParams::Assign: sizes different (thisSize="+QSn(thisSize)+" and newValsListSize="+QSn(newValsList.size())+")");
	}
	else Logs::ErrorSt("VectorParams::Assign: empty VectorParams");
}

void VectorParams::PlusAssign(const IOperand *operand2)
{
	Logs::LogSt(GetClassName() + "PlusAssign " + operand2->IOGetType() + CodeMarkers::mock);
}

bool VectorParams::IsEqual(const IOperand *operand2) const
{
	Logs::LogSt(GetClassName() + "IsEqual " + operand2->ToStrForLog() + CodeMarkers::mock);
	return false;
}

std::shared_ptr<IOperand> VectorParams::Index(int index)
{
	Error(GetClassName() + "::Index" + QSn(index) + CodeMarkers::mock);
	return nullptr;
}

IOperand *VectorParams::PointAccess(QString nameToAccess)
{
	Error(GetClassName()+"::PointAccess " + nameToAccess + " " + CodeMarkers::mock);
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
		else if(type1paramVal == ValuesTypes::uByte)		retPair.second = ValuesTypes::vectorUByte;
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
