#ifndef SYNC_H
#define SYNC_H

#include "MyQDom.h"

#include "allConstants.h"
#include "IConfigItem.h"

class SyncConfigItem: public IConfigItem
{
public:
	QString name;
	QString command;

	SyncConfigItem() = default;
	~SyncConfigItem() = default;

	virtual QString GetClassName() const override { return classSyncConfigItem; }
	virtual QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return SyncConfigItem_ns::Fields::count; }
	virtual QStringList FieldsNames() const override { return SyncConfigItem_ns::Fields::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return SyncConfigItem_ns::Fields::rowCaption; }
	inline static const QString classSyncConfigItem { "SyncConfigItem" };
};

#endif // SYNC_H
