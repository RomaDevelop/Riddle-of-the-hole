#include "code.h"

void Code::Normalize(QString &text)
{
	if(0)
		Logs::WarningSt(CodeMarkers::needOptimisation + " Compiler::Normalize"
						+ " избавиться от всех contains и replace (лишние проходы по text)");

	bool quats = false;
	// replace '\n' '\r' '\t' by spaces
	// add spaces before and after operators
	for(int i = text.length()-1; i>=0; i--)
	{
		if(text[i] == '"' && !quats) { quats = true; continue; }
		if(text[i] == '"' && quats) { quats = false; continue; }

		if(!quats && (text[i] == '\n' || text[i] == '\r' || text[i] == '\t'))
		{
			text[i] = ' ';
			continue;
		}

		if(!quats && (text[i] == '(' || text[i] == ')' || text[i] == ','
					  || text[i] == '[' || text[i] == ']'
					  || text[i] == '=' || text[i] == '+'
					  || text[i] == '>' || text[i] == '<'  || text[i] == '!'
					  || text[i] == '.'))
		{
			text.insert(i+1,' ');
			text.insert(i,' ');
			continue;
		}
	}

	//remove double spaces
	for(int i = text.length()-1; i>=0; i--)
	{
		if(text[i] == '"' && !quats) { quats = true; continue; }
		if(text[i] == '"' && quats) { quats = false; continue; }

		if(!quats && (i > 0 && text[i] == ' ' && text[i-1] == ' ')) { text.remove(i,1); continue; }
	}

	// remove spaces betwieen two-sybolic operands
	while(text.contains("= =")) text.replace("= =", "==");
	while(text.contains("! =")) text.replace("! =", "!=");
	while(text.contains("> =")) text.replace("> =", ">=");
	while(text.contains("< =")) text.replace("< =", "<=");

	while(text.contains("+ =")) text.replace("+ =", "+=");

	// remove spaces at the begining and in the end
	while(text.length() && text[0] == ' ') text.remove(0,1);
	while(text.length() && text[text.length()-1] == ' ') text.remove(text.length()-1,1);
}

QStringList Code::TextToCommands(const QString &text)
{
	QStringList commands { text.split(";", QString::SkipEmptyParts) };

	int size = commands.size();
	for(int i=size-1; i>=0; i--)
	{
		Normalize(commands[i]);
		if(commands[i] == "") { commands.removeAt(i); continue; }
		if(commands[i].left(2) == "//") { commands.removeAt(i); continue; }
	}
	return commands;
}



QStringList Code::CommandToWords(const QString &command)
{
	QStringList ret;
	int size = command.size();
	QString word;
	bool quatsNow = false;
	for(int i=0; i<size; i++)
	{
		if(!quatsNow && command[i] == '"') { word += command[i]; quatsNow = true; continue; }
		if(quatsNow  && command[i] == '"') { word += command[i]; quatsNow = false; continue; }

		if(!quatsNow && command[i] == ';') Logs::ErrorSt("Error!!! Compiler::CommandToWords command <" + command + "> contains ;");

		if(!quatsNow && command[i] == ' ') { ret += word; word = ""; continue; }
		word += command[i];
	}
	if(word != "") ret += word;
	return ret;
}

QString Code::GetFirstWord(const QString &text)
{
	return text.left(text.indexOf(' '));
}

QString Code::GetIdexesText(const QString &operand)
{
	QString indexesStr;
	int opSize = operand.size();
	bool indexesNow = false;
	int nestedIndexes = 0;
	for(int i=0; i<opSize; i++)
	{
		if(indexesNow && operand[i] == ']' && nestedIndexes == 0) break;

		if(indexesNow) indexesStr += operand[i];
		if(indexesNow && operand[i] == '[') nestedIndexes++;
		if(indexesNow && operand[i] == ']') nestedIndexes--;

		if(operand[i] == '[') indexesNow = true;
	}
//	if(operand.contains('[') && operand.contains(']'))
//	{
//		indexesStr = {operand};
//		indexesStr.remove(0,indexesStr.indexOf("[")+1);
//		indexesStr = indexesStr.left(indexesStr.indexOf("]"));
//	}
	return indexesStr;
}

