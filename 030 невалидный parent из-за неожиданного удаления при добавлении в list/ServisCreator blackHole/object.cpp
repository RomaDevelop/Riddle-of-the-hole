#include "object.h"

#include <memory>

#include "MyQDom.h"

using namespace std;

Object::Object(const QDomElement & domElement)
{
	auto attributes = MyQDom::Attributes(domElement);
	if((int)attributes.size() == Object_ns::count)
	{
		QString typeStr;
		for(auto &attr:attributes)
		{
			if(0) {}
			else if(attr.first == Object_ns::name) name = attr.second;
			else if(attr.first == Object_ns::type) typeStr = attr.second;
			else Logs::ErrorSt("Object::Object wrong attribute [" + attr.first + "]");
		}

		if(typeStr == CodeKeyWords::emulatorStr) type = CodeKeyWords::emulatorInt;
		else if(typeStr == CodeKeyWords::servisStr) type = CodeKeyWords::servisInt;
		else Error(GetClassName()+"(): wrong typeStr ["+typeStr+"]");
	}
	else Logs::ErrorSt("Object::Object wrong attributes.size() = " + QSn(attributes.size()));

	auto subElements = MyQDom::GetTopLevelElements(domElement);
	params.reserve(subElements.size());
	for(auto &subElement:subElements)
	{
		params.push_back(make_unique<Param>(subElement, this, type));
	}

	Category::FillCats(category, GetItemsListForFillCats());
	QString topCategoryName = category.GetNameNumber().name;
	if(topCategoryName.startsWith(Protocol_ns::Protocol) || topCategoryName.startsWith(Object_ns::Object))
		Warning("Top category is [] and contains keyword. It will be problem if windows have multiple connections.");

	Log("Object " + name + " created");
}

ItemsListForFillCats Object::GetItemsListForFillCats()
{
	ItemsListForFillCats ret;
	for(auto &param:params)
	{
		ret.items.push_back(param.get());
		ret.categoryStrVals.push_back(param->CategoryCell());
	}
	return ret;
}

std::vector<Param*> Object::FindParamsByName(const QString &nameParam)
{
	std::vector<Param*> findedParams;

	for(auto &p:params)
		if(p->NameCell() == nameParam) findedParams.push_back(p.get());

	if(findedParams.empty()) Error("Protocol::FindParamsByName find nothing. Tryed to find [" + nameParam + "]");
	return findedParams;
}

Param *Object::FindParamById(const QString &id)
{
	Param* findedParam {nullptr};
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i]->IdCell() == id)
		{ findedParam = params[i].get(); break; }
	}

	return findedParam;
}

Param* Object::FindParamByName(const QString &nameParam)
{
	Param* findedParam {nullptr};
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i]->NameCell() == nameParam)
		{ findedParam = params[i].get(); break; }
	}

	return findedParam;
}

std::vector<Param*> Object::FindParamsInCategory(const QString &category_, const QString &nameParam, bool includeSubCats)
{
	std::vector<Param*> findedParams;
	auto path = CategoryPath::PathFromQString(category_);
	auto findedCat = category.FindCategory(path,false);
	if(!findedCat) Warning("Object::FindParamsInCategory category [" + category_ + "] doesn't exists");
	else
	{
		auto members = findedCat->GetMembers(includeSubCats);
		for(auto member:members)
		{
			Param *param = dynamic_cast<Param*>(member);
			if(!param) Error("Object::FindParamsInCategory wrong member! It is " + member->GetClassName());
			else
			{
				if(param->NameCell() == nameParam) findedParams.push_back(param);
			}
		}
	}
	return findedParams;

//// старый поиск простым перебором. Пока не удаляю
//	std::vector<Param*> findedParams;
//	int size = params.size();
//	for(int i=0; i<size; i++)
//	{
//		bool checked = false;
//		if(!includeSubCats && params[i].categoryCell == category && params[i].nameCell == nameParam)
//			checked = true;
//		else if(includeSubCats && params[i].categoryCell.left(category.length()) == category && params[i].nameCell == nameParam)
//			checked = true;

//		if(checked)
//			findedParams.push_back(&params[i]);
//	}
//	return findedParams;
}

//Param *Object::FindParamInCategoryNumber(QString nameParam, QString categoryNumber)
//{
//	Param* findedParam {nullptr};

//	int size = params.size();
//	for(int i=0; i<size; i++)
//	{
//		if(params[i].categoryNumberCell == categoryNumber && params[i].nameCell == nameParam)
//		{
//			findedParam = &params[i];
//			break;
//		}
//	}

//	return findedParam;
//}

//Param* Object::FindParamInModuleNomer(int nomerModule, const TextConstant &nameParam)
//{
//	Param* findedParam {nullptr};

//	QString nomerModuleStr = QSn(nomerModule);
//	QString nameParamStr = nameParam.Value();
//	int size = params.size();
//	for(int i=0; i<size; i++)
//	{
//		if(params[i].moduleNumberCell == nomerModuleStr && params[i].nameCell == nameParamStr)
//		{
//			findedParam = &params[i];
//			break;
//		}
//	}

//	return findedParam;
//}

//Param *Object::FindParamInCategoryAndModule(int nomerModule, const TextConstant &nameParam, const TextConstant &nameCategory)
//{
//	Param* findedParam {nullptr};

//	QString nomerModuleStr = QSn(nomerModule);
//	QString nameParamStr = nameParam.Value();
//	QString nameCatStr = nameCategory.Value();
//	int size = params.size();
//	for(int i=0; i<size; i++)
//	{
//		if(params[i].moduleNumberCell == nomerModuleStr && params[i].nameCell == nameParamStr && params[i].categoryCell == nameCatStr)
//		{
//			findedParam = &params[i];
//			break;
//		}
//	}

//	return findedParam;
//}
