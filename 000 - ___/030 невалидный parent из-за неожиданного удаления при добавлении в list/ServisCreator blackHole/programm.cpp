#include "programm.h"

#include <thread>
using namespace std;

#include <QApplication>
#include <QHostInfo>
#include <QSettings>

#include "MyQFileDir.h"
#include "MyQDifferent.h"
#include "MyQDom.h"
#include "MyQDialogs.h"

#include "cansettingdialogdynamic.h"

int defX = 30;
int defY = 30;

Programm::Programm(const QDomElement &bodyElement, QString programmName_, QString aprojectFile,
				   NoParamsFunction deleter_, NoParamsFunction onDestructorEnds_)
{
	programmName=programmName_;
	projectFile = aprojectFile;
	interfaces = make_unique<CANInterfaces>();
	deleter = deleter_;
	onDestructorEnds = onDestructorEnds_;

	Settings::SetDefault();

	std::vector<QDomElement> elements = MyQDom::GetTopLevelElements(bodyElement);
	std::vector<QDomElement> externalWindowsElements;
	QString monitorCreationContent;
	QString controlCreationContent;
	std::vector<QStringList> synchRows;

	// поиск элементов монитора и панели управления
	for(int i=(int)elements.size()-1; i>=0; i--)
	{
		if(elements[i].tagName() == OtherConfigItem_ns::rowCaption)
		{
			QString content = elements[i].attribute(OtherConfigItem_ns::content);
			if(content.startsWith(OtherConfigItem_ns::Content::ctrlPanel))
			{
				controlCreationContent = move(content);
				elements.erase(elements.begin()+i);
			}
			else if(content.startsWith(OtherConfigItem_ns::Content::monitor))
			{
				monitorCreationContent = move(content);
				elements.erase(elements.begin()+i);
			}
			else Error(GetClassName()+"::(...) wrong Other tag content");
		}
	}

	if(controlCreationContent.size())
		CreateControlPanel(controlCreationContent);

	if(monitorCreationContent.size())
	{
		monitor.Create();
	}
	else
	{
		Settings::logDoEvrythingCommands = false;
		Settings::logFrameGet = false;
		Settings::logFrameSent = false;
		Settings::logParamChanges = false;
		controlPanel.SetCheckBoxesFromSettings();
	}

	for(auto &element:elements)
	{
		QString tagName = element.tagName();
		if(tagName == Constant_ns::rowCaptionConstants)
			constants.push_back(new Constants(element));
		else if(tagName == ChannelConfigItem_ns::Fields::rowCaption)
		{
			namespace thisNs = ChannelConfigItem_ns::Fields;
			QString name = element.attribute(thisNs::name);
			QString type = element.attribute(thisNs::type);
			QString params = element.attribute(thisNs::params);

			if(type == "CAN")
			{
				channels.push_back(make_unique<InternalChannel>());
				channels.back()->Init(name,interfaces.get(),params);
			}
			else Error("Programm(...): unsupported type channel ["+type+"] for creation ["+thisNs::rowCaption+"]");
		}
		else if(tagName == ObjectConfigItem_ns::rowCaption)
			objects.push_back(new Object(element));
		else if(tagName == Protocol_ns::Protocol)
		{
			namespace thisNs = Protocol_ns;
			QString name = element.attribute(thisNs::name);
			QString constantsName = element.attribute(thisNs::constants);
			QString chanStr = element.attribute(thisNs::channel);

			Object *objToCon = nullptr;
			for(auto &obj:objects)
				if(obj->name == name) objToCon = obj;
			if(!objToCon) Error("Programm(...): can't find object ["+name+"] for ["+thisNs::Protocol+"]");

			Constants *constantsToCon = nullptr;
			for(auto &constant:constants)
				if(constant->Name() == constantsName) constantsToCon = constant;
			if(!constantsToCon) Error("Programm(...): can't find constants ["+constantsName+"]");

			if(objToCon && constantsToCon)
				protocols.push_back(new Protocol(element, objToCon, constantsToCon));
		}
		else if(tagName == WindowConfigItem_ns::Fields::rowCaption)
		{
			if(element.attribute(WindowConfigItem_ns::Fields::windowClass).contains(WindowConfigItem_ns::Fields::external))
			{
				externalWindowsElements.push_back(element);
			}
			else
			{
				windows.push_back(Window(element));
			}
		}
		else if(tagName == SyncConfigItem_ns::Fields::rowCaption)
		{
			QStringList synchRow;
			for(int i=0; i<SynchCols::colsCount; i++) synchRow += "";
			synchRow[SynchCols::objectsToSynch] = element.attribute(SyncConfigItem_ns::Fields::command);
			synchRows.push_back(std::move(synchRow));
		}
		else Error("Programm(...): wrong tag ["+tagName+"]");
	}

	// создание Окон неконфигурационных
	for(auto &extWindowElement:externalWindowsElements)
	{
		externalWindows.push_back(IExternalWindow::Make(extWindowElement));
		if(!externalWindows.back())
		{
			Error("Programm(...): IWindow::Make(element) result is nullptr, window not created");
			externalWindows.pop_back();
		}
		else
		{
			externalWindows.back()->ConnectToObjects(this,objects,protocols);
		}
	}
	// создание Окон неконфигурационных завершено

	SetLogsMsgWorkers();

	for(auto &prot:protocols)
	{
		for(auto &internalChan:channels)
		{
			if(prot->channelStr == internalChan->name)
			{
				prot->internalChanPtr = internalChan.get();
				internalChan->AddClient(prot);
			}
		}

		if(!prot->internalChanPtr) Error("Programm(...): can't find channel ["+prot->channelStr+"] for Protocol ["+prot->name+"]");
	}

	SynchConnectionsCreate(synchRows);

	PlaceAndShowWindows(defX,defY);

	GiveWindowsPtrs();
	GiveWidgetsPtrs();

	ConnectMake();

	LoadSettings();
}