std::vector<int> Code::GetFirstIndexes(const QString &operand)
{
	std::vector<int> vectInd = DecodeStrNumbers(GetIdexesText(operand),false);
	return vectInd;
}

AllIndexes Code::GetAllIndexes(QString operand)
{
	AllIndexes indexes;
	if(!operand.contains('[') && !operand.contains(']')) return indexes;
	else if(operand.count('[') == 1 && operand.count(']') == 1)
	{
		indexes.first = GetFirstIndexes(operand);
	}
	else if(operand.count('[') == 2 && operand.count(']') == 2)
	{
		indexes.first = GetFirstIndexes(operand);
		operand.remove(0,operand.indexOf(']')+1);
		indexes.secnd = GetFirstIndexes(operand);
	}
	else Logs::ErrorSt("GetIndexesFromOperand unrealesed indexes " + operand);
	return indexes;
}

QString Code::GetInitialisationStr(const QString &command, bool printErrorIfNoInitialisation)
{
	QString initialisation;
	if(command.count('{') == 1 && command.count('}') == 1)
	{
		initialisation = command;
		initialisation.remove(0,initialisation.indexOf('{')+1);
		initialisation = initialisation.left(initialisation.indexOf('}'));

		while(initialisation.size() && initialisation[initialisation.size()-1] == ' ') initialisation.chop(1);
		while(initialisation.size() && initialisation[0] == ' ') initialisation.remove(0,1);
	}
	else if(printErrorIfNoInitialisation) Logs::ErrorSt("GetInitialisation wrong initialisation " + command);
	return initialisation;
}

bool Code::IsNumber(const QString &str)
{
	if(IsInteger(str) || IsFloating(str)) return true;
	else return false;
}

bool Code::IsInteger(const QString &str)
{
	bool ok {false};
	if(str.size())
	{
		if(str[0]=='-') str.toLongLong(&ok);
		else str.toULongLong(&ok);
	}
	return ok;
}

bool Code::IsFloating(const QString &str)
{
	bool ok;
	str.toDouble(&ok);
	return ok;
}

std::vector<int> Code::DecodeStrNumbers(const QString &strNumbers, bool printErrorIfEmpty)
{
	std::vector<int> retVect;
	auto elements = strNumbers.split(',',QString::SkipEmptyParts);
	for(auto &element:elements)
	{
		bool ok;
		int intElement = element.toInt(&ok);
		if(ok) retVect.push_back(intElement);

		if(!ok)
		{
			if(element.count('-') == 1)
			{
				QString first = element.left(element.indexOf('-'));
				QString secnd = element.remove(0,first.length()+1);
				bool ok1, ok2;
				int start = first.toInt(&ok1);
				int end = secnd.toInt(&ok2);
				if(ok1 && ok2)
				{
					for(int i=start; i<=end; i++) retVect.push_back(i);
				}
				else Logs::ErrorSt("DecodeStrNumbers error toInt " + strNumbers);
			}
			else Logs::ErrorSt("DecodeStrNumbers wrong numbers " + strNumbers);
		}
	}

	if(retVect.empty() && printErrorIfEmpty) Logs::ErrorSt("DecodeStrNumbers empty numbers: [" + strNumbers + "]");
	return retVect;
}



QString TextConstant::GetTextConstVal(const QString &txtConst)
{
	if(!(txtConst[0] == '"' && txtConst[txtConst.length()-1] == '"'))
		Logs::ErrorSt("GetTxtConstVal wrong constant " + txtConst);
	return txtConst.mid(1,txtConst.length()-2);
}

bool TextConstant::IsItTextConstant(const QString &text, bool printLog)
{
	bool ret = false;
	if(text.size() && text[0] == '\"' && text[text.length()-1] == '\"')
	{
		if(text.count('\"') == 2) ret = true;
		else if(text.count('\"') % 2 == 0)
		{
			Logs::ErrorSt("IsItTextConstant: " + text + " mock. Multiquats unrealesed.");
		}
		else if(printLog) Logs::LogSt("IsItTextConstant: " + text + " is not text constant. Wrong quats count.");
	}
	else if(printLog) Logs::LogSt("IsItTextConstant: " + text + " is not text constant. Not stats and ends with quats.");

	return ret;
}
