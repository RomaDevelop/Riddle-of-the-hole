#include "configconstants.h"
#include "constants.h"

using namespace std;

namespace ConstantsKeywords
{
	const QString setConstant {"УстановитьКонстанту"};
}

Constant::Constant(const QStringList &row):
	name		{row[Constants_ns::Excel::name]},
	category	{row[Constants_ns::Excel::category]},
	describtion {row[Constants_ns::Excel::describtion]},
	value {
		row[Constants_ns::Excel::value],
		row[Constants_ns::Excel::enc],
		row[Constants_ns::Excel::bitSize].toUInt()
  } {}

QStringList Constant::CellValuesToStringListForEditor() const
{
	QStringList ret;
	for(int i=0; i<Constants_ns::Fields::count; i++) ret.append("");

	ret[Constants_ns::Fields::nameI] = name;
	ret[Constants_ns::Fields::categoryI] = category;
	ret[Constants_ns::Fields::describtionI] = describtion;
	ret[Constants_ns::Fields::bitSizeI] = QSn(value.GetBitLength());
	ret[Constants_ns::Fields::encI] = value.GetEnc();
	ret[Constants_ns::Fields::valueI] = value.GetVal();

	return ret;
}

void Constant::SetCellValuesFromEditor(const QStringList & values)
{
	if(values.size() == Constants_ns::Fields::count)
	{
		name		= values[Constants_ns::Fields::nameI];
		category	= values[Constants_ns::Fields::categoryI];
		describtion	= values[Constants_ns::Fields::describtionI];
		value.Set(values[Constants_ns::Fields::valueI],
				values[Constants_ns::Fields::encI],
				values[Constants_ns::Fields::bitSizeI].toLongLong());
	}
	else Logs::ErrorSt("Constant::SetCellValuesFromEditor values.size() != Param_ns::Fields::count ("+QSn(values.size())+" != "+QSn(Param_ns::Fields::count)+")");
}

void Constant::SetCellValue(QString fieldName, QString newValue)
{
	if(fieldName == Constants_ns::Fields::category) category = newValue;
	else if(fieldName == Constants_ns::Fields::name) name = newValue;
	else if(fieldName == Constants_ns::Fields::describtion) describtion = newValue;
	else if(fieldName == Constants_ns::Fields::value) value.Set(newValue, value.GetEnc(), value.GetBitLength());
	else if(fieldName == Constants_ns::Fields::enc) value.Set(value.GetVal(), newValue, value.GetBitLength());
	else if(fieldName == Constants_ns::Fields::bitSize) value.Set(value.GetVal(), value.GetEnc(), newValue.toLongLong());
	else Logs::ErrorSt("Constant::SetCellValue wrong attribute ["+fieldName+"]");
}

void Constant::SetCellValuesFromDomElement(const QDomElement & paramElement)
{
	auto attrs = DomAdd::GetAttributes(paramElement);
	if((int)attrs.size() == Constants_ns::Fields::count)
	{
		QString val, enc, size;
		for(auto &attr:attrs)
		{
			if(attr.first == Constants_ns::Fields::category)			category = attr.second;
			else if(attr.first == Constants_ns::Fields::name)			name = attr.second;
			else if(attr.first == Constants_ns::Fields::describtion)	describtion = attr.second;
			else if(attr.first == Constants_ns::Fields::value)			val = attr.second;
			else if(attr.first == Constants_ns::Fields::enc)			enc = attr.second;
			else if(attr.first == Constants_ns::Fields::bitSize)		size = attr.second;
			else Logs::ErrorSt("Constant::SetCellValuesFromDomElement wrong param attribute ["+attr.first+"]");
		}
		value.Set(val,enc,size.toLongLong());
	}
	else Logs::ErrorSt("Constant::SetCellValuesFromDomElement wrong attrs size ("+QSn(attrs.size())+")");
}

QStringPairVector Constant::GetAttributes() const
{
	QStringPairVector atts;
	atts.push_back({Constants_ns::Fields::category, category});
	atts.push_back({Constants_ns::Fields::name, name});
	atts.push_back({Constants_ns::Fields::describtion, describtion});
	atts.push_back({Constants_ns::Fields::value, value.GetVal()});
	atts.push_back({Constants_ns::Fields::enc, value.GetEnc()});
	atts.push_back({Constants_ns::Fields::bitSize, QSn(value.GetBitLength())});
	return atts;
}

Constants::Constants(const std::vector<QStringList> &definesConstants, QString Version, QString startSettings)
{
	if(Version == "001")
	{
		int rowsSize = definesConstants.size();
		constants.reserve(rowsSize);
		for(int i=0; i<rowsSize; i++)
			constants.push_back(definesConstants[i]);

		// присвоение константам значений указанных в []
		for(int i=0; i<rowsSize; i++)
			if(constants[i].value.GetVal().indexOf('[') != -1)
			{
				QString nameToFind = constants[i].value.GetVal().mid(1,constants[i].name.length()-2);
				int fInd {-1};
				for(int j=0; j<rowsSize; j++)
					if(constants[j].name == nameToFind) fInd=j;
				if(fInd == -1) Logs::ErrorSt("Constants::Constants Не обнаружен идентификатор "+nameToFind+" (строка: "+constants[i].ToStr());
				else constants[i].value.Set(constants[fInd].value);
			}

		// применение начальных настроек, передаваемых строкой startSettings
		auto statrStgs {Code::TextToCommands(startSettings)};
		for(int i=statrStgs.size()-1; i>=0; i--) { if(Code::GetFirstWord(statrStgs[i]) != ConstantsKeywords::setConstant) statrStgs.removeAt(i); }

		vector<QStringList> statrStgsWords;
		statrStgsWords.reserve(statrStgs.size());
		for(int i=0; i<statrStgs.size(); i++) statrStgsWords.push_back(Code::CommandToWords(statrStgs[i]));

		for(uint ss=0; ss<statrStgsWords.size(); ss++)
		{
			Constant *findedConstSubj = FindConstantPrivate(statrStgsWords[ss][1]);
			Constant *findedConstPred = FindConstantPrivate(statrStgsWords[ss][3]);
			if(statrStgsWords[ss][2] == Operators::assign)
			{
				if(findedConstSubj && findedConstPred) findedConstSubj->value = findedConstPred->value;
				else Logs::ErrorSt("Constants::Constants Can't find constat while do setting " + statrStgsWords[ss].join(" "));
			}
			else Logs::ErrorSt("Constants::Constants Unrealesed setting operation " + statrStgsWords[ss].join(" "));
		}
	}
}

Constant *Constants::FindConstantPrivate(QString name)
{
	for(uint i=0; i<constants.size(); i++)
		if(constants[i].name == name)
			return &constants[i];
	return nullptr;
}

const Constant *Constants::FindConstant(QString name) const
{
	return const_cast<Constants*>(this)->FindConstantPrivate(name);
}
