#include "code.h"

void Code::Normalize(QString &text)
{
	if(0) qdbg << CodeMarkers::needOptimisation + " Compiler::Normalize"
						+ " избавиться от всех contains и replace (лишние проходы по text)";

	bool quats = false;
	// replace '\n' '\r' '\t' by spaces
	// add spaces before and after operators
	for(int i = text.length()-1; i>=0; i--)
	{
		if(text[i] == CodeKeyWords::textConstantSplitter && !quats) { quats = true; continue; }
		if(text[i] == CodeKeyWords::textConstantSplitter && quats) { quats = false; continue; }

		if(!quats && (text[i] == '\n' || text[i] == '\r' || text[i] == '\t'))
		{
			text[i] = ' ';
			continue;
		}

		if(!quats && (text[i] == '(' || text[i] == ')' || text[i] == ','
					  || text[i] == '[' || text[i] == ']'
					  || text[i] == '=' || text[i] == '+' || text[i] == '-'
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
		if(text[i] == CodeKeyWords::textConstantSplitter && !quats) { quats = true; continue; }
		if(text[i] == CodeKeyWords::textConstantSplitter && quats) { quats = false; continue; }

		if(!quats && (i > 0 && text[i] == ' ' && text[i-1] == ' ')) { text.remove(i,1); continue; }
	}

	// remove spaces betwieen two-sybolic operands
	while(text.contains("= =")) text.replace("= =", "==");
	while(text.contains("! =")) text.replace("! =", "!=");
	while(text.contains("> =")) text.replace("> =", ">=");
	while(text.contains("< =")) text.replace("< =", "<=");

	while(text.contains("+ =")) text.replace("+ =", "+=");
	while(text.contains("- =")) text.replace("- =", "-=");

	// remove spaces at the begining and in the end
	while(text.length() && text[0] == ' ') text.remove(0,1);
	while(text.length() && text[text.length()-1] == ' ') text.remove(text.length()-1,1);
}

QStringList Code::TextToCommands(const QString &text)
{

	QStringList commands;
	bool quats = false;
	QString command;
	int sizeText = text.size();
	for(int i = 0; i<sizeText; i++)
	{
		if(!quats && text[i] == CodeKeyWords::commandSplitter)
		{
			commands.push_back(command);
			command.clear();
			continue;
		}
		command += text[i];

		if(text[i] == CodeKeyWords::textConstantSplitter && !quats) { quats = true; continue; }
		if(text[i] == CodeKeyWords::textConstantSplitter && quats) { quats = false; continue; }
	}
	if(command.size()) commands.push_back(std::move(command));

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
		if(!quatsNow && command[i] == CodeKeyWords::textConstantSplitter) { word += command[i]; quatsNow = true; continue; }
		if(quatsNow  && command[i] == CodeKeyWords::textConstantSplitter)
		{
			if(i != 0 && command[i-1] == '\\' )
				continue;

			word += command[i]; quatsNow = false; continue;
		}

		if(!quatsNow && command[i] == CodeKeyWords::commandSplitter)
			Logs::ErrorSt("Error!!! Compiler::CommandToWords command [" + command + "] contains ;");

		if(!quatsNow && command[i] == CodeKeyWords::wordsSplitter) { ret += word; word = ""; continue; }
		word += command[i];
	}
	if(word != "") ret += word;
	if(quatsNow) Logs::ErrorSt("Code::CommandToWords not closed text constant in command [" + command + "]");
	return ret;
}

QString Code::GetFirstWord(const QString &text)
{
	return text.left(text.indexOf(CodeKeyWords::wordsSplitter));
}

QStringList Code::GetTextsInSquareBrackets(const QString &text)
{
	QStringList result;
	result.push_back(QString());
	int opSize = text.size();
	bool indexesNow = false;
	bool quatsNow = false;
	int nestedIndexes = 0;
	for(int i=0; i<opSize; i++)
	{
		if(indexesNow && text[i] == ']' && nestedIndexes == 0)
		{
			indexesNow = false;
			result.push_back(QString());
		}

		if(indexesNow) result.back() += text[i];
		if(indexesNow && text[i] == '[') nestedIndexes++;
		if(indexesNow && text[i] == ']') nestedIndexes--;

		if(!quatsNow && text[i] == CodeKeyWords::textConstantSplitter) { quatsNow = true; continue; }
		if(quatsNow && text[i] == CodeKeyWords::textConstantSplitter) { quatsNow = false; continue; }

		if(!quatsNow && text[i] == '[') indexesNow = true;
	}
	if(result.back().isEmpty()) result.removeLast();

	if(quatsNow) Logs::ErrorSt("not closed quats in text [" + text + "]");

	if(indexesNow) Logs::ErrorSt("not closed brackets in text [" + text + "]");

	return result;

	if(0) // test for this function
	{
		QStringList tests;
		tests << "" << "'sacsd[ds]sdvsdv' ['s[]d']  [[]]   [123]  ['sdvsdvsdv[]']   ['[]sdvsdv']" /*<< "[1]" << "[123]" << "[][1]" << "[1][]" << "[][][]" << "[1][2][3][4" << "[1][2][3][4]" << "[]["*/;
		for(auto &test:tests)
		{
			auto res = GetTextsInSquareBrackets(test);
			qdbg << "test" << test;
			qdbg << "content" << res.size() << res.join(" ; ");
			qdbg << " ";
		}
	}

	return result;
}

AllIndexes Code::GetAllIndexes(QString text)
{
	AllIndexes result;
	auto textsInBrackets = GetTextsInSquareBrackets(text);

	for(auto &subText:textsInBrackets)
	{
		if(!subText.isEmpty())
		{
			auto indexes = DecodeStrNumbers(subText,true);
			result.indexes.push_back(indexes);
		}
		else result.indexes.push_back({}); // если скобки пустые - добавляем пустой вектор
	}

	if(0)
	{
		QStringList tests;
		tests << "" << "[[]]   [123]  ";
		tests << "sdsdv[123]sdvsdv[][1][1,2,3,4][1-5,7-8]";
		tests << "[5-2]";
		tests << "[5-5]";
		for(auto &test:tests)
		{
			auto res = GetAllIndexes(test);
			qdbg << "test" << test;
			for(auto &indexes:res.indexes)
			{
				QString indexesStr;
				for(auto &index:indexes)
					indexesStr += QSn(index) + " ";
				qdbg << "indexes" << indexesStr;
			}
			qdbg << " ";
		}
	}

	return result;
}

void Code::RemoveEmulatorServis(QStringList & commands, int codeCurrent)
{
	for(int i=commands.size()-1; i>=0; i--)
	{
		bool startsWithEmulator = commands[i].startsWith(CodeKeyWords::emulatorStr);
		bool startsWithServis = commands[i].startsWith(CodeKeyWords::servisStr);
		if(startsWithEmulator && codeCurrent != CodeKeyWords::emulatorInt) commands.removeAt(i);
		if(startsWithServis && codeCurrent != CodeKeyWords::servisInt) commands.removeAt(i);
		if(startsWithEmulator && codeCurrent == CodeKeyWords::emulatorInt) commands[i].remove(0,CodeKeyWords::emulatorStr.size()+1);
		if(startsWithServis && codeCurrent == CodeKeyWords::servisInt) commands[i].remove(0,CodeKeyWords::servisStr.size()+1);
	}
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
	std::vector<int> result;
	auto elements = strNumbers.split(',',QString::SkipEmptyParts);
	QString error;
	for(auto &element:elements)
	{
		bool ok;
		int intElement = element.toInt(&ok);
		if(ok) result.push_back(intElement);

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
					if(end <= start) error = "DecodeStrNumbers error end("+QSn(end)+") <= start("+QSn(start)+") " + strNumbers;
					else
					{
						for(int i=start; i<=end; i++) result.push_back(i);
					}
				}
				else error = "DecodeStrNumbers error toInt " + strNumbers;
			}
			else error = "DecodeStrNumbers wrong numbers " + strNumbers;
		}
		if(!error.isEmpty()) break;
	}

	if(!error.isEmpty())
	{
		Logs::ErrorSt(error);
		result.clear();
	}

	if(printErrorIfEmpty && result.empty()) Logs::ErrorSt("DecodeStrNumbers empty numbers: [" + strNumbers + "]");

	return result;
}

