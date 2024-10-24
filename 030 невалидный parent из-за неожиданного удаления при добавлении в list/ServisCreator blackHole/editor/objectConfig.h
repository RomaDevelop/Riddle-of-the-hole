#ifndef OBJECTCLASS_H
#define OBJECTCLASS_H

#include "allConstants.h"
#include "object.h"
#include "IConfigItem.h"

struct PraramConfigItem: public IConfigItem
{
	Param param;

	PraramConfigItem() = default;
	~PraramConfigItem() = default;

	virtual QString GetClassName() const override { return classPraramCI(); }
	virtual QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return Param_ns::count; }
	virtual QStringList FieldsNames() const override { return Param_ns::all; }
	virtual QStringList FieldsValues() const override { return param.CellValuesToStringListForEditor(); }
	virtual void SetFieldValues(const QStringList &values) override { param.SetCellValuesFromEditor(values); }
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return Param_ns::rowCaption; }

	static QString classPraramCI() { return "PraramConfigItem"; }
};

struct ObjectClass: public IConfigItem
{
	QString objectClassName;

	ObjectClass() = default;
	ObjectClass(const QDomElement &objClassElement);
	~ObjectClass() = default;

	virtual QString GetClassName() const override { return classObjectClass(); }
	virtual QString ToStrForLog() const override { return "["+objectClassName+"]"; }

	virtual int FieldsCount() const override;
	virtual QStringList FieldsNames() const override;
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return true; }
	virtual bool CanHaveSubItemsInTable() const override { return true; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return ObjectClass_ns::ObjectClass; }

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
	QString startSettings;

	ObjectConfigItem() = default;
	~ObjectConfigItem() = default;

	virtual QString GetClassName() const override { return classObjectConfigItem(); }
	virtual QString ToStrForLog() const override { return "["+GetClassName()+" "+CodeMarkers::mock + " ToStrForLog]"; }

	virtual int FieldsCount() const override {return ObjectConfigItem_ns::all.size();}
	virtual QStringList FieldsNames() const override { return ObjectConfigItem_ns::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return ObjectConfigItem_ns::rowCaption; }
	static QString classObjectConfigItem() { return "ObjectConfigItem"; }
};

#endif // OBJECTCLASS_H
