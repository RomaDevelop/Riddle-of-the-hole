#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

#include "colindexes.h"

#include "param.h"

Param::Param(const QStringList &data, int protocolTip_, Constants *constants_, void *objectParent_):
	objectTip {protocolTip_},
	objectParent {objectParent_}
{
	constants = constants_;

	systemCell			= data[ParamsColIndexes::system];
	categoryCell		= data[ParamsColIndexes::category];
	moduleCell			= data[ParamsColIndexes::module];
	moduleNumberCell	= data[ParamsColIndexes::moduleNumber];
	nameCell			= data[ParamsColIndexes::name];
	idCell				= data[ParamsColIndexes::id_];
	typeCell			= data[ParamsColIndexes::type];
	variablesCell		= data[ParamsColIndexes::strPeremennie];
	frameCell			= data[ParamsColIndexes::frame];
}

std::pair<void *, QString> Param::FindWidget(QString name_)
{
	for(uint i=0; i<widgets.size(); i++)
		if(widgets[i]->ctrl.GetName() == name_) return {&widgets[i]->ctrl, WidgetTypes::whole};

	for(uint windowI=0; windowI<widgets.size(); windowI++)
		for(uint i=0; i<widgets[windowI]->ctrl.parts.size(); i++)
			if(widgets[windowI]->ctrl.parts[i].GetName() == name_)
				return {&widgets[windowI]->ctrl.parts[i], WidgetTypes::part};

	return {nullptr, ""};
}

