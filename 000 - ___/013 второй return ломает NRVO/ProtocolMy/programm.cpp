#include "programm.h"


Programm::Programm(IExcelWorkbook *wb)
{
	if(wb->Sheet(1)->Cell(1,1) == "Версия файла" && wb->Sheet(1)->Cell(2,1) == "001") version = "001";

	if(version == "")
	{
		Error("Неверно определена версия");
		return;
	}

	if(version == "001")
	{
		int shCount = wb->SheetCount();
		std::vector<QStringList> tmp;
		for(int i=1; i<=shCount; i++) // перебираем все листы
		{
			if(wb->Sheet(i)->Name().left(2) != "//" && wb->Sheet(1)->Name()[0] != '#')  // кроме закомментированных
			{
				wb->Sheet(i)->AllRows(tmp);
				int size = tmp.size();
				for(int i=0; i<size; i++)
				{
					if(tmp[i][0] == "" || tmp[i][0] == "Тип объявления" || tmp[i][0].left(2) == "//"  || tmp[i][0][0] == '#')
						continue;  // если первая ячейка пуста или это Тип объявления или закомментирована - пропускаем

					if(tmp[i][0] == GeneralDefines::version) { }
					else if(tmp[i][0] == GeneralDefines::emulator
							|| tmp[i][0] == GeneralDefines::servis) generalStrs.push_back(tmp[i]);
					else if(tmp[i][0] == GeneralDefines::window) windowsStrs.push_back(tmp[i]);
					else if(tmp[i][0] == GeneralDefines::synchron) { }

					else if(tmp[i][0] == ConstantsDefines::constant) contantsStrs.push_back(tmp[i]);
					else if(tmp[i][0] == ParamsDefines::param) paramsStrs.push_back(tmp[i]);
					else if(tmp[i][0] == WidgetsDefines::widget) widgetsStrs.push_back(tmp[i]);
					else Error("Programm::Programm unknown str" + tmp[i].join(' '));
				}
			}
		}

		WorkGeneralStrs();
	}
	else Error("Версия не поддерживается");
}

void Programm::WorkGeneralStrs()
{
	int startX = 30;
	int startY = 30;
	for(auto &genStr:generalStrs)
	{
		if(genStr[0] == GeneralDefines::emulator || genStr[0] == GeneralDefines::servis)
		{
			QString tipstr = genStr[0];
			QString name = genStr[1];
			QString chan = genStr[2];
			QString startSettings = genStr[3];

			int tip = Parametr::emulator;
			QString namePrefix {"Emulator"};
			if(tipstr == GeneralDefines::servis)
			{
				namePrefix = "Servis";
				tip = Parametr::servis;
			}

			std::vector<QStringList> widgetsStrsOfProtocol;
			for(uint i=0; i<windowsStrs.size(); i++)
			{
				QString windowType = windowsStrs[i][2];
				QString windowConnect = windowsStrs[i][3];
				if(name == windowConnect)
				{
					for(uint j=0; j<widgetsStrs.size(); j++)
						if(widgetsStrs[j][3] == windowType) widgetsStrsOfProtocol.push_back(widgetsStrs[j]);
				}
			}

			Constants *consts = new Constants(contantsStrs,version,genStr[3]);
			Object *obj = new Object(paramsStrs, widgetsStrsOfProtocol, version, genStr[1], tip, consts, genStr[2].toInt());
			constants.push_back(consts);
			objects.push_back(obj);
			obj->InitFilters();
			obj->CreateWidgets();
			obj->PlaceWidgets(startX, startY, genStr[1]);
			startX+=450;
			Log(namePrefix+" "+obj->name+" created");
		}
		else Error("Programm::Programm unknown genStr" + genStr.join(' '));
	}

	// установка связи
	for(uint i=0; i<objects.size(); i++)
	{
		for(uint j=0; j<objects.size(); j++)
		{
			if(i != j && objects[i]->channel == objects[j]->channel)
			{
				objects[i]->server.push_back(objects[j]);
			}
		}
	}
}

Programm::~Programm()
{
	for(auto c:constants) delete c;
	for(auto p:objects) delete p;
}
