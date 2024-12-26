#ifndef CODE_H
#define CODE_H

#include <QString>
#include <QStringList>
#include <QDebug>

#include "CodeMarkers.h"
#include "MyQShortings.h"

#include "logs.h"

namespace CodeKeyWords
{
	const QString commandSplitter {";"};
	const char wordsSplitter {' '};

	const char textConstantSplitter {'\''};
	const char wrongTextConstantSplitter {'\"'};
	const int textConstantSplitterLength = 1;

	const QString idFilter {"ID:"};
	const QString dataFilter {"Data:"};

	const QString emulatorStr {"Эмулятор"};
	const QString servisStr {"Сервис"};
	const int codeUndefined {0};
	const int emulatorInt {1};
	const int servisInt {2};
	QString TypeToStr(int type);

	const QString dataOperand {"Data"};
	const QString thisParam {"thisParam"};
	const QString thisFrameWorker {"thisFrameWorker"};

	const QString obrabotchik {"Обработчик"};
	const QString transform {"Transform"};
};

typedef std::pair<QString, QString> QStringPair;
typedef std::vector<QStringPair> QStringPairVector;

struct TextConstant
{
	static QString GetTextConstVal(const QString &txtConst);
	static bool IsItTextConstant(const QString &text, bool printLog);
	static QString AddQuates(const QString &text);
};

struct AllIndexes
{
	std::vector<std::vector<int>> indexes;
};
struct AllIndexesOld
{
	std::vector<int> first;
	std::vector<int> secnd;
};

class Code
{
	static void Normalize(QString &text);
public:
	static QStringList TextToCommands(const QString &text); // внутри вызывается Normalize
	static QStringList CommandToWords(const QString &command);
	static QString GetFirstWord(const QString &text);

	static std::vector<int> DecodeStrNumbers(const QString &strNumbers, bool printErrorIfEmpty);

	static QStringList GetTextsInSquareBrackets(const QString &text);
	static AllIndexes GetAllIndexes(QString operand);

	static void RemoveEmulatorServis(QStringList &commands, int codeCurrent);
	static QString GetInitialisationStr(const QString &command, bool printErrorIfNoInitialisation);

	static bool IsInteger(const QString &str);
	static bool IsFloating(const QString &str);
	static bool IsNumber(const QString &str);
};

#endif // CODE_H
