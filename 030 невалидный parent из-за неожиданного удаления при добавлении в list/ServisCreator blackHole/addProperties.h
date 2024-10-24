#ifndef ADDPROPERTIES_H
#define ADDPROPERTIES_H

#include "logs.h"

namespace nsAddProperties {
	const QString nameAndValueSplitter {":"};
	const QString propertiesSplitter {"::"};
	const QString notFound {"notFound"};
}

typedef std::pair<QString, QString> QStringPair;

class AddProperties
{
	std::vector<QStringPair>  addProperties;
public:

	void Init(QString addPropsCell);
	const QString& PropertyValue(QString propertyName);
	QString ToStrForLog();

};

#endif // ADDPROPERTIES_H
