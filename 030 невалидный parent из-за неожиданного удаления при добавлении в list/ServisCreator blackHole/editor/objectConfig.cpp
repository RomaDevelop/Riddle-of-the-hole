#include "objectConfig.h"

#include "configItemFabric.h"
#include "MyQDom.h"

ObjectClass::ObjectClass(const QDomElement & objClassElement)
{
	Set(objClassElement);
}

int ObjectClass::FieldsCount() const
{
	return ObjectClass_ns::count;
}

QStringList ObjectClass::FieldsNames() const
{
	return ObjectClass_ns::all;
}

QStringList ObjectClass::FieldsValues() const
{
	return { objectClassName };
}

void ObjectClass::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
	{
		objectClassName = values[ObjectClass_ns::nameIndex];
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
	if(fieldName == ObjectClass_ns::name)
		objectClassName = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong "+ObjectClass_ns::ObjectClass+" attribute name ["+fieldName+"]");
}

const QStringList * ObjectClass::WhatCanBeAdded() const
{
	return &ObjectClass_ns::canBeAdded;
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

const QStringList * PraramConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals PraramConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}

QStringList ObjectConfigItem::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	namespace Fields = ObjectConfigItem_ns;
	FieldToStringList(ret, Fields::objectClassIndex,	objectClass		);
	FieldToStringList(ret, Fields::protocolClassIndex,	protocolClass	);
	FieldToStringList(ret, Fields::constantsIndex,		constants		);
	FieldToStringList(ret, Fields::typeIndex,			type			);
	FieldToStringList(ret, Fields::nameIndex,			name			);
	FieldToStringList(ret, Fields::channelIndex,		channel			);
	FieldToStringList(ret, Fields::startSettingsIndex,	startSettings	);
	return ret;
}

void ObjectConfigItem::SetFieldValues(const QStringList & values)
{
	namespace Fields = ObjectConfigItem_ns;
	if(values.size() == Fields::all.size())
	{
		objectClass		= values[Fields::objectClassIndex];
		protocolClass	= values[Fields::protocolClassIndex];
		constants		= values[Fields::constantsIndex];
		type			= values[Fields::typeIndex];
		name			= values[Fields::nameIndex];
		channel			= values[Fields::channelIndex];
		startSettings	= values[Fields::startSettingsIndex];
	}
	else Logs::ErrorSt(GetClassName() + "::SetFieldValues wrong size (get "
					   +QSn(values.size())+" but must be "+QSn(Fields::all.size())+")");
}

void ObjectConfigItem::SetField(QString fieldName, QString newValue)
{
	namespace Fields = ObjectConfigItem_ns;
	if(fieldName == Fields::objectClass	) objectClass	= newValue;
	else if(fieldName == Fields::protocolClass	) protocolClass = newValue;
	else if(fieldName == Fields::constants		) constants		= newValue;
	else if(fieldName == Fields::type			) type			= newValue;
	else if(fieldName == Fields::name			) name			= newValue;
	else if(fieldName == Fields::channel		) channel		= newValue;
	else if(fieldName == Fields::startSettings	) startSettings = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong fieldName name ["+fieldName+"]");
}

const QStringList * ObjectConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded unrealesed");
	return nullptr;
}

TreeItemVals ObjectConfigItem::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = RowCaption() + " " + name;
	tree.ptr = PtrToStr(this);
	return tree;
}
