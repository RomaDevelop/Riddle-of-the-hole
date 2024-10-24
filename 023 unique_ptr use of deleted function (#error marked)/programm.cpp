
#include <QApplication>

#include "MyQFileDir.h"
#include "MyQDifferend.h"

#include "settings.h"
#include "configconstants.h"
#include "programm.h"

using namespace std;

namespace TypesDefines
{
	const QString version {"Версия файла"};
	const QString progName {"Имя програмы"};
	const QString object {"Объект"};
	const QString window {"Окно"};
	const QString synchron {"Синхронизаиция"};
	const QString monitor {"Монитор"};

	const QString constant {"Константа"};
	const QString param{"Параметр"};
	const QString copyParams{"Копия параметров"};
	const QString widget{"Виджет"};
	const QString copyWidgets{"Копия виджетов"};
}

int defX = 30;
int defY = 30;

Programm::Programm(IExcelWorkbook *wb)
{
	monitor.Create();
	Logs::SetAllToTextBrowser(monitor.tBrowser, "");
	CreateControlPanel();

	if(wb->Sheet(1)->Cell(1,1) == TypesDefines::version && wb->Sheet(1)->Cell(2,1) == "001") version = "001";

	if(version == "001")
	{
		int shCount = wb->SheetCount();
		std::vector<QStringList> tmp;
		for(int i=1; i<=shCount; i++) // перебираем все листы
		{
			if(wb->Sheet(i)->Name().left(2) != "//" && wb->Sheet(i)->Name()[0] != '#')  // кроме закомментированных
			{
				wb->Sheet(i)->AllRows(tmp);
				int size = tmp.size();
				for(int i=0; i<size; i++)
				{
					if(tmp[i][0] == "" || tmp[i][0] == "Тип объявления" || tmp[i][0].left(2) == "//"  || tmp[i][0][0] == '#')
						continue;  // если первая ячейка пуста или это Тип объявления или закомментирована - пропускаем

					if(tmp[i][0] == TypesDefines::version) { }
					else if(tmp[i][0] == TypesDefines::progName) { programmName = tmp[i][1]; }
					else if(tmp[i][0] == TypesDefines::object) objectsRows.push_back(tmp[i]);
					else if(tmp[i][0] == TypesDefines::window) windowsRows.push_back(tmp[i]);
					else if(tmp[i][0] == TypesDefines::synchron) synchRows.push_back(tmp[i]);
					else if(tmp[i][0] == TypesDefines::monitor) monitorRows.push_back(tmp[i]);

					else if(tmp[i][0] == TypesDefines::constant) contantsRows.push_back(tmp[i]);
					else if(tmp[i][0] == TypesDefines::param) paramsRows.push_back(tmp[i]);
					else if(tmp[i][0] == TypesDefines::copyParams) copyParamsRows.push_back(tmp[i]);
					else if(tmp[i][0] == TypesDefines::widget) widgetsRows.push_back(tmp[i]);
					else if(tmp[i][0] == TypesDefines::copyWidgets) copyWidgetsRows.push_back(tmp[i]);
					else Error("Programm::Programm unknown type define " + tmp[i][0] + " in str " + tmp[i].join(' '));
				}
			}
		}

		CreateCopyParamRows();

		vector<int> frameWorkersIndexes;
		for(uint i=0; i<paramsRows.size(); i++)
		{
			if(FrameTypes::all.contains(paramsRows[i][ParamsColIndexes::type]))
			{
				frameWorkersRows.push_back(paramsRows[i]);
				frameWorkersIndexes.push_back(i);
			}
		}
		for(int i=frameWorkersIndexes.size()-1; i>=0; i--)
		{
			paramsRows.erase(paramsRows.begin()+frameWorkersIndexes[i]);
		}

		CreateCopyWidgetRows();

		CreateConstsObjectsProtocols();

		CreateSynchConnections();

		SetLogsMsgWorkers();

		CreateWindows();
		CreateWidgets();
		PlaceWidgets();
		PlaceWindows(defX,defY);

		GiveWindowsPtrs();
		GiveWidgetsPtrs();

		ConnectMake();

		// установка связи для пересылки сообщений
		for(uint i=0; i<protocols.size(); i++)
		{
			for(uint j=0; j<objects.size(); j++)
			{
				if(i != j && protocols[i]->channel == protocols[j]->channel)
				{
					protocols[i]->network.push_back(protocols[j]);
				}
			}
		}

		LoadSettings();
	}
	else
	{
		Error("Версия [" + version + "] не поддерживается");
	}
}

Programm::~Programm()
{
	SaveSettings();

	Log("~Programm()");
	for(auto c:constants) delete c;
	for(auto o:objects) delete o;
}

bool Programm::IsProgrammShowsNothing()
{
	for(auto &window:windows)
		if(window.windowPtQWidget && window.windowPtQWidget->isVisible())
			return false;
	if(monitor.windowPtQWidget && monitor.windowPtQWidget->isVisible())
		return false;

	return true;
}

void Programm::SetLogsMsgWorkers()
{
	monitor.SetProgrammName(programmName);
	this->SetAllLogsToTextBrowser(monitor.tBrowser, programmName+": ");

	for(auto object:objects)
	{
		object->SetAllLogsToTextBrowser(monitor.tBrowser, object->name+": ");
		for(auto &param:object->params)
		{
			param->SetAllLogsToTextBrowser(monitor.tBrowser, object->name+": ");
		}
	}

	for(auto prot:protocols)
	{
		prot->SetAllLogsToTextBrowser(monitor.tBrowser, prot->name+": ");
		for(auto &fw:prot->frameWorkers)
		{
			fw->SetAllLogsToTextBrowser(monitor.tBrowser, prot->name+": ");
		}
	}
}

void Programm::CreateSynchConnections()
{
	for(auto &synchRow:synchRows)
	{
		QString objectsToSynchCell = synchRow[SynchCols::objectsToSynch];
		auto objectsToSynchList = Code::TextToCommands(objectsToSynchCell);
		std::vector<Protocol*> protsToSynch;
		for(auto &protName:objectsToSynchList)
		{
			bool nameFinded = false;
			for(auto prot:protocols)
				if(protName == prot->name)
				{
					if(nameFinded) Error("Synchron set wrong behavior. Name " + protName + " finded twice.");
					protsToSynch.push_back(prot);
					nameFinded = true;
				}
			if(!nameFinded) Error("Synchron set wrong behavior. Name " + protName + " not found.");
		}

		int objectsToSynchSize = protsToSynch.size();
		for(int i=0; i<objectsToSynchSize; i++)
		{
			for(int j=0; j<objectsToSynchSize; j++)
			{
				if(i!=j)
				{
					Protocol *obj1 = protsToSynch[i];
					Protocol *obj2 = protsToSynch[j];
					if(obj1->frameWorkers.size() != obj2->frameWorkers.size())
						Error("Synchron set wrong behavior. Different param size ("+QSn(obj1->frameWorkers.size())
							  +" != " + QSn(obj2->frameWorkers.size())+") in objects" + obj1->name + " " + obj2->name);
					else
					{
						int fwSize = obj1->frameWorkers.size();
						for(int fw_i=0; fw_i<fwSize; fw_i++)
						{
							if(obj1->frameWorkers[fw_i]->CellValuesToStrForLog() != obj2->frameWorkers[fw_i]->CellValuesToStrForLog())
								Error("Synchron set wrong behavior. Different cell values ("+obj1->frameWorkers[fw_i]->CellValuesToStrForLog()
									  +" != " + obj2->frameWorkers[fw_i]->CellValuesToStrForLog()+")");
							else
							{
								obj1->frameWorkers[fw_i]->synchronFWs.push_back(obj2->frameWorkers[fw_i].get());
							}
						}
					}
				}
			}
		}
	}
}

void Programm::CreateCopyParamRows()
{
	for(auto &copyParamStr:copyParamsRows)
	{
		namespace pci = ParamsColIndexes;
		namespace cpci = CopyParamsColIndexes;

		auto idPSrc = Code::DecodeStrNumbers(copyParamStr[cpci::idPSrc],true);
		auto idPDst = Code::DecodeStrNumbers(copyParamStr[cpci::idPDst],true);

		uint sizePSrc = idPSrc.size();
		if(sizePSrc == 0 || sizePSrc != idPDst.size())
		{
			if(idPSrc.size() == 0)
				Error("Copying params: idPSrc size=0\n\tcopyParamStr: " + CopyParamStrToStrForLog(copyParamStr));
			if(idPSrc.size() != idPDst.size())
				Error("Copying params: different size idPSrc and idPDst. idPSrc size=" + QSn(idPSrc.size()) + "; idPDst size=" + QSn(idPDst.size())
				  + ";\n\tcopyParamStr: " +  CopyParamStrToStrForLog(copyParamStr));
		}
		else
		{

			std::vector<QStringList> newParamsStrs;

			int prevIndex = 0;
			for(uint i=0; i<sizePSrc; i++)
			{
				if(idPSrc[i] == idPDst[i])
				{
					Error("Copying params: idPSrc ("+QSn(idPSrc[i])+") is same idPDst ("+QSn(idPDst[i])
						  +");\n\tcopyParamStr: " +  CopyParamStrToStrForLog(copyParamStr));
				}
				else
				{
					int index = FindIndexParamRow(QSn(idPSrc[i]),prevIndex);
					if(index == notFound) Error("Copying widgets: id " + QSn(idPSrc[i]) + " notFind copyStr: " +  CopyParamStrToStrForLog(copyParamStr));
					else
					{
						prevIndex = index;

						QStringList newParamStr = paramsRows[index];
						if(copyParamStr[cpci::category] != "") newParamStr[pci::category] = copyParamStr[cpci::category];
						if(copyParamStr[cpci::name] != "") newParamStr[pci::name] = copyParamStr[cpci::name];
						newParamStr[pci::id_] = QSn(idPDst[i]);
						if(copyParamStr[cpci::type] != "") newParamStr[pci::type] = copyParamStr[cpci::type];

						if(copyParamStr[cpci::modification] != "")
						{
							ModificateCopyRow(newParamStr,copyParamStr[cpci::modification]);
						}

						newParamsStrs.push_back(std::move(newParamStr));
					}
				}
			}

			for(auto &np:newParamsStrs)
				paramsRows.push_back(std::move(np));
		}
	}
}

QString Programm::CopyParamStrToStrForLog(QStringList strList)
{
	QString ret;
	ret += strList[CopyParamsColIndexes::defenitionType] + " ";
	ret += strList[CopyParamsColIndexes::category] + " ";
	ret += strList[CopyParamsColIndexes::name] + " ";
	ret += strList[CopyParamsColIndexes::id_] + " ";
	ret += strList[CopyParamsColIndexes::idPSrc] + " ";
	ret += strList[CopyParamsColIndexes::idPDst] + " ";
	return ret;
}

void Programm::CreateCopyWidgetRows()
{
	for(auto &copyWidgetStr:copyWidgetsRows)
	{
		namespace wci = WidgetColIndexes;
		namespace cwci = CopyWidgetsColIndexes;

		auto idWSrc = Code::DecodeStrNumbers(copyWidgetStr[cwci::idWSrc],true);
		auto idWDst = Code::DecodeStrNumbers(copyWidgetStr[cwci::idWDst],true);
		auto idPCon = Code::DecodeStrNumbers(copyWidgetStr[cwci::idPConnect],true);

		if(idWSrc.size() == 0 || idWSrc.size() != idWDst.size() || idWSrc.size() != idPCon.size())
			Error("Copying widgets: wrong size idWSrc or idWDst or idPCon:" + QSn(idWSrc.size()) + " " + QSn(idWDst.size())
				  + " " + idPCon .size() + " copyWidgetStr: " + copyWidgetStr.join(' '));
		else
		{
			uint sizeId = idWSrc.size();
			std::vector<QStringList> newWidgetsStrs;

			int prevIndex = 0;
			for(uint i=0; i<sizeId; i++)
			{
				int index = FindIndexWidgetRow(copyWidgetStr[cwci::classWindow],QSn(idWSrc[i]),prevIndex);
				if(index == notFound) Error("Copying widgets: id " + QSn(idWSrc[i]) + " notFind copyStr: " + copyWidgetStr.join(' '));
				else
				{
					prevIndex = index;

					QStringList newWidgetStr = widgetsRows[index];
					if(copyWidgetStr[cwci::tabName] != "") newWidgetStr[wci::tabName] = copyWidgetStr[cwci::tabName];
					if(copyWidgetStr[cwci::tableName] != "") newWidgetStr[wci::tableName] = copyWidgetStr[cwci::tableName];
					newWidgetStr[wci::idWidget] = QSn(idWDst[i]);
					newWidgetStr[wci::idParam] = QSn(idPCon[i]);
					newWidgetsStrs.push_back(std::move(newWidgetStr));
				}
			}

			for(auto &nw:newWidgetsStrs)
				widgetsRows.push_back(std::move(nw));
		}
	}
}

void Programm::ModificateCopyRow(QStringList &copyRow, QString modificationCellValue)
{
	auto commands = Code::TextToCommands(modificationCellValue);
	for(auto &command:commands)
	{
		auto words = Code::CommandToWords(command);
		const QString modifReplace = "replace";
		if(words.size() && words[0] == modifReplace && words.size() == 3)
		{
			QString find = TextConstant::GetTextConstVal(words[1]);
			QString replace = TextConstant::GetTextConstVal(words[2]);
			for(auto &cell:copyRow)
			{
				cell.replace(find,replace);
			}
		}
		else Error("Copy param modification wrong command " + command);
	}
}

void CorrectPos(QWidget *widget)
{
	//int xInc = 8, yInc = 30; // windows 7
	int xInc = 1, yInc = 31;  // windows 10
	if(widget->isVisible())
		widget->move(widget->x()+xInc,widget->y()+yInc);
}

void Programm::SaveSettings()
{
	if(Settings::notSaveSettings) return;

	QString endParam = "[progP;] ";

	QStringList settingsWindows;
	QStringList settingsTables;
	QStringList settingsMonitors;
	QStringList settingsOther;

	for(auto &w:windows)
	{
		CorrectPos(w.windowPtQWidget);

		QString settingWindow = "programmStng001" + endParam + programmName + endParam + "WindowsGeo" + endParam
				+ w.parentObject->name + endParam + w.name + endParam + QSn(w.windowPtQWidget->x()) + endParam + QSn(w.windowPtQWidget->y()) + endParam
				+ QSn(w.windowPtQWidget->width()) + endParam + QSn(w.windowPtQWidget->height()) + endParam ;
		settingsWindows += settingWindow;
		for(auto t:w.tables)
		{
			QString settingTable = "programmStng001" + endParam + programmName + endParam + "TablesColsWidths" + endParam
					+ w.parentObject->name + endParam + w.name + endParam + t->objectName() + endParam;
			for(int i=0; i<t->columnCount(); i++)
				settingTable += QSn(t->columnWidth(i)) + endParam;
			settingsTables += settingTable;
		}
	}

	if(monitor.windowPtQWidget)
	{
		CorrectPos(monitor.windowPtQWidget);

		QString settingWindow = "programmStng001" + endParam + programmName + endParam + "MonitorsGeo" + endParam
				+ monitor.name + endParam + QSn(monitor.windowPtQWidget->x()) + endParam + QSn(monitor.windowPtQWidget->y()) + endParam
				+ QSn(monitor.windowPtQWidget->width()) + endParam + QSn(monitor.windowPtQWidget->height()) + endParam ;
		settingsMonitors += settingWindow;
	}

	if(controlPanel)
	{
		CorrectPos(controlPanel);

		QString settingControlPanel = "programmStng001" + endParam + programmName + endParam + "ControlPanelGeo" + endParam
				+ QSn(controlPanel->x()) + endParam + QSn(controlPanel->y()) + endParam
				+ QSn(controlPanel->width()) + endParam + QSn(controlPanel->height()) + endParam;
		settingsOther += settingControlPanel;
	}

	QString path = mqd::GetPathToExe() + "/files/programms_settings";
	if(!MQFD::CreatePath(path))
		Error("~Programm can't create path " + path);

	if(!mqd::SaveSettings(path+"/"+programmName+".stgs",{},settingsWindows+settingsTables+settingsMonitors+settingsOther))
		Error("~Programm can't save settings");
}

void Programm::LoadSettings()
{
	QStringList settings;
	std::vector<QWidget*> pustishka;
	QString path = mqd::GetPathToExe() + "/files/programms_settings";
	if(!mqd::LoadSettings(path+"/"+programmName+".stgs",pustishka,{settings}))
		Error("~Programm can't load settings. Default set.");

	QString endParam = "[progP;] ";
	for(auto &stng:settings)
	{
		auto params = stng.split(endParam,QString::SkipEmptyParts);
		if(params.size() && params[0] == "programmStng001")
		{
			if(params.size() >=2 && params[1] == programmName)
			{
				if(params.size() >=3 && params[2] == "TablesColsWidths")
				{
					int countNotWidth = 6;
					if(params.size() > countNotWidth)
					{
						QString objName = params[3];
						QString windName = params[4];
						QString tableName = params[5];
						for(int i=0; i<countNotWidth; i++)
							params.removeFirst();

						Window *w = FindWindow(objName,windName);
						QTableWidget *t = nullptr;
						if(w) t = w->FindTable(tableName, false);

						if(t)
						{
							for(int c=0; c<params.size(); c++)
							{
								if(c < t->columnCount())
									t->setColumnWidth(c,params[c].toUInt());
							}
						}
						else Error("Read settings: can't find Object or Window or QTableWidget in setting "+stng);
					}
					else Error("Read settings: wrong params TablesColsWidths count "+QSn(params.size())+" in setting "+stng);
				}
				else if(params.size() == 9 && params[2] == "WindowsGeo")
				{
					QString objName = params[3];
					QString windName = params[4];
					int x = params[5].toInt();
					int y = params[6].toInt();
					int w = params[7].toInt();
					int h = params[8].toInt();

					Window *window = FindWindow(objName,windName);
					if(window) window->windowPtQWidget->setGeometry(x,y,w,h);
					else Error("Read settings: can't find Object or Window in setting "+stng);
				}
				else if(params.size() == 8 && params[2] == "MonitorsGeo")
				{
					QString monName = params[3];
					int x = params[4].toInt();
					int y = params[5].toInt();
					int w = params[6].toInt();
					int h = params[7].toInt();
					//qdbg << x << y;
					if(monitor.windowPtQWidget) monitor.windowPtQWidget->setGeometry(x,y,w,h);
					else Error("Read settings: can't find Monitor in setting "+stng);
				}
				else if(params.size() >= 3 && params[2] == "ControlPanelGeo" && params.size() == 7)
				{
					int x = params[3].toInt();
					int y = params[4].toInt();
					int w = params[5].toInt();
					int h = params[6].toInt();
					if(controlPanel) controlPanel->setGeometry(x,y,w,h);
				}
				else Error("Read settings: unknown command "+params[2]+" in setting "+stng);
			}
			else Error("Read settings: wrong file name "+params[0]+" in setting "+stng);
		}
		else Error("Wrong version in setting "+stng);
	}// cycle, no else
}

void Programm::CreateControlPanel()
{
	controlPanel = new QWidget;
	controlPanel->setWindowTitle("Control panel " + programmName);
	controlPanel->resize(150,150);

	QGridLayout *glo = new QGridLayout(controlPanel);

	QPushButton *btn = new QPushButton ("Default window geo");
	glo->addWidget(btn,glo->rowCount(),1,1,-1);
	QObject::connect(btn,&QPushButton::clicked,[this](){ PlaceWindows(defX,defY); });

	QCheckBox *chLogDoEvrythingCommands = new QCheckBox("LogDoEvrythingCommands");
	glo->addWidget(chLogDoEvrythingCommands,glo->rowCount(),1,1,-1);
	QObject::connect(chLogDoEvrythingCommands,&QCheckBox::clicked,[chLogDoEvrythingCommands](){
		if(chLogDoEvrythingCommands->isChecked()) Settings::logDoEvrythingCommands = true;
		else Settings::logDoEvrythingCommands = false;
	});

	btn = new QPushButton ("Close");
	glo->addWidget(btn,glo->rowCount(),1,1,-1);
	QObject::connect(btn,&QPushButton::clicked,[](){ QApplication::exit(); });

	btn = new QPushButton ("Close (not save settings)");
	glo->addWidget(btn,glo->rowCount(),1,1,-1);
	QObject::connect(btn,&QPushButton::clicked,[](){ Settings::notSaveSettings = true; QApplication::exit(); });

	controlPanel->show();
}

void Programm::GiveWidgetsPtrs()
{
	for(auto &window:windows)
	{
		if(window.parentObject)
		{
			for(auto &widjetPair:window.widgets)
			{
				Param *paramPt = window.parentObject->FindParamById(widjetPair.strIdToConnect);
				FrameWorker *fwPt = window.parentProtocol->FindFrameWorkerById(widjetPair.strIdToConnect);
				if(paramPt)
				{
					if(fwPt) Warning("GiveWidgetsPtrOfParams: id [" + widjetPair.strIdToConnect + "] finded both in params and frames");

					widjetPair.parentParam = paramPt;
					CBParamChanged cbForParam;
					cbForParam.cbHandler = &widjetPair;
					cbForParam.cbFunction = [](const Value &newValue, HaveClassName *handler)
					{
						WidgetPair* wpHandler = dynamic_cast<WidgetPair*>(handler);
						if(wpHandler)
						{
							wpHandler->view.SetWidgetValue(newValue);
						}
						else Logs::ErrorSt("GiveWidgetsPtrOfParams: cbForParam can't convert handler to WidgetPair*");
					};

					paramPt->AddCBParamChanged(cbForParam);
				}
				else if(fwPt)
				{
					widjetPair.parentFrameWorker = fwPt;
				}
				else Error("GiveWidgetsPtrOfParams:Can't find param or frameWorker for widget! Tryed to find by id (" + widjetPair.strIdToConnect + "). Object name " + window.parentObject->name + "; Window name "
						   + window.name + "; Widget str: " + widjetPair.ToStr());
			}
		}
		else Error("GiveWidgetsPtrOfParams: window.parentObject is nullptr!");
	}
}

void Programm::GiveWindowsPtrs()
{
	for(uint wi=0; wi<windows.size(); wi++)
	{
		bool findObjectForWindow = false;
		for(uint oi=0; oi<objects.size(); oi++)
			if(windows[wi].connectObjectName == objects[oi]->name)
			{
				if(!findObjectForWindow)
				{
					windows[wi].parentObject = objects[oi];
					findObjectForWindow = true;
				}
				else Error("GiveWindowsPtrOfObjects: window has not one object to connect! window name: "
						   + windows[wi].name + " object name to connect: " + objects[oi]->name);
			}
		if(!findObjectForWindow)
			Error("GiveWindowsPtrOfObjects: window has no object connect! window name: "
				  + windows[wi].name + " object name to connect: " + windows[wi].connectObjectName);

		bool findProtForWindow = false;
		for(uint pi=0; pi<protocols.size(); pi++)
			if(windows[wi].connectObjectName == protocols[pi]->name)
			{
				if(!findProtForWindow)
				{
					windows[wi].parentProtocol = protocols[pi];
					findProtForWindow = true;
				}
				else Error("GiveWindowsPtrOfObjects: window has not one protocol to connect! window name: "
						   + windows[wi].name + " protocol name to connect: " + protocols[pi]->name);
			}
		if(!findProtForWindow)
			Error("GiveWindowsPtrOfObjects: window has no protocol connect! window name: "
				  + windows[wi].name + " protocol name to connect: " + windows[wi].connectObjectName);
	}
}



Window *Programm::FindWindow(QString objectName, QString windowName)
{
	for(auto &window:windows)
	{
		if(window.parentObject)
		{
			if(window.name == windowName && window.parentObject->name == objectName)
				return &window;
		}
		else Error("FindWindow find nullptr parentObject in window " + window.name);
	}
	Error("FindWindow can't window " + windowName + " whith object " + objectName);
	return nullptr;
}

int Programm::FindIndexParamRow(QString idSrcParam, int startIndex)
{
	int paramsSize = paramsRows.size();
	if(startIndex >= paramsSize) Error("FindParamStr: startIndex >= paramsStrs.size() " + QSn(startIndex) + " " + QSn(paramsSize));
	for(int i=startIndex; i<paramsSize; i++)
	{
		if(paramsRows[i][ParamsColIndexes::id_] == idSrcParam)
			return i;
	}
	for(int i=0; i<startIndex && i<paramsSize; i++)
	{
		if(paramsRows[i][ParamsColIndexes::id_] == idSrcParam)
			return i;
	}
	return notFound;
}

int Programm::FindIndexWidgetRow(QString windowClass, QString idSrcWidget, int startIndex)
{
	int widgStrsSize = widgetsRows.size();
	if(startIndex >= widgStrsSize || startIndex < 0) Warning("FindIndexWidgetRow: wrong startIndex " + QSn(startIndex) + " for widgStrsSize " + QSn(widgStrsSize));
	for(int i=startIndex; i<widgStrsSize; i++)
	{
		if(widgetsRows[i][WidgetColIndexes::classWindow] == windowClass && widgetsRows[i][WidgetColIndexes::idWidget] == idSrcWidget)
			return i;
	}
	for(int i=0; i<startIndex && i<widgStrsSize; i++)
	{
		if(widgetsRows[i][WidgetColIndexes::classWindow] == windowClass && widgetsRows[i][WidgetColIndexes::idWidget] == idSrcWidget)
			return i;
	}
	return notFound;
}

void Programm::CreateConstsObjectsProtocols()
{
	for(auto &objRow:objectsRows)
	{
		int tip = Protocol::undefined;
		if(objRow[ObjectsCols::category] == ObjectsCols::emulator) tip = Protocol::emulator;
		else if(objRow[ObjectsCols::category] == ObjectsCols::servis) tip = Protocol::servis;
		else Error("Programm::Programm unknown genStr" + objRow.join(' '));

		QString name = objRow[ObjectsCols::name];
		int chan = objRow[ObjectsCols::canChannel].toInt();
		QString startSettings = objRow[ObjectsCols::startSettings];

		Constants *consts = new Constants(contantsRows,version,startSettings);
		Object *obj = new Object(paramsRows, version, name);
		Protocol *prot = new Protocol(frameWorkersRows,obj,version,name,tip,consts,chan);
		constants.push_back(consts);
		objects.push_back(obj);
		protocols.push_back(prot);
		prot->InitFilters();
	}
}

void Programm::CreateWindows()
{
	windows.reserve(windowsRows.size());

	for(uint i=0; i<windowsRows.size(); i++)
	{
		Window newWindow;
		newWindow.name = windowsRows[i][WindowColIndexes::name];
		newWindow.classWindow = windowsRows[i][WindowColIndexes::classWindow];
		newWindow.connectObjectName = windowsRows[i][WindowColIndexes::conObj];
		if(windowsRows[i][WindowColIndexes::w].toUInt() > 150)
			newWindow.w = windowsRows[i][WindowColIndexes::w];
		if(windowsRows[i][WindowColIndexes::h].toUInt() > 150)
			newWindow.h = windowsRows[i][WindowColIndexes::h];
		newWindow.windowPtQWidget = new QWidget;
		newWindow.tabWidget = nullptr;
		windows.push_back(std::move(newWindow));
	}
}

void Programm::CreateWidgets()
{
	for(uint windowi=0; windowi<windows.size(); windowi++)
	{
		for(uint i=0; i<widgetsRows.size(); i++)
		{
			if(windows[windowi].classWindow == widgetsRows[i][WidgetColIndexes::classWindow])
			{
				WidgetPair widget;

				widget.strIdToConnect = widgetsRows[i][WidgetColIndexes::idParam];
				widget.strParamDescribtion = widgetsRows[i][WidgetColIndexes::paramDescribtion];
				widget.strClassWindow = widgetsRows[i][WidgetColIndexes::classWindow];
				widget.strCaption = widgetsRows[i][WidgetColIndexes::caption];
				widget.strTabName = widgetsRows[i][WidgetColIndexes::tabName];
				widget.strTableName = widgetsRows[i][WidgetColIndexes::tableName];
				widget.strViewWidg = widgetsRows[i][WidgetColIndexes::viewWidg];
				widget.strCtrlWidg = widgetsRows[i][WidgetColIndexes::ctrlWidg];

				widget.view.CreateWidget(widget.strViewWidg);
				widget.ctrl.CreateWidget(widget.strCtrlWidg);
				windows[windowi].widgets.push_back(std::move(widget));
			}
		}
	}
}

void Programm::ConnectMake()
{
	for(auto &window:windows)
	{
		for(auto &widget:window.widgets)
		{
			Protocol *protocol = window.parentProtocol;
			FrameWorker *frWorker = widget.parentFrameWorker;
			Param *param_ = widget.parentParam;
			int objectType = protocol->type;
			for(auto &wpt:widget.ctrl.parts)
			{
				if(wpt.GetType() == WidgetPartTypes::button)
				{
					QPushButton *btn = (QPushButton *)wpt.GetPtQWidget();
					QString buttonName {wpt.GetName()};

					QObject::connect(btn, &QPushButton::clicked, [protocol, &widget, buttonName](){
						int size = widget.ctrl.obrabotchiki.size();
						if(size && !widget.parentFrameWorker)
						{
							widget.Error("ConnectMake:DoObrabotchiki: widget.parentFrameWorker is nullptr");
							return;
						}

						for(int i=0; i<size; i++)
						{
							auto obrabotchik = widget.ctrl.obrabotchiki[i];
							auto words = Code::CommandToWords(obrabotchik);
							if(words.size() < 3) widget.Error("ConnectMake:DoObrabotchiki: wrong widget command obrabotchik not enougth words " + obrabotchik);
							else
							{
								QString senderNameMustBe = words[1];

								if(senderNameMustBe == buttonName)
								{
									words.removeAt(1); // removing senderName
									words.removeAt(0); // removing obrabotchik keyword

									CANMsg_t msgPustishka;
									protocol->DoEverything(words.join(" "), widget.parentFrameWorker, msgPustishka, Protocol::DEretNullptr());
								}
							}
						}
					});

					if(frWorker && FrameTypes::all.contains(frWorker->TypeCell()))
					{
						if((objectType == Protocol::servis && frWorker->TypeCell() == FrameTypes::inFrame)
								|| (objectType == Protocol::emulator && frWorker->TypeCell() == FrameTypes::outFrame))
						{
							QObject::connect(btn, &QPushButton::clicked, [protocol, frWorker](){
								auto paramsAndFrames { protocol->ConstructFrames(frWorker) };
								protocol->SendFrames(paramsAndFrames);
							});
						}
						else Error("ConnectWidg wrong connect code 26021 param->typeDefined = "
								   + frWorker->TypeCell() + "; param->typeDefined = " + frWorker->FrameCell());
					}

					if(objectType == Protocol::emulator)
					{
						if(param_)
						{
							QString currCtrlWidgetOfParamType = widget.ctrl.GetType();
							std::vector<QWidget*> currPtQWidgCtrls;
							for(auto &wpt:widget.ctrl.parts)
								currPtQWidgCtrls.push_back(wpt.GetPtQWidget());

							if(currCtrlWidgetOfParamType == WidgetTypes::buttons)
							{
								for(uint i=0; i<currPtQWidgCtrls.size(); i++)
								{
									QPushButton *btn = (QPushButton *)currPtQWidgCtrls[i];
									QString newValue = btn->text();
									QObject::connect(btn, &QPushButton::clicked, [param_, newValue]()
									{
										param_->SetParamValue(Value(newValue,ValuesTypes::text));
									});
								}
							}
							else if(currCtrlWidgetOfParamType == WidgetTypes::editButton)
							{
								QPushButton *btn = (QPushButton *)currPtQWidgCtrls[1];
								QObject::connect(btn, &QPushButton::clicked, [param_, &widget]()
								{
									param_->SetParamValue(widget.ctrl.GetWidgetValue());
								});
							}
							else if(currCtrlWidgetOfParamType == WidgetTypes::chekBoxesNButton)
							{
								QObject::connect(btn, &QPushButton::clicked, [param_, &widget]()
								{
									param_->SetParamValue(widget.ctrl.GetWidgetValue());
								});
							}
							else if(widget.ctrl.parts.size() == 1
									&& widget.ctrl.parts[0].GetType() == WidgetPartTypes::checkBox)
							{
								QCheckBox *ch = (QCheckBox *)currPtQWidgCtrls[0];
								QObject::connect(ch, &QCheckBox::clicked, [param_, &widget]()
								{
									param_->SetParamValue(widget.ctrl.GetWidgetValue());
								});
							}
						}
					}
				}
			}
		}
	}
}

void Programm::PlaceWidgets()
{
	for(uint windowI=0; windowI<windows.size(); windowI++)
	{
		QGridLayout *layOutAll  = new QGridLayout(windows[windowI].windowPtQWidget);
		layOutAll->setObjectName("QVBoxLayout_all");

		for(uint widgI=0; widgI<windows[windowI].widgets.size(); widgI++)
		{
			WidgetPair *ptWidgetPair = &windows[windowI].widgets[widgI];
			QGridLayout *loForWidget = layOutAll;
			if(ptWidgetPair->strTabName != "")
			{
				if(!windows[windowI].tabWidget)
				{
					windows[windowI].CreateTabs(layOutAll);
				}

				loForWidget = windows[windowI].FindTabLayOut(ptWidgetPair->strTabName);
			}
			else { /*Nothing need to do. loForWidget = layOutAll default*/ }

			QTableWidget *tablePt = nullptr;
			if(ptWidgetPair->strTableName != "")
			{
				tablePt = windows[windowI].FindTable(ptWidgetPair->strTableName, false);
				if(!tablePt)
				{
					tablePt = windows[windowI].CreateTable(ptWidgetPair->strTableName,loForWidget);
				}
				if(!tablePt) Error("PlaceWidgets: nullptr rusult CreateTable");
			}
			else { /*Nothing need to do. tablePt = nullptr default*/ }

			if(ptWidgetPair->strTableName == "")
				ptWidgetPair->PlaceWidgetPairInGrLayout(loForWidget,true);
			else ptWidgetPair->PlaceWidgetPairInTable(tablePt);
		}
	}
}

void Programm::PlaceWindows(int startX, int startY)
{
	for(uint wi=0; wi<windows.size(); wi++)
	{
		windows[wi].windowPtQWidget->setWindowTitle(windows[wi].name);
		windows[wi].windowPtQWidget->resize(windows[wi].w.toUInt(),windows[wi].h.toUInt());
		for(auto t:windows[wi].tables)
		{
			t->setMinimumHeight(40 + t->rowCount()*t->rowHeight(0));
			t->setMinimumWidth(30 + t->columnCount()*t->columnWidth(0));

			if(t->minimumHeight()>600) t->setMinimumHeight(600);
			if(t->minimumWidth()>900) t->setMinimumWidth(900);
		}

		windows[wi].windowPtQWidget->show();

		windows[wi].windowPtQWidget->move(startX,startY);
	}

	if(monitor.windowPtQWidget) monitor.windowPtQWidget->move(startX, startY);
	if(controlPanel) controlPanel->move(startX, startY);
}
