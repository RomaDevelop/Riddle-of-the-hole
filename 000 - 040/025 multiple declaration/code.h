#ifndef COMPILER_H
#define COMPILER_H

#include <QString>
#include <QStringList>
#include <QDomDocument>

#include "MyQShortings.h"

#include "logs.h"

namespace CodeMarkers
{
	const QString mock {"MOCK"};
	const QString needOptimisation {"NEED OPTIMISATION"};
};

namespace CommandsKeyWords
{
	const QString commandSplitter {";"};
	const QString idFilter {"ID:"};
	const QString dataFilter {"Data:"};
	const QString emulator {"Эмулятор"};
	const QString servis {"Сервис"};
	const QString dataOperand {"Data"};

	const QString obrabotchik {"Обработчик"};
	const QString transform {"Transform"};
};

typedef std::pair<QString, QString> QStringPair;
typedef std::vector<QStringPair> QStringPairVector;

class TextConstant
{
	QString value;
public:
	TextConstant(const QString &textConstant): value { GetTextConstVal(textConstant) } {}
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

class Code
{
	static void Normalize(QString &text);
public:
	static QStringList TextToCommands(const QString &text); // внутри вызывается Normalize
	static QStringList CommandToWords(const QString &command);
	static QString GetFirstWord(const QString &text);

	static std::vector<int> DecodeStrNumbers(const QString &strNumbers, bool printErrorIfEmpty);

	static QString GetIdexesText(const QString &operand);
	static std::vector<int> GetFirstIndexes(const QString &operand);
	static AllIndexes GetAllIndexes(QString operand);

	static QString GetInitialisationStr(const QString &command, bool printErrorIfNoInitialisation);

	static bool IsInteger(const QString &str);
	static bool IsFloating(const QString &str);
	static bool IsNumber(const QString &str);
};

struct DomAdd
{
	static std::vector<QDomElement> GetTopLevelElements(const QDomNode &node)
	{
		std::vector<QDomElement> elemtnts;
		QDomNode childNode = node.firstChild();
		while(!childNode.isNull())
		{
			QDomElement e = childNode.toElement(); // try to convert the node to an element.
			if(!e.isNull())
			{
				elemtnts.push_back(e);
			}
			childNode = childNode.nextSibling();
		}
		return elemtnts;
	}

	static QStringPairVector GetAttributes(const QDomElement &element)
	{
		QStringPairVector attrsVector;
		auto attrs = element.attributes();
		int size = attrs.size();
		for(int i=0; i<size; i++)
		{
			auto attr = attrs.item(i).toAttr();
			attrsVector.push_back({attr.name(),attr.value()});
		}
		return attrsVector;
	}
};

#endif // COMPILER_H
