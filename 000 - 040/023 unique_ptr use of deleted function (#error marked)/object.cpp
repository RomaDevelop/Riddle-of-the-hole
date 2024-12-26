#include <memory>

#include "settings.h"
#include "configconstants.h"
#include "object.h"

using namespace std;

Object::Object(const std::vector<QStringList> &paramsStrs, QString version, QString name_)
{
	if(version == "001")
	{
		name = name_;
		int paramsCount = paramsStrs.size();
		params.resize(paramsCount);
		for(int i=0; i<paramsCount; i++)
		{
			params[i] = make_unique<Param>();
			params[i]->InitParam(paramsStrs[i], this);
			params[i]->InitParamValue();
		}

		// распределение параметров по категориям
		if(paramsCount)
		{
			auto headCat = CategoryPath::FirstCatFromPath(params[0]->CategoryCell());
			category.Set(headCat.name, headCat.number, nullptr);
		}
		if(category.GetNameNumber().name == CategoryKeywords::undefined)
			Error("Object::Object undefined head category");
		for(int i=0; i<paramsCount; i++)
		{
			auto catPath = CategoryPath::PathFromQString(params[i]->CategoryCell());
			category.InsertMember(*params[i].get(), catPath, true);
		}
	}

	Log("Object " + name + " created");
}

std::vector<Param*> Object::FindParamsByName(const TextConstant &nameToFind)
{
	std::vector<Param*> findedParams;

	for(auto &p:params)
		if(p->NameCell() == nameToFind.Value()) findedParams.push_back(p.get());

	if(findedParams.empty()) Error("Protocol::FindParamsByName find nothing. Tryed to find " + nameToFind.Value());
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

Param* Object::FindParamByName(const TextConstant &nameParam)
{
	Param* findedParam {nullptr};
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i]->NameCell() == nameParam.Value())
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
