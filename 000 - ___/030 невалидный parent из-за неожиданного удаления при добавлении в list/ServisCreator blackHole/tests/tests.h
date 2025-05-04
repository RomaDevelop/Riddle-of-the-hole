#ifndef TESTS_H
#define TESTS_H

#include "category.h"
#include "programm.h"

class Tests
{
public:
	static void DoTests()
	{
		bool correct = true;
		if(!TestGetStringListFromValueVector()) correct = false;

		if(correct) Logs::LogSt("Тесты в DoTests() завершены без ошибок");
		else Logs::ErrorSt("Тесты в DoTests() завершены с ошибками");
	}

	static void TestDataOperandFromWords();
	static void TestCategory();
	static void TestCommandToWords();
	static bool TestGetStringListFromValueVector();
};

#endif // TESTS_H
