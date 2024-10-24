#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

#include "parametr.h"

namespace ParametrColIndexes {
	const int system {1};
	const int module {2};
	const int category {3};
	const int name {4};
	const int id_ {5}; // если без _ почемуто ниже подсвечивает синим
	const int type {6};
	const int strPeremennie {7};
	const int frame {8};
}

const int Parametr::maxIndexDataForParametr {12};
Parametr::Parametr(const QStringList &data, int protocolTip_, Constants *constants_):
	objectTip {protocolTip_}
{
	constants = constants_;

	system			= data[ParametrColIndexes::system];
	module			= data[ParametrColIndexes::module];
	category		= data[ParametrColIndexes::category];
	name			= data[ParametrColIndexes::name];
	id				= data[ParametrColIndexes::id_];
	strType			= data[ParametrColIndexes::type];
	strPeremennie	= data[ParametrColIndexes::strPeremennie];
	frame			= data[ParametrColIndexes::frame];

	InitParamValue();
}

void Parametr::AddWidgetStrs(const QStringList &widgRow)
{
	strViewWidgs.push_back(widgRow[4]);
	strCtrlWidgs.push_back(widgRow[5]);
}

std::pair<void *, QString> Parametr::FindWidget(QString name_, int windowIndex)
{
	if(ctrlWidgs[windowIndex].GetName() == name_) return {&ctrlWidgs[windowIndex], WidgetTypes::whole};

	for(uint i=0; i<ctrlWidgs[windowIndex].parts.size(); i++)
		if(ctrlWidgs[windowIndex].parts[i].GetName() == name_) return {&ctrlWidgs[windowIndex].parts[i], WidgetTypes::part};

	return {nullptr, ""};
}

QString Parametr::ToStr()
{
	QString viewWidgsVals;
	for(auto &v:strViewWidgs) viewWidgsVals += v + ";";
	QString ctrlWidgsVals;
	for(auto &v:strCtrlWidgs) ctrlWidgsVals += v + ";";

	return	"system="+		system		+
			"; module="+	module		+
			"; category="+	category	+
			"; name="+		name		+
			"; type="+		strType		+
			"; infoFrame="+	frame		+
			"; viewWidgS="+	viewWidgsVals	+
			"; ctrlWidgS=" + ctrlWidgsVals + ";";
}

void Parametr::SetValue(QString newValue, QString valueType)
{
	value.Set(newValue, valueType);
	for(auto &w:viewWidgets) w.SetWidgetValue(value.ToStr());
	QString log;
	if(module!="") log += "(модуль " + module + ")";
	log += " установлен в " + GetParamValue().ToStr();
	Log(log);
}

void Parametr::SetValue(const Value &value_)
{
	auto vt {value_.IOGetValueAndType()};
	SetValue(vt.first, vt.second);
}

void Parametr::InitFilter(CANMsgIDFilter_t &filter, QString IDcommand, const Constants *constants)
{
	filter.mask = 0xFFFFFFFF;
	filter.filter = ConstuctID(IDcommand, constants);
}

void Parametr::InitFilters()
{
	if(frame != "")
	{
		QString idCommand { Compiler::TextToCommands(frame)[0] };
		incomingFiltersStrs = { ConstructIDStrList(idCommand) };
		for(int i=0; i<incomingFiltersStrs.size(); i++)
		{
			incomingFilters.push_back(CANMsgIDFilter_t());
			InitFilter(incomingFilters.back(),incomingFiltersStrs[i],constants);
		}
		if(incomingFilters.size())
		{
			outgoingID = incomingFilters[0].filter;
			outgoingIDStr = incomingFiltersStrs[0];
		}
	}
}

void Parametr::InitParamValue()
{
	auto commands {Compiler::TextToCommands(strType)};
	for(auto &cmd:commands)
	{
		auto words {Compiler::CommandToWords(cmd)};
		if(words.empty()) Error("InitByProtocolStr wrong strValueInProtocol words.empty() " + strType);
		else
		{
			QString type {words[0]};

			if(ValuesTypes::all.contains(type))
			{
				QString initValue;
				if(words.size() == 2) initValue = Compiler::GetInitialisationVal(strType);

				if(type == ValuesTypes::paramPt) Error("InitByProtocolStr Param Value can't be paramPt " + strType);
				else value.Init(name,initValue,type);
			}
			else if(FrameTypes::all.contains(type)) {/* Here do nothing */}
			else Error("InitParamValue wrong strType " + strType);
		}
	}
}

QStringList Parametr::ConstructIDStrList(QString idCommand)
{
	idCommand.remove(0,3);
	idCommand.remove('[');
	idCommand.remove(' ');
	QStringList words { idCommand.split(']', QString::SkipEmptyParts) };

	QStringList idList;
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

	return idList;
}

unsigned int Parametr::ConstuctID(QString idCommand, const Constants *constants)
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
		else ErrorSt("ConstuctID: unnnown constant: " + w + "in command " + idCommandIshodn);
	}
	if(id.length() != 29) ErrorSt("ConstuctID: wrong id length: " + QSn(id.length()));

	return id.toUInt(nullptr, 2);
}

Value* Parametr::FindPeremennaya(QString nameToFind, bool giveErrorIfNotFind)
{
	for(auto &p:peremennie)
	{
		if(p.GetName() == nameToFind) return &p;
	}
	if(giveErrorIfNotFind) Error("GetPeremennaya can't find name" + nameToFind);
	return nullptr;
}

void Parametr::DoObrabotchik(QString obrabotchik, QString senderName, int windowIndex)
{
	auto words = Compiler::CommandToWords(obrabotchik);
	if(words.size() < 5) Error("DoObrabotchik CtrlWidg Wrong widget command obrabotchik not enougth words " + obrabotchik);

	QString nameIniter = words[1];
	QString subj = words[2];
	QString oper = words[3];
	QString pred = words[4];

	if(nameIniter == senderName)
	{
		Value *valSubjPt = FindPeremennaya(subj,false);
		if(valSubjPt)
		{
			Value valPred;
			Value *valPredPt = &valPred;
			if(pred == Operators::clear) valPredPt->Set("", valSubjPt->IOGetValueAndType().second);
			else
			{
				WidgPart *predVidg { this->ctrlWidgs[windowIndex].FindWidgPartByName(pred) };
				if(predVidg) valPred.Set(predVidg->GetWidgPartValue());
				else Error("DoObrabotchiki can't define predicate " + pred);
			}

			if(oper == Operators::assign)
			{
				valSubjPt->Set(valPred);
			}
			else if(oper == Operators::plusAssign)
			{
				valSubjPt->PlusAssign(valPred);
			}
			else Error("DoObrabotchiki unrealesed operation " + obrabotchik);
		}
		else Error("DoObrabotchik can't find peremennaya " + subj);
	}
}

void Parametr::DoObrabotchiki(int windowIndex, const QString senderName)
{
	int size = this->ctrlWidgs[windowIndex].obrabotchiki.size();
	for(int i=0; i<size; i++)
	{
		DoObrabotchik(this->ctrlWidgs[windowIndex].obrabotchiki[i], senderName, windowIndex);
	}
}

QString Parametr::TypeToStr()
{
	if(objectTip == emulator) return "emulator";
	if(objectTip == servis) return "servis";
	Error("TypeToStr unnown tip" + QSn(objectTip));
	return "unnown tip";
}
