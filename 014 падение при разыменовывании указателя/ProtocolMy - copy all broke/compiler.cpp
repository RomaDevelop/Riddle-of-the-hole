#include "compiler.h"

void Compiler::RemoveJungAndAddSpaces(QString &text)
{
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

		if(!quats && (text[i] == '(' || text[i] == ')' || text[i] == '=' || text[i] == '+' || text[i] == '>' || text[i] == '<'  || text[i] == '!'))
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

	// remove spaces before and after ,
	while(text.contains(" ,")) text.replace(" ,", ",");
	while(text.contains(", ")) text.replace(", ", ",");

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

QStringList Compiler::TextToCommands(const QString &text)
{
	QStringList commands { text.split(";", QString::SkipEmptyParts) };

	int size = commands.size();
	for(int i=size-1; i>=0; i--)
	{
		RemoveJungAndAddSpaces(commands[i]);
		if(commands[i] == "") { commands.removeAt(i); continue; }
		if(commands[i].left(2) == "//") { commands.removeAt(i); continue; }
	}
	return commands;
}

QStringList Compiler::CommandToWords(const QString &command)
{
	QStringList ret;
	int size = command.size();
	QString word;
	bool quatsNow = false;
	for(int i=0; i<size; i++)
	{
		if(!quatsNow && command[i] == '"') { word += command[i]; quatsNow = true; continue; }
		if(quatsNow  && command[i] == '"') { word += command[i]; quatsNow = false; continue; }

		if(!quatsNow && command[i] == ';') LogsSt::ErrorSt("Error!!! Compiler::CommandToWords command <" + command + "> contains ;");

		if(!quatsNow && command[i] == ' ') { ret += word; word = ""; continue; }
		word += command[i];
	}
	if(word != "") ret += word;
	return ret;
}

QString Compiler::GetFirstWord(const QString &text)
{
	return text.left(text.indexOf(' '));
}

QStringList Compiler::GetParamList(QString command)
{
	command.remove(0,command.indexOf("(")+1);
	command.remove(command.lastIndexOf(")"),command.length());
	return command.split(",", QString::SkipEmptyParts);
}


QString Compiler::GetIdexesText(const QString &operand)
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

std::vector<int> Compiler::GetFirstIndexes(const QString &operand)
{
	std::vector<int> vectInd = DecodeStrNumbers(GetIdexesText(operand),false);
	return vectInd;
}

AllIndexes Compiler::GetAllIndexes(QString operand)
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
	else LogsSt::ErrorSt("GetIndexesFromOperand unrealesed indexes " + operand);
	return indexes;
}

QString Compiler::GetInitialisationStr(const QString &command, bool printErrorIfNoInitialisation)
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
	else if(printErrorIfNoInitialisation) LogsSt::ErrorSt("GetInitialisation wrong initialisation " + command);
	return initialisation;
}

bool Compiler::IsNumber(const QString &str)
{
	if(IsInteger(str) || IsFloating(str)) return true;
	else return false;
}

bool Compiler::IsInteger(const QString &str)
{
	bool ok {false};
	if(str.size())
	{
		if(str[0]=='-') str.toLongLong(&ok);
		else str.toULongLong(&ok);
	}
	return ok;
}

bool Compiler::IsFloating(const QString &str)
{
	bool ok;
	str.toDouble(&ok);
	return ok;
}

QString Compiler::GetOperandName(QString operand)
{
	if(operand.contains('.') && operand.contains('[')) LogsSt::ErrorSt("GetOperandName: unrealesed! operand contains . and [");

	if(operand.contains('.')) operand = operand.left(operand.indexOf('.'));
	if(operand.contains('[')) operand = operand.left(operand.indexOf('['));
	return operand;
}

QString Compiler::GetFirstSubObjectName(QString operand)
{
	QString ret;
	if(operand.contains('.')) ret = operand.remove(0, operand.indexOf('.')+1);
	return ret;
}

std::vector<int> Compiler::DecodeStrNumbers(const QString &strNumbers, bool giveErrorIfEmpty)
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
				else LogsSt::ErrorSt("DecodeStrNumbers error toInt " + strNumbers);
			}
			else LogsSt::ErrorSt("DecodeStrNumbers wrong numbers " + strNumbers);
		}
	}

	if(retVect.empty() && giveErrorIfEmpty) LogsSt::ErrorSt("DecodeStrNumbers empty numbers: [" + strNumbers + "]");
	return retVect;
}

TextConstant::TextConstant(const QString &textConstant): all{textConstant}
{
	value = GetTextConstVal(all);
}

QString TextConstant::GetTextConstVal(const QString &txtConst)
{
	if(!(txtConst[0] == '"' && txtConst[txtConst.length()-1] == '"'))
		LogsSt::ErrorSt("GetTxtConstVal wrong constant " + txtConst);
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
			LogsSt::ErrorSt("IsItTextConstant: " + text + " mock. Multiquats unrealesed.");
		}
		else if(printLog) LogsSt::LogSt("IsItTextConstant: " + text + " is not text constant. Wrong quats count.");
	}
	else if(printLog) LogsSt::LogSt("IsItTextConstant: " + text + " is not text constant. Not stats and ends with quats.");

	return ret;
}
