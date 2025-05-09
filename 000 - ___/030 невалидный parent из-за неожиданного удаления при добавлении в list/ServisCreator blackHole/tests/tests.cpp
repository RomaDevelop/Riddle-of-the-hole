#include "tests.h"

void Tests::TestDataOperandFromWords()
{
	Logs::ErrorSt("TestDataOperandFromWords need remake");
	QStringList test = {
		"Data",
		"Data[1]",
		"Data[][2-3]",
		"Data=34",
		"Data[2]=34",
		"Data[2][sub[]]=25",
		"Data[sub[1]][2][3]=3434" };

	Logs::LogSt("TestDataOperandFromWords");
	auto commands = Code::TextToCommands(test.join(";"));
	for(auto &command:commands)
	{
		auto words = Code::CommandToWords(command);
		int count = -1;
		QString dataOperand;
		//dataOperand = DataOperandFromWords(words,0,count,nullptr);
		Logs::LogSt(words.join(' '));
		Logs::LogSt(dataOperand);
		Logs::LogSt(QSn(count));
	}
}

void Tests::TestCategory()
{
	Category headCat("БКЗ 27","1",nullptr);
	Param test;

	std::vector<std::list<NameNumber>> categoriesPaths;
	//QString paths = MyQDialogs::InputText("Input paths (1 row 1 path)", 600);

	QString paths = (QString)"БКЗ 27:1::МУП БКЗ:1\n"
							 "БКЗ 27:1::МВДСы:2\n"
							 "БКЗ 27:1::МВДСы:2::МВДС 20 1:1\n"
							 "БКЗ 27:1::МВДСы:2::МВДС 20 2:2\n"
							 "БКЗ 27:1::МВДСы:2::МВДС 30 1:3\n"
							 "БКЗ 27:1::МВДСы:2::МВДС 30 2:4\n"
							 "БКЗ 27:1::МВДСы:2::МВДС 40 1:5::Канал:1::Подканал:300::Под-под:200::под-под-под:100\n";


	//paths += "БКЗ 115:2::МВДСы:2::МВДС 40 1:5\n"; // должна быть ошибка
	//paths += "БКЗ 115:2\n"; // должна быть ошибка
	//paths += "БКЗ 27:1::МУП БКЗ:1\n"; // должна быть ошибка
	//paths += "БКЗ 27:1::МВДСы:2::МВДС 40 1:5\n"; // должна быть ошибка

	auto pathsList = paths.split("\n",QString::SkipEmptyParts);
	for(auto &path:pathsList)
	{
		headCat.InsertCategory(CategoryPath::PathFromQString(path));
	}

	headCat.InsertMember(test,CategoryPath::PathFromQString("БКЗ 27:1::МВДСы:2::МВДС 20 1:1::Канал:1"),true); // норм
	headCat.InsertMember(test,CategoryPath::PathFromQString("БКЗ 27:1::МВДСы:2::МВДС 20 1:1::Канал:1"),true);
	headCat.InsertMember(test,CategoryPath::PathFromQString("БКЗ 27:1::МВДСы:2::МВДС 20 1:1::Канал:1"),true);

	headCat.InsertMember(test,CategoryPath::PathFromQString("БКЗ 27:1::МВДСы:2::МВДС 20 2:2::Канал:2"),true);
	headCat.InsertMember(test,CategoryPath::PathFromQString("БКЗ 27:1::МВДСы:2::МВДС 20 2:2::Канал:2"),true);

	headCat.InsertMember(test,CategoryPath::PathFromQString("БКЗ 27:1::МВДСы:2::МВДС 30 1:3::Канал:1"),true);
	headCat.InsertMember(test,CategoryPath::PathFromQString("БКЗ 27:1::МВДСы:2::МВДС 40 1:5::Канал:2::Подканал:300::Под-под:200::под-под-под:100"),true);
	//headCat.InsertMember(test,CategoryPath::PathFromQString("БКЗ 27:1::МВДСы:2::МВДС 40 1:5::Канал:1"),false); // ошибка

	auto resList = headCat.ToStrForLog(true,true).split("\n");
	for(auto &str:resList)
		Logs::LogSt(str);
}

void Tests::TestCommandToWords()
{
	qdbg << Code::CommandToWords("sda sdsdv ; asasc sdvsdv");
}

bool Tests::TestGetStringListFromValueVector()
{
	QStringList inputs;
	QStringList outputsMustBe;
	bool correct = true;

	inputs			+= "1,2,3,4";
	inputs			+= " 1 , 2 , 3 , 4 ";
	inputs			+= "1 ,2 ,3 ,4 " ;
	inputs			+= " 1,2 ,3 ,4 ";
	inputs			+= " 1 ,   2 2 , 3 ,    4    ";

	outputsMustBe	+= "1;2;3;4";
	outputsMustBe	+= "1;2;3;4";
	outputsMustBe	+= "1;2;3;4";
	outputsMustBe	+= "1;2;3;4";
	outputsMustBe	+= "1;2 2;3;4";

	for(int i=0; i<inputs.size(); i++)
	{
		auto res = Value::GetStringListFromValueVector(inputs[i]).join(";");
		if(res != outputsMustBe[i])
		{
			correct = false;
			Logs::ErrorSt("Тест Value::GetStringListFromValueVector(\""+inputs[i]+"\") выдал ошибку!"
						  +"\n\tрезультат " + res + ", а ожидается " + outputsMustBe[i]);
		}
	}

	return correct;
}