Programm::~Programm()
{
	SaveSettings();

	Log("~Programm()");

	for(auto &w:windows) if(w.windowPtQWidget) delete w.windowPtQWidget;
	for(auto &p:protocols) delete p;
	for(auto &o:objects) delete o;
	for(auto &c:constants) delete c;
	DeleteControlPanel();
	monitor.Delete();
	if(onDestructorEnds) onDestructorEnds();
}

void Programm::PrintGetedConfig()
{
	if(getedConfigWritten)
		MyQDialogs::ShowText(getedConfig,1100,500);
	else QMbi(nullptr,"Не готово","Не готово");
}

void Programm::PrintStructureObjects()
{
	QStringList structure;
	QString separatorBold = "===============================================================================";
	QString separatorThin = "-------------------------------------------------------------------------------";
	structure += separatorBold;
	structure += "Constants";
	for(auto &constants:constants)
	{
		structure += "Constants " + constants->Name();
		structure += constants->ToStr();
	}
	structure += separatorBold;
	structure += "Objects";
	for(auto &object:objects)
	{
		structure += "Object " + object->name + " " + CodeKeyWords::TypeToStr(object->type);
		for(auto &param:object->params)
			structure += "\t" + param->ToStrForLogShort();
	}
	structure += separatorBold;
	structure += "Protocols";
	structure += separatorBold;
	structure += "Windows";
	structure += separatorBold;
	structure += "ExternalWindows";
	structure += separatorBold;
	MyQDialogs::ShowText(structure.join("\n"),1500,800);
}

void Programm::CloseAll()
{
	for(auto &w:windows) if(w.windowPtQWidget) w.windowPtQWidget->close();
	for(auto &w:externalWindows) if(w->Window()) w->Window()->close();
	if(controlPanel.window) controlPanel.window->close();
	if(monitor.windowPtQWidget) monitor.windowPtQWidget->close();
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
	if(!monitor.windowPtQWidget) return;

	monitor.SetProgrammName(programmName);

	for(auto object:objects)
	{
		object->SetLogPrefix(object->name+": ");
		for(auto &param:object->params)
			param->SetLogPrefix(object->name+": ");
	}

	for(auto prot:protocols)
	{
		prot->SetLogPrefix(prot->name+": ");
		for(auto &worker:prot->workers)
			worker->SetLogPrefix(prot->name+": ");
	}
}

void Programm::SynchConnectionsCreate(const std::vector<QStringList> &synchRows)
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
					if(obj1->workers.size() != obj2->workers.size())
						Error("Synchron set wrong behavior. Different param size ("+QSn(obj1->workers.size())
							  +" != " + QSn(obj2->workers.size())+") in objects" + obj1->name + " " + obj2->name);
					else
					{
						int fwSize = obj1->workers.size();
						for(int fw_i=0; fw_i<fwSize; fw_i++)
						{
							if(obj1->workers[fw_i]->CellValuesToStrForLog() != obj2->workers[fw_i]->CellValuesToStrForLog())
								Error("Synchron set wrong behavior. Different cell values ("+obj1->workers[fw_i]->CellValuesToStrForLog()
									  +" != " + obj2->workers[fw_i]->CellValuesToStrForLog()+")");
							else
							{
								obj1->workers[fw_i]->synchronWorkers.push_back(obj2->workers[fw_i].get());
							}
						}
					}
				}
			}
		}
	}
}

void Programm::SaveSettings()
{
	if(Settings::notSaveSettings) return;
	if(!Programm::CheckProgrammName(true)) return;

	QSettings settings(projectFile+".ini", QSettings::IniFormat);
	settings.clear();
	settings.setValue("settings version","001");
	settings.setValue("host name", QHostInfo::localHostName());

	settings.beginGroup("windows");
	for(auto &w:windows)
	{
		if(w.windowPtQWidget && w.connectedObjects.size())
		{
			settings.beginGroup(w.name);
			settings.setValue("objectName", w.connectedObjects[0]->name);
			settings.setValue("geo",MyQDifferent::GetGeo(*w.windowPtQWidget));
			if(w.tables.size())
			{
				settings.beginGroup("tables");
				for(auto table:w.tables)
				{
					if(table)
					{
						settings.beginGroup(table->objectName());
						QString tableColsWidths;
						for(int i=0; i<table->columnCount(); i++)
							tableColsWidths += QSn(table->columnWidth(i)) + ";";
						settings.setValue("cols widths", tableColsWidths);
						settings.endGroup();
					}
					else Error(GetClassName()+"::SaveSettings nullptr table");
				}
				settings.endGroup();
			}
			settings.endGroup();
		}
		else Warning(GetClassName()+"::SaveSettings can't save window settings windowPtQWidget or parentObject is nullptr");
	}
	settings.endGroup();

	settings.beginGroup("external windows");
	for(auto &ew:externalWindows)
	{
		settings.beginGroup(ew->Name());
		settings.setValue("settings", ew->SaveSettings());
		settings.endGroup();
		QString setting;
	}
	settings.endGroup();

	if(monitor.windowPtQWidget)
	{
		settings.setValue("monitor geo", MyQDifferent::GetGeo(*monitor.windowPtQWidget));
	}

	if(controlPanel.window)
	{
		settings.setValue("control panel geo", MyQDifferent::GetGeo(*controlPanel.window));
	}
}

