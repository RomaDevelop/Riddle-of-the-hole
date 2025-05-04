#ifndef WINDOWCLASS_H
#define WINDOWCLASS_H

#include "allConstants.h"
#include "widget.h"
#include "IConfigItem.h"

class WidgetConfigItem: public IConfigItem
{
public:
	WidgetPair widget;

	WidgetConfigItem() = default;
	~WidgetConfigItem() = default;

	virtual QString GetClassName() const override { return classWidgetConfigItem; }
	virtual QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override { return Widget_ns::count; }
	virtual QStringList FieldsNames() const override { return Widget_ns::all; }
	virtual QStringList FieldsValues() const override { return widget.CellValuesToStringListForEditor(); }
	virtual void SetFieldValues(const QStringList &values) override { widget.SetCellValuesFromEditor(values); }
	virtual void SetField(QString fieldName, QString newValue) override { widget.SetCellValue(fieldName,newValue); }
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return Widget_ns::rowCaption; }

	inline static const QString classWidgetConfigItem { "WidgetConfigItem" };
};

class RowConfigItem: public IConfigItem
{
public:
	QString caption;

	RowConfigItem() = default;
	RowConfigItem(const QDomElement &element);
	~RowConfigItem() = default;

	virtual QString GetClassName() const override { return classRowConfigItem; }
	virtual QString ToStrForLog() const override { return "["+caption+"]"; }

	virtual int FieldsCount() const override { return WindowClass_ns::Row::count; }
	virtual QStringList FieldsNames() const override { return WindowClass_ns::Row::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override { return &WindowClass_ns::Row::canBeAdded; }
	virtual bool CanHaveSubItems() const override { return true; }
	virtual bool CanHaveSubItemsInTable() const override { return true; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return WindowClass_ns::Row::rowCaption; }
	inline static const QString classRowConfigItem { "RowConfigItem" };
};

class WindowClass: public IConfigItem
{
public:
	QString name;

	WindowClass() = default;
	WindowClass(const QDomElement &element);
	~WindowClass() = default;

	virtual QString GetClassName() const override { return classWindowClass; }
	virtual QString ToStrForLog() const override { return "["+name+"]"; }

	virtual int FieldsCount() const override { return WindowClass_ns::count; }
	virtual QStringList FieldsNames() const override { return WindowClass_ns::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override { return &WindowClass_ns::canBeAdded; }
	virtual bool CanHaveSubItems() const override { return true; }
	virtual bool CanHaveSubItemsInTable() const override { return true; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return WindowClass_ns::rowCaption; }
	inline static const QString classWindowClass { "WindowClass" };
};

struct WindowConfigItem: public IConfigItem
{
	QString name;
	QString windowClass;
	QString objectToConnect;
	QString other;

	WindowConfigItem() = default;
	~WindowConfigItem() = default;

	virtual QString GetClassName() const override { return classWindowConfigItem; }
	virtual QString ToStrForLog() const override { return "["+GetClassName()+" "+CodeMarkers::mock + " ToStrForLog]"; }

	virtual int FieldsCount() const override { return WindowConfigItem_ns::Fields::all.size(); }
	virtual QStringList FieldsNames() const override { return WindowConfigItem_ns::Fields::all; }
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return WindowConfigItem_ns::Fields::rowCaption; }
	inline static const QString classWindowConfigItem { "WindowConfigItem" };
};

#endif // WINDOWCLASS_H
