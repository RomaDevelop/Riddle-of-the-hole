#ifndef ICONFIGITEM_H
#define ICONFIGITEM_H

#include <QTreeWidget>

#include "ioperand.h"

struct TreeItemVals
{
	QString caption;
	QString ptr;
	std::vector<TreeItemVals> subVals;

	TreeItemVals() = default;
	TreeItemVals(QString caption_, QString ptr_): caption{caption_}, ptr{ptr_} {}

	void ToTree(QTreeWidget *tree);
	void SubValsToItemRecursion(QTreeWidgetItem *item);
};

class IConfigItem: public HaveClassName
{
public:
	virtual ~IConfigItem() = default;

	virtual QString GetClassName() const override = 0;
	virtual QString ToStrForLog() const override = 0;

	virtual int FieldsCount() const = 0;
	virtual QStringList FieldsNames() const = 0;
	virtual QStringList FieldsValues() const = 0;
	virtual QStringPairVector Fields() const final
	{
		QStringPairVector ret;
		auto fieldsNames = FieldsNames();
		auto fieldsVals = FieldsValues();
		if(fieldsNames.size() == fieldsVals.size())
		{
			while(!fieldsNames.empty())
			{
				ret.push_back(QStringPair(fieldsNames.front(),fieldsVals.front()));
				fieldsNames.removeFirst();
				fieldsVals.removeFirst();
			}
		}
		else Logs::ErrorSt(GetClassName()+"::Fields fieldsNames.size() != fieldsVals.size()");
		return ret;
	}

	virtual void FieldToStringList(QStringList &strList, int index, const QString &fieldValue) const final
	{
		int size = strList.size();
		if(size > index) strList[index] = fieldValue;
		else Logs::ErrorSt(GetClassName() + " FieldToStringList final: wrong size [" + QSn(size) + "] QStringList &strList or index ["+index+"]");
	}
	virtual QDomElement ToDomElement(QDomDocument &domDocumentCreateIn) const final
	{
		QDomElement domE = domDocumentCreateIn.createElement(RowCaption());
		QStringPairVector fields = Fields();
		for(auto &field:fields)
		{
			domE.setAttribute(field.first, field.second);
		}
		return domE;
	}
	virtual void SetFieldValues(const QStringList &values) = 0;
	virtual void Set(const QDomElement &element) = 0;
	virtual void SetField(QString fieldName, QString newValue) = 0;
	virtual const QStringList* WhatCanBeAdded() const = 0;
	virtual void AddSubItem(const QString &subItemType) = 0;
	virtual bool HaveSubDefinitionsInTable() const = 0;
	virtual std::vector<IConfigItem*> SubDefinitions() = 0;
	virtual QString ToStrForConfig() const = 0;
	virtual TreeItemVals ToTreeItemVals() const = 0;

	virtual QString RowCaption() const = 0;

	static QString PtrToStr(const IConfigItem* ptr, int base = 16);
	static IConfigItem* PtrFromStr(QString strPtr, int base = 16);
};

#endif // ICONFIGITEM_H
