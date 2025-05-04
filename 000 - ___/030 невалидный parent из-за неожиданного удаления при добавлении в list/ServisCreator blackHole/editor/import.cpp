#include "import.h"

namespace thisNs = Import_ns;

QStringList ImportConfigItem::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	FieldToStringList(ret, thisNs::nameIndex,				name	);
	FieldToStringList(ret, thisNs::typeIndex,				type	);
	FieldToStringList(ret, thisNs::policyIndex,				policy	);
	FieldToStringList(ret, thisNs::modificationIndex,		modification	);
	FieldToStringList(ret, thisNs::postModificationIndex,	postModification);
	return ret;
}

void ImportConfigItem::SetFieldValues(const QStringList & values)
{
	if(values.size() == FieldsCount())
	{
		type =				values[thisNs::typeIndex];
		name =				values[thisNs::nameIndex];
		policy =			values[thisNs::policyIndex];
		modification =		values[thisNs::modificationIndex];
		postModification =	values[thisNs::postModificationIndex];
	}
	else Logs::ErrorSt(GetClassName() + ":SetFieldValues: values.size() != FieldsCount()");
}

void ImportConfigItem::SetField(QString fieldName, QString newValue)
{
	if(		fieldName == thisNs::type				) type	= newValue;
	else if(fieldName == thisNs::name				) name = newValue;
	else if(fieldName == thisNs::policy				) policy = newValue;
	else if(fieldName == thisNs::modification		) modification = newValue;
	else if(fieldName == thisNs::postModification	) postModification = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong fieldName name ["+fieldName+"]");
}

const QStringList * ImportConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals ImportConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}
