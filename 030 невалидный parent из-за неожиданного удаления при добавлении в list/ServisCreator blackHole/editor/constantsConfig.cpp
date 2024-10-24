#include "constantsConfig.h"

#include "MyQDom.h"

#include "configItemFabric.h"

ConstantsConfigItem::ConstantsConfigItem(const QDomElement & element)
{
	Set(element);
}

int ConstantsConfigItem::FieldsCount() const
{
	return ConstantsVector_ns::count;
}

QStringList ConstantsConfigItem::FieldsNames() const
{
	return ConstantsVector_ns::all;
}

QStringList ConstantsConfigItem::FieldsValues() const
{
	return { name };
}

void ConstantsConfigItem::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
	{
		name = values[ConstantsVector_ns::nameIndex];
		if(values.size() > FieldsCount())
		{
			for(int i = FieldsCount(); i<values.size(); i++)
				if(values[i] != "")
					Logs::WarningSt(GetClassName()+" unused value ["+values[i]+"]");
		}
	}
	else Logs::ErrorSt(GetClassName()+"::SetFieldValues values.size() < FieldsCount()");
}

void ConstantsConfigItem::SetField(QString fieldName, QString newValue)
{
	if(fieldName == ConstantsVector_ns::name)
		name = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong field name ["+fieldName+"]");
}

const QStringList * ConstantsConfigItem::WhatCanBeAdded() const
{
	return &ConstantsVector_ns::canBeAdded;
}

TreeItemVals ConstantsConfigItem::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = Constant_ns::rowCaptionConstants + " " + name;
	tree.ptr = PtrToStr(this);
	return tree;
}

int ConstantConfigItem::FieldsCount() const
{
	return Constant_ns::count;
}

QStringList ConstantConfigItem::FieldsNames() const
{
	return Constant_ns::all;
}

const QStringList * ConstantConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals ConstantConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}


