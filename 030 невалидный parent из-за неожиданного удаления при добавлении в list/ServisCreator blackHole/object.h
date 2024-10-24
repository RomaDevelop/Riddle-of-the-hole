#ifndef OBJECT_H
#define OBJECT_H

#include <memory>
#include <vector>
#include <queue>

#include "category.h"
#include "param.h"

class Object: public LogedClass, public HaveClassName
{
	ItemsListForFillCats GetItemsListForFillCats();

public:
	QString name;
	int type = CodeKeyWords::codeUndefined;
	std::vector<std::unique_ptr<Param>> params;
	Category category;

	Object() = default;
	Object(const QDomElement & domElement);

	~Object() { Log("desroyed"); }

	std::vector<Param*> FindParamsByName(const QString &nameParam);
	std::vector<Param*> FindParamsInCategory(const QString &category, const QString &nameParam, bool includeSubCats);
	Param* FindParamById(const QString &id);
	Param* FindParamByName(const QString &nameParam);
	//Param* FindParamInCategoryNumber(QString nameParam, QString categoryNumber);
	//Param* FindParamInModuleNomer(int nomerModule, const TextConstant &nameParam);
	//Param* FindParamInCategoryAndModule(int nomerModule, const TextConstant &nameParam, const TextConstant &nameCategory);

	virtual QString GetClassName() const override { return classObject; }
	virtual QString ToStrForLog() const override { return "["+name+"]"; }

	inline static const QString classObject { Object_ns::Object };
};

#endif // OBJECT_H
