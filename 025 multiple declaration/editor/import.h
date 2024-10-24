#ifndef IMPORT_H
#define IMPORT_H

#include "configconstants.h"
#include "iconfigitem.h"

class ImportConfigItem: public IConfigItem
{
public:
	QString type;
	QString name;

	ImportConfigItem() = default;
	~ImportConfigItem() = default;

	virtual QString GetClassName() const override { return classImportConfigItem(); }
	QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return Import_ns::Fields::count; }
	virtual QStringList FieldsNames() const override { return Import_ns::Fields::all; }
	virtual QStringList FieldsValues() const override { return {type, name}; }
	virtual void SetFieldValues(const QStringList &values) override
	{
		if(values.size() == FieldsCount())
		{
			type = values[Import_ns::Fields::typeIndex];
			name = values[Import_ns::Fields::nameIndex];
		}
		else Logs::ErrorSt(GetClassName() + ":SetFieldValues: values.size() != FieldsCount()");
	}
	virtual void SetField(QString fieldName, QString newValue) override
	{
		Logs::ErrorSt("["+CodeMarkers::mock+" "+GetClassName()+"] SetField " + fieldName + " " + newValue);
	}
	virtual void Set(const QDomElement &element) override
	{
		auto attrs = DomAdd::GetAttributes(element);
		if((int)attrs.size() != Import_ns::Fields::count)
			Logs::ErrorSt(GetClassName() + "::Constructor wrong "+element.tagName()+" attrs size ("+QSn(attrs.size())+")");
		for(auto &attr:attrs)
		{
			if(attr.first == Import_ns::Fields::type)
				type = attr.second;
			else if(attr.first == Import_ns::Fields::name)
				name = attr.second;
			else Logs::ErrorSt(GetClassName() + "::Constructor wrong "+Import_ns::Fields::rowCaption+" attribute name ["+attr.first+"]");
		}

		auto importElements = DomAdd::GetTopLevelElements(element);
		if(!importElements.empty())
			Logs::ErrorSt("["+CodeMarkers::mock+" "+GetClassName()+"] Set ");
//		for(auto &impElement:importElements)
//		{
//			if(impElement.tagName() == Param_ns::param)
//			{
//				params.push_back(PraramConfigItem());
//				params.back().Set(objElement);
//			}
//			else Logs::ErrorSt(GetClassName() + "::Constructor wrong tag ["+impElement.tagName()+"]");
//		}
	}
	virtual const QStringList* WhatCanBeAdded() const override
	{
		Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded forbidden");
		return nullptr;
	}
	virtual void AddSubItem(const QString &subItemType) override
	{
		Logs::ErrorSt(GetClassName() + "::AddSubItem forbidden " + subItemType);
	}
	virtual bool HaveSubDefinitionsInTable() const override { return false; }
	virtual std::vector<IConfigItem*> SubDefinitions() override
	{
		Logs::ErrorSt(GetClassName() + "::SubDefinitions forbidden");
		return {};
	}
	virtual QString ToStrForConfig() const override
	{
		Logs::ErrorSt(GetClassName() + "::ToStrForConfig forbidden");
		return {};
	}
	virtual TreeItemVals ToTreeItemVals() const override
	{
		Logs::ErrorSt(GetClassName() + "::ToTreeItemVals forbidden");
		return {};
	}

	virtual QString RowCaption() const override { return Import_ns::Fields::rowCaption; }

	static QString classImportConfigItem() { return "ImportConfigItem"; }
};

#endif // IMPORT_H
