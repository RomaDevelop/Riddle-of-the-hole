#include "compiler.h"

void Compiler::RemoveJungAddSpaces(QString &text)
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

	while(text.contains("= =")) text.replace("= =", "==");
	while(text.contains("! =")) text.replace("! =", "!=");
	while(text.contains("= =")) text.replace("+ =", "+=");
	while(text.contains("> =")) text.replace("> =", ">=");
	while(text.contains("< =")) text.replace("< =", "<=");

	while(text.length() && text[0] == ' ') text.remove(0,1); // remove spaces at begin
	while(text.length() && text[text.length()-1] == ' ') text.remove(text.length()-1,1); // remove spaces at end
}

QStringList Compiler::TextToCommands(const QString &text)
{
	QStringList commands { text.split(";", QString::SkipEmptyParts) };

	int size = commands.size();
	for(int i=size-1; i>=0; i--)
	{
		RemoveJungAddSpaces(commands[i]);
		if(commands[i] == "") { commands.removeAt(i); continue; }
		if(commands[i].left(2) == "//") { commands.removeAt(i); continue; }
	}
	return commands;
}

QStringList Compiler::CommandToWords(const QString &command)
{
	if(command.contains(';')) qdbg << "Error!!! Compiler::CommandToWords command <<" << command << ">> contains ;";
	QStringList ret;
	int size = command.size();
	QString word;
	bool quatsNow = false;
	for(int i=0; i<size; i++)
	{
		if(!quatsNow && command[i] == '"') { word += command[i]; quatsNow = true; continue; }
		if(quatsNow  && command[i] == '"') { word += command[i]; quatsNow = false; continue; }

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


std::vector<int> Compiler::GetFirstIndexes(QString str)
{
	std::vector<int> vectInd;
	if(str.contains('[') && str.contains(']'))
	{
		QString tmpDatOp {str};
		tmpDatOp.remove(0,tmpDatOp.indexOf("[")+1);
		tmpDatOp = tmpDatOp.left(tmpDatOp.indexOf("]"));

		bool ok;
		int index = tmpDatOp.toInt(&ok);
		if(ok) vectInd = {index};
		else if(tmpDatOp.contains(','))
		{
			auto indStrList { tmpDatOp.split(',',QString::SkipEmptyParts) };
			for(auto &indStr:indStrList)
			{
				index = indStr.toInt(&ok);
				if(ok) vectInd.push_back(index);
				else Error("GetFirstIndexes error toInt " + str);
			}
		}
		else if(tmpDatOp == "") {/* empty indexes, will return default empty vectInd */}
		else Error("GetFirstIndexes indexes " + str);
	}
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
	else Error("GetIndexesFromOperand unrealesed indexes " + operand);
	return indexes;
}

QString Compiler::GetInitialisationVal(QString command)
{
	if(command.count('{') == 1 && command.count('}') == 1)
	{
		command.remove(0,command.indexOf('{')+1);
		command = command.left(command.indexOf('}'));
	}
	else Error("GetInitialisation wrong initialisation " + command);
	return command;
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

bool Compiler::IsNumber(const QString &str)
{
	if(IsInteger(str) || IsFloating(str)) return true;
	else return false;
}

QString Compiler::GetOperandName(QString operand)
{
	if(operand.contains('.') && operand.contains('[')) Error("GetOperandName: unrealesed! operand contains . and [");

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

TextConstant::TextConstant(const QString &textConstant): all{textConstant}
{
	value = GetTextConstVal(all);
}

QString TextConstant::GetTextConstVal(const QString &txtConst)
{
	if(!(txtConst[0] == '"' && txtConst[txtConst.length()-1] == '"'))
		qdbg << "TextConstant::GetTxtConstVal wrong constant" << txtConst;
	return txtConst.mid(1,txtConst.length()-2);
}
