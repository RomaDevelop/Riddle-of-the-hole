#include "IConfigItem.h"

#include "MyCppDifferent.h"
#include "MyQDom.h"

#include "configItemFabric.h"

#define MSG_PREFIX "IConfigItem(" + GetClassName() + ")"

void TreeItemVals::ToTree(QTreeWidget * tree)
{
	auto item = new QTreeWidgetItem;
	item->setData(Editor_ns::colCaption, 0, caption);
	item->setData(Editor_ns::colPtrConfigItem,0, ptr);
	tree->addTopLevelItem(item);
	SubValsToItemRecursion(item);
}

void TreeItemVals::SubValsToItemRecursion(QTreeWidgetItem * item)
{
	for(auto &subVal:subVals)
	{
		auto subItem = new QTreeWidgetItem;
		subItem->setData(Editor_ns::colCaption, 0, subVal.caption);
		subItem->setData(Editor_ns::colPtrConfigItem,0, subVal.ptr);
		item->addChild(subItem);
		if(!subVal.subVals.empty())
			subVal.SubValsToItemRecursion(subItem);
	}
}

QStringPairVector IConfigItem::Fields() const
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
	else Logs::ErrorSt(MSG_PREFIX + "::Fields fieldsNames.size() != fieldsVals.size()");
	return ret;
}

void IConfigItem::ClearSubItems()
{
	subItems.clear();
}

IConfigItem::vector_sh_ptr_IConfigItem & IConfigItem::SubItems(bool throwWarning)
{
	if(!CanHaveSubItems() && throwWarning)
		Logs::WarningSt(MSG_PREFIX + "::SubItemsNew error, class can't have SubItems, but SubItems executed. Empty vector returned");
	return subItems;
}

const IConfigItem::vector_sh_ptr_IConfigItem & IConfigItem::SubItems(bool throwWarning) const
{
	if(!CanHaveSubItems() && throwWarning)
		Logs::WarningSt(MSG_PREFIX + "::SubItemsNew error, class can't have SubItems, but SubItems executed. Empty vector returned");
	return subItems;
}

std::vector<IConfigItem *> IConfigItem::SubItemPtrs(bool deep)
{
	std::vector<IConfigItem*> allItems;
	for(auto &subItem:subItems)
	{
		allItems.push_back(subItem.get());
		if(deep)
		{
			auto allItemsOfSubItem = subItem->SubItemPtrs(deep);
			allItems.insert(allItems.end(), allItemsOfSubItem.begin(), allItemsOfSubItem.end());
		}
	}
	return allItems;
}

IConfigItem * IConfigItem::AddSubItem(const QString & subItemType)
{
	const QStringList* subItemTypes = WhatCanBeAdded();
	if(subItemTypes)
	{
		if(subItemTypes->contains(subItemType))
		{
			auto newSubItem = ConfigItemFabric::Make(subItemType);
			if(newSubItem)
			{
				subItems.push_back(newSubItem);
				subItems.back()->parent = this;
				return subItems.back().get();
			}
			else Logs::ErrorSt(MSG_PREFIX + "::AddSubItem ConfigItemFabric::Make from tag ["+subItemType+"] result is nullptr");
		}
		else Logs::ErrorSt(MSG_PREFIX + "::AddSubItem wrong subItemType [" + subItemType + "]");
	}
	else Logs::ErrorSt(MSG_PREFIX + "::AddSubItem WhatCanBeAdded result is nullptr");
	return nullptr;
}

IConfigItem * IConfigItem::AddSubItem(std::shared_ptr<IConfigItem> subItem)
{
	if(!subItem) {Logs::ErrorSt(MSG_PREFIX + "::AddSubItem adding subItem is nullptr"); return nullptr; }

	const QStringList* subItemTypes = WhatCanBeAdded();
	if(subItemTypes)
	{
		if(subItemTypes->contains(subItem->RowCaption()))
		{
			subItems.push_back(subItem);
			subItems.back()->parent = this;
			return subItems.back().get();
		}
		else Logs::ErrorSt(MSG_PREFIX + "::AddSubItem wrong subItemType [" + subItem->RowCaption() + "]");
	}
	else Logs::ErrorSt(MSG_PREFIX + "::AddSubItem WhatCanBeAdded result is nullptr");
	return nullptr;
}

