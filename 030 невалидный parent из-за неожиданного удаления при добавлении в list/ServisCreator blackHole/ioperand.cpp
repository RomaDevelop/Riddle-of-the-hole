#include "ioperand.h"

void Functions::Check::ParamsCount(const QString & word, int & min, int & max)
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

bool Functions::Check::IsFunction(const QString & word)
{
	return std::any_of(all.begin(),all.end(),[word](const Function &f){ return f.name == word;});
}

void Operator::CorrectVectorDefinition(QStringList & words)
{
	if(words.size() >=3 && words[1] == Operators::index1 && words[2] == Operators::index2)
	{
		words[0] += words[1]+words[2];
		words.removeAt(2);
		words.removeAt(1);
	}
}

Operator::Operator(QString oper_): oper{oper_}
{
	if(oper == Operators::point) { priority = 1; type = binarn; }
	else if(oper == Operators::index1) { priority = 1; type = canHaveNestedExpr; }
	else if(oper == Operators::index2) { priority = 1; type = canHaveNestedExpr; }
	else if(oper == Operators::roundBracer1) { priority = 1; type = canHaveNestedExpr; }
	else if(oper == Operators::roundBracer2) { priority = 1; type = canHaveNestedExpr; }

	else if(oper == Operators::minus) { priority = 6; type = binarn; }

	else if(oper == Operators::cmpEqual) { priority = 10; type = binarn; }
	else if(oper == Operators::cmpNotEqual) { priority = 10; type = binarn; }

	else if(oper == Operators::assign) { priority = 16; type = binarn; }
	else if(oper == Operators::plusAssign) { priority = 16; type = binarn; }



	else if(oper == Operators::comma) { priority = 14; type = special; }
	else Logs::ErrorSt("Can't define operator [" + oper + "] priority");
}

QString Operator::TypeToStr(int type)
{
	if(type == undefinedType) return "undefined";
	if(type == unarn) return "unarn";
	if(type == binarn) return "binarn";
	if(type == special) return "special";
	if(type == canHaveNestedExpr) return "canHaveNestedExpr";
	Logs::ErrorSt("Operator::TypeToStr unrealesed type " + QSn(type));
	return "error";
}

bool Expression::CheckNullptr()
{
	for(auto word:words) if(!word.ptr) return false;
	return true;
}

bool Expression::operator ==(const Expression & exprToCmp)
{
	if(words.size() != exprToCmp.words.size()) return false;
	else
	{
		for(uint i=0; i<words.size(); i++)
			if(words[i].ptr != exprToCmp.words[i].ptr) return false;
	}
	return true;
}

QString Expression::CommandFromStrWords()
{
	QString ret;
	for(uint i=0; i<words.size(); i++)
		ret += words[i].str + " ";
	return ret;
}

QString Expression::ToStrForLog() const
{
	QString ret;
	for(uint i=0; i<words.size(); i++)
	{
		ret += "\n\t\t";
		if(words[i].ptr)
		{
			ret += words[i].ptr->GetClassName() + " " + words[i].ptr->ToStrForLog();
		}
		else ret += "nullptr";
	}
	return ret;
}

QString Expression::ToStrForLog2(const QString & command) const
{
	return "\n\tcommand ["+command+"]\n\texpression: "+ToStrForLog();
}

Expression Expression::GetNestedWords(int indexOpener)
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

std::vector<Expression> Expression::SplitWordsAsFunctionParams(Expression expression)
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
