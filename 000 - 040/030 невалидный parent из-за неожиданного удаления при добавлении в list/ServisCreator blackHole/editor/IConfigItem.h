#ifndef ICONFIGITEM_H
#define ICONFIGITEM_H

#include <QTreeWidget>
#include <QDomDocument>

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

struct XMLTag;

class IConfigItem: public HaveClassName
{
	using vector_sh_ptr_IConfigItem = std::vector<std::shared_ptr<IConfigItem>>;
	vector_sh_ptr_IConfigItem subItems;
public:

	IConfigItem *parent = nullptr;

	inline static std::vector<IConfigItem*> removedItems {};

	virtual ~IConfigItem() = default;

	// final group
	virtual void ClearSubItems() final;
	virtual vector_sh_ptr_IConfigItem& SubItems(bool throwWarning = true) final;
	virtual const vector_sh_ptr_IConfigItem& SubItems(bool throwWarning = true) const final;
	virtual std::vector<IConfigItem*> SubItemPtrs(bool deep = true);

	/// returns ptr to created SubItem or nullptr
	virtual IConfigItem* AddSubItem(const QString &subItemType) final;
	virtual IConfigItem* AddSubItem(std::shared_ptr<IConfigItem> subItem) final;
	enum insertPlaces { begin, end, beforeMe, afterMe};
	virtual IConfigItem* InsertSibling(std::shared_ptr<IConfigItem> sibling, int placeCode) final;
	virtual IConfigItem* PreviousSibling() final;

	virtual std::shared_ptr<IConfigItem> CloneItem(bool deep = true) final;
	virtual void RemoveMe() final;

	virtual QString Field(const QString &fieldName) const final;
	virtual void FieldToStringList(QStringList &strList, int index, const QString &fieldValue) const final;
	virtual QStringPairVector Fields() const final;
	virtual void Set(const QDomElement & element) final;
	virtual void ReplaceInFields(const QString & replaceWhat, const QString & replaceTo) final;
	virtual void CreateSubElementsIfCanAndHave(const QDomElement &domElementFrom) final;
	virtual QDomElement ToDomElement(QDomDocument &domDocumentCreateIn) final;
	virtual void ExportToDomDocument(QDomDocument &domDocumentCreateIn, QDomNode &parentNode) final;
	virtual XMLTag ExportToXMLTag(bool deep) final;
	// final group end

	// pure group
	virtual QString GetClassName() const override = 0;
	virtual QString ToStrForLog() const override = 0;

	virtual int FieldsCount() const = 0;
	virtual QStringList FieldsNames() const = 0;
	virtual QStringList FieldsValues() const = 0;
	virtual void SetFieldValues(const QStringList &values) = 0;
	virtual void SetField(QString fieldName, QString newValue) = 0;
	virtual const QStringList* WhatCanBeAdded() const = 0;
	virtual bool CanHaveSubItems() const = 0;
	virtual bool CanHaveSubItemsInTable() const = 0;
	virtual TreeItemVals ToTreeItemVals() const = 0;
	virtual QString RowCaption() const = 0;
	// pure group end

	static QString PtrToStr(const IConfigItem* ptr, int base = 16);
	static IConfigItem* PtrFromStr(QString strPtr, int base = 16);
};

struct XMLTag
{
	QString tagName;
	QStringPairVector attributes;
	std::vector<XMLTag> nestedTags;

public:
	QString ToXMLCode(int indent);
private:
	QString ToXMLCodeRecursion(QString indent, bool zeroLevel);
};

#endif // ICONFIGITEM_H
