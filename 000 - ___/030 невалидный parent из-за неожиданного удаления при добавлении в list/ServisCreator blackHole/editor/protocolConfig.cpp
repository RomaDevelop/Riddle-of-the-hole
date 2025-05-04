#include "protocolConfig.h"
#include "MyQDom.h"

ProtocolClass::ProtocolClass(const QDomElement & element)
{
	Set(element);
}

int ProtocolClass::FieldsCount() const
{
	return ProtocolClass_ns::count;
}

QStringList ProtocolClass::FieldsNames() const
{
	return ProtocolClass_ns::all;
}

QStringList ProtocolClass::FieldsValues() const
{
	return { protocolClassName };
}

void ProtocolClass::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
	{
		protocolClassName = values[ProtocolClass_ns::nameIndex];
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
	if(fieldName == ProtocolClass_ns::name)
		protocolClassName = newValue;
	else Logs::ErrorSt("ProtocolClass::SetField wrong "+ProtocolClass_ns::ProtocolClass+" attribute name ["+fieldName+"]");
}

const QStringList * ProtocolClass::WhatCanBeAdded() const
{
	return &ProtocolClass_ns::canBeAdded;
}

TreeItemVals ProtocolClass::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = GetClassName() + " " + protocolClassName;
	tree.ptr = PtrToStr(this);
	return tree;
}

QStringList WorkerConfigItem::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	FieldToStringList(ret, Worker_ns::nameI		,	name		);
	FieldToStringList(ret, Worker_ns::categoryI	,	category	);
	FieldToStringList(ret, Worker_ns::id_I			,	id			);
	FieldToStringList(ret, Worker_ns::typeI		,	type		);
	FieldToStringList(ret, Worker_ns::addPropsI	,	addProps	);
	FieldToStringList(ret, Worker_ns::variablesI	,	variables	);
	FieldToStringList(ret, Worker_ns::frameI		,	frame		);
	return ret;
}

void WorkerConfigItem::SetFieldValues(const QStringList & values)
{
	if(values.size() == FieldsCount())
	{
		name		= values[Worker_ns::nameI		];
		category	= values[Worker_ns::categoryI	];
		id			= values[Worker_ns::id_I		];
		type		= values[Worker_ns::typeI		];
		addProps	= values[Worker_ns::addPropsI	];
		variables   = values[Worker_ns::variablesI	];
		frame		= values[Worker_ns::frameI		];
	}
	else Logs::ErrorSt(GetClassName() + ":SetFieldValues: values.size() != FieldsCount()");
}

void WorkerConfigItem::SetField(QString fieldName, QString newValue)
{
	if(0) {}
	else if(fieldName == Worker_ns::name		) name		= newValue;
	else if(fieldName == Worker_ns::category	) category	= newValue;
	else if(fieldName == Worker_ns::id_		) id			= newValue;
	else if(fieldName == Worker_ns::type		) type		= newValue;
	else if(fieldName == Worker_ns::addProps	) addProps	= newValue;
	else if(fieldName == Worker_ns::variables  ) variables = newValue;
	else if(fieldName == Worker_ns::frame		) frame		= newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong fieldName name ["+fieldName+"]");
}

const QStringList * WorkerConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals WorkerConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}
