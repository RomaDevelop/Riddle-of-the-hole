#ifndef constantsconfig_H
#define constantsconfig_H

#include "constants.h"
#include "iconfigitem.h"

class ConstantConfigItem: public IConfigItem
{
public:
	Constant consant;

	ConstantConfigItem() = default;
	~ConstantConfigItem() = default;

	virtual QString GetClassName() const override { return classConstantConfigItem(); }
	QString ToStrForLog() const override { return "["+CodeMarkers::mock+" "+GetClassName()+"]"; }

	virtual int FieldsCount() const override;
	virtual QStringList FieldsNames() const override;
	virtual QStringList FieldsValues() const override { return consant.CellValuesToStringListForEditor(); }
	virtual void SetFieldValues(const QStringList &values) override { consant.SetCellValuesFromEditor(values); }
	virtual void SetField(QString fieldName, QString newValue) override { consant.SetCellValue(fieldName,newValue); }
	virtual void Set(const QDomElement &element) override { consant.SetCellValuesFromDomElement(element); }
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual void AddSubItem(const QString &subItemType) override;
	virtual bool HaveSubDefinitionsInTable() const override { return false; }
	virtual std::vector<IConfigItem*> SubDefinitions() override;
	virtual QString ToStrForConfig() const override;
	virtual TreeItemVals ToTreeItemVals() const override;

	virtual QString RowCaption() const override;

	static QString classConstantConfigItem() { return "classConstantConfigItem"; }
};

struct ConstantsConfigItem: public IConfigItem
{
	QString name;
	std::vector<ConstantConfigItem> constants;

	ConstantsConfigItem() = default;
	ConstantsConfigItem(const QDomElement &element);
	~ConstantsConfigItem() = default;

	virtual QString GetClassName() const override { return classConstantsConfigItem(); }
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

	virtual QString RowCaption() const override;

	static QString classConstantsConfigItem() { return "ConstantsConfigItem"; }
};

#endif // constantsconfig_H
