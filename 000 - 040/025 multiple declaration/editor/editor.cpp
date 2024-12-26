#include "editor.h"

#include "protocolconfig.h"
#include "objectcongif.h"
#include "constantsconfig.h"
#include "windowclass.h"

#include "configitemfabric.h"

Editor::Editor(QWidget * parent) : QWidget(parent)
{
	basicFont = QFont("Courier new",12);

	auto layOutMain = new QVBoxLayout(this);
	auto layOutTop = new QHBoxLayout();
	auto splitterCenter = new QSplitter(Qt::Horizontal);
	layOutMain->addLayout(layOutTop);
	layOutMain->addWidget(splitterCenter);

	// top
	auto btnNew = new QPushButton("New");
	layOutTop->addWidget(btnNew);
	auto btnOpen = new QPushButton("Open");
	layOutTop->addWidget(btnOpen);
	connect(btnOpen,&QPushButton::clicked,this,&Editor::Open);
	auto btnSave = new QPushButton("Save");
	connect(btnSave,&QPushButton::clicked,this,&Editor::Save);
	layOutTop->addWidget(btnSave);
	auto btnAddinTree = new QPushButton("AddInTree");
	connect(btnAddinTree,&QPushButton::clicked,this,&Editor::AddInTree);
	layOutTop->addWidget(btnAddinTree);
	auto btnAddInTable = new QPushButton("AddInTable");
	connect(btnAddInTable,&QPushButton::clicked,this,&Editor::AddInTable);
	layOutTop->addWidget(btnAddInTable);
	layOutTop->addStretch();

	// center left
	tree = new QTreeWidget;
	tree->setMinimumWidth(320);
	tree->setMaximumWidth(320);
	tree->setHeaderHidden(true);
	tree->setFont(basicFont);
	connect(tree, &QTreeWidget::itemActivated, this, &Editor::TreeItemActivated);
	splitterCenter->addWidget(tree);

	// center right
	QWidget *widgetCenterRight = new QWidget;
	auto layCenterRight = new QVBoxLayout(widgetCenterRight);
	layCenterRight->setMargin(0);
	auto splitterCenterRight = new QSplitter(Qt::Vertical);
	table.Create(basicFont);
	connect(table.ptr, &QTableWidget::itemSelectionChanged, this, &Editor::CellSelection);
	splitterCenterRight->addWidget(table.ptr);
	textEdit.Create(basicFont);
	splitterCenterRight->addWidget(textEdit.ptr);
	layCenterRight->addWidget(splitterCenterRight);
	splitterCenter->addWidget(widgetCenterRight);

	setGeometry(50,100,1800,600);
}

void Editor::Open()
{
	QString file = "C:/Work/C++/CAN/ProtocolMy/Проект1.main";
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
		TreeItemVals treeItemVals = file.ToTreeItemVals();
		treeItemVals.ToTree(tree);
	}
}

void Editor::TreeItemActivated(QTreeWidgetItem *item)
{
	auto defineItemPtr = DefineTreeItemPtr(item,true);
	if(defineItemPtr)
	{
		if(dynamic_cast<ConfigFile*>(defineItemPtr))
			table.ConfigItemToTable(*defineItemPtr);
		else FindConfigItemInTable(*defineItemPtr);
	}
	else Error("Editor::TreeItemActivated defineItemPtr is nullptr");
}

void Editor::FindConfigItemInTable(IConfigItem & configItem)
{
	Error(CodeMarkers::mock + " FindConfigItemInTable(IConfigItem &configItem) " + configItem.GetClassName());
}

IConfigItem* Editor::DefineTreeItemPtr(QTreeWidgetItem * item, bool checkExistance)
{
	QString ptrStr = item->data(Editor_ns::colPtrConfigItem, 0).toString();
	IConfigItem* ret = dynamic_cast<IConfigItem*>(IConfigItem::PtrFromStr(ptrStr));
	if(checkExistance)
	{
		try { ret->GetClassName(); }
		catch (...)
		{
			Error("Editor::DefineItemPtr: cathed exeption while checkExistance of ptr ["+ptrStr+"]");
			ret = nullptr;
		}
	}
	return ret;
}

