#include "addProperties.h"

void AddProperties::Init(QString addPropsCell)
{
	auto strList = addPropsCell.split(nsAddProperties::propertiesSplitter,QString::SkipEmptyParts);
	QStringList names;
	for(auto &str:strList)
	{
		auto twoStr = str.split(nsAddProperties::nameAndValueSplitter,QString::SkipEmptyParts);
		if(twoStr.size() != 2)
			Logs::ErrorSt("AddProperties::Init wrong propery [" + str + "] sise [" + QString::number(twoStr.size())
						  + "] in addPropsCell [" + addPropsCell + "]");
		else
		{
			if(names.contains(twoStr[0])) Logs::ErrorSt("AddProperties::Init дублирование имени [" + twoStr[0] + "] в addPropsCell [" + addPropsCell + "]");
			else
			{
				addProperties.push_back(QStringPair(twoStr[0],twoStr[1]));
				names += twoStr[0];
			}
		}
	}
}

const QString & AddProperties::PropertyValue(QString propertyName)
{
	for(auto &pair:addProperties)
		if(pair.first == propertyName) return pair.second;
	Logs::ErrorSt("AddProperties::PropertyValue свойство с именем [" + propertyName + "] не обнаружено в свойствах " + ToStrForLog());
	return nsAddProperties::notFound;
}

QString AddProperties::ToStrForLog()
{
	QString ret = "[";
	for(auto &pair:addProperties)
		ret += pair.first + nsAddProperties::nameAndValueSplitter + pair.second + nsAddProperties::propertiesSplitter;
	ret.chop(nsAddProperties::propertiesSplitter.length());
	return ret;
}
