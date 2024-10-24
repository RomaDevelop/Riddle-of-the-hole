#include "objectcongif.h"

#include "configitemfabric.h"

ObjectClass::ObjectClass(const QDomElement & objClassElement)
{
	Set(objClassElement);
}

int ObjectClass::FieldsCount() const
{
	return ObjectClass_ns::Fields::count;
}

QStringList ObjectClass::FieldsNames() const
{
	return ObjectClass_ns::Fields::all;
}

QStringList ObjectClass::FieldsValues() const
{
	return { objectClassName };
}

void ObjectClass::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
	{
		objectClassName = values[ObjectClass_ns::Fields::nameIndex];
		if(values.size() > FieldsCount())
		{
			for(int i = FieldsCount(); i<values.size(); i++)
				if(values[i] != "")
					Logs::WarningSt(GetClassName()+" unused value ["+values[i]+"]");
		}
	}
	else Logs::ErrorSt(GetClassName()+"::SetFieldValues values.size() < FieldsCount()");
}

void ObjectClass::SetField(QString fieldName, QString newValue)
{
	if(fieldName == ObjectClass_ns::Fields::name)
		objectClassName = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong "+ObjectClass_ns::ObjectClass+" attribute name ["+fieldName+"]");
}

void ObjectClass::Set(const QDomElement & element)
{
	auto attrs = DomAdd::GetAttributes(element);
	if((int)attrs.size() != ObjectClass_ns::Fields::count)
		Logs::ErrorSt(GetClassName()+"::Set wrong "+element.tagName()+" attrs size ("+QSn(attrs.size())+")");
	for(auto &attr:attrs)
	{
		if(attr.first == ObjectClass_ns::Fields::name)
			objectClassName = attr.second;
		else Logs::ErrorSt(GetClassName()+"::Set wrong "+ObjectClass_ns::ObjectClass+" attribute name ["+attr.first+"]");
	}

	auto subElements = DomAdd::GetTopLevelElements(element);
	for(auto &subElement:subElements)
	{
		const QStringList* subItemTypes = WhatCanBeAdded();
		if(subItemTypes)
		{
			if(subItemTypes->contains(subElement.tagName()))
			{
				auto newSubItem = ConfigItemFabric::Make(subElement.tagName());
				if(newSubItem)
				{
					newSubItem->Set(subElement);
					subItems.push_back(newSubItem);
				}
				else Logs::ErrorSt(GetClassName() + "::Set ConfigItemFabric::Make from tag ["+subElement.tagName()+"] result is nullptr");
			}
			else Logs::ErrorSt(GetClassName()+"::Set wrong tag ["+subElement.tagName()+"]");
		}
		else Logs::ErrorSt(GetClassName()+"::WhatCanBeAdded result is nullptr");
	}
}

const QStringList * ObjectClass::WhatCanBeAdded() const
{
	return &ObjectClass_ns::canBeAdded;
}

void ObjectClass::AddSubItem(const QString & subItemType)
{
	const QStringList* subItemTypes = WhatCanBeAdded();
	if(subItemTypes)
	{
		if(subItemTypes->contains(subItemType))
		{
			auto newSubItem = ConfigItemFabric::Make(subItemType);
			if(newSubItem)
			{
				subItems.push_back(newSubItem);
			}
			else Logs::ErrorSt(GetClassName() + "::Set ConfigItemFabric::Make from tag ["+subItemType+"] result is nullptr");
		}
		else Logs::ErrorSt(GetClassName()+"::AddSubItem wrong subItemType [" + subItemType + "]");
	}
	else Logs::ErrorSt(GetClassName()+"::WhatCanBeAdded result is nullptr");
}

std::vector<IConfigItem *> ObjectClass::SubDefinitions()
{
	std::vector<IConfigItem*> ret;
	for(auto &subItem:subItems)
	{
		ret.push_back(subItem.get());
	}
	return ret;
}

QString ObjectClass::ToStrForConfig() const
{
	QDomDocument xml;
	QDomElement objectClassXml = xml.createElement(ObjectClass_ns::ObjectClass);
	xml.appendChild(objectClassXml);
	objectClassXml.setAttribute(ObjectClass_ns::Fields::name, objectClassName);
	for(auto &subItem:subItems)
	{
//		QDomElement paramXml = xml.createElement(subItem->RowCaption());
//		auto fieldNames = subItem->FieldsNames();
//		auto fieldVals = subItem->FieldsValues();
//		auto paramAtts = param.param.GetAttributes();
//		for(auto &pair:paramAtts)
//		{
//			paramXml.setAttribute(pair.first, pair.second);
//		}
//		objectClassXml.appendChild(paramXml);
		objectClassXml.appendChild(subItem->ToDomElement(xml));
	}
	return xml.toString();
}

TreeItemVals ObjectClass::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = GetClassName() + " " + objectClassName;
	tree.ptr = PtrToStr(this);
	return tree;
}

void PraramConfigItem::SetField(QString fieldName, QString newValue)
{
	param.SetCellValue(fieldName,newValue);
}

void PraramConfigItem::Set(const QDomElement & element)
{
	param.SetCellValuesFromDomElement(element);
}

const QStringList * PraramConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

void PraramConfigItem::AddSubItem(const QString & subItemType)
{
	Logs::ErrorSt(GetClassName() + "::AddSubItem forbidden " + subItemType);
}

std::vector<IConfigItem *> PraramConfigItem::SubDefinitions()
{
	Logs::ErrorSt(GetClassName() + "::SubDefinitions forbidden");
	return {};
}

QString PraramConfigItem::ToStrForConfig() const
{
	Logs::ErrorSt(GetClassName() + "::ToStrForConfig forbidden");
	return {};
}

TreeItemVals PraramConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}