void Programm::LoadSettings()
{
	if(!Programm::CheckProgrammName(true)) return;

	QString settingsFile = projectFile+".ini";
	if(!QFileInfo::exists(settingsFile))
	{
		Log("LoadSettings:: settings file ["+settingsFile+"] not found, default set");
		return;
	}

	QSettings settings(settingsFile, QSettings::IniFormat);
	QString version = settings.value("settings version").toString();
	if(version == "001")
	{
		if(settings.value("host name") != QHostInfo::localHostName())
			return;

		settings.beginGroup("windows");
		QStringList windowNamesInSettings = settings.childGroups();
		for(auto &windowNameInSettings:windowNamesInSettings)
		{
			settings.beginGroup(windowNameInSettings);
			QString objectName = settings.value("objectName").toString();
			Window *w = FindWindow(objectName,windowNameInSettings, false);
			if(w)
			{
				auto geo = settings.value("geo").toString();
				MyQDifferent::SetGeo(geo, *w->windowPtQWidget);
				settings.beginGroup("tables");
				QStringList tableNamesInSettings = settings.childGroups();
				for(auto &tableName:tableNamesInSettings)
				{
					settings.beginGroup(tableName);
					QTableWidget *t = w->FindTable(tableName, false);
					if(t)
					{
						auto widths = settings.value("cols widths").toString().split(";",QString::SkipEmptyParts);
						for(int c=0; c<widths.size(); c++)
						{
							if(c < t->columnCount())
								t->setColumnWidth(c,widths[c].toUInt());
						}
					}
					else Warning("LoadSettings: can't find TableWidget ["+tableName+"] in settings");
					settings.endGroup();
				}
				settings.endGroup();
			}
			else Warning("LoadSettings: can't find Object ["+objectName+"] or Window ["+windowNameInSettings+"]");
			settings.endGroup();
		}
		settings.endGroup();

		settings.beginGroup("external windows");
		QStringList externalWindowsNames = settings.childGroups();
		for(auto &externalWindowName:externalWindowsNames)
		{
			if(IExternalWindow * extWindowPtr = FindExternalWindow(externalWindowName))
			{
				settings.beginGroup(externalWindowName);
				extWindowPtr->LoadSettings(settings.value("settings").toString());
				settings.endGroup();
			}
			else Warning("LoadSettings: can't find External window ["+externalWindowName+"]");
		}
		settings.endGroup();

		QStringList keys = settings.childKeys();
		if(keys.contains("monitor geo") && monitor.windowPtQWidget)
			MyQDifferent::SetGeo(settings.value("monitor geo").toString(),*monitor.windowPtQWidget);

		if(keys.contains("control panel geo") && controlPanel.window)
			MyQDifferent::SetGeo(settings.value("control panel geo").toString(),*controlPanel.window);
	}
	else Warning("Wrong version ["+version+"] in settings " + settingsFile);
}

bool Programm::CheckProgrammName(bool printError)
{
	if(programmName == "")
	{
		if(printError) Error("Programm::CheckProgrammName error: programmName empty");
		return false;
	}
	return true;
}

