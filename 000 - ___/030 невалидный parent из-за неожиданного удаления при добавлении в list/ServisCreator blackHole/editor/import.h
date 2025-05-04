#ifndef IMPORT_H
#define IMPORT_H

#include "MyQDom.h"

#include "allConstants.h"
#include "IConfigItem.h"

class ImportConfigItem: public IConfigItem
{
public:
	QString type;
	QString name;
	QString policy;
	QString modification;
	QString postModification;

	ImportConfigItem() = default;
	~ImportConfigItem() = default;

	virtual QString GetClassName() const override { return classImportConfigItem(); }
	virtual QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return Import_ns::count; }
	virtual QStringList FieldsNames() const override { return Import_ns::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return Import_ns::rowCaption; }
	static QString classImportConfigItem() { return "ImportConfigItem"; }
};

#endif // IMPORT_H
