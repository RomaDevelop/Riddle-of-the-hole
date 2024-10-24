
#include "configconstants.h"
#include "constants.h"

using namespace std;

namespace ConstantsKeywords
{
	const QString setConstant {"УстановитьКонстанту"};
}

Constant::Constant(const QStringList &row):
	name		{row[ConstantsColIndexes::name]},
	category	{row[ConstantsColIndexes::category]},
	describtion {row[ConstantsColIndexes::describtion]},
	value {
		row[ConstantsColIndexes::value],
		row[ConstantsColIndexes::enc],
		row[ConstantsColIndexes::bitSize].toUInt()
	} {}

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