void Programm::CreateControlPanel(QString creationStr)
{
	controlPanel.window = new QWidget;
	controlPanel.window->setWindowTitle("Control panel " + programmName);
	controlPanel.window->resize(250,30);

	bool defWindowsGeo				= true;
	bool printGetedConfig			= true;
	bool printStructureObjects		= true;
	bool stopProtTimers				= true;
	bool connectSettings			= true;
	bool logDoEveryThingCommands	= true;
	bool warnNotPassed				= true;
	bool logParamChanged			= true;
	bool logFrameSent				= true;
	bool logFrameGet				= true;
	bool close						= true;
	bool closeNotSaveSettings		= true;
	bool terminateApp				= true;
	bool terminateAppNotSaveStgs	= true;

	auto commands = Code::TextToCommands(creationStr);
	if(commands.size() == 1)
	{
		auto words = Code::CommandToWords(commands[0]);
		if(words.size() > 2)
		{
			for(int i=2; i<words.size(); i+=2)
			{
				bool newValue = true;
				bool noError = true;
				if(words[i] == "-") newValue = false;
				else if(words[i] == "+") newValue = true;
				else
				{
					noError = false;
					Error(GetClassName() + "::CreateControlPanel wrong case in creationStr ["+creationStr+"]");
				}

				if(noError)
				{
					if(i+1 < words.size())
					{
						if(0){}
						else if(words[i+1] == OtherConfigItem_ns::ControlPanel::all)
						{
							defWindowsGeo				= newValue;
							printGetedConfig			= newValue;
							printGetedConfig			= newValue;
							printStructureObjects		= newValue;
							stopProtTimers				= newValue;
							connectSettings				= newValue;
							logDoEveryThingCommands		= newValue;
							warnNotPassed				= newValue;
							logParamChanged				= newValue;
							logFrameSent				= newValue;
							logFrameGet					= newValue;
							close						= newValue;
							closeNotSaveSettings		= newValue;
							terminateApp				= newValue;
							terminateAppNotSaveStgs		= newValue;
						}
						else if(words[i+1] == OtherConfigItem_ns::ControlPanel::connectSettings)
						{
							connectSettings				= newValue;
						}
						else if(words[i+1] == OtherConfigItem_ns::ControlPanel::close)
						{
							close						= newValue;
						}
						else if(words[i+1] == OtherConfigItem_ns::ControlPanel::closeNotSaveSettings)
						{
							closeNotSaveSettings		= newValue;
						}
						else Error(GetClassName() + "::CreateControlPanel unknown setting ["+words[i+1]+"] in creationStr ["+creationStr+"]");
					}
					else Error(GetClassName() + "::CreateControlPanel not finished creationStr ["+creationStr+"]");
				}
			}
		}
	}
	else Error(GetClassName() + "::CreateControlPanel wrong creationStr commans size");

	QGridLayout *glo = new QGridLayout(controlPanel.window);

	if(defWindowsGeo)
	{
		QPushButton *btn = new QPushButton ("Default window geo", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[this](){ PlaceAndShowWindows(defX,defY); });
	}

	if(printGetedConfig)
	{
		QPushButton *btn = new QPushButton ("Print geted config", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[this](){ PrintGetedConfig(); });
	}

	if(printStructureObjects)
	{
		QPushButton *btn = new QPushButton ("Print structure objects", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[this](){ PrintStructureObjects(); });
	}

	if(stopProtTimers)
	{
		QPushButton *btn = new QPushButton ("Stop protocols timers", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[this, btn]()
		{
			QString toDo = btn->text();
			for(auto &prot:protocols)
				for(auto &timer:prot->timers)
				{
					if(toDo == "Stop protocols timers")
					{
						if(timer.qTimer)
							timer.qTimer->stop();
						btn->setText("Start protocols timers");
					}
					else if(toDo == "Start protocols timers")
					{
						if(timer.qTimer)
							timer.qTimer->start();
						btn->setText("Stop protocols timers");
					}
				}
			if(toDo == "Stop protocols timers") Warning("Timers stoped");
			if(toDo == "Start protocols timers") Warning("Timers started");
		});
	}

	if(connectSettings)
	{
		QPushButton *btn = new QPushButton ("Настройка подключения", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[this]()
		{
			QDialog *dialogConnSettings = new QDialog();
			QVBoxLayout *loMain = new QVBoxLayout(dialogConnSettings);
			QGridLayout *loGrid = new QGridLayout;
			loMain->addLayout(loGrid);
			int col = 0;
			loGrid->addWidget(new QLabel("",dialogConnSettings),0,col++);
			loGrid->addWidget(new QLabel("Имя",dialogConnSettings),0,col++);
			loGrid->addWidget(new QLabel("CAN №",dialogConnSettings),0,col++);
			loGrid->addWidget(new QLabel("Режим",dialogConnSettings),0,col++);
			loGrid->addWidget(new QLabel("Скорость",dialogConnSettings),0,col++);
			loGrid->addWidget(new QLabel("Статус",dialogConnSettings),0,col++);
			for(auto &channel:channels)
			{
				ICANChannel *chanPtr = channel->externalChannel;
				InternalChannel *internalChanPtr = channel.get();
				int row = loGrid->rowCount();
				int col = 0;
				loGrid->addWidget(new QLabel("Канал",dialogConnSettings),row,col++);
				loGrid->addWidget(new QLabel(channel->name,dialogConnSettings),row,col++);

				if(!chanPtr) loGrid->addWidget(new QLabel("не обнаружен",dialogConnSettings),row,col++);
				if(chanPtr)
				{
					auto labelNumber = new QLabel(QSn(chanPtr->GetNumberAbsolut()),dialogConnSettings);
					auto labelRegime = new QLabel(ICANChannel::RegimeToStr(chanPtr->GetRegime()),dialogConnSettings);
					auto labelBaud__ = new QLabel(ICANChannel::BaudToStr(chanPtr->GetBaud()),dialogConnSettings);
					auto labelStatus = new QLabel(ICANChannel::StatusToStr(chanPtr->GetStatus()),dialogConnSettings);
					labelNumber->setAlignment(Qt::AlignCenter);
					loGrid->addWidget(labelNumber,row,col++);
					loGrid->addWidget(labelRegime,row,col++);
					loGrid->addWidget(labelBaud__,row,col++);
					loGrid->addWidget(labelStatus,row,col++);
					QPushButton *btn = new QPushButton ("Настроить", dialogConnSettings);
					loGrid->addWidget(btn,row,col++);
					QObject::connect(btn,&QPushButton::clicked,[this, dialogConnSettings,
									 labelNumber, labelRegime, labelBaud__, labelStatus, internalChanPtr]()
					{
						internalChanPtr->externalChannel->Stop();
						internalChanPtr->externalStarted = false;
						labelStatus->setText(ICANChannel::StatusToStr(internalChanPtr->externalChannel->GetStatus()));
						vector<ICANChannel*> allChannels = interfaces->GetChannels_if([](ICANChannel *chan){
								return chan->GetStatus() == ICANChannel::AVIABLE; });
						CANSettingDialogDynamic *CANSettingDialog1 = new CANSettingDialogDynamic(dialogConnSettings);
						if(CANSettingDialog1->Execute(&allChannels,ICANChannel::BR_250K,true))
						{
							ICANChannel *newChanPtr = nullptr;
							for(auto ch:allChannels) if(ch->GetStatus() == ICANChannel::STARTED)
								newChanPtr = ch;
							if(newChanPtr)
							{
								internalChanPtr->externalStarted = true;
								internalChanPtr->externalChannel = newChanPtr;
							}
						}
						labelNumber->setText(QSn(internalChanPtr->externalChannel->GetNumberAbsolut()));
						labelRegime->setText(ICANChannel::RegimeToStr(internalChanPtr->externalChannel->GetRegime()));
						labelBaud__->setText(ICANChannel::BaudToStr(internalChanPtr->externalChannel->GetBaud()));
						labelStatus->setText(ICANChannel::StatusToStr(internalChanPtr->externalChannel->GetStatus()));
					});
				}
			}
			dialogConnSettings->exec();
			delete dialogConnSettings;
		});
	}

	if(logDoEveryThingCommands)
	{
		controlPanel.chLogDoEvrythingCommands = new QCheckBox("Log DoEvrything Commands", controlPanel.window);
		glo->addWidget(controlPanel.chLogDoEvrythingCommands,glo->rowCount(),1,1,-1);
		QObject::connect(controlPanel.chLogDoEvrythingCommands,&QCheckBox::clicked,[this](){
			if(controlPanel.chLogDoEvrythingCommands->isChecked()) Settings::logDoEvrythingCommands = true;
			else Settings::logDoEvrythingCommands = false;
		});
	}

	if(warnNotPassed)
	{
		controlPanel.chWarningNotPassedFilterFrames = new QCheckBox("Warning not passed filter frames", controlPanel.window);
		glo->addWidget(controlPanel.chWarningNotPassedFilterFrames,glo->rowCount(),1,1,-1);
		QObject::connect(controlPanel.chWarningNotPassedFilterFrames,&QCheckBox::clicked,[this](){
			if(controlPanel.chWarningNotPassedFilterFrames->isChecked()) Settings::warningNotPassedFilterFrames = true;
			else Settings::warningNotPassedFilterFrames = false;
		});
	}

	if(logParamChanged)
	{
		controlPanel.chLogParamChanges = new QCheckBox("Log params changes", controlPanel.window);
		controlPanel.chLogParamChanges->setChecked(true);
		glo->addWidget(controlPanel.chLogParamChanges,glo->rowCount(),1,1,-1);
		QObject::connect(controlPanel.chLogParamChanges,&QCheckBox::clicked,[this](){
			if(controlPanel.chLogParamChanges->isChecked()) Settings::logParamChanges = true;
			else Settings::logParamChanges = false;
		});
	}

	if(logFrameSent)
	{
		controlPanel.chLogFrameSent = new QCheckBox("Log frame sent", controlPanel.window);
		controlPanel.chLogFrameSent->setChecked(true);
		glo->addWidget(controlPanel.chLogFrameSent,glo->rowCount(),1,1,-1);
		QObject::connect(controlPanel.chLogFrameSent,&QCheckBox::clicked,[this](){
			if(controlPanel.chLogFrameSent->isChecked()) Settings::logFrameSent = true;
			else Settings::logFrameSent = false;
		});
	}

	if(logFrameGet)
	{
		controlPanel.chLogFrameGet = new QCheckBox("Log frame get", controlPanel.window);
		controlPanel.chLogFrameGet->setChecked(true);
		glo->addWidget(controlPanel.chLogFrameGet,glo->rowCount(),1,1,-1);
		QObject::connect(controlPanel.chLogFrameGet,&QCheckBox::clicked,[this](){
			if(controlPanel.chLogFrameGet->isChecked()) Settings::logFrameGet = true;
			else Settings::logFrameGet = false;
		});
	}

	if(close)
	{
		QPushButton *btn = new QPushButton ("Close", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[this](){ Settings::notSaveSettings = false; CloseAll(); if(deleter) deleter(); });
	}

	if(closeNotSaveSettings)
	{
		QPushButton *btn = new QPushButton ("Close (not save settings)", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[this](){ Settings::notSaveSettings = true; CloseAll(); if(deleter) deleter(); });
	}

	if(terminateApp)
	{
		QPushButton *btn = new QPushButton ("Terminate app", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[](){ Settings::notSaveSettings = false; QApplication::exit(); });
	}

	if(terminateAppNotSaveStgs)
	{
		QPushButton *btn = new QPushButton ("Terminate app (not save settings)", controlPanel.window);
		glo->addWidget(btn,glo->rowCount(),1,1,-1);
		QObject::connect(btn,&QPushButton::clicked,[](){ Settings::notSaveSettings = true; QApplication::exit(); });
	}

	controlPanel.SetCheckBoxesFromSettings();
}

