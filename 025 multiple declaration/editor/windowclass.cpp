#include "windowclass.h"

const QStringList * WidgetConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

void WidgetConfigItem::AddSubItem(const QString & subItemType)
{
	Logs::ErrorSt(GetClassName() + "::AddSubItem forbidden " + subItemType);
}

std::vector<IConfigItem *> WidgetConfigItem::SubDefinitions()
{
	Logs::ErrorSt(GetClassName() + "::SubDefinitions forbidden");
	return {};
}

QString WidgetConfigItem::ToStrForConfig() const
{
	Logs::ErrorSt(GetClassName() + "::ToStrForConfig forbidden");
	return {};
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

int WindowClass::FieldsCount() const
{
	return WindowClass_ns::Fields::count;
}

QStringList WindowClass::FieldsNames() const
{
	return WindowClass_ns::Fields::all;
}

QStringList WindowClass::FieldsValues() const
{
	return { name };
}

void WindowClass::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
	{
		name = values[WindowClass_ns::Fields::nameIndex];
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
	if(fieldName == WindowClass_ns::Fields::name)
		name = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong field name ["+fieldName+"]");
}

void WindowClass::Set(const QDomElement & element)
{
	auto attrs = DomAdd::GetAttributes(element);
	if((int)attrs.size() != WindowClass_ns::Fields::count)
		Logs::ErrorSt(GetClassName()+"::Set wrong "+element.tagName()+" attrs size ("+QSn(attrs.size())+")");
	for(auto &attr:attrs)
	{
		if(attr.first == WindowClass_ns::Fields::name)
			name = attr.second;
		else Logs::ErrorSt(GetClassName()+"::Set wrong attribute name ["+attr.first+"]");
	}

	auto subElements = DomAdd::GetTopLevelElements(element);
	for(auto &subElement:subElements)
	{
		if(subElement.tagName() == Widget_ns::Fields::rowCaption)
		{
			widgets.push_back(WidgetConfigItem());
			widgets.back().Set(subElement);
		}
		else Logs::ErrorSt(GetClassName()+"::Set wrong tag ["+subElement.tagName()+"]");
	}
}

const QStringList * WindowClass::WhatCanBeAdded() const
{
	return &WindowClass_ns::canBeAdded;
}

void WindowClass::AddSubItem(const QString & subItemType)
{
	if(subItemType == Widget_ns::Fields::rowCaption)
	{
		widgets.push_back(WidgetConfigItem());
	}
	else Logs::ErrorSt(GetClassName()+"::AddSubItem wrong subItemType [" + subItemType + "]");
}

std::vector<IConfigItem *> WindowClass::SubDefinitions()
{
	std::vector<IConfigItem*> ret;
	for(auto &widget:widgets)
	{
		ret.push_back(&widget);
	}
	return ret;
}

QString WindowClass::ToStrForConfig() const
{
	QDomDocument xml;
	QDomElement thisItemXml = xml.createElement(WindowClass_ns::Fields::rowCaption);
	xml.appendChild(thisItemXml);
	thisItemXml.setAttribute(WindowClass_ns::Fields::name, name);
	for(auto &widget:widgets)
	{
		QDomElement subItemXml = xml.createElement(Widget_ns::Fields::rowCaption);
		auto subItemAttrs = widget.widget.GetAttributes();
		for(auto &pair:subItemAttrs)
		{
			subItemXml.setAttribute(pair.first, pair.second);
		}
		thisItemXml.appendChild(subItemXml);
	}
	return xml.toString();
}

TreeItemVals WindowClass::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = WindowClass_ns::Fields::rowCaption + " " + name;
	tree.ptr = PtrToStr(this);
	return tree;
}
