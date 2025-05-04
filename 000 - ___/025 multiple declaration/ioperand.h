#ifndef IOPERAND_H
#define IOPERAND_H

#include <memory>

#include "code.h"

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
	virtual bool IsEqual(const IOperand *operand2) const = 0;
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
	static QString classParam() { return "Param"; }
	static QString classParamPtr() { return "ParamPtr"; }
	static QString classVectorParams() { return "VectorParams"; }
	static QString classFrameWorker() { return "FrameWorker"; }
	static QString classFrameWorkerPtr() { return "FrameWorkerPtr"; }
	static QString classDataOperand() { return "DataOperand"; }

	static QStringList allIOClassNames()
	{
		return {	classValue(), classSubValue(), classWidjet(),
					classParam(), classParamPtr(), classVectorParams(),
					classFrameWorker(), classFrameWorkerPtr(),
					classDataOperand() };
	}
};

namespace Operators
{
	const QString point {"."};
	const QString index1 {"["};
	const QString index2 {"]"};
	const QString roundBracer1 {"("};
	const QString roundBracer2 {")"};

	const QString cmpEqual {"=="};

	const QString assign {"="};
	const QString plusAssign {"+="};

	const QString comma {","};

	const QStringList all {point, index1, index2, roundBracer1, roundBracer2, cmpEqual, assign, plusAssign, comma};
	const QStringList unarn  {};
	const QStringList binarn {point, index1, index2, cmpEqual, assign, plusAssign};

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
	static QString TypeToStr(int type)
	{
		if(type == undefinedType) return "undefined";
		if(type == unarn) return "unarn";
		if(type == binarn) return "binarn";
		if(type == special) return "special";
		if(type == canHaveNestedExpr) return "canHaveNestedExpr";
		Logs::ErrorSt("Operator::TypeToStr unrealesed type " + QSn(type));
		return "error";
	}

	Operator(QString oper_): oper{oper_}
	{
		if(oper == Operators::point) { priority = 1; type = binarn; }
		else if(oper == Operators::index1) { priority = 1; type = canHaveNestedExpr; }
		else if(oper == Operators::index2) { priority = 1; type = canHaveNestedExpr; }
		else if(oper == Operators::roundBracer1) { priority = 1; type = canHaveNestedExpr; }
		else if(oper == Operators::roundBracer2) { priority = 1; type = canHaveNestedExpr; }

		else if(oper == Operators::cmpEqual) { priority = 8; type = binarn; }

		else if(oper == Operators::assign) { priority = 12; type = binarn; }
		else if(oper == Operators::plusAssign) { priority = 12; type = binarn; }

		else if(oper == Operators::comma) { priority = 14; type = special; }
		else Logs::ErrorSt("Can't define operator [" + oper + "] priority");
	}

	virtual QString GetClassName() const override { return Operator::classOperator(); }
	virtual QString ToStrForLog() const override { return "["+oper+"]"; };

	static QString classOperator() { return "Operator"; }

	static void CorrectVectorDefinition(QStringList &words)
	{
		if(words.size() >=3 && words[1] == Operators::index1 && words[2] == Operators::index2)
		{
			words[0] += words[1]+words[2];
			words.removeAt(2);
			words.removeAt(1);
		}
	}
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
	const Function FindFWInThisCategory		("FindFWInThisCategory",		1,	2);
	const Function sendFrame				("SendFrame",					1,	1);
	const Function print					("Print",						2,	10);
	const Function if_						("if",							1,	1);

	const std::vector<Function> all {FindParamsInThisCategory, FindFWInThisCategory, sendFrame, print, if_};

	struct Check
	{
		static bool IsFunction(const QString &word)
		{
			return std::any_of(all.begin(),all.end(),[word](const Function &f){ return f.name == word;});
		}
		static void ParamsCount(const QString &word, int &min, int &max)
		{
			for(auto &f:all)
				if(f.name == word)
				{
					min = f.paramsCountMin;
					max = f.paramsCountMax;
					return;
				}
			Logs::ErrorSt("Functions::Check::ParamsCount undefined params count of function " + word);
			min = -1;
			max = -1;
		}
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