void Programm::DeleteControlPanel()
{
	if(controlPanel.window)
	{
		delete controlPanel.window;
		controlPanel.window = nullptr;
	}
}

void Programm::GiveWidgetsPtrs()
{
	for(auto &window:windows)
	{
		if(window.connectedObjects.empty() || window.connectedProtocols.empty())
		{
			Error(GetClassName()+":: GiveWidgetsPtrs window.connectedObjects.empty() || window.connectedProtocols.empty()");
			continue;
		}

		for(auto &widjetPair:window.widgetPairs)
		{
			if(widjetPair.IsDecorative()) continue;

			QString paramOrWorker, categoryToFind, nameToFind;
			int indexToConnect;
			widjetPair.GetDataForConnect(paramOrWorker, categoryToFind, nameToFind, indexToConnect);
			if(indexToConnect >= (int)window.connectedObjects.size() || indexToConnect >= (int)window.connectedProtocols.size())
			{
				Error("Ошибка подключения виджета к объекту или протоколу!\n\tУ окна" + window.name + " есть подключение к "
					  + QSn(window.connectedObjects.size()) + " объекту и " +QSn(window.connectedProtocols.size())
					  + " протоколу.\n\tВиджет пытается подключиться к № " + QSn(indexToConnect+1)
					  + "\n\tВиджет: " + widjetPair.ToStrForLog());
				continue;
			}
			widjetPair.parentObject = window.connectedObjects[indexToConnect];
			widjetPair.parentProtocol = window.connectedProtocols[indexToConnect];

			if(paramOrWorker == Widget_ns::param && (int)window.connectedObjects.size() > indexToConnect)
			{
				auto paramsPtrs = window.connectedObjects[indexToConnect]->FindParamsInCategory(categoryToFind,nameToFind,false);
				if(paramsPtrs.size() == 1)
				{
					widjetPair.parentParam = paramsPtrs[0];
					widjetPair.view.SetWidgetValue(paramsPtrs[0]->GetParamValue());
					CBParamChanged cbForParam;
					cbForParam.cbHandler = &widjetPair;
					cbForParam.cbFunction = [](const Value &newValue, HaveClassName *handler)
					{
						WidgetPair* wpHandler = dynamic_cast<WidgetPair*>(handler);
						if(wpHandler)
						{
							wpHandler->view.SetWidgetValue(newValue);
						}
						else Logs::ErrorSt("Programm::GiveWidgetsPtrOfParams: cbForParam can't convert handler to WidgetPair*");
					};

					paramsPtrs[0]->AddCBParamChanged(cbForParam);
				}
				else Error(GetClassName() + "::GiveWidgetsPtrs wrong size paramPtrs ("+QSn(paramsPtrs.size())
						   +")\n\tTryed to find by category [" + categoryToFind + "] and name [" + nameToFind + "]");
			}
			else if(paramOrWorker == Widget_ns::worker && (int)window.connectedProtocols.size() > indexToConnect)
			{
				auto workersPtrs = window.connectedProtocols[indexToConnect]->FindWorkersInCategory(categoryToFind,nameToFind,false);
				if(workersPtrs.size() == 1)
				{
					widjetPair.parentWorker = workersPtrs[0];
				}
				else Error(GetClassName() + "::GiveWidgetsPtrs wrong size fwPtrs ("+QSn(workersPtrs.size())
						   +")\n\tTryed to find by category [" + categoryToFind + "] and name [" + nameToFind + "]");
			}
			else Error(GetClassName() + "::GiveWidgetsPtrs bad result GetDataForConnect(paramOrFrame, category, name);\n\tTryed to find by ("
					   + widjetPair.strParam + " or " + widjetPair.strWorker + ")");
		}
	}
}