void Editor::AddInTree()
{
	auto currentConfigItemPtr = DefineTreeItemPtr(tree->currentItem(),true);
	if(currentConfigItemPtr)
	{
		auto canBeAddedPtr = currentConfigItemPtr->WhatCanBeAdded();
		if(canBeAddedPtr)
		{
			QStringList canBeAdded = *canBeAddedPtr;
			QString cansel = "Nothing";
			canBeAdded += cansel;
			auto toAdd = MyQDialogs::CustomDialog("AddInTree","Chose what to add", canBeAdded);
			if(toAdd != cansel)
			{
				currentConfigItemPtr->AddSubItem(toAdd);
				int newRow = table.ptr->rowCount();
				table.ConfigItemToTable(*currentConfigItemPtr,newRow);
			}
		}
		else Error("Editor::AddInTree can't define what can add in " + currentConfigItemPtr->GetClassName());
	}
	else Error("Editor::AddInTree DefineTreeItemPtr(tree->currentItem(),true) returned nullptr");
}

void Editor::AddInTable()
{
	auto currentConfigItemPtr = table.ConfigItemOfRow();
	if(currentConfigItemPtr)
	{
		auto canBeAddedPtr = currentConfigItemPtr->WhatCanBeAdded();
		if(canBeAddedPtr)
		{
			QStringList canBeAdded = *canBeAddedPtr;
			QString cansel = "Nothing";
			canBeAdded += cansel;
			auto toAdd = MyQDialogs::CustomDialog("AddInTable","Chose what to add", canBeAdded);
			if(toAdd != cansel)
			{
				currentConfigItemPtr->AddSubItem(toAdd);
				if(table.configItemOfTable)
				{
					int newRow = table.ptr->rowCount();
					table.ConfigItemToTable(*table.configItemOfTable,newRow);
				}
			}
		}
		else Error("Editor::AddInTable can't define what can add in " + currentConfigItemPtr->GetClassName());
	}
	else Error("Editor::AddInTable DefineTreeItemPtr(tree->currentItem(),true) returned nullptr");
}

void Editor::Save()
{
	table.UpdateConfigItemsFromTable();
	for(auto &cFile:config.files)
		cFile.UpdateFileFromDefinitions();
}

void Editor::CellSelection()
{
	if(!table.changeRowsCountNow)
	{
		IConfigItem *configItemOfRow = table.ConfigItemOfRow();
		if(configItemOfRow)
		{
			auto colNames = configItemOfRow->FieldsNames();
			while(colNames.size() < table.ptr->columnCount())
				colNames += "";
			table.ptr->setHorizontalHeaderLabels(colNames);

			QStringList fieldsNames = configItemOfRow->FieldsNames();
			if(table.ptr->currentColumn() < fieldsNames.size())
				textEdit.ConnectToItem(table.ptr->currentItem(), configItemOfRow, fieldsNames[table.ptr->currentColumn()]);
			else textEdit.ConnectToItem(nullptr, nullptr, "");
		}
	}
	// else { если сигнал был послан во время changeRowCountNow, то ничего делать не надо }
}

ConfigFile * Editor::FindConfigFile(const QString & fName, const QString & fPath)
{
	for(uint i=0; i<config.files.size(); i++)
	{
		if(config.files[i].name == fName && config.files[i].path == fPath)
			return &config.files[i];
	}
	Error("FindConfigFile can't find file ["+fPath+"/"+fName+"], return nullptr");
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
		else Error("Editor::ReadConfig: can't open file [" + fileName + "]");
	}
	else Error("Editor::ReadConfig: file doesn't exists [" + fileName + "]");

	if(mainFileContent == "")
	{
		Error("Editor::ReadConfig: empty mainFileContent");
		return;
	}

	config.files.clear();
	config.files.push_back(ConfigFile(fileName,mainFileContent));
	auto commands = Code::TextToCommands(mainFileContent);
	for(auto &command:commands)
	{
		QString fileName = MainFileDefinition::FileNameFromMainFileRow(command);
		file.setFileName(fileName);
		if(file.open(QFile::ReadOnly))
		{
			config.files.push_back(ConfigFile(fileName,file.readAll()));
			file.close();
		}
		else Error("Editor::ReadConfig: can't open file [" + fileName + "]");
	}
}

