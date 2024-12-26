#include "channelConfig.h"

namespace thisFields = ChannelConfigItem_ns::Fields;

QStringList ChannelConfigItem::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	FieldToStringList(ret, thisFields::nameIndex,	name	);
	FieldToStringList(ret, thisFields::typeIndex,	type	);
	FieldToStringList(ret, thisFields::paramsIndex,	params	);
	return ret;
}

void ChannelConfigItem::SetFieldValues(const QStringList & values)
{
	if(values.size() == FieldsCount())
	{
		name = values[thisFields::nameIndex];
		type = values[thisFields::typeIndex];
		params = values[thisFields::paramsIndex];
	}
	else Logs::ErrorSt(GetClassName() + ":SetFieldValues: values.size() != FieldsCount()");
}

void ChannelConfigItem::SetField(QString fieldName, QString newValue)
{
	if(0) {}
	else if(fieldName == thisFields::name) name	= newValue;
	else if(fieldName == thisFields::type) type	= newValue;
	else if(fieldName == thisFields::params) params	= newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong fieldName name ["+fieldName+"]");
}

const QStringList * ChannelConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals ChannelConfigItem::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = RowCaption();
	tree.ptr = PtrToStr(this);
	return tree;
}
