#include "iconfigitem.h"

#include "configconstants.h"

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