QString TextConstant::GetTextConstVal(const QString &txtConst)
{
	if(IsItTextConstant(txtConst, false))
	{
		QString ret = txtConst;
		ret.chop(CodeKeyWords::textConstantSplitterLength);
		ret.remove(0,CodeKeyWords::textConstantSplitterLength);
		return ret;
	}
	else Logs::ErrorSt("GetTxtConstVal wrong text constant [" + txtConst + "]");
	return "";
}

bool TextConstant::IsItTextConstant(const QString &text, bool printLog)
{
	bool ret = false;
	if(text.size() && text[0] == CodeKeyWords::textConstantSplitter && text[text.length()-1] == CodeKeyWords::textConstantSplitter)
	{
		if(text.count(CodeKeyWords::textConstantSplitter) == 2) ret = true;
		else if(printLog) Logs::ErrorSt("IsItTextConstant: " + text + " mock. Multiquats unrealesed.");
	}
	else if(printLog) Logs::LogSt("IsItTextConstant: " + text + " is not text constant. Not stats and ends with quats.");

	return ret;
}

QString TextConstant::AddQuates(const QString & str)
{
	return CodeKeyWords::textConstantSplitter + str + CodeKeyWords::textConstantSplitter;
}

QString CodeKeyWords::TypeToStr(int type)
{
	if(type == emulatorInt) return "Эмулятор";
	if(type == servisInt) return "Сервис";
	if(type == codeUndefined) return "undefined";
	Logs::ErrorSt("TypeToStr unnown type" + QSn(type));
	return "unnown type";
}
