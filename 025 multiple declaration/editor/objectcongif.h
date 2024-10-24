#ifndef OBJECTCLASS_H
#define OBJECTCLASS_H

#include "configconstants.h"
#include "object.h"
#include "iconfigitem.h"

struct PraramConfigItem: public IConfigItem
{
	Param param;

	PraramConfigItem() = default;
	~PraramConfigItem() = default;

	virtual QString GetClassName() const override { return classPraramCI(); }
	QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return Param_ns::Fields::count; }
	virtual QStringList FieldsNames() const override { return Param_ns::Fields::all; }
	virtual QStringList FieldsValues() const override { return param.CellValuesToStringListForEditor(); }
	virtual void SetFieldValues(const QStringList &values) override { param.SetCellValuesFromEditor(values); }
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual void Set(const QDomElement &element) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual void AddSubItem(const QString &subItemType) override;
	virtual bool HaveSubDefinitionsInTable() const override { return false; }
	virtual std::vector<IConfigItem*> SubDefinitions() override;
	virtual QString ToStrForConfig() const override;
	virtual TreeItemVals ToTreeItemVals() const override;

	virtual QString RowCaption() const override { return Param::classParam(); }

	static QString classPraramCI() { return "PraramCI"; }
};

struct ObjectClass: public IConfigItem
{
	QString objectClassName;
	std::vector<std::shared_ptr<IConfigItem>> subItems;

	ObjectClass() = default;
	ObjectClass(const QDomElement &objClassElement);
	~ObjectClass() = default;

	virtual QString GetClassName() const override { return classObjectClass(); }
	QString ToStrForLog() const override { return "["+objectClassName+"]"; }

	virtual int FieldsCount() const override;
	virtual QStringList FieldsNames() const override;
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual void Set(const QDomElement &element) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual void AddSubItem(const QString &subItemType) override;
	virtual bool HaveSubDefinitionsInTable() const override { return true; }
	virtual std::vector<IConfigItem*> SubDefinitions() override;
	virtual QString ToStrForConfig() const override;
	virtual TreeItemVals ToTreeItemVals() const override;

	virtual QString RowCaption() const override { return classObjectClass(); }

	static QString classObjectClass() { return ObjectClass_ns::ObjectClass; }
};


struct ObjectConfigItem: public IConfigItem
{
	QString objectClass;
	QString protocolClass;
	QString constants;
	QString type;
	QString name;
	QString channel;
	QString regime;
	QString baud;
	QString startSettings;

	ObjectConfigItem() = default;
	~ObjectConfigItem() = default;

	virtual QString GetClassName() const override { return classObjectConfigItem(); }
	QString ToStrForLog() const override { return "["+GetClassName()+" "+CodeMarkers::mock + " ToStrForLog]"; }

	virtual int FieldsCount() const override {return ObjectConfigItem_ns::Fields::all.size();}
	virtual QStringList FieldsNames() const override { return ObjectConfigItem_ns::Fields::all; }
	virtual QStringList FieldsValues() const override
	{
		QStringList ret;
		int size = FieldsCount();
		for(int i=0; i<size; i++) ret += "";
		namespace Fields = ObjectConfigItem_ns::Fields;
		FieldToStringList(ret, Fields::objectClassIndex,	objectClass		);
		FieldToStringList(ret, Fields::protocolClassIndex,	protocolClass	);
		FieldToStringList(ret, Fields::constantsIndex,		constants		);
		FieldToStringList(ret, Fields::typeIndex,			type			);
		FieldToStringList(ret, Fields::nameIndex,			name			);
		FieldToStringList(ret, Fields::channelIndex,		channel			);
		FieldToStringList(ret, Fields::regimeIndex,			regime			);
		FieldToStringList(ret, Fields::baudIndex,			baud			);
		FieldToStringList(ret, Fields::startSettingsIndex,	startSettings	);
		return ret;
	}
	virtual void SetFieldValues(const QStringList &values) override
	{
		namespace Fields = ObjectConfigItem_ns::Fields;
		if(values.size() == Fields::all.size())
		{
			objectClass		= values[Fields::objectClassIndex];
			protocolClass	= values[Fields::protocolClassIndex];
			constants		= values[Fields::constantsIndex];
			type			= values[Fields::typeIndex];
			name			= values[Fields::nameIndex];
			channel			= values[Fields::channelIndex];
			regime			= values[Fields::regimeIndex];
			baud			= values[Fields::baudIndex];
			startSettings	= values[Fields::startSettingsIndex];
		}
		else Logs::ErrorSt(GetClassName() + "::SetFieldValues wrong size (get "
						   +QSn(values.size())+" but must be "+QSn(Fields::all.size())+")");
	}
	virtual void SetField(QString fieldName, QString newValue) override
	{
		namespace Fields = ObjectConfigItem_ns::Fields;
			 if(fieldName == Fields::objectClass	) objectClass	= newValue;
		else if(fieldName == Fields::protocolClass	) protocolClass = newValue;
		else if(fieldName == Fields::constants		) constants		= newValue;
		else if(fieldName == Fields::type			) type			= newValue;
		else if(fieldName == Fields::name			) name			= newValue;
		else if(fieldName == Fields::channel		) channel		= newValue;
		else if(fieldName == Fields::regime			) regime		= newValue;
		else if(fieldName == Fields::baud			) baud			= newValue;
		else if(fieldName == Fields::startSettings	) startSettings = newValue;
	}
	virtual void Set(const QDomElement &element) override
	{
		namespace Fields = ObjectConfigItem_ns::Fields;
		auto attrs = DomAdd::GetAttributes(element);
		if((int)attrs.size() != Fields::count)
			Logs::ErrorSt(GetClassName()+"::Set wrong "+element.tagName()+" attrs size ("+QSn(attrs.size())+")");
		for(auto &attr:attrs)
		{
				 if(attr.first == Fields::objectClass	    ) objectClass	= attr.second;
			else if(attr.first == Fields::protocolClass	    ) protocolClass = attr.second;
			else if(attr.first == Fields::constants		    ) constants		= attr.second;
			else if(attr.first == Fields::type			    ) type			= attr.second;
			else if(attr.first == Fields::name			    ) name			= attr.second;
			else if(attr.first == Fields::channel		    ) channel		= attr.second;
			else if(attr.first == Fields::regime			) regime		= attr.second;
			else if(attr.first == Fields::baud			    ) baud			= attr.second;
			else if(attr.first == Fields::startSettings	    ) startSettings = attr.second;
			else Logs::ErrorSt(GetClassName()+"::Set wrong attribute name ["+attr.first+"]");
		}

		auto subElements = DomAdd::GetTopLevelElements(element);
		if(!subElements.empty())
			Logs::ErrorSt(GetClassName()+"::Set wrong "+element.tagName()+" has subelements, but not realesed");
	}
	virtual const QStringList* WhatCanBeAdded() const override
	{
		Logs::ErrorSt(GetClassName() + "::WhatCanBeAdded unrealesed");
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

	virtual QString RowCaption() const override { return ObjectConfigItem_ns::Fields::rowCaption; }

	static QString classObjectConfigItem() { return "ObjectConfigItem"; }
};

#endif // OBJECTCLASS_H