void Programm::GiveWindowsPtrs()
{
	for(uint wi=0; wi<windows.size(); wi++)
	{
		QStringList namesToConnect = windows[wi].connectStr.split(CodeKeyWords::commandSplitter,QString::SkipEmptyParts);

		for(auto &nameToConnect:namesToConnect)
		{
			bool findObjectForWindow = false;
			for(uint oi=0; oi<objects.size(); oi++)
				if(nameToConnect == objects[oi]->name)
				{
					if(!findObjectForWindow)
					{
						windows[wi].connectedObjects.push_back(objects[oi]);
						findObjectForWindow = true;
					}
					else Error(GetClassName()+"::GiveWindowsPtrs: window has not one object to connect! window name: "
							   + windows[wi].name + " object name to connect: " + objects[oi]->name);
				}
			if(!findObjectForWindow)
				Error(GetClassName()+"::GiveWindowsPtrs: window has no object connect! window name: "
					  + windows[wi].name + " object name to connect: " + windows[wi].connectStr);

			bool findProtForWindow = false;
			for(uint pi=0; pi<protocols.size(); pi++)
				if(nameToConnect == protocols[pi]->name)
				{
					if(!findProtForWindow)
					{
						windows[wi].connectedProtocols.push_back(protocols[pi]);
						findProtForWindow = true;
					}
					else Error(GetClassName()+"::GiveWindowsPtrs: window has not one protocol to connect! window name: "
							   + windows[wi].name + " protocol name to connect: " + protocols[pi]->name);
				}
			if(!findProtForWindow)
				Error(GetClassName()+"::GiveWindowsPtrs: window has no protocol connect! window name: "
					  + windows[wi].name + " protocol name to connect: " + windows[wi].connectStr);
		}
	}
}

Window *Programm::FindWindow(QString objectName, QString windowName, bool printError)
{
	for(auto &window:windows)
	{
		if(window.connectedObjects.size())
		{
			if(window.name == windowName && window.connectedObjects[0]->name == objectName)
				return &window;
		}
		else
		{
			if(printError) Error("FindWindow find nullptr parentObject in window " + window.name);
		}
	}
	if(printError) Error("FindWindow: can't find window " + windowName + " whith object " + objectName);
	return nullptr;
}

IExternalWindow * Programm::FindExternalWindow(QString name)
{
	for(auto &window:externalWindows)
	{
		if(window->Name() == name)
			return window.get();
	}
	return nullptr;
}