IConfigItem * IConfigItem::InsertSibling(std::shared_ptr<IConfigItem> sibling, int placeCode)
{
	if(!sibling) {Logs::ErrorSt(MSG_PREFIX + "::InsertSibling adding subItem is nullptr"); return nullptr; }
	if(!parent) {Logs::ErrorSt(MSG_PREFIX + "::InsertSibling parent is nullptr"); return nullptr; }

	const QStringList* subItemTypes = parent->WhatCanBeAdded();
	if(subItemTypes)
	{
		if(subItemTypes->contains(sibling->RowCaption()))
		{
			if(placeCode == beforeMe)
			{
				for(uint i=0; i<parent->subItems.size(); i++)
				{
					if(parent->subItems[i].get() == this)
					{
						parent->subItems.insert(parent->subItems.begin()+i, sibling);
						sibling->parent = parent;
						return sibling.get();
					}
				}
				Logs::ErrorSt(MSG_PREFIX + "::InsertSibling this not found in parent subItems");
			}
			else Logs::ErrorSt(MSG_PREFIX + "::InsertSibling wrong placeCode [" + placeCode + "]");
		}
		else Logs::ErrorSt(MSG_PREFIX + "::InsertSibling wrong subItemType [" + sibling->RowCaption() + "]");
	}
	else Logs::ErrorSt(MSG_PREFIX + "::InsertSibling WhatCanBeAdded result is nullptr");
	return nullptr;
}

IConfigItem * IConfigItem::PreviousSibling()
{
	if(!parent) {Logs::ErrorSt(MSG_PREFIX + "::PreviousSibling parent is nullptr"); return nullptr; }

	bool found = false;
	for(uint i=0; i<parent->subItems.size(); i++)
	{
		if(parent->subItems[i].get() == this)
		{
			found = true;
			if(i > 0)
				return parent->subItems[i-1].get();
		}
	}
	if(!found) Logs::ErrorSt(MSG_PREFIX + "::PreviousSibling this not found in parent subItems");
	return nullptr;
}

void IConfigItem::RemoveMe()
{
	if(!parent) {Logs::ErrorSt(MSG_PREFIX + "::RemoveMe parent is nullptr"); return; }

	for(uint i=0; i<parent->subItems.size(); i++)
	{
		if(parent->subItems[i].get() == this)
		{
			parent->subItems.erase(parent->subItems.begin()+i);
			parent = nullptr;
			return;
		}
	}
	Logs::ErrorSt(MSG_PREFIX + "::RemoveMe this not found in parent subItems");
}

QString IConfigItem::Field(const QString & fieldName) const
{
	auto names = FieldsNames();
	auto values = FieldsValues();
	int index = names.indexOf(fieldName);
	if(index != -1)
	{
		if(values.size()>index) return values[index];
		else Logs::ErrorSt(MSG_PREFIX + "::Field can't wrond size values");
	}
	else Logs::ErrorSt(MSG_PREFIX + "::Field can't find fieldName ["+fieldName+"]");
	return "";
}

void IConfigItem::FieldToStringList(QStringList & strList, int index, const QString & fieldValue) const
{
	int size = strList.size();
	if(size > index) strList[index] = fieldValue;
	else Logs::ErrorSt(MSG_PREFIX + "::FieldToStringList: wrong size [" + QSn(size) + "] QStringList &strList or index ["+index+"]");
}

void IConfigItem::Set(const QDomElement & element)
{
	auto attrs = MyQDom::Attributes(element);
	if((int)attrs.size() != FieldsCount())
	{
		QString error = "Неверное количество атрибутов ("+QSn(attrs.size())
				+", а должно быть "+QSn(FieldsCount())+") у элемента: " + MyQDom::ToStringIgnoreNested(element) + "\n";
		if((int)attrs.size() > FieldsCount())
		{
			for(int i=attrs.size()-1; i>=0; i--)
				if(!FieldsNames().contains(attrs[i].first))
				{
					error = error += "Лишний атрибут " + attrs[i].first + "\n";
					attrs.erase(attrs.begin()+i);
				}
		}
		else
		{
			QStringList attrNames;
			for(auto &attr:attrs) attrNames += attr.first;

			for(auto &fieldName:FieldsNames())
			{
				if(!attrNames.contains(fieldName))
					error = error += "Нехватает атрибута " + fieldName + "\n";
			}
		}
		error += "Лишние атрибуты игнорируются, недостающие устанавливаются по-умолчанию";
		Logs::ErrorSt(error);
	}

	for(auto &attr:attrs)
		SetField(attr.first, attr.second);

	CreateSubElementsIfCanAndHave(element);
}

void IConfigItem::ReplaceInFields(const QString & replaceWhat, const QString & replaceTo)
{
	auto fields = Fields();
	int size = fields.size();
	for(int i=0; i<size; i++)
	{
		if(fields[i].second.contains(replaceWhat))
		{
			SetField(fields[i].first, fields[i].second.replace(replaceWhat,replaceTo));
		}
	}
}

