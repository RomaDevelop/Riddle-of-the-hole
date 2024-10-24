#include "constants.h"

using namespace std;

#include "MyQDom.h"

namespace ConstantsKeywords
{
	const QString setConstant {"УстановитьКонстанту"};
}

QStringList Constant::CellValuesToStringListForEditor() const
{
	QStringList ret;
	for(int i=0; i<Constant_ns::count; i++) ret.append("");

	ret[Constant_ns::nameI] = name;
	ret[Constant_ns::categoryI] = category;
	ret[Constant_ns::describtionI] = describtion;
	ret[Constant_ns::bitSizeI] = QSn(value.GetBitLength());
	ret[Constant_ns::encI] = value.GetEnc();
	ret[Constant_ns::valueI] = value.GetVal();

	return ret;
}

void Constant::SetCellValuesFromEditor(const QStringList & values)
{
	if(values.size() == Constant_ns::count)
	{
		name		= values[Constant_ns::nameI];
		category	= values[Constant_ns::categoryI];
		describtion	= values[Constant_ns::describtionI];
		value.Set(values[Constant_ns::valueI],
				values[Constant_ns::encI],
				values[Constant_ns::bitSizeI].toLongLong());
	}
	else Logs::ErrorSt("Constant::SetCellValuesFromEditor values.size() != Param_ns::Fields::count ("
					   +QSn(values.size())+" != "+QSn(Param_ns::count)+")");
}

void Constant::SetCellValue(QString fieldName, QString newValue)
{
	if(fieldName == Constant_ns::category) category = newValue;
	else if(fieldName == Constant_ns::name) name = newValue;
	else if(fieldName == Constant_ns::describtion) describtion = newValue;
	else if(fieldName == Constant_ns::value) value.SetValue(newValue);
	else if(fieldName == Constant_ns::enc) value.SetEncoding(newValue);
	else if(fieldName == Constant_ns::bitSize) value.SetBitSize(newValue.toLongLong());
	else Logs::ErrorSt("Constant::SetCellValue wrong attribute ["+fieldName+"]");
}

void Constant::SetCellValuesFromDomElement(const QDomElement & paramElement)
{
	auto attrs = MyQDom::Attributes(paramElement);
	if((int)attrs.size() == Constant_ns::count)
	{
		QString val, enc, size;
		for(auto &attr:attrs)
		{
			if(attr.first == Constant_ns::category)			category = attr.second;
			else if(attr.first == Constant_ns::name)			name = attr.second;
			else if(attr.first == Constant_ns::describtion)	describtion = attr.second;
			else if(attr.first == Constant_ns::value)			val = attr.second;
			else if(attr.first == Constant_ns::enc)			enc = attr.second;
			else if(attr.first == Constant_ns::bitSize)		size = attr.second;
			else Logs::ErrorSt("Constant::SetCellValuesFromDomElement wrong param attribute ["+attr.first+"]");
		}
		value.Set(val,enc,size.toLongLong());
	}
	else Logs::ErrorSt("Constant::SetCellValuesFromDomElement wrong attrs size ("+QSn(attrs.size())+")");
}

QStringPairVector Constant::GetAttributes() const
{
	QStringPairVector atts;
	atts.push_back({Constant_ns::category, category});
	atts.push_back({Constant_ns::name, name});
	atts.push_back({Constant_ns::describtion, describtion});
	atts.push_back({Constant_ns::value, value.GetVal()});
	atts.push_back({Constant_ns::enc, value.GetEnc()});
	atts.push_back({Constant_ns::bitSize, QSn(value.GetBitLength())});
	return atts;
}

QString Constant::ToStr() const
{
	return name+" "+category+" "+describtion+" "+value.GetEnc()+" "+value.GetVal();
}

Constants::Constants(const QDomElement & domElement)
{
	auto attributes = MyQDom::Attributes(domElement);
	if((int)attributes.size() == ConstantsVector_ns::count)
	{
		for(auto &attr:attributes)
		{
			if(attr.first == ConstantsVector_ns::name) name = attr.second;
			else Logs::ErrorSt("Constants::Constants wrong attribute [" + QSn(attributes.size()) + "]");
		}
	}
	else Logs::ErrorSt("Constants::Constants wrong attributes.size() = " + QSn(attributes.size()));

	auto subElements = MyQDom::GetTopLevelElements(domElement);
	int rowsSize = subElements.size();
	constants.reserve(rowsSize);
	for(int i=0; i<rowsSize; i++)
	{
		constants.push_back(Constant());
		constants.back().SetCellValuesFromDomElement(subElements[i]);
		constantsNamesMap[constants.back().name] = &constants.back();
	}
}

Constant* Constants::FindConstantPrivate(QString name)
{
	if(auto findRes = constantsNamesMap.find(name); findRes != constantsNamesMap.end())
		return findRes->second;
	return nullptr;
}

const Constant *Constants::FindConstant(QString name) const
{
	return const_cast<Constants*>(this)->FindConstantPrivate(name);
}

QString Constants::SetConstant(QString name, QString newValue)
{
	auto findedConst = FindConstantPrivate(name);
	if(findedConst)
	{
		findedConst->value.SetValue(newValue);
		return "";
	}
	else return className + "::SetConstant find constant result is nullptr";
}

QString Constants::ToStr() const
{
	QString str;
	for(auto &c:constants)
		str += "\t\t" + c.ToStr() +"\n";
	return str;
}
