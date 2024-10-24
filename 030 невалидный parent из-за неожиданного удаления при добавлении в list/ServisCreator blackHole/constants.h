#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vector>

#include <QDebug>
#include <QString>
#include <QDomDocument>

#include "allConstants.h"
#include "value.h"


class Constant
{
public:
	QString name;
	QString category;
	QString describtion;
	EncondingValue value;

	Constant() = default;

	QStringList CellValuesToStringListForEditor() const;
	void SetCellValuesFromEditor(const QStringList & values);
	void SetCellValue(QString fieldName, QString newValue);
	void SetCellValuesFromDomElement(const QDomElement & paramElement);
	QStringPairVector GetAttributes() const;

	QString ToStr() const;
};

class Constants
{
	QString name;
	std::vector<Constant> constants;
	std::map<QString,Constant*> constantsNamesMap;

	Constant* FindConstantPrivate(QString name);

public:
	Constants(const QDomElement &domElement);
	QString Name() const { return name; }
	const Constant* FindConstant(QString name) const;
	QString SetConstant(QString name, QString newValue); // возвращает сообщение об ошибке или пустую строку
	QString ToStr() const;

	inline static const QString className = "Constants";
};

#endif // CONSTANTS_H
