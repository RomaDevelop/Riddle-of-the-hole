#include "windowConfig.h"
#include "MyQDom.h"

const QStringList * WidgetConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals WidgetConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}

WindowClass::WindowClass(const QDomElement & element)
{
	Set(element);
}

QStringList WindowClass::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	FieldToStringList(ret, WindowClass_ns::nameIndex		,	name		);
	return ret;
}

void WindowClass::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
	{
		name = values[WindowClass_ns::nameIndex];
		if(values.size() > FieldsCount())
		{
			for(int i = FieldsCount(); i<values.size(); i++)
				if(values[i] != "")
					Logs::WarningSt(GetClassName()+" unused value ["+values[i]+"]");
		}
	}
	else Logs::ErrorSt(GetClassName()+"::SetFieldValues values.size() < FieldsCount()");
}

void WindowClass::SetField(QString fieldName, QString newValue)
{
	if(fieldName == WindowClass_ns::name)
		name = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong field name ["+fieldName+"]");
}



TreeItemVals WindowClass::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = WindowClass_ns::rowCaption + " " + name;
	tree.ptr = PtrToStr(this);
	return tree;
}

QStringList WindowConfigItem::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	namespace Fields = WindowConfigItem_ns::Fields;
	FieldToStringList(ret, Fields::nameIndex			,	name			);
	FieldToStringList(ret, Fields::windowClassIndex		,	windowClass		);
	FieldToStringList(ret, Fields::objectToConnectIndex	,	objectToConnect	);
	FieldToStringList(ret, Fields::OtherIndex			,	other	);
	return ret;
}

void WindowConfigItem::SetFieldValues(const QStringList & values)
{
	namespace Fields = WindowConfigItem_ns::Fields;
	if(values.size() == Fields::all.size())
	{
		name			= values[Fields::nameIndex			    ];
		windowClass		= values[Fields::windowClassIndex		];
		objectToConnect	= values[Fields::objectToConnectIndex	];
		other			= values[Fields::OtherIndex			    ];
	}
	else Logs::ErrorSt(GetClassName() + "::SetFieldValues wrong size (get "
					   +QSn(values.size())+" but must be "+QSn(Fields::all.size())+")");
}

void WindowConfigItem::SetField(QString fieldName, QString newValue)
{
	namespace Fields = WindowConfigItem_ns::Fields;
	if(0) {}
	else if(fieldName == Fields::name			) name			    = newValue;
	else if(fieldName == Fields::windowClass	) windowClass		= newValue;
	else if(fieldName == Fields::objectToConnect) objectToConnect	= newValue;
	else if(fieldName == Fields::Other			) other			    = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong fieldName name ["+fieldName+"]");
}

const QStringList * WindowConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded unrealesed");
	return nullptr;
}

TreeItemVals WindowConfigItem::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = RowCaption() + " " + name;
	tree.ptr = PtrToStr(this);
	return tree;
}

//----------------------------------------------------------------------------------------------------
namespace thisFields =  WindowClass_ns::Row;

QStringList RowConfigItem::FieldsValues() const
{
	QStringList ret;
	int size = FieldsCount();
	for(int i=0; i<size; i++) ret += "";
	FieldToStringList(ret, thisFields::captionI,	caption	);
	return ret;
}

void RowConfigItem::SetFieldValues(const QStringList & values)
{
	if(values.size() == FieldsCount())
	{
		caption = values[thisFields::captionI];
	}
	else Logs::ErrorSt(GetClassName() + ":SetFieldValues: values.size() != FieldsCount()");
}

void RowConfigItem::SetField(QString fieldName, QString newValue)
{
	if(0) {}
	else if(fieldName == thisFields::caption) caption	= newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong fieldName name ["+fieldName+"]");
}

TreeItemVals RowConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}

//----------------------------------------------------------------------------------------------------
