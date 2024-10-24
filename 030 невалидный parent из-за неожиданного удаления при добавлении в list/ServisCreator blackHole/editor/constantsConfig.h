#ifndef constantsconfig_H
#define constantsconfig_H

#include "constants.h"
#include "IConfigItem.h"

class ConstantConfigItem: public IConfigItem
{
public:
	Constant consant;

	ConstantConfigItem() = default;
	~ConstantConfigItem() = default;

	virtual QString GetClassName() const override { return classConstantConfigItem(); }
	virtual QString ToStrForLog() const override { return "["+consant.ToStr()+"]"; }

	virtual int FieldsCount() const override;
	virtual QStringList FieldsNames() const override;
	virtual QStringList FieldsValues() const override { return consant.CellValuesToStringListForEditor(); }
	virtual void SetFieldValues(const QStringList &values) override { consant.SetCellValuesFromEditor(values); }
	virtual void SetField(QString fieldName, QString newValue) override { consant.SetCellValue(fieldName,newValue); }
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;	
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return Constant_ns::rowCaptionConstant; }
	static QString classConstantConfigItem() { return "classConstantConfigItem"; }
};

struct ConstantsConfigItem: public IConfigItem
{
	QString name;

	ConstantsConfigItem() = default;
	ConstantsConfigItem(const QDomElement &element);
	~ConstantsConfigItem() = default;

	virtual QString GetClassName() const override { return classConstantsConfigItem(); }
	virtual QString ToStrForLog() const override { return "["+name+"]"; }

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

	static QString RowCaptionSt() { return Constant_ns::rowCaptionConstants; }
	static QString classConstantsConfigItem() { return "ConstantsConfigItem"; }
};

#endif // constantsconfig_H