ConfigFile::ConfigFile(QString fileNameWithPath_, QString content_)
{
	QFileInfo fi(fileNameWithPath_);
	fileNameWithPath = fileNameWithPath_;
	path = fi.path();
	name = fi.fileName();
	suffix = fi.suffix();
	strContent = content_;

	if(suffix == Editor_ns::Extentions::main_ext)
	{
		auto commands = Code::TextToCommands(strContent);
		for(int i=0; i<commands.size(); i++)
		{
			definitions.push_back(std::make_shared<MainFileDefinition>(commands[i]));
		}
	}
	else if(suffix == Editor_ns::Extentions::xml)
	{
		QDomDocument xml;
		QString errMsg;
		int errLine, errCol;
		if(xml.setContent(strContent, &errMsg, &errLine, &errCol))
		{
			auto elements = DomAdd::GetTopLevelElements(xml);
			if(elements.size() == 1 && elements[0].tagName() == Editor_ns::xmlTagBody)
			{
				elements = DomAdd::GetTopLevelElements(elements[0]);
				for(auto &element:elements)
				{
					auto newDef = ConfigItemFabric::Make(element.tagName());
					if(newDef)
					{
						newDef->Set(element);
						definitions.push_back(newDef);
					}
					else Error(GetClassName() + "::Constructor ConfigItemFabric::Make from tag ["+element.tagName()+"] result is nullptr");
				}
			}
			else Error(GetClassName() + "::Constructor wrong size or top level tag (must me 1 and "+Editor_ns::xmlTagBody+")");
		}
		else
		{
			Error(GetClassName() + "::Constructor error parsing file " + ToStrForLog());
			Error(GetClassName() + "::Constructor errMsg: " + errMsg);
			Error(GetClassName() + "::Constructor errLine: " + QSn(errLine));
			Error(GetClassName() + "::Constructor errCol: " + QSn(errCol));
		}
	}
	else Error(GetClassName() + "::ConfigFile(QString fileNameWithPath, QString content_): parsing content: wrong suffix " + suffix);
}

void ConfigFile::UpdateFileFromDefinitions()
{
	strContent = ToStrForConfig();
	QFile file(fileNameWithPath);
	if(file.open(QFile::WriteOnly))
	{
		file.write(strContent.toUtf8());
		file.close();
	}
	else Error(GetClassName() + "::SaveFile can't open file ["+fileNameWithPath+"]");
}

int ConfigFile::FieldsCount() const
{
	if(suffix == Editor_ns::Extentions::main_ext)
		return Editor_ns::colsCountInMainFile;
	else if(suffix == Editor_ns::Extentions::xml)
		return Editor_ns::colsCountInXMLFile;
	else
	{
		Error(GetClassName() + "::FieldsCount wrong suffix at file ["+ToStrForLog()+"]");
		return 0;
	}
}

QStringList ConfigFile::FieldsNames() const
{
	Warning(GetClassName() + "::FieldsNames() at file ["+ToStrForLog()+"]");
	return Editor_ns::headerXML;
}

QStringList ConfigFile::FieldsValues() const
{
	Warning(GetClassName() + "::FieldsValues() at file ["+ToStrForLog()+"]");
	return { GetClassName() + "::FieldsValues()" };
}

void ConfigFile::SetFieldValues(const QStringList & values)
{
	Warning(GetClassName() + "::SetFieldValues() at file ["+ToStrForLog()+"] [" + values.join(";") + "]");
}

void ConfigFile::SetField(QString fieldName, QString newValue)
{
	Logs::ErrorSt(GetClassName() + "::SetField forbidden " + fieldName + " " + newValue);
}

void ConfigFile::Set(const QDomElement & element)
{
	Logs::ErrorSt(GetClassName() + "::Set(const QDomElement & element) forbidden " + element.tagName());
}

const QStringList * ConfigFile::WhatCanBeAdded() const
{
	if(suffix == Editor_ns::Extentions::main_ext)
		return &Editor_ns::canBeAddedAtFileMain;
	if(suffix == Editor_ns::Extentions::xml)
		return &Editor_ns::canBeAddedAtFileXML;
	else
	{
		Error(GetClassName() + "::WhatCanBeAdded wrong configFile type ["+suffix+"]");
		return nullptr;
	}
}

void ConfigFile::AddSubItem(const QString & subItemType)
{
	if(suffix == Editor_ns::Extentions::main_ext)
	{
		if(subItemType == Editor_ns::FileInMain)
		{
			definitions.push_back(std::make_shared<MainFileDefinition>());
		}
		else Error(GetClassName() + "::AddSubItem: wrong toAdd ["+subItemType+"] in file " + ToStrForLog());
	}
	else if(suffix == Editor_ns::Extentions::xml)
	{
		auto newDef = ConfigItemFabric::Make(subItemType);
		if(newDef)
		{
			definitions.push_back(newDef);
		}
		else Error(GetClassName() + "::Constructor ConfigItemFabric::Make subItemType ["+subItemType+"] result is nullptr");
	}
	else Error(GetClassName() + "::AddSubItem wrong file suffix ["+suffix+"] of file " + ToStrForLog());
}

std::vector<IConfigItem *> ConfigFile::SubDefinitions()
{
	std::vector<IConfigItem*> ret;
	for(auto &def:definitions)
		ret.push_back(def.get());
	return ret;
}