	bool CheckNullptr()
	{
		for(auto word:words) if(!word.ptr) return false;
		return true;
	}

	bool operator == (const Expression &exprToCmp)
	{
		if(words.size() != exprToCmp.words.size()) return false;
		else
		{
			for(uint i=0; i<words.size(); i++)
				if(words[i].ptr != exprToCmp.words[i].ptr) return false;
		}
		return true;
	}

	QString CommandFromStrWords()
	{
		QString ret;
		for(uint i=0; i<words.size(); i++)
			ret += words[i].str + " ";
		return ret;
	}

	QString ToStrForLog() const
	{
		QString ret;
		for(uint i=0; i<words.size(); i++)
		{
			ret += "\n\t\tptr[";
			if(words[i].ptr)
			{
				QString clName = words[i].ptr->GetClassName();
				ret += words[i].ptr->GetClassName() + "]" + words[i].ptr->ToStrForLog();
			}
			else ret += "nullptr]";

			ret += "str[" + words[i].str + "] ";
		}
		return ret;
	}

	QString ToStrForLog2(const QString &command) const
	{
		return "\n\tcommand ["+command+"]\n\texpression: "+ToStrForLog();
	}

	Expression GetNestedWords(int indexOpener)
	{
		Expression ret;
		if(indexOpener < (int)words.size())
		{
			if(words[indexOpener].ptr->GetClassName() == Operator::classOperator())
			{
				auto opener = static_cast<Operator*>(words[indexOpener].ptr);
				QString openerStr, closerStr;
				if(opener->oper == Operators::index1)
				{
					openerStr = Operators::index1;
					closerStr = Operators::index2;
				}
				else if(opener->oper == Operators::roundBracer1)
				{
					openerStr = Operators::roundBracer1;
					closerStr = Operators::roundBracer2;
				}
				else Logs::ErrorSt("GetNestedExpression: words[indexOpener] is not opener, it is " + opener->oper);

				if(openerStr.size() && closerStr.size())
				{
					int countNestedOpeners = 0;
					for(uint i=indexOpener+1; i<words.size(); i++)
					{
						if(words[i].ptr->GetClassName() == Operator::classOperator() && static_cast<Operator*>(words[i].ptr)->oper == openerStr)
							countNestedOpeners++;

						if(words[i].ptr->GetClassName() == Operator::classOperator() && static_cast<Operator*>(words[i].ptr)->oper == closerStr)
						{
							if(countNestedOpeners) countNestedOpeners--;
							else break;
						}

						ret.AddWord(words[i].ptr,words[i].str);
					}
				}
			}
			else Logs::ErrorSt("GetNestedExpression: words[indexOpener] is not Operator, it is " + words[indexOpener].ptr->GetClassName());
		}
		else Logs::ErrorSt("GetNestedExpression: indexOpener >= words.size() (" + QSn(indexOpener) + " >= " + QSn(words.size()) + ")");

		return ret;
	}
	static std::vector<Expression> SplitWordsAsFunctionParams(Expression expression)
	{
		std::vector<Expression> ret(1);
		for(uint i=0; i<expression.Size(); i++)
		{
			if(expression.words[i].ptr->GetClassName() == Operator::classOperator())
			{
				auto oper = static_cast<Operator*>(expression.words[i].ptr);
				if(oper->oper == Operators::comma)
				{
					if(ret.back().words.empty()) Logs::ErrorSt("SplitExpressionAsFuckionParams: empty param in words");
					ret.push_back(Expression());
					continue;
				}
			}

			ret.back().AddWord(expression.words[i].ptr, expression.words[i].str);
		}
		return ret;
	}
};

#endif // IOPERAND_H
