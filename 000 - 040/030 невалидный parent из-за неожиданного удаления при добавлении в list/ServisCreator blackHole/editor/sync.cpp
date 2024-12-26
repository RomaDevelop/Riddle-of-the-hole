#include "sync.h"

namespace thisFields = SyncConfigItem_ns::Fields;

QStringList SyncConfigItem::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	FieldToStringList(ret, thisFields::nameIndex,	name	);
	FieldToStringList(ret, thisFields::commandIndex,command	);
	return ret;
}

void SyncConfigItem::SetFieldValues(const QStringList & values)
{
	if(values.size() == FieldsCount())
	{
		name = values[thisFields::nameIndex];
		command = values[thisFields::commandIndex];
	}
	else Logs::ErrorSt(GetClassName() + ":SetFieldValues: values.size() != FieldsCount()");
}

void SyncConfigItem::SetField(QString fieldName, QString newValue)
{
	if(0) {}
	else if(fieldName == thisFields::name) name	= newValue;
	else if(fieldName == thisFields::command) command	= newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong fieldName name ["+fieldName+"]");
}

const QStringList * SyncConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals SyncConfigItem::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = RowCaption();
	tree.ptr = PtrToStr(this);
	return tree;
}
