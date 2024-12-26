#ifndef COMPILER_H
#define COMPILER_H

#include <QString>
#include <QStringList>

#include "MyQShortings.h"

#include "logedclass.h"

namespace CodeMarkers
{
	const QString mock {"MOCK MOCK MOCK"};
	const QString needOptimisation {"NEED OPTIMISATION"};
};

namespace CommandsKeyWords
{
	const QString idFilter {"ID:"};
	const QString dataFilter {"Data:"};
	const QString emulator {"Эмулятор"};
	const QString servis {"Сервис"};
	const QString dataOperand {"Data"};

	const QString obrabotchik {"Обработчик"};
	const QString transform {"Transform"};

	const QString null {"null"};
};

namespace Operators
{
	const QString assign {"="};
	const QString plusAssign {"+="};
	const QString cmpEqual {"=="};
	const QString clear {"clear"};
	const QString index1 {"["};
	const QString index2 {"]"};

	const QStringList all {assign, plusAssign, cmpEqual, clear, index1};
	const QStringList unarn  {clear};
	const QStringList binarn {assign, plusAssign, cmpEqual, index1};

	const QString undefined {"undefined"};
};

class TextConstant
{
	QString all;
	QString value;
public:

	TextConstant(const QString &textConstant);
	inline const QString& Value() const { return value; }

	static QString GetTextConstVal(const QString &txtConst);
	static bool IsItTextConstant(const QString &text, bool printLog);
};

class AllIndexes
{
public:
	std::vector<int> first;
	std::vector<int> secnd;
};

class Compiler
{
	static void RemoveJungAndAddSpaces(QString &command);
public:
	static QStringList TextToCommands(const QString &text);
	static QStringList CommandToWords(const QString &text);
	static QString GetFirstWord(const QString &text);

	static QStringList GetParamList(QString command);

	static QString GetOperandName(QString operand);

	static QString GetFirstSubObjectName(QString operand);

	static std::vector<int> DecodeStrNumbers(const QString &strNumbers, bool giveErrorIfEmpty);

	static QString GetIdexesText(const QString &operand);
	static std::vector<int> GetFirstIndexes(const QString &operand);
	static AllIndexes GetAllIndexes(QString operand);

	static QString GetInitialisationStr(const QString &command, bool printErrorIfNoInitialisation);

	static bool IsInteger(const QString &str);
	static bool IsFloating(const QString &str);
	static bool IsNumber(const QString &str);
};

#endif // COMPILER_H
