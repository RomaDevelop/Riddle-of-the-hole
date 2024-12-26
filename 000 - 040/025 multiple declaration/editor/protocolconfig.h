#ifndef ProtocolClass_H
#define ProtocolClass_H

#include "configconstants.h"
#include "protocol.h"
#include "iconfigitem.h"

struct FrameWorkerConfigItem: public IConfigItem
{
	FrameWorker frameWorker;

	FrameWorkerConfigItem() = default;
	~FrameWorkerConfigItem() = default;

	virtual QString GetClassName() const override { return classFrameWorkerConfigItem(); }
	QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return FrameWorker_ns::Fields::count; }
	virtual QStringList FieldsNames() const override { return FrameWorker_ns::Fields::all; }
	virtual QStringList FieldsValues() const override { return frameWorker.CellValuesToStringListForEditor(); }
	virtual void SetFieldValues(const QStringList &values) override { frameWorker.SetCellValuesFromEditor(values); }
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual void Set(const QDomElement &element) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual void AddSubItem(const QString &subItemType) override;
	virtual bool HaveSubDefinitionsInTable() const override { return false; }
	virtual std::vector<IConfigItem*> SubDefinitions() override;
	virtual QString ToStrForConfig() const override;
	virtual TreeItemVals ToTreeItemVals() const override;

	virtual QString RowCaption() const override { return FrameWorker::classFrameWorker(); }

	static QString classFrameWorkerConfigItem() { return "FrameWorkerConfigItem"; }
};

struct ProtocolClass: public IConfigItem
{
	QString protocolClassName;
	std::vector<FrameWorkerConfigItem> frameWorkers;

	ProtocolClass() = default;
	ProtocolClass(const QDomElement &element);
	~ProtocolClass() = default;

	virtual QString GetClassName() const override { return classProtocolClass(); }
	QString ToStrForLog() const override { return "["+protocolClassName+"]"; }

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

	virtual QString RowCaption() const override { return classProtocolClass(); }

	static QString classProtocolClass() { return "ProtocolClass"; }
};

#endif // ProtocolClass_H