QString ConfigFile::ToStrForConfig() const
{
	QString ret;
	for(auto &def:definitions)
		ret += def->ToStrForConfig() + "\n";

	if(suffix == Editor_ns::Extentions::xml)
		ret = "<"+Editor_ns::xmlTagBody+">\n\n" + ret + "</"+Editor_ns::xmlTagBody+">";

	return ret;
}

TreeItemVals ConfigFile::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = GetClassName() + " " + name;
	tree.ptr = PtrToStr(this);
	if(suffix == Editor_ns::Extentions::xml)
	{
		for(auto &def:definitions)
		{
			tree.subVals.push_back(def->ToTreeItemVals());
		}
	}
	return tree;
}

void TableWidget::Create(const QFont & font)
{
	changeRowsCountNow = true;
	if(ptr) Logs::ErrorSt("TableWidget::Create repeated creation of TableWidget");
	ptr = new QTableWidget;
	ptr->setMinimumWidth(1000);
	ptr->setFont(font);
	QString hStyle = "QHeaderView::section:horizontal{"
			"border-style: none;"
			"border-right: 1px solid grey;"
			"}"
			"QHeaderView{"
			"border-style: none;"
			"border-bottom: 1px solid grey;"
			"}";
	QString vStyle =
			"QHeaderView::section:vertical{	 "
			"border-style: none;			 "
			"border-bottom: 1px solid grey;	 "
			"}								 "
			"QHeaderView{					 "
			"border-style: none;			 "
			"border-right: 1px solid grey;	 "
			"}								 ";

	ptr->verticalHeader()->setStyleSheet(vStyle);
	ptr->verticalHeader()->setFixedWidth(100);
	ptr->horizontalHeader()->setStyleSheet(hStyle);
	changeRowsCountNow = false;
}

void TableWidget::Clear()
{
	changeRowsCountNow = true;
	ptr->clear();
	ptr->setRowCount(0);
	ptr->setColumnCount(0);
	configItemOfTable = nullptr;
	tableRowsItemsPtrs.clear();
	changeRowsCountNow = false;
}

void TableWidget::AddRow(IConfigItem & tableRowItem, const QStringList & itemValues)
{
	changeRowsCountNow = true;
	int newRowIndex = ptr->rowCount();
	ptr->setRowCount(newRowIndex+1);

	tableRowsItemsPtrs.push_back(&tableRowItem);

	ptr->setVerticalHeaderItem(newRowIndex, new QTableWidgetItem(" " + tableRowItem.RowCaption()));

	if(itemValues.size() > ptr->columnCount())
		ptr->setColumnCount(itemValues.size());
	int colIndex=0;
	for(auto &caption:itemValues)
	{
		ptr->setItem(newRowIndex,colIndex, new QTableWidgetItem(caption));
		colIndex++;
	}
	changeRowsCountNow = false;
}

IConfigItem * TableWidget::ConfigItemOfRow()
{
	if((uint)ptr->rowCount() == tableRowsItemsPtrs.size())
		return tableRowsItemsPtrs[ptr->currentRow()];
	else
	{
		Logs::ErrorSt("TableWidget::CurrentConfigItem error ptr->rowCount() != tableRowsItemsPtrs.size() ("
					  +QSn(ptr->rowCount())+" != "+QSn(tableRowsItemsPtrs.size())+")");
		return nullptr;
	}
}

void TableWidget::ConfigItemSubItemsToTableRecursion(IConfigItem & configItem)
{
	if(configItem.HaveSubDefinitionsInTable())
	{
		for(auto &def:configItem.SubDefinitions())
		{
			AddRow(*def,def->FieldsValues());
			ConfigItemSubItemsToTableRecursion(*def);
		}
	}
}

void TableWidget::ConfigItemToTable(IConfigItem & configItem, uint selectRow)
{
	Clear();
	configItemOfTable = &configItem;
	ConfigItemSubItemsToTableRecursion(configItem);
	if(ptr->rowCount()) ptr->selectRow(selectRow);

	int colCount = ptr->columnCount();
	int newColW = (ptr->width() - ptr->verticalHeader()->width() - 50) / colCount;
	for(int c=0; c<colCount; c++)
		ptr->setColumnWidth(c,newColW);
}