int Programm::ConnectExternalWidget(const ConnectData &connectData)
{
	if(!CheckConnectData(connectData)) return IConnectExternalWidgets::badConnectData;

	Object *object = dynamic_cast<Object*>(connectData.objectPtr);
	Protocol *protocol = dynamic_cast<Protocol*>(connectData.objectPtr);
	if((object && protocol) || (!object && !protocol)) return IConnectExternalWidgets::badObjectPtr;

	Param *param = nullptr;
	if(object)
	{
		auto params = object->FindParamsInCategory(connectData.elementInObjectCat,connectData.elementInObjectName,false);
		if(params.size() == 1) param = params[0];
	}
	Worker *frameWorker = nullptr;
	if(protocol)
	{
		auto fWorkers = protocol->FindWorkersInCategory(connectData.elementInObjectCat,connectData.elementInObjectName,false);
		if(fWorkers.size() == 1) frameWorker = fWorkers[0];
	}
	if((param && frameWorker) || (!param && !frameWorker)) return IConnectExternalWidgets::cantFindElement;

	if(connectData.widgetCat == ConnectData::ctrlWidget)
	{
		if(!protocol) return IConnectExternalWidgets::nullptrProtocol;
		QStringList obrabotchikCommands = Code::TextToCommands(connectData.obrabotchik);
		if(obrabotchikCommands.empty()) return IConnectExternalWidgets::emptyCtrlWidgetObrabotchik;
		if(QPushButton* btn {dynamic_cast<QPushButton*>(connectData.widgetPtr)})
		{
			QObject::connect(btn,&QPushButton::clicked,[protocol, param, frameWorker, obrabotchikCommands]()
			{
				CANMsg_t msgPustishka;
				for(auto &obrabotchikCommand:obrabotchikCommands)
					protocol->DoEverything(obrabotchikCommand, param, frameWorker, msgPustishka, nullptr, Protocol::DEretNullptr());
			});
		}
		else Error(GetClassName() + "::ConnectExternalWidget unrealesed ctrlWidget className ["
				   +connectData.widgetPtr->metaObject()->className()+"]");
		return noError;
	}
	else if(connectData.widgetCat == ConnectData::viewWidget)
	{
		if(param)
		{
			CBParamChangedForExternal cbForParam;
			cbForParam.cbWidgetHandler = connectData.widgetPtr;
			cbForParam.cbFunction = [](const Value &newValue, QWidget *qWidgetHandler)
			{
				if(QLabel *resolvedHandler {dynamic_cast<QLabel*>(qWidgetHandler)})
				{
					resolvedHandler->setText(newValue.ToStr());
				}
				else Logs::ErrorSt((QString)"Programm::ConnectExternalWidget: cbForParam can't convert handler("
								   +qWidgetHandler->metaObject()->className()+") to concret component");
			};

			param->AddCBParamChangedForExternal(cbForParam);
			return noError;
		}
		else return IConnectExternalWidgets::cantFindElement;
	}
	else return unknownError;
}

bool Programm::CheckConnectData(const ConnectData &connectData)
{
	if(!connectData.objectPtr) return false;
	if(!connectData.widgetPtr) return false;
	if(!connectData.elementInObjectCat.size()) return false;
	if(!connectData.elementInObjectName.size()) return false;
	return true;
}

void Programm::ConnectMake()
{
	for(auto &window:windows)
	{
		for(auto &widgetPair:window.widgetPairs)
		{
			if(widgetPair.IsDecorative()) continue;

			if(!widgetPair.parentObject || !widgetPair.parentProtocol)
			{
				widgetPair.Error(GetClassName() + "::DoObrabotchiki: widget.parentObject or widget.parentProtocol is nullptr. Widget "
							 + widgetPair.ToStrForLog());
				continue;
			}

			Protocol *protocol = widgetPair.parentProtocol;
			Worker *frWorker = widgetPair.parentWorker;
			Param *param = widgetPair.parentParam;
			int protType = protocol->type;
			for(auto &wpt:widgetPair.ctrl.parts)
			{
				if(!wpt.obrabotchiki.empty())
				{
					auto doObrabotchiki = [protocol, &widgetPair, &wpt]()
					{
						vector<Value> vectWidgVals = widgetPair.GetCtrlWidgetValues();
						if(wpt.obrabotchiki.join(" ").contains("новыеСостояния = гк"))
							qdbg << "stop here";
						for(auto &command:wpt.obrabotchiki)
						{
							CANMsg_t msgPustishka;
							protocol->DoEverything(command, widgetPair.parentParam, widgetPair.parentWorker,
												   msgPustishka, &vectWidgVals, Protocol::DEretNullptr());
						}
					};

					if(wpt.GetType() == WidgetPartTypes::button)
					{
						if(auto btn = dynamic_cast<QPushButton *>(wpt.GetPtQWidget()))
							QObject::connect(btn, &QPushButton::clicked, doObrabotchiki);
						else Error(GetClassName() + "::ConnectMake nullptr dynamic_cast to QPushButton; wpt ["+wpt.ToStrForLog()+"]");
					}
					if(wpt.GetType() == WidgetPartTypes::checkBox)
					{
						if(auto chBox = dynamic_cast<QCheckBox *>(wpt.GetPtQWidget()))
							QObject::connect(chBox, &QCheckBox::clicked, doObrabotchiki);
						else Error(GetClassName() + "::ConnectMake nullptr dynamic_cast to QPushButton; wpt ["+wpt.ToStrForLog()+"]");
					}
				}

				if(wpt.GetType() == WidgetPartTypes::button)
				{
					QPushButton *btn = (QPushButton *)wpt.GetPtQWidget();

					if(frWorker && FrameTypes::allFrames.contains(frWorker->TypeCell()))
					{
						if((protType == CodeKeyWords::servisInt && frWorker->TypeCell() == FrameTypes::inFrame)
								|| (protType == CodeKeyWords::emulatorInt && frWorker->TypeCell() == FrameTypes::outFrame))
						{
							QObject::connect(btn, &QPushButton::clicked, [protocol, frWorker, &widgetPair]()
							{
								vector<Value> vectWidgVals = widgetPair.GetCtrlWidgetValues();
								auto paramsAndFrames { protocol->ConstructFrames(frWorker, &vectWidgVals) };
								protocol->SendFrames(paramsAndFrames);
							});
						}
						else Error(GetClassName() + "::ConnectMake wrong connect. Тип протокола " + CodeKeyWords::TypeToStr(protType)
								   + " но виджет подключен к обработчику типа " + frWorker->TypeCell());
					}

					if(protType == CodeKeyWords::emulatorInt)
					{
						if(param)
						{
							QString currCtrlWidgetOfParamType = widgetPair.ctrl.GetType();

							if(currCtrlWidgetOfParamType == WidgetTypes::buttons)
							{
								QObject::connect(btn, &QPushButton::clicked, [param, btn]()
								{
									param->SetParamValue(Value(btn->text(),ValuesTypes::text));
								});
							}
							else if(currCtrlWidgetOfParamType == WidgetTypes::editButton ||
									currCtrlWidgetOfParamType == WidgetTypes::chekBoxesNButton ||
									currCtrlWidgetOfParamType == WidgetTypes::editorsNButton)
							{
								QObject::connect(btn, &QPushButton::clicked, [param, &widgetPair]()
								{
									param->SetParamValue(widgetPair.ctrl.GetWidgetValue());
								});
							}
						}
					}
				}
			}
		}
	}
}

