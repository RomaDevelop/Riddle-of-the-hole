при запуске программы в режиме пошаговой отладки происходил краш при выполнении импорта

класс
class IConfigItem: public HaveClassName
{
	using vector_sh_ptr_IConfigItem = std::vector<std::shared_ptr<IConfigItem>>;
	vector_sh_ptr_IConfigItem subItems;

	IConfigItem *parent = nullptr;
	IConfigItem *nextSibling = nullptr;
	...
	
функция в или после которой происходил краш
через точки остановки выяснил что краш происходит или при выполнении return или после, при уничтожении локальных переменных или при переходе на следующую строку
bool Config::ResolveOneRegularImport(IConfigItem * importElement, Config &importFrom, bool deep)
{
	QString before;
	auto parentForLog = importElement->parent;
	int &counterLog = forRORIIC::showsCount;
	counterLog--;
	if(counterLog > 0) before = forRORIIC::funcName + importElement->parent->ExportToXMLTag(true).ToXMLCode(2, true);

	QString type = importElement->Field(Import_ns::type);
	QString name = importElement->Field(Import_ns::name);
	QString modification = importElement->Field(Import_ns::modification);
	QString policy = importElement->Field(Import_ns::policy);

	// ищем импортируемый элемент
	IConfigItem * importingElementPtr = nullptr;
	std::vector<IConfigItem*> findRes = importFrom.Find_all_if([type,name](const IConfigItem* item){
		return type == item->RowCaption() && name == item->Field(Import_ns::name);
	});
	if(!findRes.empty()) importingElementPtr = findRes[0];
	if(findRes.size() > 1) Logs::ErrorSt(GetClassName() + "::ResolveStandartImport found not one importing item ["+type+":"+name+"]");

	if(!importingElementPtr)
	{
		Logs::ErrorSt(GetClassName() + "::ResolveStandartImport can't find importing ["+type+":"+name+"]");
		if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
		return false;
	}

	// проверяем на вложенный импорт
	if(deep)
	{
		std::vector<IConfigItem*> subItemsOfImportingEl;
		importingElementPtr->SubItemPtrs(subItemsOfImportingEl, true);
		for(auto &sub:subItemsOfImportingEl)
		{
			if(sub->RowCaption() == Import_ns::rowCaption)
			{
				return ResolveOneRegularImport(sub, importFrom, deep);
			}
		}
	}

	// клонируем импортируемый итем,
	// производим модификацию,
	// вставляем его содержимое до импорта,
	// удаляем импорт
	auto importingElementClone = importingElementPtr->CloneItem(true);
	if(importingElementClone)
	{
		if(!modification.isEmpty() && !ImportModification(importingElementClone.get(), modification))
		{
			Logs::ErrorSt(GetClassName() + "::ResolveStandartImport err ImportModification");
			if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
			return false;
		}

		auto &subElements = importingElementClone->SubItems(true);
		for(auto &subElement:subElements)
		{
			if(!importElement->InsertSibling(subElement,IConfigItem::beforeMe))
			{
				Logs::ErrorSt(GetClassName() + "::ResolveStandartImport err inserting");
				if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
				return false;
			}
		}

		importElement->RemoveMe();
		if(counterLog > 0) MyQDialogs::ShowText(before + "\n\n" + parentForLog->ExportToXMLTag(true).ToXMLCode(2, true));
		return true;
	}
	else Logs::ErrorSt(GetClassName() + "::ResolveStandartImport err cloning ["
					   +importingElementPtr->ExportToXMLTag(false).ToXMLCode(2, true)+"]");

	if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
	return false;
}

нашел проблему:
void IConfigItem::RemoveMe()
{
	if(!parent) {Logs::ErrorSt(MSG_PREFIX + "::RemoveMe parent is nullptr"); return; }

	for(uint i=0; i<parent->subItems.size(); i++)
	{
		if(parent->subItems[i].get() == this)
		{
			parent->subItems.erase(parent->subItems.begin()+i);
			// удаление из массива std::vector<std::shared_ptr<IConfigItem>> subItems;
			// а если никто его shared_ptr больше не хранит, будет this будет удалён
			parent = nullptr; // запись в поле удалённого объекта
			return;
		}
	}
	Logs::ErrorSt(MSG_PREFIX + "::RemoveMe this not found in parent subItems");
}

исправление:
void IConfigItem::RemoveMe()
{
	if(!parent) {Logs::ErrorSt(MSG_PREFIX + "::RemoveMe parent is nullptr"); return; }

	auto &siblings = parent->subItems;
	for(uint i=0; i<siblings.size(); i++)
	{
		if(siblings[i].get() == this)
		{
			parent = nullptr;
			if(i != 0) siblings[i-1]->nextSibling = nextSibling;
			siblings.erase(siblings.begin()+i);
			// после erase не должно быть обращения к this, так как он возможно будет уничтожен
			return;
		}
	}
	Logs::ErrorSt(MSG_PREFIX + "::RemoveMe this not found in parent subItems");
}