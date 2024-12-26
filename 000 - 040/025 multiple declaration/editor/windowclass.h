#ifndef WINDOWCLASS_H
#define WINDOWCLASS_H

#include "configconstants.h"
#include "widget.h"
#include "iconfigitem.h"

class WidgetConfigItem: public IConfigItem
{
public:
	WidgetPair widget;

	WidgetConfigItem() = default;
	~WidgetConfigItem() = default;

	virtual QString GetClassName() const override { return classWidgetConfigItem(); }
	QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return Widget_ns::Fields::count; }
	virtual QStringList FieldsNames() const override { return Widget_ns::Fields::all; }
	virtual QStringList FieldsValues() const override { return widget.CellValuesToStringListForEditor(); }
	virtual void SetFieldValues(const QStringList &values) override { widget.SetCellValuesFromEditor(values); }
	virtual void SetField(QString fieldName, QString newValue) override { widget.SetCellValue(fieldName,newValue); }
	virtual void Set(const QDomElement &element) override { widget.SetCellValuesFromDomElement(element); }
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual void AddSubItem(const QString &subItemType) override;
	virtual bool HaveSubDefinitionsInTable() const override { return false; }
	virtual std::vector<IConfigItem*> SubDefinitions() override;
	virtual QString ToStrForConfig() const override;
	virtual TreeItemVals ToTreeItemVals() const override;

	virtual QString RowCaption() const override { return Widget_ns::Fields::rowCaption; }

	static QString classWidgetConfigItem() { return "WidgetConfigItem"; }
};

class WindowClass: public IConfigItem
{
public:
	QString name;
	std::vector<WidgetConfigItem> widgets;

	WindowClass() = default;
	WindowClass(const QDomElement &element);
	~WindowClass() = default;

	virtual QString GetClassName() const override { return classWindowClass(); }
	QString ToStrForLog() const override { return "["+name+"]"; }

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

	virtual QString RowCaption() const override { return WindowClass_ns::Fields::rowCaption; }

	static QString classWindowClass() { return "WindowClass"; }
};

#endif // WINDOWCLASS_H
