#include "colindexes.h"

#include "programm.h"

#include "MyQFileDir.h"
#include "MyQDifferend.h"

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

Programm::Programm(IExcelWorkbook *wb)
{
	if(wb->Sheet(1)->Cell(1,1) == TypesDefines::version && wb->Sheet(1)->Cell(2,1) == "001") version = "001";

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
		CreateCopyWidgetRows();

		CreateConstsAndObjects();

		CreateSynchConnections();

		CreateWindows();
		CreateWidgets();

		monitors.push_back({programmName});
		LogsSt::SetAllToTextBrowser(this->monitors[monitors.size()-1].tBrowser, "Static ");
		this->SetAllToTextBrowser(this->monitors[monitors.size()-1].tBrowser, programmName+": ");

		for(uint i=0; i<monitorRows.size(); i++)
		{
			QString monitorName = monitorRows[i][MonitorColIndexes::name];
			QString connectObjName = monitorRows[i][MonitorColIndexes::conObj];
			Object* object = FindObject(connectObjName);
			if(!object) Error("Creation monitor error. Can't find object " + connectObjName);
			else
			{
				monitors.push_back(monitorName);
				object->SetAllToTextBrowser(this->monitors[monitors.size()-1].tBrowser, object->name+": ");
				for(auto &param:object->params)
				{
					param.SetAllToTextBrowser(this->monitors[monitors.size()-1].tBrowser, object->name+": ");
				}

				for(auto window:object->windows)
				{
					window->SetAllToTextBrowser(this->monitors[monitors.size()-1].tBrowser, window->name+": ");
					for(auto &widget:window->widgets)
					{
						widget.SetAllToTextBrowser(this->monitors[monitors.size()-1].tBrowser, window->name+": ");
					}
				}
			}
		}

		PlaceWidgets();

		int startX = 30;
		int startY = 100;
		PlaceWindows(startX,startY);

		GiveObjectsPtrOfWindows();

		GiveParamsPtrOfWidgets();

		for(auto &obj:objects)
			obj->ConnectMake();

		// установка связи для пересылки сообщений
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

		//TestWindow();

		LoadSettings();
	}
	else Error("Версия не поддерживается" + version);
}

Programm::~Programm()
{
	SaveSettings();

	for(auto c:constants) delete c;
	for(auto o:objects) delete o;
}

