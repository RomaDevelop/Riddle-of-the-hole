#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vector>

#include <QDebug>
#include <QString>

#include "value.h"

class Constant
{
public:
	QString name;
	QString category;
	QString describtion;
	EncondingValue value;

	Constant() = default;
	Constant(const QStringList &row);

	QStringList CellValuesToStringListForEditor() const;
	void SetCellValuesFromEditor(const QStringList & values);
	void SetCellValue(QString fieldName, QString newValue);
	void SetCellValuesFromDomElement(const QDomElement & paramElement);
	QStringPairVector GetAttributes() const;

	QString ToStr() const {return "name="+name+"; category="+category+"; describtion="+describtion+"; encoding="+value.GetEnc()+"; value="+value.GetVal()+";"; }
};

class Constants
{
	std::vector<Constant> constants;

	Constant* FindConstantPrivate(QString name);

public:
	Constants(const std::vector<QStringList> &definesConstants, QString Version, QString startSettings);
	QStringList NamesList()
	{
		QStringList list;
		int size = constants.size();
		for(int i=0; i<size; i++)
		{
			list += constants[i].name;
		}
		return list;
	}
	const Constant* FindConstant(QString name) const;
	QString ToStr() const { QString str; for(auto &c:constants) str += c.ToStr() +"\n"; return str; }
};

#endif // CONSTANTS_H
