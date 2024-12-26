#ifndef CATEGORY_H
#define CATEGORY_H

#include "ioperand.h"

namespace CategoryKeywords {
	const QString splitterCategory = "::";
	const QString splitterNameNumber = ":";
	const QString undefined = "undefined";
}

struct NameNumber
{
	QString name = CategoryKeywords::undefined;
	QString number = CategoryKeywords::undefined;

	NameNumber() = default;
	NameNumber(const QString &name, const QString &number): name{name}, number{number} {}
	QString ToStrForLog() const { return "[" + name + CategoryKeywords::splitterNameNumber + number + "]"; }
	bool Cmp(const NameNumber& toCmp) const { return (name == toCmp.name && number == toCmp.number);}
	bool operator == (const NameNumber& toCmp) const {return Cmp(toCmp);}
	bool operator != (const NameNumber& toCmp) const {return !Cmp(toCmp);}
};

class CategoryPath
{
	std::vector<NameNumber> path;

public:
	CategoryPath(QString path): path{PathFromQString(path)} {}

	static QString ToStrForLog(std::vector<NameNumber> categoryPath);
	static std::vector<NameNumber> PathFromQString(const QString &pathStr);
	static NameNumber FirstCatFromPath(const QString &pathStr);
};

struct ItemsListForFillCats
{
	std::vector<HaveClassName*> items;
	QStringList categoryStrVals;
};

class Category
{
	QString name = CategoryKeywords::undefined;
	QString number = CategoryKeywords::undefined;
	Category* parentCat = nullptr;

	std::vector<std::unique_ptr<Category>> subCats;
	std::vector<HaveClassName*> members;

public:
	Category() = default;
	Category(const QString &name, const QString &number, Category *parentCat): name{name}, number{number}, parentCat {parentCat} {}
	void InitHead(const QString &name_, const QString &number_) { name = name_; number = number_; parentCat = nullptr; }
	bool Inited() { return name != CategoryKeywords::undefined; }

	int GetDepth();
	NameNumber GetNameNumber() const { return {name,number}; }
	std::vector<HaveClassName*> GetMembers(bool includeSubCats);

	QString ToStrForLog(bool getSubCats, bool getMembers);
	QString MembersToStr();
	QString SubCatsToStr(bool getMembers);

	Category* FindCategory(const std::vector<NameNumber> &path, bool printErrorIfCantFind);
	Category* FindSubCat(const NameNumber &pathItem);

	void InsertCategory(const std::vector<NameNumber> &path);
	void InsertMember(HaveClassName &newMember, const std::vector<NameNumber> &path, bool createCatIfNotExist);

	static void FillCats(Category &category, const ItemsListForFillCats &itemsList);
};


#endif // CATEGORY_H
