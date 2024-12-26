#include "frameworker.h"

#include "configconstants.h"

void FrameWorker::InitFrameWorker(const QStringList &config, void *protocolParent_)
{
	if(config.size() >= FrameWorker_ns::Excel::ColsCount)
	{
		categoryCell	= config[FrameWorker_ns::Excel::category];
		nameCell		= config[FrameWorker_ns::Excel::name];
		idCell			= config[FrameWorker_ns::Excel::id_];
		typeCell		= config[FrameWorker_ns::Excel::type];
		addPropsCell	= config[FrameWorker_ns::Excel::addProterties];
		variablesCell	= config[FrameWorker_ns::Excel::variables];
		frameCell		= config[FrameWorker_ns::Excel::frame];
	}
	else Error("FrameWorker::InitFrameWorker wrong attrs size ("+QSn(config.size())+") attrs ["+config.join(";")+"]");

	protocolParent = protocolParent_;
	addProperties.Init(addPropsCell);
}

QString FrameWorker::CellValuesToStrForLog() const
{
	QString ret;
	if(categoryCell != "") ret += categoryCell + " ";
	if(nameCell != "") ret += nameCell + " ";
	if(idCell != "") ret += "(id=" + idCell + ") ";
	if(typeCell != "") ret += typeCell + " ";
	if(addPropsCell != "") ret += addPropsCell + " ";
	if(variablesCell != "") ret += "\n\t" + variablesCell + " ";
	if(frameCell != "") ret += "\n\t" + frameCell + " ";
	while(ret.right(1) == " ") ret.chop(1);
	return ret;
}

QStringList FrameWorker::CellValuesToStringListForEditor() const
{
	QStringList ret;
	for(int i=0; i<FrameWorker_ns::Fields::count; i++) ret.append("");

	ret[FrameWorker_ns::Fields::nameI] = nameCell;
	ret[FrameWorker_ns::Fields::categoryI] = categoryCell;
	ret[FrameWorker_ns::Fields::id_I] = idCell;
	ret[FrameWorker_ns::Fields::typeI] = typeCell;
	ret[FrameWorker_ns::Fields::addProtertiesI] = addPropsCell;
	ret[FrameWorker_ns::Fields::variablesI] = variablesCell;
	ret[FrameWorker_ns::Fields::frameI] = frameCell;

	return ret;
}

void FrameWorker::SetCellValuesFromEditor(const QStringList & values)
{
	if(values.size() == FrameWorker_ns::Fields::count)
	{
		categoryCell		= values[FrameWorker_ns::Fields::categoryI];
		nameCell			= values[FrameWorker_ns::Fields::nameI];
		idCell				= values[FrameWorker_ns::Fields::id_I];
		typeCell			= values[FrameWorker_ns::Fields::typeI];
		addPropsCell		= values[FrameWorker_ns::Fields::addProtertiesI];
		variablesCell		= values[FrameWorker_ns::Fields::variablesI];
		frameCell			= values[FrameWorker_ns::Fields::frameI];
	}
	else Error("FrameWorker::SetCellValuesFromEditor values.size() != Param_ns::Fields::count ("+QSn(values.size())+" != "+QSn(Param_ns::Fields::count)+")");
}

void FrameWorker::SetCellValuesFromDomElement(const QDomElement & frameWorkerElement)
{
	auto attrs = DomAdd::GetAttributes(frameWorkerElement);
	if((int)attrs.size() == FrameWorker_ns::Fields::count)
	{
		for(auto &attr:attrs)
		{
			if(attr.first == FrameWorker_ns::Fields::category) categoryCell = attr.second;
			else if(attr.first == FrameWorker_ns::Fields::name) nameCell = attr.second;
			else if(attr.first == FrameWorker_ns::Fields::id_) idCell = attr.second;
			else if(attr.first == FrameWorker_ns::Fields::type) typeCell = attr.second;
			else if(attr.first == FrameWorker_ns::Fields::addProterties) addPropsCell = attr.second;
			else if(attr.first == FrameWorker_ns::Fields::variables) variablesCell = attr.second;
			else if(attr.first == FrameWorker_ns::Fields::frame) frameCell = attr.second;
			else Error("FrameWorker::SetCellValuesFromDomElement wrong param attribute ["+attr.first+"]");
		}
	}
	else Error("FrameWorker::SetCellValuesFromDomElement wrong attrs size ("+QSn(attrs.size())+")");
}

void FrameWorker::SetCellValue(QString fieldName, QString newValue)
{
	if(fieldName == FrameWorker_ns::Fields::category) categoryCell = newValue;
	else if(fieldName == FrameWorker_ns::Fields::name) nameCell = newValue;
	else if(fieldName == FrameWorker_ns::Fields::id_) idCell = newValue;
	else if(fieldName == FrameWorker_ns::Fields::type) typeCell = newValue;
	else if(fieldName == FrameWorker_ns::Fields::addProterties) addPropsCell = newValue;
	else if(fieldName == FrameWorker_ns::Fields::variables) variablesCell = newValue;
	else if(fieldName == FrameWorker_ns::Fields::frame) frameCell = newValue;
	else Error("FrameWorker::SetCellValue wrong param attribute ["+fieldName+"]");
}

