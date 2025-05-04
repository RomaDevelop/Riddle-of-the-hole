#include "constantsconfig.h"

#include "configconstants.h"

ConstantsConfigItem::ConstantsConfigItem(const QDomElement & element)
{
	Set(element);
}

int ConstantsConfigItem::FieldsCount() const
{
	return ConstantsVector_ns::Fields::count;
}

QStringList ConstantsConfigItem::FieldsNames() const
{
	return ConstantsVector_ns::Fields::all;
}

QStringList ConstantsConfigItem::FieldsValues() const
{
	return { name };
}

void ConstantsConfigItem::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
	{
		name = values[ConstantsVector_ns::Fields::nameIndex];
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
	if(fieldName == ConstantsVector_ns::Fields::name)
		name = newValue;
	else Logs::ErrorSt(GetClassName()+"::SetField wrong field name ["+fieldName+"]");
}

void ConstantsConfigItem::Set(const QDomElement & element)
{
	auto attrs = DomAdd::GetAttributes(element);
	if((int)attrs.size() != ConstantsVector_ns::Fields::count)
		Logs::ErrorSt(GetClassName()+"::Set wrong "+element.tagName()+" attrs size ("+QSn(attrs.size())+")");
	for(auto &attr:attrs)
	{
		if(attr.first == ConstantsVector_ns::Fields::name)
			name = attr.second;
		else Logs::ErrorSt(GetClassName()+"::Set wrong attribute name ["+attr.first+"]");
	}

	auto constantsElements = DomAdd::GetTopLevelElements(element);
	for(auto &constantElement:constantsElements)
	{
		if(constantElement.tagName() == Constants_ns::Fields::rowCaptionConstant)
		{
			constants.push_back(ConstantConfigItem());
			constants.back().Set(constantElement);
		}
		else Logs::ErrorSt(GetClassName()+"::Set wrong tag ["+constantElement.tagName()+"]");
	}
}

const QStringList * ConstantsConfigItem::WhatCanBeAdded() const
{
	return &ConstantsVector_ns::canBeAdded;
}

void ConstantsConfigItem::AddSubItem(const QString & subItemType)
{
	if(subItemType == Constants_ns::Fields::rowCaptionConstant)
	{
		constants.push_back(ConstantConfigItem());
	}
	else Logs::ErrorSt(GetClassName()+"::AddSubItem wrong subItemType [" + subItemType + "]");
}

std::vector<IConfigItem *> ConstantsConfigItem::SubDefinitions()
{
	std::vector<IConfigItem*> ret;
	for(auto &constant:constants)
	{
		ret.push_back(&constant);
	}
	return ret;
}

QString ConstantsConfigItem::ToStrForConfig() const
{
	QDomDocument xml;
	QDomElement constantsXml = xml.createElement(Constants_ns::Fields::rowCaptionConstants);
	xml.appendChild(constantsXml);
	constantsXml.setAttribute(ConstantsVector_ns::Fields::name, name);
	for(auto &constant:constants)
	{
		QDomElement constantXml = xml.createElement(Constants_ns::Fields::rowCaptionConstant);
		auto constantAttrs = constant.consant.GetAttributes();
		for(auto &pair:constantAttrs)
		{
			constantXml.setAttribute(pair.first, pair.second);
		}
		constantsXml.appendChild(constantXml);
	}
	return xml.toString();
}

TreeItemVals ConstantsConfigItem::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = Constants_ns::Fields::rowCaptionConstants + " " + name;
	tree.ptr = PtrToStr(this);
	return tree;
}

QString ConstantsConfigItem::RowCaption() const { return Constants_ns::Fields::rowCaptionConstants; }

int ConstantConfigItem::FieldsCount() const { return Constants_ns::Fields::count; }

QStringList ConstantConfigItem::FieldsNames() const { return Constants_ns::Fields::all; }

const QStringList * ConstantConfigItem::WhatCanBeAdded() const
{
	Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
	return nullptr;
}

void ConstantConfigItem::AddSubItem(const QString & subItemType)
{
	Logs::ErrorSt(GetClassName() + "::AddSubItem forbidden " + subItemType);
}

std::vector<IConfigItem *> ConstantConfigItem::SubDefinitions()
{
	Logs::ErrorSt(GetClassName() + "::SubDefinitions forbidden");
	return {};
}

QString ConstantConfigItem::ToStrForConfig() const
{
	Logs::ErrorSt(GetClassName() + "::ToStrForConfig forbidden");
	return {};
}

TreeItemVals ConstantConfigItem::ToTreeItemVals() const
{
	Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
	return {};
}

QString ConstantConfigItem::RowCaption() const { return Constants_ns::Fields::rowCaptionConstant; }
