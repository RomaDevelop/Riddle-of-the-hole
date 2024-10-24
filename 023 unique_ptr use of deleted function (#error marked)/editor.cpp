#include "editor.h"



Editor::Editor(QWidget * parent) : QWidget(parent)
{
	basicFont = QFont("Courier new",12);

	auto loMain = new QVBoxLayout(this);
	auto loTop = new QHBoxLayout();
	auto splCenter = new QSplitter(Qt::Horizontal);
	auto loBootom = new QHBoxLayout;
	loMain->addLayout(loTop);
	loMain->addWidget(splCenter);
	loMain->addLayout(loBootom);

	// loTop
	auto btnNew = new QPushButton("New");
	loTop->addWidget(btnNew);
	auto btnOpen = new QPushButton("Open");
	loTop->addWidget(btnOpen);
	connect(btnOpen,&QPushButton::clicked,this,&Editor::Open);
	auto btnSave = new QPushButton("Save");
	loTop->addWidget(btnSave);
	auto btnAdd = new QPushButton("Add");
	connect(btnAdd,&QPushButton::clicked,this,&Editor::Add);
	loTop->addWidget(btnAdd);
	loTop->addStretch();

	// splCenter
	tree = new QTreeWidget;
	tree->setMinimumWidth(300);
	tree->setHeaderHidden(true);
	tree->setFont(basicFont);
	connect(tree,&QTreeWidget::itemActivated,this,&Editor::TreeItemActivated);

	table = new QTableWidget;
	table->setMinimumWidth(1000);
	table->setFont(basicFont);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setStyleSheet("QHeaderView { border-style: none; border-bottom: 1px solid gray; }");

	splCenter->addWidget(tree);
	splCenter->addWidget(table);

	move(150,440);
}

void Editor::Open()
{
	QString file = "C:/Work/C++/CAN/ProtocolMy/Протокол1.main";
	ReadConfig(file);
	PrintConfigTree();
}

void Editor::PrintConfigTree()
{
	tree->clear();
	tree->setColumnCount(Editor_ns::colsCountInTree);
	for(int i=1; i<Editor_ns::colsCountInTree; i++)
		tree->setColumnHidden(i, true);
	for(auto &file:config.files)
	{
		auto item = new QTreeWidgetItem;
		item->setData(Editor_ns::colCaption, 0, file.name);
		item->setData(Editor_ns::colPtrConfigItem,0,Config::PtrToStr(&file));
		if(file.suffix == Editor_ns::Extentions::main_ext)
			item->setData(Editor_ns::colType, 0, Editor_ns::fileMain);
		else if(file.suffix == Editor_ns::Extentions::xml)
			item->setData(Editor_ns::colType, 0, Editor_ns::fileXML);
		else Logs::ErrorSt("Editor::PrintConfigTree: wrong suffix ["+ file.suffix +"]");
		item->setData(Editor_ns::colFileName, 0, file.name);
		item->setData(Editor_ns::colFilePath, 0, file.path);
		tree->addTopLevelItem(item);
	}
}

void Editor::TreeItemActivated(QTreeWidgetItem *item)
{
	auto defineItemPtr = DefineItemPtr(item,true);
	if(defineItemPtr)
	{
		if(ConfigFile *configFile {dynamic_cast<ConfigFile*>(defineItemPtr)})
		{
			PrintConfigFile(*configFile);
		}
		else Logs::ErrorSt("Editor::TreeItemActivated unrealesed class ["+defineItemPtr->GetClassName()+"]");
	}
	else Logs::ErrorSt("Editor::TreeItemActivated defineItemPtr is nullptr");
}

void Editor::PrintConfigFile(ConfigFile & configFile)
{
	if(configFile.suffix == Editor_ns::Extentions::main_ext)
	{
		table->clear();
		table->setColumnCount(1);
		table->setColumnWidth(0, table->width() - 55);
		for(uint i=0; i<configFile.definitions.size(); i++)
		{
			if(MainFileDefinition *defPtr {dynamic_cast<MainFileDefinition*>(configFile.definitions[i].get())})
			{
				table->setRowCount(i+1);
				table->setItem(i,0,new QTableWidgetItem(defPtr->definition));
			}
			else Logs::ErrorSt("Editor::PrintConfigFile wrong definition type ["+configFile.definitions[i]->GetClassName()+"]");
		}
	}
	else if(configFile.suffix == Editor_ns::Extentions::xml)
	{
		PrintXMLFile(configFile);
	}
	else Logs::ErrorSt("PresentConfigFile: wrong extention of file " + configFile.ToStrForLog());
}

void Editor::PrintXMLFile(ConfigFile & configFile)
{
	table->clear();
	table->setColumnCount(Editor_ns::colsCountInXMLFile);
	for(uint i=0; i<configFile.definitions.size(); i++)
	{
		if(Object *obj {dynamic_cast<Object*>(configFile.definitions[i].get())})
		{
			table->setRowCount(i+1);
			table->setItem(i,0,new QTableWidgetItem(obj->classObj));
		}
		else Error("Editor::PrintXMLFile wrong definition" + configFile.definitions[i]->GetClassName());
	}
}

HaveClassName* Editor::DefineItemPtr(QTreeWidgetItem * item, bool checkExistance)
{
	QString ptrStr = item->data(Editor_ns::colPtrConfigItem, 0).toString();
	HaveClassName* ret = Config::PtrFromStr(ptrStr);
	if(checkExistance)
	{
		try { ret->GetClassName(); }
		catch (...)
		{
			Logs::ErrorSt("Editor::DefineItemPtr: cathed exeption while checkExistance of ptr ["+ptrStr+"]");
			ret = nullptr;
		}
	}
	return ret;
}

void Editor::Add()
{
	auto currentConfigItemPtr = DefineItemPtr(tree->currentItem(),true);
	QString CurrentConfigItemClass = "nullptr";
	if(currentConfigItemPtr) CurrentConfigItemClass = currentConfigItemPtr->GetClassName();
	if(ConfigFile *configFile {dynamic_cast<ConfigFile *>(currentConfigItemPtr)})
	{
		if(configFile->suffix == Editor_ns::Extentions::main_ext)
		{
			auto canBeAdded = Editor_ns::canBeAddedAtFileMain;
			auto toAdd = MyQDialogs::CustomDialog("Add","Chose what to add",canBeAdded);
			if(toAdd == Editor_ns::FileInMain)
			{
				configFile->definitions.push_back(std::make_unique<MainFileDefinition>());
				PrintConfigFile(*configFile);
			}
		}
		else if(configFile->suffix == Editor_ns::Extentions::xml)
		{
			auto canBeAdded = Editor_ns::canBeAddedAtFileXML;
			auto toAdd = MyQDialogs::CustomDialog("Add","Chose what to add",canBeAdded);
			AddInXMLFile(*configFile,toAdd);
		}
		else Logs::ErrorSt("Editor::Add: wrong file suffix ["+configFile->suffix+"] of file " + configFile->ToStrForLog());
	}
	else Logs::ErrorSt("Editor::Add: wrong currentConfigItemPtr ["+CurrentConfigItemClass+"] to add");
}

void Editor::AddInXMLFile(ConfigFile & configFile, QString whatAdding)
{
	if(Editor_ns::canBeAddedAtFileXML.contains(whatAdding))
	{
		if(whatAdding == Editor_ns::ObjectClass)
		{
			configFile.definitions.push_back(std::make_unique<Object>("",""));
			PrintXMLFile(configFile);
		}
		else Error("Editor::AddInXMLFile 2 ["+whatAdding+"]");
	}
	else Error("Editor::AddInXMLFile 1 ["+whatAdding+"]");
}

ConfigFile * Editor::FindConfigFile(const QString & fName, const QString & fPath)
{
	for(uint i=0; i<config.files.size(); i++)
	{
		if(config.files[i].name == fName && config.files[i].path == fPath)
			return &config.files[i];
	}
	Logs::ErrorSt("FindConfigFile can't find file ["+fPath+"/"+fName+"], return nullptr");
	return nullptr;
}

void Editor::ReadConfig(QString fileName)
{
	QFile file(fileName);
	QString mainFileContent;
	if(file.exists())
	{
		if(file.open(QFile::ReadOnly))
		{
			mainFileContent = file.readAll();
			file.close();
		}
		else Logs::ErrorSt(CodeMarkers::mock + "(insert normal func name): can't open file [" + fileName + "]");
	}
	else Logs::ErrorSt(CodeMarkers::mock + "(insert normal func name): file doesn't exists [" + fileName + "]");

	if(mainFileContent == "")
	{
		Logs::ErrorSt(CodeMarkers::mock + "(insert normal func name): empty mainFileContent");
		return;
	}

	config.files.push_back(ConfigFile(fileName,mainFileContent));
	auto commands = Code::TextToCommands(mainFileContent);
	for(auto &command:commands)
	{
		auto words = Code::CommandToWords(command);
		bool check = false;
		QString fileName;
		if(words.size() == 2)
		{
			if(words[0] == Editor_ns::FileInMain)
			{
				if(TextConstant::IsItTextConstant(words[1],false))
				{
					fileName = TextConstant::GetTextConstVal(words[1]);
					if(QFile(fileName).exists())
					{
						if(Editor_ns::Extentions::all.contains(QFileInfo(fileName).suffix()))
						{
							check = true;
						}
						else Logs::ErrorSt("ReadConfig: wrong extention of file in command [" + command + "]");
					}
					else Logs::ErrorSt(CodeMarkers::mock
									   + "(insert normal func name): wrong word[1] (file doesn't exists) in command [" + command + "]");
				}
				else Logs::ErrorSt(CodeMarkers::mock + "(insert normal func name): wrong word[1] (not text constant) in command ["
								   + command + "]");
			}
			else Logs::ErrorSt(CodeMarkers::mock + "(insert normal func name): wrong word[0] in command [" + command + "]");
		}
		else Logs::ErrorSt(CodeMarkers::mock + "(insert normal func name): wrong words count in command [" + command + "]");

		if(check)
		{
			file.setFileName(fileName);
			if(file.open(QFile::ReadOnly))
			{
				config.files.push_back(ConfigFile(fileName,file.readAll()));
				file.close();
			}
			else Logs::ErrorSt(CodeMarkers::mock + "(insert normal func name): can't open file [" + fileName + "]");
		}
	}
}

ConfigFile::ConfigFile(QString fileNameWithPath, QString content_)
{
	QFileInfo fi(fileNameWithPath);
	path = fi.path();
	name = fi.fileName();
	suffix = fi.suffix();
	content = content_;

	if(suffix == Editor_ns::Extentions::main_ext)
	{
		auto commands = Code::TextToCommands(content);
		for(int i=0; i<commands.size(); i++)
		{
			definitions.push_back(std::make_unique<MainFileDefinition>(commands[i]));
		}
	}
	else if(suffix == Editor_ns::Extentions::xml)
	{
		Error(CodeMarkers::mock + " ConfigFile::ConfigFile(QString fileNameWithPath, QString content_) if(suffix == Editor_ns::Extentions::xml)");
	}
	else Error("ConfigFile::ConfigFile(QString fileNameWithPath, QString content_): parsing content: wrong suffix " + suffix);
}
