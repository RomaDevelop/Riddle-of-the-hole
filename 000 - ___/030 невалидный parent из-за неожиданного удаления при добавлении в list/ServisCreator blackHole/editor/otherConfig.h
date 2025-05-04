#ifndef OTHERCONFIGITEM_H
#define OTHERCONFIGITEM_H

#include "MyQDom.h"

#include "allConstants.h"
#include "IConfigItem.h"

class OtherConfigItem: public IConfigItem
{
public:
	QString content;

	OtherConfigItem() = default;
	~OtherConfigItem() = default;

	virtual QString GetClassName() const override { return classOtherConfigItem; }
	virtual QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return OtherConfigItem_ns::count; }
	virtual QStringList FieldsNames() const override { return OtherConfigItem_ns::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return OtherConfigItem_ns::rowCaption; }
	inline static const QString classOtherConfigItem { "OtherConfigItem" };
};

#endif // OTHERCONFIGITEM_H