QString Param::CellValuesToStr()
{
	return	"system="+			systemCell		+
			"; category="+		categoryCell	+
			"; module="+		moduleCell		+
			"; moduleNumber="+	moduleNumberCell+
			"; name="+			nameCell		+
			"; id="+			idCell			+
			"; type="+			typeCell		+
			"; variables="+		variablesCell	+
			"; frame="+			frameCell		+ ";";
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

void Param::Clear()
{
	Log("mock Clear");
}

bool Param::IsEqual(const IOperand *operand2) const
{
	Log("mock Param IsEqual " + operand2->IOGetValueAndTypeAndJoinPair());
	return false;
}

QString Param::IOGetClassName() const
{
	return IOperand::classParam();
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

QString Param::IOGetTypeValue() const
{
	return value.GetType();
}

void Param::InitParamValue()
{
	auto commands {Compiler::TextToCommands(typeCell)};
	if(commands.size() == 1)
	{
		for(auto &cmd:commands)
		{
			auto words {Compiler::CommandToWords(cmd)};
			if(words.empty()) Error("InitByProtocolStr wrong strValueInProtocol words.empty() " + typeCell);
			else
			{
				typeDefined = words[0];

				if(ValuesTypes::all.contains(typeDefined))
				{
					QString initStr = Compiler::GetInitialisationStr(cmd, false);

					if(initStr == ParamsColNames::moduleNumber) initStr = moduleNumberCell;

					if(typeDefined == ValuesTypes::paramPt) Error("InitByProtocolStr Param Value can't be paramPt " + typeCell);
					else
					{
						value.Init(nameCell,initStr,typeDefined);
						value.owner = this;
						value.cbValueChanged = [](Value *sender){
							if(!sender || !sender->owner) sender->Error("cbValueChanged executed, but sender or owner is nullptr");
							else
							{
								Param *paramOwner = static_cast<Param*>(sender->owner);
								for(uint i=0; i<paramOwner->widgets.size(); i++)
									paramOwner->widgets[i]->view.SetWidgetValue(paramOwner->value);
								QString log;
								if(paramOwner->moduleCell!="") log += "(модуль " + paramOwner->moduleCell + ")";
								log += " установлен в [" + paramOwner->value.ToStr()+ "]";
								paramOwner->Log(log);
							}
						};
					}
				}
				else if(FrameTypes::all.contains(typeDefined)) {/* Here do nothing */}
				else
					Error("InitParamValue wrong defined type "+typeDefined+" in param " + CellValuesToStr());
			}
		}
	}
	else Error("InitParamValue: commands.size() != 1");
}

QStringList Param::GenerateIDStrsList(QString idCommand)
{
	QStringList idList;

	if(idCommand.left(CommandsKeyWords::idFilter.length()) == CommandsKeyWords::idFilter)
	{
		idCommand.remove(0,3);
		idCommand.remove('[');
		idCommand.remove(' ');
		QStringList words { idCommand.split(']', QString::SkipEmptyParts) };

		idList += "ID: ";

		for(auto &w:words)
		{
			int oldSize = idList.size();
			if(w.contains('|'))
			{
				QString left {w.left(w.indexOf('|'))};
				QString righ {w.right(w.length() - (left.size()+1))};
				for(int i=0; i<oldSize; i++)
				{
					idList.push_back(idList[i] + '[' + righ + ']');
					idList[i] += '[' + left + ']';
				}
			}
			else
			{
				for(int i=0; i<oldSize; i++)
					idList[i] += '[' + w + ']';
			}
		}
	}
	else LogsSt::ErrorSt("ConstructIDStrList: wrong idCommand" + idCommand);

	return idList;
}

unsigned int Param::ConstuctID(QString idCommand, const Constants *constants)
{
	QString idCommandIshodn = idCommand;
	idCommand.remove(0,3);
	idCommand.remove('[');
	idCommand.remove(' ');
	QStringList words { idCommand.split(']', QString::SkipEmptyParts) };
	QString id;
	QStringList idList;
	for(auto &w:words)
	{
		const Constant *cnst = constants->FindConstant(w);
		if(cnst != nullptr)
		{
			id += cnst->value.ToBinStr();
		}
		else LogsSt::ErrorSt("ConstuctID: unnnown constant: " + w + " in command " + idCommandIshodn);
	}

	if(id.length() != 29)
		LogsSt::ErrorSt("ConstuctID: wrong id length: " + QSn(id.length()) + " idCommand: " + idCommand);

	return id.toUInt(nullptr, 2);
}

Value* Param::FindVariable(QString nameToFind, bool giveErrorIfNotFind)
{
	for(auto &p:variables)
	{
		if(p.GetName() == nameToFind) return &p;
	}
	if(giveErrorIfNotFind) Error("GetPeremennaya can't find name" + nameToFind);
	return nullptr;
}

QString Param::ObjectTypeToStr()
{
	if(objectTip == emulator) return "emulator";
	if(objectTip == servis) return "servis";
	Error("TypeToStr unnown tip" + QSn(objectTip));
	return "unnown tip";
}

void VectorParams::Set(const std::vector<Param *> &params_)
{
	int size = params_.size();
	if(size)
	{
		QStringList types;
		for(int i=0; i<size; i++)
		{
			QString paramT = params_[i]->IOGetTypeValue();
			if(!types.contains(paramT))
				types += paramT;
		}

		int typesSize {types.size()};
		if(typesSize == 1)
			params = params_;
		else if(types.size() > 1) LogsSt::ErrorSt("Set: different values types in params_: " + types.join(';'));
		else LogsSt::ErrorSt("Set: impossible behavior");
	}
	else LogsSt::ErrorSt("Set: empty params_");
}

void VectorParams::Assign(const IOperand *operand2)
{
	int thisSize = params.size();
	if(thisSize)
	{
		QString thisType {IOGetTypeValue()};
		QString type1paramVal { params[0]->IOGetTypeValue() };

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
		else LogsSt::ErrorSt("Assign: sizes different ("+QSn(thisSize)+" "+QSn(newValsList.size())+")");
	}
	else LogsSt::ErrorSt("Assign: empty VectorParams");
}

QStringPair VectorParams::IOGetValueAndType() const
{
	QStringPair retPair {"",ValuesTypes::undefined};
	int size = params.size();
	if(size)
	{
		QString type1paramVal { params[0]->IOGetTypeValue() };
		QString retVal;
		for(int i=0; i<size; i++)
			retPair.first += params[i]->IOGetValueAndType().first + ';';

		if(type1paramVal == ValuesTypes::sByte)			retPair.second = ValuesTypes::vectorSByte;
		else if(type1paramVal == ValuesTypes::sShort)	retPair.second = ValuesTypes::vectorShort;
		else if(type1paramVal == ValuesTypes::sLong)	retPair.second = ValuesTypes::vectorSLong;
		else if(type1paramVal == ValuesTypes::sLLong)	retPair.second = ValuesTypes::vectorSLLong;
		else if(type1paramVal == ValuesTypes::ubool)	retPair.second = ValuesTypes::vectorBool;
		else if(type1paramVal == ValuesTypes::uByte)		retPair.second = ValuesTypes::vectorUByte;
		else if(type1paramVal == ValuesTypes::uShort)	retPair.second = ValuesTypes::vectorUShort;
		else if(type1paramVal == ValuesTypes::uLong)	retPair.second = ValuesTypes::vectorULong;
		else LogsSt::ErrorSt("IOGetValueAndType: unrealesed param type " + type1paramVal);
	}
	else LogsSt::ErrorSt("IOGetValueAndType: empty VectorParams");

	return retPair;
}

QString VectorParams::IOGetValue(QString outputValueType) const
{
	auto curVT {IOGetValueAndType()};
	auto convertRes = Value(curVT.first,curVT.second).ConvertToType(outputValueType);
	if(convertRes.second != "") Error("VectorParams::IOGetValue ConvertToType error: " + convertRes.second);
	return convertRes.first.GetValue();
}

QString VectorParams::IOGetTypeValue() const
{
	QString ret {ValuesTypes::undefined};
	int size = params.size();
	if(size)
	{
		QString t0 {params[0]->IOGetTypeValue()};

		if(t0 == ValuesTypes::sByte)		ret = ValuesTypes::vectorSByte;
		else if(t0 == ValuesTypes::sShort)	ret = ValuesTypes::vectorShort;
		else if(t0 == ValuesTypes::sLong)	ret = ValuesTypes::vectorSLong;
		else if(t0 == ValuesTypes::sLLong)	ret = ValuesTypes::vectorSLLong;
		else if(t0 == ValuesTypes::ubool)	ret = ValuesTypes::vectorBool;
		else if(t0 == ValuesTypes::uByte)	ret = ValuesTypes::vectorUByte;
		else if(t0 == ValuesTypes::uShort)	ret = ValuesTypes::vectorUShort;
		else if(t0 == ValuesTypes::uLong)	ret = ValuesTypes::vectorULong;
		else LogsSt::ErrorSt("IOGetType: unrealesed param type " + ret);
	}
	else LogsSt::ErrorSt("IOGetType: empty VectorParams");
	return ret;
}
