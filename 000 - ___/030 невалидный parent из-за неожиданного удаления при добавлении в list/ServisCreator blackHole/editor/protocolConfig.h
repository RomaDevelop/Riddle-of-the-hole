#ifndef ProtocolClass_H
#define ProtocolClass_H

#include "allConstants.h"
#include "protocol.h"
#include "IConfigItem.h"

struct WorkerConfigItem: public IConfigItem
{
	QString category;
	QString name;
	QString id;
	QString type;
	QString addProps;
	QString variables;
	QString frame;

	WorkerConfigItem() = default;
	~WorkerConfigItem() = default;

	virtual QString GetClassName() const override { return classWorkerConfigItem(); }
	virtual QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return Worker_ns::count; }
	virtual QStringList FieldsNames() const override { return Worker_ns::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return true; }
	virtual bool CanHaveSubItemsInTable() const override { return true; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return Worker_ns::Worker; }

	static QString classWorkerConfigItem() { return "WorkerConfigItem"; }
};

struct ProtocolClass: public IConfigItem
{
	QString protocolClassName;

	ProtocolClass() = default;
	ProtocolClass(const QDomElement &element);
	~ProtocolClass() = default;

	virtual QString GetClassName() const override { return classProtocolClass(); }
	virtual QString ToStrForLog() const override { return "["+protocolClassName+"]"; }

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

	static QString RowCaptionSt() { return ProtocolClass_ns::ProtocolClass; }

	static QString classProtocolClass() { return "ProtocolClass"; }
};

#endif // ProtocolClass_H
