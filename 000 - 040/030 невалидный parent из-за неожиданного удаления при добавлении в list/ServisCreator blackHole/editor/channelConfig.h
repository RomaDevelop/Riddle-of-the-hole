#ifndef CHANNELCONFIGITEM_H
#define CHANNELCONFIGITEM_H

#include "MyQDom.h"

#include "IConfigItem.h"

class ChannelConfigItem: public IConfigItem
{
public:
	QString name;
	QString type;
	QString params;

	ChannelConfigItem() = default;
	~ChannelConfigItem() = default;

	virtual QString GetClassName() const override { return classChannelConfigItem; }
	virtual QString ToStrForLog() const override { return "["+name+" "+type+" "+params+"]"; }

	virtual int FieldsCount() const override { return ChannelConfigItem_ns::Fields::count; }
	virtual QStringList FieldsNames() const override { return ChannelConfigItem_ns::Fields::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return ChannelConfigItem_ns::Fields::rowCaption; }
	inline static const QString classChannelConfigItem { "ChannelConfigItem" };
};

#endif // CHANNELCONFIGITEM_H
