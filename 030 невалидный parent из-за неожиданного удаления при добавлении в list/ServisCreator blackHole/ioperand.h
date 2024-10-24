#ifndef IOPERAND_H
#define IOPERAND_H

#include <memory>

#include "code.h"
#include "allConstants.h"

class HaveClassName
{
public:
	virtual ~HaveClassName() = default;

	virtual QString GetClassName() const = 0;
	virtual QString ToStrForLog() const = 0;
};

class IOperand: public LogedClass, public HaveClassName
{
public:
	virtual ~IOperand() = default;

	// virtual pure group begin
	virtual QString GetClassName() const override = 0;
	virtual QString ToStrForLog() const override = 0;

	virtual void Assign(const IOperand *operand2) = 0;
	virtual void PlusAssign(const IOperand *operand2) = 0;
	virtual std::shared_ptr<IOperand> Minus(const IOperand *operand2) = 0;

	virtual bool CmpOperation(const IOperand *operand2, QString operationCode) const = 0;

	virtual std::shared_ptr<IOperand> Index(int index) = 0;
	virtual IOperand* PointAccess(QString nameToAccess) = 0;

	virtual QString IOGetName() const = 0;
	virtual QString IOGetValue(QString outputType) const = 0;
	virtual QString IOGetType() const = 0;
	virtual QStringPair IOGetValueAndType() const = 0;
	// virtual pure group end

	// class names group
	static QString classValue() { return "Value"; }
	static QString classSubValue() { return "SubValue"; }
	static QString classWidjet() { return "Widget"; }
	static QString classParam() { return ObjectClass_ns::Param; }
	static QString classParamPtr() { return "ParamPtr"; }
	static QString classVectorParams() { return "VectorParams"; }
	static QString classWorker() { return ProtocolClass_ns::Worker; }
	static QString classWorkerPtr() { return "WorkerPtr"; }
	static QString classDataOperand() { return "DataOperand"; }

	static QStringList allIOClassNames()
	{
		return {	classValue(), classSubValue(), classWidjet(),
					classParam(), classParamPtr(), classVectorParams(),
					classWorker(), classWorkerPtr(),
					classDataOperand() };
	}
};

namespace Operators
{
	const QString plus {"+"};
	const QString minus {"-"};
	const QString point {"."};
	const QString index1 {"["};
	const QString index2 {"]"};
	const QString roundBracer1 {"("};
	const QString roundBracer2 {")"};

	const QString cmpEqual {"=="};
	const QString cmpNotEqual {"!="};

	const QString assign {"="};
	const QString plusAssign {"+="};

	const QString comma {","};

	const QStringList all {plus, minus, point, index1, index2, roundBracer1, roundBracer2, cmpEqual, cmpNotEqual, assign, plusAssign, comma};
	const QStringList unarn  {};
	const QStringList binarn {point, index1, index2, cmpEqual, cmpNotEqual, assign, plusAssign};
	const QStringList cmpOperators {cmpEqual, cmpNotEqual};

	const QString ifResultFalse {"ifResultFalse"};
	const QString ifResultTrue {"ifResultTrue"};
	const QString cmpResultFalse {"cmpResultFalse"};
	const QString cmpResultTrue {"cmpResultTrue"};

	const QString undefined {"undefined"};
};

class Operator: public HaveClassName
{
public:
	QString oper{Operators::undefined};
	int priority = udefinedPriority;
	int type = undefinedType;
	enum { udefinedPriority = -1, undefinedType, unarn, binarn, canHaveNestedExpr, special };
	static QString TypeToStr(int type);

	Operator(QString oper_);

	virtual QString GetClassName() const override { return Operator::classOperator(); }
	virtual QString ToStrForLog() const override { return "["+oper+"]"; };

	static QString classOperator() { return "Operator"; }

	static void CorrectVectorDefinition(QStringList &words);
};

class Function: public HaveClassName
{
public:
	QString name;
	int paramsCountMin;
	int paramsCountMax;

	Function(QString funcName): name{funcName} {}
	Function(QString funcName, int paramsCountMin, int paramsCountMax):
		name{funcName},
		paramsCountMin{paramsCountMin},
		paramsCountMax{paramsCountMax}
	{}

	static QString classFunction() { return "Function"; }
	virtual QString GetClassName() const override { return classFunction(); }
	virtual QString ToStrForLog() const override { return "["+name+"]"; };
};

namespace Functions
{
	const Function FindParamsInThisCategory	("FindParamsInThisCategory",	1,	2);
	const Function FindParams				("FindParams",					1,	1);
	const Function FindParamsInCategory		("FindParamsInCategory",		2,	3);
	const Function FindWorkerInThisCategory	("FindWorkerInThisCategory",	1,	2);
	const Function FindWorker				("FindWorker",					1,	1);
	const Function sendFrame				("SendFrame",					1,	1);
	const Function doCommands				("DoCommands",					1,	1);
	const Function print					("Print",						2,	10);
	const Function stop						("Stop",						0,	0);
	const Function if_						("if",							1,	1);

	const std::vector<Function> all {FindParamsInThisCategory, FindParams, FindParamsInCategory, FindWorkerInThisCategory, FindWorker, sendFrame, doCommands, print, stop, if_};

	struct Check
	{
		static bool IsFunction(const QString &word);
		static void ParamsCount(const QString &word, int &min, int &max);
	};

	const QString printKeyWordLog		{"Log"};
	const QString printKeyWordWarning	{"Warning"};
	const QString printKeyWordError		{"Error"};
	const QString keyWordIncludeSubCats	{"IncludeSubCats"};
}

struct ExpressionWord
{
	HaveClassName* ptr;
	QString str;
};

class Expression
{
	std::vector<ExpressionWord> words;
public:
	void AddWord(HaveClassName* word, QString strWord) { words.push_back({word,strWord}); }
	void SetWord(int index, HaveClassName* newWord, QString newStrWord) { words[index] = { newWord, newStrWord }; }
	void ChangeStrWord(int index, QString newStrWord) {  words[index].str = newStrWord; }
	void Erase(int index) { words.erase(words.begin()+index); }
	uint Size() { return words.size(); }
	ExpressionWord Word(int index) { return words[index]; }
	bool CheckNullptr();
	bool operator == (const Expression &exprToCmp);
	QString CommandFromStrWords();
	QString ToStrForLog() const;
	QString ToStrForLog2(const QString &command) const;
	Expression GetNestedWords(int indexOpener);
	static std::vector<Expression> SplitWordsAsFunctionParams(Expression expression);
};


















#endif // IOPERAND_H
