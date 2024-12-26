#include "protocolconfig.h"

ProtocolClass::ProtocolClass(const QDomElement & element)
{
	Set(element);
}

int ProtocolClass::FieldsCount() const
{
	return ProtocolClass_ns::Fields::count;
}

QStringList ProtocolClass::FieldsNames() const
{
	return ProtocolClass_ns::Fields::all;
}

QStringList ProtocolClass::FieldsValues() const
{
	return { protocolClassName };
}

void ProtocolClass::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
	{
		protocolClassName = values[ProtocolClass_ns::Fields::nameIndex];
		if(values.size() > FieldsCount())
		{
			for(int i = FieldsCount(); i<values.size(); i++)
				if(values[i] != "")
					Logs::WarningSt(GetClassName()+" unused value ["+values[i]+"]");
		}
	}
	else Logs::ErrorSt(GetClassName()+"::SetFieldValues values.size() < FieldsCount()");
}

void ProtocolClass::SetField(QString fieldName, QString newValue)
{
	if(fieldName == ProtocolClass_ns::Fields::name)
		protocolClassName = newValue;
	else Logs::ErrorSt("ProtocolClass::SetField wrong "+ProtocolClass_ns::ProtocolClass+" attribute name ["+fieldName+"]");
}

void ProtocolClass::Set(const QDomElement & element)
{
	auto attrs = DomAdd::GetAttributes(element);
	if((int)attrs.size() != ProtocolClass_ns::Fields::count)
		Logs::ErrorSt("ProtocolClass::Constructor wrong "+element.tagName()+" attrs size ("+QSn(attrs.size())+")");
	for(auto &attr:attrs)
	{
		if(attr.first == ProtocolClass_ns::Fields::name)
			protocolClassName = attr.second;
		else Logs::ErrorSt("ProtocolClass::Constructor wrong "+element.tagName()+" attribute name ["+attr.first+"]");
	}

	auto protElements = DomAdd::GetTopLevelElements(element);
	for(auto &frWorkerElement:protElements)
	{
		if(frWorkerElement.tagName() == FrameWorker_ns::FrameWorker)
		{
			frameWorkers.push_back(FrameWorkerConfigItem());
			frameWorkers.back().Set(frWorkerElement);
		}
		else Logs::ErrorSt("ProtocolClass::Constructor wrong tag ["+frWorkerElement.tagName()+"]");
	}
}

const QStringList * ProtocolClass::WhatCanBeAdded() const
{
	return &ProtocolClass_ns::canBeAdded;
}

void ProtocolClass::AddSubItem(const QString & subItemType)
{
	if(subItemType == ProtocolClass_ns::FrameWorker)
	{
		frameWorkers.push_back(FrameWorkerConfigItem());
	}
	else Logs::ErrorSt(GetClassName()+"::AddSubItem wrong subItemType [" + subItemType + "]");
}

std::vector<IConfigItem *> ProtocolClass::SubDefinitions()
{
	std::vector<IConfigItem*> ret;
	for(auto &frWorker:frameWorkers)
	{
		ret.push_back(&frWorker);
	}
	return ret;
}

QString ProtocolClass::ToStrForConfig() const
{
	QDomDocument xml;
	QDomElement protClassXml = xml.createElement(ProtocolClass_ns::ProtocolClass);
	xml.appendChild(protClassXml);
	protClassXml.setAttribute(ProtocolClass_ns::Fields::name, protocolClassName);
	for(auto &frWorker:frameWorkers)
	{
		QDomElement frWorkerXml = xml.createElement(FrameWorker_ns::FrameWorker);
		auto frWorkerAttrs = frWorker.frameWorker.GetAttributes();
		for(auto &pair:frWorkerAttrs)
		{
			frWorkerXml.setAttribute(pair.first, pair.second);
		}
		protClassXml.appendChild(frWorkerXml);
	}
	return xml.toString();
}

TreeItemVals ProtocolClass::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = GetClassName() + " " + protocolClassName;
	tree.ptr = PtrToStr(this);
	return tree;
}

void FrameWorkerConfigItem::SetField(QString fieldName, QString newValue)
{
	frameWorker.SetCellValue(fieldName,newValue);
}

void FrameWorkerConfigItem::Set(const QDomElement & element)
{
	frameWorker.SetCellValuesFromDomElement(element);
}

const QStringList * FrameWorkerConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

void FrameWorkerConfigItem::AddSubItem(const QString & subItemType)
{
	Logs::ErrorSt(GetClassName() + "::AddSubItem forbidden " + subItemType);
}

std::vector<IConfigItem *> FrameWorkerConfigItem::SubDefinitions()
{
	Logs::ErrorSt(GetClassName() + "::SubDefinitions forbidden");
	return {};
}

QString FrameWorkerConfigItem::ToStrForConfig() const
{
	Logs::ErrorSt(GetClassName() + "::ToStrForConfig forbidden");
	return {};
}

TreeItemVals FrameWorkerConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}