void Programm::PlaceAndShowWindows(int startX, int startY)
{
	int delta = 30;

	if(monitor.windowPtQWidget)
	{
		monitor.windowPtQWidget->move(startX, startY);
		monitor.windowPtQWidget->show();
		startX+=delta; startY+=delta;
	}

	if(controlPanel.window)
	{
		controlPanel.window->move(startX, startY);
		controlPanel.window->show();
		startX+=delta; startY+=delta;
	}

	for(uint wi=0; wi<windows.size(); wi++)
	{
		windows[wi].windowPtQWidget->setWindowTitle(windows[wi].name);
		windows[wi].windowPtQWidget->resize(windows[wi].defW,windows[wi].defH);
		for(auto t:windows[wi].tables)
		{
			t->setMinimumHeight(40 + t->rowCount()*t->rowHeight(0));
			t->setMinimumWidth(30 + t->columnCount()*t->columnWidth(0));

			if(t->minimumHeight()>600) t->setMinimumHeight(600);
			if(t->minimumWidth()>900) t->setMinimumWidth(900);
		}

		windows[wi].windowPtQWidget->move(startX,startY);
		windows[wi].windowPtQWidget->show();
		startX+=delta; startY+=delta;
	}

	for(uint ewi=0; ewi<externalWindows.size(); ewi++)
	{
		externalWindows[ewi]->Window()->move(startX,startY);
		externalWindows[ewi]->Window()->show();
		startX+=delta; startY+=delta;
	}
}

void Monitor::Create()
{
	windowPtQWidget = new QWidget();
	windowPtQWidget->setWindowTitle("Monitor");
	windowPtQWidget->resize(1500,500);
	vLayout = new QVBoxLayout(windowPtQWidget);
	tBrowser = new QTextBrowser;
	QFont font = tBrowser->font();
	font.setFamily("Courier new");
	font.setPointSize(12);
	tBrowser->setFont(font);
	vLayout->addWidget(tBrowser);
	QPushButton *btnClear = new QPushButton("Очистить");
	vLayout->addWidget(btnClear);
	QStatusBar *statusBar = new QStatusBar;
	QLabel *labeStBar = new QLabel;
	statusBar->addWidget(labeStBar);
	vLayout->addWidget(statusBar);

	QObject::connect(btnClear, &QPushButton::clicked, [this](){ tBrowser->clear(); });
	QTimer *timer = new QTimer(windowPtQWidget);
	QObject::connect(timer, &QTimer::timeout, [labeStBar]()
	{
		labeStBar->setText("Предупреждений: " + QSn(Logs::WarningsCount()) + "\t\tОшибок: " + QSn(Logs::ErrorsCount()));
	});
	timer->start(50);

	Logs::SetTextBrowser(tBrowser);
}

void Monitor::Delete()
{
	Logs::SetTextBrowser(nullptr);
	if(windowPtQWidget)
	{
		delete windowPtQWidget;
		windowPtQWidget = nullptr;
	}
}

void Monitor::SetProgrammName(QString name_)
{
	name = name_;
	if(name != "" && windowPtQWidget)
		windowPtQWidget->setWindowTitle("Monitor " + name);
}

void ControlPanel::SetCheckBoxesFromSettings()
{
	if(chLogDoEvrythingCommands) chLogDoEvrythingCommands->setChecked(Settings::logDoEvrythingCommands);
	if(chWarningNotPassedFilterFrames) chWarningNotPassedFilterFrames->setChecked(Settings::warningNotPassedFilterFrames);
	if(chLogParamChanges) chLogParamChanges->setChecked(Settings::logParamChanges);
	if(chLogFrameSent) chLogFrameSent->setChecked(Settings::logFrameSent);
	if(chLogFrameGet) chLogFrameGet->setChecked(Settings::logFrameGet);
}
