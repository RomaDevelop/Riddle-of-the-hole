#include "worker.h"
#include "MyQDom.h"

void Worker::InitWorker(const QDomElement &workerElement, Constants *constants, HaveClassName *aProtocolParent, int aProtocolType)
{
	auto attrs = MyQDom::Attributes(workerElement);
	if((int)attrs.size() == Worker_ns::count)
	{
		for(auto &attr:attrs)
		{
			if(attr.first == Worker_ns::category) categoryCell = attr.second;
			else if(attr.first == Worker_ns::name) nameCell = attr.second;
			else if(attr.first == Worker_ns::id_) idCell = attr.second;
			else if(attr.first == Worker_ns::type) typeCell = attr.second;
			else if(attr.first == Worker_ns::addProps) addPropsCell = attr.second;
			else if(attr.first == Worker_ns::variables) variablesCell = attr.second;
			else if(attr.first == Worker_ns::frame) frameCell = attr.second;
			else Error(GetClassName()+"::InitWorker wrong param attribute ["+attr.first+"]");
		}
	}
	else Error(GetClassName()+"::InitWorker wrong attrs size ("+QSn(attrs.size())+")");

	auto subElements = MyQDom::GetTopLevelElements(workerElement);
	if(subElements.size()) Error(GetClassName()+"::InitWorker wrong subElements size ("+QSn(subElements.size())+")");

	this->constants = constants;
	protocolParent = aProtocolParent;
	protocolType = aProtocolType;
	addProperties.Init(addPropsCell);
	instructions = Code::TextToCommands(frameCell);
	Code::RemoveEmulatorServis(instructions,protocolType);
}

void Worker::AddVariable(std::shared_ptr<IOperand> variable, QString variableName)
{
	if(constants)
	{
		if(auto constant = constants->FindConstant(variableName))
		{
			Error(GetClassName()+" AddVariable name ["+variableName+"] exists in constants ("+constant->ToStr()+")");
			return;
		}
	}
	variables.push_back(variable);
	variablesNamesMap[variableName] = variable.get();
}

QString Worker::CellValuesToStrForLog() const
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

void Worker::AddIdFilter(const ID &id)
{
	idFilters.push_back(CANMsgIDFilter_t(id.id,0xFFFFFFFF));
	if(idFilters.size() == 1)
	{
		idFilterFragmentsLengths = id.idFragmentsLengths;
		outgoingID = id.id;
	}
}

void Worker::ClearFilters()
{
	idFilters.clear();
	idFilterFragmentsLengths.clear();
	outgoingID = defaultOutgoingID;
	dataFilter = CANMsgDataFilter_t();
}

IOperand *Worker::FindVariable(QString nameToFind, bool printErrorIfNotFind)
{
	if(auto resFind = variablesNamesMap.find(nameToFind); resFind != variablesNamesMap.end())
		return resFind->second;

	if(printErrorIfNotFind) Error("Param::FindVariable can't find name [" + nameToFind+ "]");
	return nullptr;
}

QStringList Worker::GenerateIDStrsList(QString idCommand)
{
	QStringList idList;

	if(idCommand.left(CodeKeyWords::idFilter.length()) == CodeKeyWords::idFilter)
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

ID Worker::ConstuctID(QString idCommand, const Constants *constants)
{
	QString idCommandIshodn = idCommand;
	idCommand.remove(0,3);
	idCommand.remove('[');
	idCommand.remove(' ');
	QStringList words { idCommand.split(']', QString::SkipEmptyParts) };
	ID id;
	QString idStr;
	QStringList idList;
	for(auto &w:words)
	{
		const Constant *constant = constants->FindConstant(w);
		if(constant != nullptr)
		{
			QString binStr = constant->value.ToBinStr();
			idStr += binStr;
			id.idFragmentsLengths.push_back(binStr.length());
		}
		else Logs::ErrorSt("ConstuctID: unnnown constant: " + w + " in command " + idCommandIshodn);
	}

	const int idSizeMustBe = 29;
	if(idStr.length() != idSizeMustBe)
		Logs::ErrorSt("ConstuctID: wrong id length: " + QSn(idStr.length()) + "(must be "+QSn(idSizeMustBe)+") idCommand: " + idCommand);

	id.id = idStr.toUInt(nullptr, 2);
	return id;
}

QString Worker::GetClassName() const { return IOperand::classWorker(); }

QString Worker::ToStrForLog() const { return categoryCell+"::"+nameCell; }

void Worker::Assign(const IOperand *operand2) { Error(GetClassName() + "::Assign is forbidden. operand2 " + operand2->ToStrForLog()); }

std::shared_ptr<IOperand> Worker::Minus(const IOperand * operand2)
{
	Error(GetClassName() + "::FrameWorker is forbidden. operand2 " + operand2->ToStrForLog());
	return nullptr;
}

void Worker::PlusAssign(const IOperand *operand2)
{
	Error(GetClassName() + "::PlusAssign is forbidden. operand2 " + operand2->ToStrForLog());
}

bool Worker::CmpOperation(const IOperand *operand2, QString operationCode) const
{
	Error(GetClassName() + "::IsEqual is forbidden. operand2 " + operand2->ToStrForLog() + " " + operationCode);
	return false;
}

std::shared_ptr<IOperand> Worker::Index(int index)
{ Error(GetClassName() + "::Index is forbidden. index " + QSn(index));
	return nullptr;
}

IOperand *Worker::PointAccess(QString nameToAccess)
{
	if(PointAccessAll().contains(nameToAccess))
	{
		if(0) { }
		else if(0/*nameToAccess == SendFrame*/)
		{
			qdbg << CodeMarkers::note;	// нельзя создать FrameWorker PointAccess SendFrame потому что при создании фрейма
										//вызывается Protocol::DoEverything
		}
		else Error(GetClassName() + "::PointAccess unrealesed nameToAccess ["+nameToAccess+"]");
	}
	else if(auto resFind = FindVariable(nameToAccess, false); resFind != nullptr)
	{
		return resFind;
	}
	else Error(GetClassName() + "::PointAccess unknown nameToAccess ["+nameToAccess+"]");
	return nullptr;
}

QString Worker::IOGetName() const { Error(GetClassName() + "::IOGetName is forbidden"); return {}; }

QStringPair Worker::IOGetValueAndType() const { Error(GetClassName() + "::IOGetValueAndType is forbidden"); return {}; }

QString Worker::IOGetValue(QString outputValueType) const
{
	Error(GetClassName() + "::IOGetValue is forbidden. outputValueType " + outputValueType);
	return {};
}

QString Worker::IOGetType() const { Error(GetClassName() + "::IOGetType is forbidden");  return {}; }

WorkerPtr::WorkerPtr(Worker *frameWorker_, QString name_)
{
	if(!frameWorker_) Logs::WarningSt("ParamPtr() get nullptr paramPtr");
	name = name_;
	worker = frameWorker_;
}

QString WorkerPtr::IOGetName() const
{
	if(!name.size()) Warning("FrameWorkerPtr:IOGetName, but name empty");
	return name;
}

QStringPair WorkerPtr::IOGetValueAndType() const
{
	return worker->IOGetValueAndType();
}

QString WorkerPtr::IOGetValue(QString outputValueType) const
{
	return worker->IOGetValue(outputValueType);
}

QString WorkerPtr::IOGetType() const
{
	return worker->IOGetType();
}