void Programm::CreateSynchConnections()
{
	for(auto &synchRow:synchRows)
	{
		QString objectsToSynchCell = synchRow[SynchCols::objectsToSynch];
		auto objectsToSynchList = Compiler::TextToCommands(objectsToSynchCell);
		std::vector<Object*> objectsToSynch;
		for(auto &objName:objectsToSynchList)
		{
			bool nameFinded = false;
			for(auto obj:objects)
				if(objName == obj->name)
				{
					if(nameFinded) Error("Synchron set wrong behavior. Name " + objName + " finded twice.");
					objectsToSynch.push_back(obj);
					nameFinded = true;
				}
			if(!nameFinded) Error("Synchron set wrong behavior. Name " + objName + " not found.");
		}

		int objectsToSynchSize = objectsToSynch.size();
		for(int i=0; i<objectsToSynchSize; i++)
		{
			for(int j=0; j<objectsToSynchSize; j++)
			{
				if(i!=j)
				{
					Object *obj1 = objectsToSynch[i];
					Object *obj2 = objectsToSynch[j];
					if(obj1->params.size() != obj2->params.size())
						Error("Synchron set wrong behavior. Different param size ("+QSn(obj1->params.size())
							  +" != " + QSn(obj2->params.size())+") in objects" + obj1->name + " " + obj2->name);
					else
					{
						int pSize = obj1->params.size();
						for(int p=0; p<pSize; p++)
						{
							if(obj1->params[p].CellValuesToStr() != obj2->params[p].CellValuesToStr())
								Error("Synchron set wrong behavior. Different cell values ("+obj1->params[p].CellValuesToStr()
									  +" != " + obj2->params[p].CellValuesToStr()+")");
							else
							{
								obj1->params[p].synchronParams.push_back(&obj2->params[p]);
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

		auto idPSrc = Compiler::DecodeStrNumbers(copyParamStr[cpci::idPSrc],true);
		auto idPDst = Compiler::DecodeStrNumbers(copyParamStr[cpci::idPDst],true);

		if(idPSrc.size() == 0 || idPSrc.size() != idPDst.size())
			Error("Copying params: wrong size idPSrc or idPDst:" + QSn(idPSrc.size()) + " " + QSn(idPDst.size())
				  + " copyParamStr: " + copyParamStr.join(' '));
		else
		{
			uint sizeId = idPSrc.size();
			std::vector<QStringList> newParamsStrs;

			int prevIndex = 0;
			for(uint i=0; i<sizeId; i++)
			{
				int index = FindIndexParamRow(QSn(idPSrc[i]),prevIndex);
				prevIndex = index;
				if(index == notFound) Error("Copying params: index == notFind copyParamStr: " + copyParamStr.join(' '));
				else
				{
					QStringList newParamStr = paramsRows[index];
					if(copyParamStr[cpci::system] != "") newParamStr[pci::system] = copyParamStr[cpci::system];
					if(copyParamStr[cpci::category] != "") newParamStr[pci::category] = copyParamStr[cpci::category];
					if(copyParamStr[cpci::module] != "") newParamStr[pci::module] = copyParamStr[cpci::module];
					if(copyParamStr[cpci::moduleNumber] != "") newParamStr[pci::moduleNumber] = copyParamStr[cpci::moduleNumber];
					if(copyParamStr[cpci::name] != "") newParamStr[pci::name] = copyParamStr[cpci::name];
					newParamStr[pci::id_] = QSn(idPDst[i]);
					newParamsStrs.push_back(std::move(newParamStr));
				}
			}

			for(auto &np:newParamsStrs)
				paramsRows.push_back(std::move(np));
		}
	}
}

void Programm::CreateCopyWidgetRows()
{
	for(auto &copyWidgetStr:copyWidgetsRows)
	{
		namespace wci = WidgetColIndexes;
		namespace cwci = CopyWidgetsColIndexes;

		auto idWSrc = Compiler::DecodeStrNumbers(copyWidgetStr[cwci::idWSrc],true);
		auto idWDst = Compiler::DecodeStrNumbers(copyWidgetStr[cwci::idWDst],true);
		auto idPCon = Compiler::DecodeStrNumbers(copyWidgetStr[cwci::idPConnect],true);

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
				prevIndex = index;
				if(index == notFound) Error("Copying widgets: index == notFind copyParamStr: " + copyWidgetStr.join(' '));
				else
				{
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

void Programm::SaveSettings()
{
	QString endParam = "[progP;] ";

	QStringList settingsWindows;
	QStringList settingsTables;
	QStringList settingsMonitors;

	for(auto o:objects)
	{
		for(auto w:o->windows)
		{
			QString settingWindow = "programmStng001" + endParam + programmName + endParam + "WindowsGeo" + endParam
					+ o->name + endParam + w->name + endParam + QSn(w->windowPtQWidget->x()) + endParam + QSn(w->windowPtQWidget->y()) + endParam
					+ QSn(w->windowPtQWidget->width()) + endParam + QSn(w->windowPtQWidget->height()) + endParam ;
			settingsWindows += settingWindow;
			for(auto t:w->tables)
			{
				QString settingTable = "programmStng001" + endParam + programmName + endParam + "TablesColsWidths" + endParam
						+ o->name + endParam + w->name + endParam + t->objectName() + endParam;
				for(int i=0; i<t->columnCount(); i++)
					settingTable += QSn(t->columnWidth(i)) + endParam;
				settingsTables += settingTable;
			}
		}
	}

	for(auto &monitor:monitors)
	{
		QString settingWindow = "programmStng001" + endParam + programmName + endParam + "MonitorsGeo" + endParam
				+ monitor.name + endParam + QSn(monitor.windwowPtQWidget->x()) + endParam + QSn(monitor.windwowPtQWidget->y()) + endParam
				+ QSn(monitor.windwowPtQWidget->width()) + endParam + QSn(monitor.windwowPtQWidget->height()) + endParam ;
		settingsWindows += settingWindow;
	}

	QString path = mqd::GetPathToExe() + "/files/programms_settings";
	if(!MQFD::CreatePath(path))
		Error("~Programm can't create path " + path);

	if(!mqd::SaveSettings(path+"/"+programmName+".stgs",{},settingsTables+settingsWindows))
		Error("~Programm can't save settings");
}

void Programm::LoadSettings()
{
	QStringList settings;
	std::vector<QWidget*> mock;
	QString path = mqd::GetPathToExe() + "/files/programms_settings";
	if(!mqd::LoadSettings(path+"/"+programmName+".stgs",mock,{settings}))
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

						Object *o = FindObject(objName);
						Window *w = nullptr;
						QTableWidget *t = nullptr;
						if(o) w = o->FindWindow(windName);
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

					Object *o = FindObject(objName);
					Window *window = nullptr;
					if(o) window = o->FindWindow(windName);
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
					Monitor *monitor = nullptr;
					for(auto &m:monitors)
						if(m.name == monName) monitor = &m;
					if(monitor) monitor->windwowPtQWidget->setGeometry(x,y,w,h);
					else Error("Read settings: can't find Monitor in setting "+stng);
				}
				else Error("Read settings: unknown command "+params[1]+" in setting "+stng);
			}
			else Error("Read settings: wrong file name "+params[0]+" in setting "+stng);
		}
		else Error("Wrong version in setting "+stng);
	}// cycle, no else
}

void Programm::TestWindow()
{
	QWidget *mainWindow = new QWidget;
	QGridLayout *glo = new QGridLayout(mainWindow);
	vector<QLabel*> windowsH;
	vector<QLabel*> windowsW;
	for(uint i=0; i<windows.size(); i++)
	{
		int row = glo->rowCount();
		glo->addWidget(new QLabel(windows[i].name + ":"),row,0);
		glo->addWidget(new QLabel("h ="),row,1);

		windowsH.push_back(new QLabel);
		glo->addWidget(windowsH.back(),row,2);
		glo->addWidget(new QLabel("w ="),row,3);
		windowsW.push_back(new QLabel);
		glo->addWidget(windowsW.back(),row,4);
	}

	auto btnTest = new QPushButton("test");
	int row = glo->rowCount();
	glo->addWidget(btnTest,row,0);
	QObject::connect(btnTest, &QPushButton::clicked,
					 [this](){
		for(auto &window:windows)
			for(auto &widget:window.widgets)
				for(auto &wpt:widget.ctrl.parts)
					if(wpt.GetType() == WidgetPartTypes::button && ((QPushButton*)wpt.GetPtQWidget())->text() == "Установить1")
						qdbg << wpt.GetPtQWidget()->width();
	});

	mainWindow->show();
	mainWindow->resize(100,50);
	if(windows.size())
		mainWindow->move( - (100 + mainWindow->width()),
						  windows[0].windowPtQWidget->y());

	QTimer *refresher = new QTimer;
	QObject::connect(refresher, &QTimer::timeout,[this,windowsH,windowsW](){
		for(uint i=0; i<windows.size(); i++)
		{
			windowsH[i]->setText(QSn(windows[i].windowPtQWidget->height()));
			windowsW[i]->setText(QSn(windows[i].windowPtQWidget->width()));
		}
	});
	refresher->start(100);
}

void Programm::GiveParamsPtrOfWidgets()
{
	for(uint oi=0; oi<objects.size(); oi++)
	{
		for(auto window:objects[oi]->windows)
		{
			for(auto &widget:window->widgets)
			{
				if(widget.strIdParam != CommandsKeyWords::null)
				{
					Param *param = objects[oi]->FindParamById(widget.strIdParam);
					if(param)
					{
						param->widgets.push_back(&widget);
						auto paramValue = param->IOGetValueAndType();
						if(paramValue.first != ValuesTypes::undefined)
							widget.view.SetWidgetValue(paramValue);
						param->ctrlWidgetsNames.push_back(widget.ctrl.GetName());
						for(auto &wpt:widget.ctrl.parts)
							param->ctrlWidgetsNames.push_back(wpt.GetName());
					}
					else
						Error("Can't find param for widget! Object name " + objects[oi]->name + "; Window name " + window->name + "; Widget str: " + widget.ToStr());
				}
			}
		}
	}
}

void Programm::GiveObjectsPtrOfWindows()
{
	for(uint wi=0; wi<windows.size(); wi++)
	{
		bool findObjectForWindow = false;
		for(uint oi=0; oi<objects.size(); oi++)
			if(windows[wi].connectObjectName == objects[oi]->name)
			{
				if(!findObjectForWindow)
				{
					objects[oi]->windows.push_back(&windows[wi]);
					findObjectForWindow = true;
				}
				else Error("Programm(IExcelWorkbook *wb): window has not one object connect! window name: " + windows[wi].name + " object name: " + objects[oi]->name);

			}
		if(!findObjectForWindow)
			Error("Programm(IExcelWorkbook *wb): window has no object connect! window name: " + windows[wi].name + " connetc name: " + windows[wi].connectObjectName);
	}
}

Object *Programm::FindObject(QString name)
{
	for(auto o:objects)
		if(o->name == name) return o;
	return nullptr;
}

int Programm::FindIndexParamRow(QString idSrcParam, uint startIndex)
{
	uint paramsSize = paramsRows.size();
	if(startIndex >= paramsSize) Error("FindParamStr: startIndex >= paramsStrs.size() " + QSn(startIndex) + " " + QSn(paramsSize));
	for(uint i=startIndex; i<paramsSize; i++)
	{
		if(paramsRows[i][ParamsColIndexes::id_] == idSrcParam)
			return i;
	}
	for(uint i=0; i<startIndex && i<paramsSize; i++)
	{
		if(paramsRows[i][ParamsColIndexes::id_] == idSrcParam)
			return i;
	}
	return notFound;
}

int Programm::FindIndexWidgetRow(QString windowClass, QString idSrcWidget, uint startIndex)
{
	uint widgStrsSize = widgetsRows.size();
	if(startIndex >= widgStrsSize) Error("FindWidgetStr: startIndex >= widgStrsSize " + QSn(startIndex) + " " + QSn(widgStrsSize));
	for(uint i=startIndex; i<widgStrsSize; i++)
	{
		if(widgetsRows[i][WidgetColIndexes::classWindow] == windowClass && widgetsRows[i][WidgetColIndexes::idWidget] == idSrcWidget)
			return i;
	}
	for(uint i=0; i<startIndex && i<widgStrsSize; i++)
	{
		if(widgetsRows[i][WidgetColIndexes::classWindow] == windowClass && widgetsRows[i][WidgetColIndexes::idWidget] == idSrcWidget)
			return i;
	}
	return notFound;
}

void Programm::CreateConstsAndObjects()
{
	for(auto &genStr:objectsRows)
	{
		int tip = Param::undefined;
		if(genStr[ObjectsCols::category] == ObjectsCols::emulator) tip = Param::emulator;
		else if(genStr[ObjectsCols::category] == ObjectsCols::servis) tip = Param::servis;
		else Error("Programm::Programm unknown genStr" + genStr.join(' '));

		QString name = genStr[ObjectsCols::name];
		QString chan = genStr[ObjectsCols::canChannel];
		QString startSettings = genStr[ObjectsCols::startSettings];

		Constants *consts = new Constants(contantsRows,version,startSettings);
		Object *obj = new Object(paramsRows, version, name, tip, consts, chan.toInt());
		constants.push_back(consts);
		objects.push_back(obj);
		obj->InitFilters();
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

				widget.strIdParam = widgetsRows[i][WidgetColIndexes::idParam];
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
				if(!windows[windowI].tabWidget) windows[windowI].CreateTabs(layOutAll);

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
		startX = startX + windows[wi].windowPtQWidget->width() + 30;
	}
}
