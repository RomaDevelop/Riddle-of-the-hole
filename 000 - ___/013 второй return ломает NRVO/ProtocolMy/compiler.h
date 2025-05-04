#ifndef COMPILER_H
#define COMPILER_H

#include <QString>
#include <QStringList>
#include <QDebug>

#include "MyQShortings.h"

namespace Operators
{
	const QString assign {"="};
	const QString plusAssign {"+="};
	const QString cmpEqual {"=="};
	const QString clear {"clear"};

	const QStringList all {assign, plusAssign, cmpEqual, clear};
	const QStringList unarn  {clear};
	const QStringList binarn {assign, plusAssign, cmpEqual};

	const QString undefined {"undefined"};
};

namespace OperandTypes
{
	const QString textConstant {"textConstant"};
	const QString unknown {"unknown"};
};

class TextConstant
{
	QString all;
	QString value;
public:

	TextConstant(const QString &textConstant);
	inline const QString& Value() const { return value; }

	static QString GetTextConstVal(const QString &txtConst);
};

class AllIndexes
{
public:
	std::vector<int> first;
	std::vector<int> secnd;
};

class Compiler
{
	static void RemoveJungAddSpaces(QString &command);
public:
	static QStringList TextToCommands(const QString &text);
	static QStringList CommandToWords(const QString &text);
	static QString GetFirstWord(const QString &text);

	static QStringList GetParamList(QString command);

	static QString GetOperandName(QString operand);

	static QString GetFirstSubObjectName(QString operand);

	static std::vector<int> GetFirstIndexes(QString str);
	static AllIndexes GetAllIndexes(QString operand);

	static QString GetInitialisationVal(QString command);

	static bool IsInteger(const QString &str);
	static bool IsFloating(const QString &str);
	static bool IsNumber(const QString &str);

	static void Log(const QString &logStr)	 { qdbg << "Log:   Compiler::" + logStr; }
	static void Error(const QString &errStr) { qdbg << "Error: Compiler::" + errStr; }
};

#endif // COMPILER_H
