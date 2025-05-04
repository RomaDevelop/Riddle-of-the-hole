#include "category.h"


QString CategoryPath::ToStrForLog(std::vector<NameNumber> categoryPath)
{
	QString ret = "<";
	for(auto &cat:categoryPath)
		ret += cat.ToStrForLog() + CategoryKeywords::splitterCategory;
	ret.chop(CategoryKeywords::splitterCategory.length());
	ret += ">";
	return ret;
}

std::vector<NameNumber> CategoryPath::PathFromQString(const QString &pathStr)
{
	std::vector<NameNumber> ret;
	auto path = pathStr.split(CategoryKeywords::splitterCategory);
	if(path.empty()) Logs::ErrorSt("CategoryPath::PathFromQString empty path, pathStr = [" + pathStr + "]");
	for(auto &pathPart:path)
	{
		QString name = pathPart.left(pathPart.indexOf(CategoryKeywords::splitterNameNumber));
		QString number = QString(pathPart).remove(0,name.size()+CategoryKeywords::splitterNameNumber.size());
		ret.push_back(NameNumber(name,number));
	}
	return ret;
}

NameNumber CategoryPath::FirstCatFromPath(const QString &pathStr)
{
	auto path = PathFromQString(pathStr);
	if(!path.empty()) return path[0];
	else Logs::ErrorSt("CategoryPath::FirstCatFromPath get empty path from path str [" + pathStr + "]");
	return {};
}

int Category::GetDepth()
{
	int ret = 0;
	Category *parent = parentCat;
	while(parent)
	{
		ret++;
		parent = parent->parentCat;
	}
	return ret;
}

std::vector<HaveClassName *> Category::GetMembers(bool includeSubCats)
{
	std::vector<HaveClassName*> retMembers = members;
	if(includeSubCats)
	{
		for(auto &subCat:subCats)
		{
			auto subMembers = subCat->GetMembers(includeSubCats);
			retMembers.insert(retMembers.end(),subMembers.begin(),subMembers.end());
		}
	}
	return retMembers;
}

QString Category::MembersToStr()
{
	QString ret;
	if(members.size()) ret += "members: ";
	else ret += "members empty";
	for(auto &member:members)
		ret += "[" + member->GetClassName() + "]";
	return ret;
}

QString Category::SubCatsToStr(bool getMembers)
{
	QString ret;
	QString otstup = "   ";
	int otstupCount = GetDepth();
	for(int i=0; i<otstupCount; i++)
		otstup += "   ";
	
	for(auto &nestedCat:subCats)
	{
		ret += otstup + nestedCat->ToStrForLog(false,getMembers);
		if(!nestedCat->subCats.empty())
		{
			ret += nestedCat->SubCatsToStr(getMembers);
		}
	}

	return ret;
}

QString Category::ToStrForLog(bool getNestedCats, bool getMembers)
{
	QString ret = "[" + name + CategoryKeywords::splitterNameNumber + number + "]";
	if(getMembers) ret += " " + MembersToStr();
	ret += "\n";
	if(getNestedCats)
	{
		ret+= SubCatsToStr(getMembers);
	}
	return ret;
}

Category *Category::FindCategory(const std::vector<NameNumber> &path, bool printErrorIfCantFind)
{
	if(path.empty()) { Logs::ErrorSt("Category::FindCategory empty categoriesPath"); return nullptr; }

	Category *finded = nullptr;
	Category *catToFindIn = this;

	if(catToFindIn->GetNameNumber() == path[0])
	{
		if(path.size() == 1) finded = catToFindIn;

		for(uint i=1; i<path.size(); i++) //  i=1 because [0] cheched in if(categoryPath[0] == ToNameNumber())
		{
			catToFindIn = catToFindIn->FindSubCat(path[i]);
			if(!catToFindIn) break;

			if(i == path.size()-1) finded = catToFindIn;
		}
	}

	if(!finded && printErrorIfCantFind) { Logs::ErrorSt("Category::FindCategory can't find path " + CategoryPath::ToStrForLog(path) + " in\n" + ToStrForLog(true,false)); }
	return finded;
}

Category *Category::FindSubCat(const NameNumber &pathItem)
{
	Category* res = nullptr;
	for(auto &nestedCat:subCats)
	{
		if(nestedCat->GetNameNumber() == pathItem)
		{
			if(res) Logs::ErrorSt("Category::FindNestedCat find more one category " + pathItem.ToStrForLog() + " in category " + ToStrForLog(true,false));
			res = nestedCat.get();
		}
	}
	return res;
}

void Category::InsertCategory(const std::vector<NameNumber> &path)
{
	if(path.empty())
	{
		Logs::ErrorSt("Category::InsertCategory insert empty path");
		return;
	}
	if(FindCategory(path, false))
	{
		Logs::ErrorSt("Category::InsertCategory " + CategoryPath::ToStrForLog(path) + " already exists");
		return;
	}

	if(path[0] == GetNameNumber())
	{
		bool inserted = false;
		Category *categoryToFindIn = this;
		for(uint i=1; i<path.size(); i++) //  i=1 because [0] cheched in if(categoryPath[0] == ToNameNumber())
		{
			auto resFind = categoryToFindIn->FindSubCat(path[i]);
			if(resFind) categoryToFindIn = resFind;
			else
			{
				categoryToFindIn->subCats.push_back(
							std::make_unique<Category>(path[i].name, path[i].number, categoryToFindIn)
							);
				Category *justCreatedCat = categoryToFindIn->subCats.back().get();
				categoryToFindIn = justCreatedCat;

				if(i == path.size()-1) inserted = true;
			}
		}
		if(!inserted) Logs::WarningSt("Category::InsertCategory try insert " + CategoryPath::ToStrForLog(path) + " to\n"
									  + ToStrForLog(true,false) + " but inserted nothing");
	}
	else Logs::ErrorSt("Can't insert path " + CategoryPath::ToStrForLog(path) + " to category " + ToStrForLog(false,false));
}

void Category::InsertMember(HaveClassName &newMember, const std::vector<NameNumber> &path, bool createCatIfNotExist)
{
	if(path.empty()) Logs::ErrorSt("Category::Insert empty categoriesPath. Inserting " + newMember.ToStrForLog());
	else
	{
		auto CatToInsert = FindCategory(path, false);
		
		if(!CatToInsert && createCatIfNotExist)
		{
			InsertCategory(path);
			CatToInsert = FindCategory(path, true);
		}
		
		if(CatToInsert) CatToInsert->members.push_back(&newMember);
		else
		{
			Logs::ErrorSt("Category::Insert error. \n\tTry find path "
						  + CategoryPath::ToStrForLog(path)
						  + "\n\tfor member " + newMember.GetClassName() + " " + newMember.ToStrForLog());
		}
	}
}

void Category::FillCats(Category & category, const ItemsListForFillCats & itemsList)
{
	int Count = itemsList.items.size();
	if(Count)
	{
		if(Count == itemsList.categoryStrVals.size())
		{
			auto headCat = CategoryPath::FirstCatFromPath(itemsList.categoryStrVals[0]);
			category.InitHead(headCat.name, headCat.number);
			if(category.Inited())
			{
				for(int i=0; i<Count; i++)
				{
					auto catPath = CategoryPath::PathFromQString(itemsList.categoryStrVals[i]);
					category.InsertMember(*itemsList.items[i], catPath, true);
				}
			}
			else Logs::ErrorSt("Category::FillCats error head category not inited");
		}
		else Logs::ErrorSt("Category::FillCats Count != itemsCategories.size()");
	}
}
