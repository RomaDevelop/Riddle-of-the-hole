#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vector>

#include <QDebug>
#include <QString>

#include "MyQShortings.h"
#include "MyQExcel.h"

#include "value.h"

class Constant
{
public:
	QString name;
	QString category;
	QString describtion;
	EncondingValue value;

	Constant(const QStringList &row);

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
	void Error(const QString &errStr) { qdbg << errStr; }
};

#endif // CONSTANTS_H
