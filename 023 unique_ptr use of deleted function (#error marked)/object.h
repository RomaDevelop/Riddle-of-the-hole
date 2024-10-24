#ifndef OBJECT_H
#define OBJECT_H

#include <memory>
#include <vector>
#include <queue>

#include "icanchannel.h"

#include "value.h"
#include "code.h"
#include "constants.h"
#include "category.h"
#include "param.h"

class Object: public LogedClass, public HaveClassName
{
public:
	QString classObj;
	QString name;
	std::vector<std::unique_ptr<Param>> params;
	Category category;

	Object(QString classObj_, QString name_): classObj{classObj_}, name{name_} {}
	Object(const std::vector<QStringList> &paramsStrs, QString version, QString name_);
	~Object() { Log("desroyed"); }

	std::vector<Param*> FindParamsByName(const TextConstant &nameToFind);
	std::vector<Param*> FindParamsInCategory(const QString &category, const QString &nameParam, bool includeSubCats);
	Param* FindParamById(const QString &id);
	Param* FindParamByName(const TextConstant &nameParam);
	//Param* FindParamInCategoryNumber(QString nameParam, QString categoryNumber);
	//Param* FindParamInModuleNomer(int nomerModule, const TextConstant &nameParam);
	//Param* FindParamInCategoryAndModule(int nomerModule, const TextConstant &nameParam, const TextConstant &nameCategory);

	virtual QString GetClassName() const override { return classObject(); }
	QString ToStrForLog() const override { return "["+classObj+":"+name+"]"; }

	static QString classObject() { return "Object"; }
};

#endif // OBJECT_H