void TableWidget::UpdateConfigItemsFromTable()
{
	if((uint)ptr->rowCount() == tableRowsItemsPtrs.size())
	{
		for(uint r=0; r<tableRowsItemsPtrs.size(); r++)
		{
			QStringList rowVal;
			int fCount = tableRowsItemsPtrs[r]->FieldsCount();
			for(int c=0; c<fCount; c++)
			{
				auto item = ptr->item(r,c);
				if(item) rowVal += item->text();
				else
				{
					Logs::ErrorSt("TableWidget::UpdateItemsFromTable ptr->item("+QSn(r)+","+QSn(c)+") returned nullptr");
				}
			}
			tableRowsItemsPtrs[r]->SetFieldValues(rowVal);
		}
	}
	else
	{
		Logs::ErrorSt("TableWidget::UpdateItemsFromTable error ptr->rowCount() != tableRowsItemsPtrs.size() ("
					  +QSn(ptr->rowCount())+" != "+QSn(tableRowsItemsPtrs.size())+")");
	}
}



QString MainFileDefinition::FileNameFromMainFileRow(QString rowInMainFile)
{
	QString fileName;
	auto words = Code::CommandToWords(rowInMainFile);
	if(words.size() == 2)
	{
		if(words[0] == Editor_ns::FileInMain)
		{
			if(TextConstant::IsItTextConstant(words[1],false))
			{
				fileName = TextConstant::GetTextConstVal(words[1]);
				if(QFile(fileName).exists())
				{
					if(!Editor_ns::Extentions::all.contains(QFileInfo(fileName).suffix()))
						Logs::ErrorSt("ReadConfig: wrong extention of file in command [" + rowInMainFile + "]");
					// else not need, all correct
				}
				else Logs::ErrorSt("Editor::ReadConfig: wrong word[1] (file doesn't exists) in command [" + rowInMainFile + "]");
			}
			else Logs::ErrorSt("Editor::ReadConfig: wrong word[1] (not text constant) in command ["
							   + rowInMainFile + "]");
		}
		else Logs::ErrorSt("Editor::ReadConfig: wrong word[0] in command [" + rowInMainFile + "]");
	}
	else Logs::ErrorSt("Editor::ReadConfig: wrong words count in command [" + rowInMainFile + "]");
	return fileName;
}

int MainFileDefinition::FieldsCount() const
{
	return Editor_ns::colsCountInMainFile;
}

QStringList MainFileDefinition::FieldsNames() const
{
	return Editor_ns::headerMain;
}

QStringList MainFileDefinition::FieldsValues() const
{
	return {definition};
}

void MainFileDefinition::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
		definition = values[Editor_ns::mainFileFileIndex];
	else Logs::ErrorSt("MainFileDefinition::SetFieldValues values.size() < FieldsCount()");
}

void MainFileDefinition::Set(const QDomElement & element)
{
	Logs::ErrorSt("MainFileDefinition::Set(const QDomElement &element) forbidden " + element.tagName());
}

void MainFileDefinition::SetField(QString fieldName, QString newValue)
{
	Logs::ErrorSt("MainFileDefinition::SetField forbidden " + fieldName + " " + newValue);
}

const QStringList * MainFileDefinition::WhatCanBeAdded() const
{
	Logs::ErrorSt("MainFileDefinition::WhatCanBeAdded forbidden");
	return nullptr;
}

void MainFileDefinition::AddSubItem(const QString & subItemType)
{
	Logs::ErrorSt("MainFileDefinition::AddSubItem forbidden " + subItemType);
}

std::vector<IConfigItem *> MainFileDefinition::SubDefinitions()
{
	Logs::ErrorSt("MainFileDefinition::SubDefinitions forbidden");
	return {};
}

QString MainFileDefinition::ToStrForConfig() const
{
	return definition + CommandsKeyWords::commandSplitter;
}

TreeItemVals MainFileDefinition::ToTreeItemVals() const
{
	Logs::ErrorSt("MainFileDefinition::ToTreeItemVals forbidden");
	return {};
}



void TextEdit::Create(const QFont & font)
{
	if(ptr) Logs::ErrorSt("TextEdit::Create repeated creation of TextEdit");
	ptr = new QTextEdit;
	ptr->setFont(font);
}

void TextEdit::ConnectToItem(QTableWidgetItem * item, IConfigItem * configItem_, QString fieldName_)
{
	tableItem = item;
	configItem = configItem_;
	fieldName = fieldName_;

	QObject::disconnect(ptr, &QTextEdit::textChanged, nullptr, nullptr);
	if(tableItem && configItem && configItem->FieldsNames().contains(fieldName_))
	{
		ptr->setEnabled(true);
		ptr->setText(tableItem->text());
		QObject::connect(ptr, &QTextEdit::textChanged,[this]()
		{
			QString newText = ptr->toPlainText();
			tableItem->setText(newText);
			configItem->SetField(fieldName, newText);
		});
	}
	else
	{
		ptr->setEnabled(false);
		ptr->setText("");
	}
}