QStringPairVector FrameWorker::GetAttributes() const
{
	QStringPairVector atts;
	atts.push_back({FrameWorker_ns::Fields::category,categoryCell});
	atts.push_back({FrameWorker_ns::Fields::name,nameCell});
	atts.push_back({FrameWorker_ns::Fields::id_,idCell});
	atts.push_back({FrameWorker_ns::Fields::type,typeCell});
	atts.push_back({FrameWorker_ns::Fields::addProterties,addPropsCell});
	atts.push_back({FrameWorker_ns::Fields::variables,variablesCell});
	atts.push_back({FrameWorker_ns::Fields::frame,frameCell});
	return atts;
}

void FrameWorker::AddIdFilter(const CANMsgIDFilter_t &filter)
{
	idFilters.push_back(filter);
	if(idFilters.size() == 1) outgoingID = filter.filter;
}

IOperand *FrameWorker::FindVariable(QString nameToFind, bool printErrorIfNotFind)
{
	for(auto &v:variables)
	{
		if(v->IOGetName() == nameToFind) return v.get();
	}
	if(printErrorIfNotFind) Error("Param::FindVariable can't find name [" + nameToFind+ "]");
	return nullptr;
}

QStringList FrameWorker::GenerateIDStrsList(QString idCommand)
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
	else Logs::ErrorSt("ConstructIDStrList: wrong idCommand" + idCommand);

	return idList;
}

unsigned int FrameWorker::ConstuctID(QString idCommand, const Constants *constants)
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
		else Logs::ErrorSt("ConstuctID: unnnown constant: " + w + " in command " + idCommandIshodn);
	}

	if(id.length() != 29)
		Logs::ErrorSt("ConstuctID: wrong id length: " + QSn(id.length()) + " idCommand: " + idCommand);

	return id.toUInt(nullptr, 2);
}

QString FrameWorker::GetClassName() const { return IOperand::classFrameWorker(); }

QString FrameWorker::ToStrForLog() const { return "["+CellValuesToStrForLog()+"]"; }

void FrameWorker::Assign(const IOperand *operand2) { Error(GetClassName() + "::Assign is forbidden. operand2 " + operand2->ToStrForLog()); }

void FrameWorker::PlusAssign(const IOperand *operand2) { Error(GetClassName() + "::PlusAssign is forbidden. operand2 " + operand2->ToStrForLog()); }

bool FrameWorker::IsEqual(const IOperand *operand2) const { Error(GetClassName() + "::IsEqual is forbidden. operand2 " + operand2->ToStrForLog()); return false; }

std::shared_ptr<IOperand> FrameWorker::Index(int index) { Error(GetClassName() + "::Index is forbidden. index " + QSn(index)); return nullptr; }

IOperand *FrameWorker::PointAccess(QString nameToAccess)
{
	if(PointAccessAll().contains(nameToAccess))
	{
		if(0)
		{

		}
		else Error(GetClassName() + "::PointAccess unrealesed nameToAccess ["+nameToAccess+"]");
	}
	else if(variablesNames.contains(nameToAccess))
	{
		return FindVariable(nameToAccess, true);
	}
	else Error(GetClassName() + "::PointAccess unknown nameToAccess ["+nameToAccess+"]");
	return nullptr;
}

QString FrameWorker::IOGetName() const { Error(GetClassName() + "::IOGetName is forbidden"); return {}; }

QStringPair FrameWorker::IOGetValueAndType() const { Error(GetClassName() + "::IOGetValueAndType is forbidden"); return {}; }

QString FrameWorker::IOGetValue(QString outputValueType) const
{
	Error(GetClassName() + "::IOGetValue is forbidden. outputValueType " + outputValueType);
	return {};
}

QString FrameWorker::IOGetType() const { Error(GetClassName() + "::IOGetType is forbidden");  return {}; }

void FrameWorker::SetAllLogsToTextBrowser(QTextBrowser *tBrowser_, const QString &msgPrefix)
{
	this->LogedClass::SetAllLogsToTextBrowser(tBrowser_, msgPrefix);

	for(auto &var:variables)
		var->SetAllLogsToTextBrowser(tBrowser_, msgPrefix);
}

FrameWorkerPtr::FrameWorkerPtr(FrameWorker *frameWorker_, QString name_)
{
	if(!frameWorker_) Logs::WarningSt("ParamPtr() get nullptr paramPtr");
	name = name_;
	frameWorker = frameWorker_;
}

QString FrameWorkerPtr::IOGetName() const
{
	if(!name.size()) Warning("FrameWorkerPtr:IOGetName, but name empty");
	return name;
}

QStringPair FrameWorkerPtr::IOGetValueAndType() const
{
	return frameWorker->IOGetValueAndType();
}

QString FrameWorkerPtr::IOGetValue(QString outputValueType) const
{
	return frameWorker->IOGetValue(outputValueType);
}

QString FrameWorkerPtr::IOGetType() const
{
	return frameWorker->IOGetType();
}