void IConfigItem::CreateSubElementsIfCanAndHave(const QDomElement & domElementFrom)
{
	auto subElements = MyQDom::GetTopLevelElements(domElementFrom);
	if(!subElements.empty() && CanHaveSubItems() == false)
	{
		Logs::ErrorSt(MSG_PREFIX + "::CreateSubElements domElementFrom have nested tags but class can't have subItems");
		return;
	}
	if(subElements.empty()) return;

	const QStringList* subItemTypesStrList = WhatCanBeAdded();
	if(!subItemTypesStrList)
	{
		Logs::ErrorSt(MSG_PREFIX + "::CreateSubElements WhatCanBeAdded result is nullptr");
		return;
	}

	for(auto &subElement:subElements)
	{
		if(subItemTypesStrList->contains(subElement.tagName()))
		{
			IConfigItem* newSubItem = AddSubItem(subElement.tagName());
			if(newSubItem)
			{
				newSubItem->Set(subElement);
			}
			else Logs::ErrorSt(MSG_PREFIX + "::CreateSubElements AddSubItem from tag ["+subElement.tagName()+"] result is nullptr");
		}
		else Logs::ErrorSt(MSG_PREFIX + "::CreateSubElements wrong tag ["
						   +subElement.tagName()+"], WhatCanBeAdded result ["+subItemTypesStrList->join("|")+"]");
	}
}

QDomElement IConfigItem::ToDomElement(QDomDocument & domDocumentCreateIn)
{
	QStringPairVector fields = Fields();
	QDomElement domE = domDocumentCreateIn.createElement(RowCaption());
	for(auto &field:fields)
		domE.setAttribute(field.first, field.second);

	if(CanHaveSubItems())
	{
		auto& subDefinitions = SubItems(true);
		for(auto &subDefinition:subDefinitions)
		{
			subDefinition->ExportToDomDocument(domDocumentCreateIn,domE);
		}
	}

	return domE;
}

void IConfigItem::ExportToDomDocument(QDomDocument & domDocumentCreateIn, QDomNode & parentNode)
{
	QStringPairVector fields = Fields();
	QDomElement domE = domDocumentCreateIn.createElement(RowCaption());
	for(auto &field:fields)
		domE.setAttribute(field.first, field.second);
	parentNode.appendChild(domE);

	if(CanHaveSubItems())
	{
		auto& subDefinitions = SubItems(true);
		for(auto &subDefinition:subDefinitions)
		{
			subDefinition->ExportToDomDocument(domDocumentCreateIn,domE);
		}
	}
}

XMLTag IConfigItem::ExportToXMLTag(bool deep)
{
	XMLTag xmlTag;
	xmlTag.tagName = RowCaption();
	xmlTag.attributes = Fields();
	for(auto &attr:xmlTag.attributes)
	{
		attr.second.replace("\"","&quot;");
		attr.second.replace("\n","&#xa;");
	}

	if(deep && CanHaveSubItems())
	{
		auto& subDefinitions = SubItems(true);
		for(auto &subDefinition:subDefinitions)
		{
			xmlTag.nestedTags.push_back(subDefinition->ExportToXMLTag(deep));
		}
	}
	return xmlTag;
}

std::shared_ptr<IConfigItem> IConfigItem::CloneItem(bool deep)
{
	auto clone = ConfigItemFabric::Make(RowCaption());
	if(!clone) { Logs::ErrorSt("in CloneItem make error, nullptr return"); return nullptr; }

	auto fieldValues = FieldsValues();
	clone->SetFieldValues(fieldValues);
	if(deep && CanHaveSubItems())
	{
		auto& subItems = SubItems(true);
		for(auto &subItem:subItems)
			clone->AddSubItem(subItem->CloneItem(true));
	}
	return clone;
}

QString IConfigItem::PtrToStr(const IConfigItem * ptr, int base)
{
	quint64 ptrVal = (quint64)ptr;
	return QString::number(ptrVal,base);
}

IConfigItem * IConfigItem::PtrFromStr(QString strPtr, int base)
{
	bool ok;
	quint64 ptrVal = strPtr.toULongLong(&ok,base);
	if(!ok) Logs::ErrorSt("PtrFromStr: wrong strPtr ["+strPtr+"]");
	return (IConfigItem*)ptrVal;
}

QString XMLTag::ToXMLCode(int indent)
{
	QString indentStr;
	for(int i=0; i<indent; i++) indentStr += " ";
	return ToXMLCodeRecursion(indentStr, true);
}

QString XMLTag::ToXMLCodeRecursion(QString indent, bool zeroLevel)
{
	QString tmpIndent = indent;
	if(zeroLevel) indent = "";
	QString ret = indent + "<" + tagName;
	for(auto &attr:attributes)
		ret += " " + attr.first + "=\"" + attr.second + "\"";
	if(nestedTags.empty())
	{
		ret += "/>\n";
	}
	else
	{
		if(!zeroLevel) tmpIndent = tmpIndent + tmpIndent;
		ret += ">\n";
		for(auto &nestdTag:nestedTags)
			ret += nestdTag.ToXMLCodeRecursion(tmpIndent, false);
		ret += indent + "</" + tagName + ">\n";
	}

	if(zeroLevel) ret += "\n";
	return ret;
}
