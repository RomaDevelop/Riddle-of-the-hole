#include "otherConfig.h"

namespace thisFields = OtherConfigItem_ns;

QStringList OtherConfigItem::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	FieldToStringList(ret, thisFields::contentIndex,	content	);
	return ret;
}

void OtherConfigItem::SetFieldValues(const QStringList & values)
{
	if(values.size() == FieldsCount())
	{
		content = values[thisFields::contentIndex];
	}
	else Logs::ErrorSt(GetClassName() + ":SetFieldValues: values.size() != FieldsCount()");
}

void OtherConfigItem::SetField(QString fieldName, QString newValue)
{
	if(0) {}
	else if(fieldName == thisFields::content)	content	= newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong fieldName name ["+fieldName+"]");
}

const QStringList * OtherConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals OtherConfigItem::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = RowCaption();
	tree.ptr = PtrToStr(this);
	return tree;
}
