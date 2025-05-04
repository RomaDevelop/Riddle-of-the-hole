#include "editor.h"

#include <thread>
#include <chrono>
using namespace std;

#include <QFileDialog>
#include <QSettings>
#include <QHostInfo>
#include <QScrollBar>

#include "MyQDom.h"
#include "MyQDifferent.h"
#include "MyQFileDir.h"

#include "ConfigConversions.h"
#include "xmleditor.h"
#include "tests.h"

Editor::Editor(QWidget * parent) :
	QWidget(parent)
{
	basicFont = QFont("Courier new",12);

	auto layOutMain = new QVBoxLayout(this);
	auto layOutTop = new QHBoxLayout();
	splitterCenter = new QSplitter(Qt::Horizontal, this);
	layOutMain->addLayout(layOutTop);
	layOutMain->addWidget(splitterCenter);

	// top
	QPushButton *btnNew = new QPushButton("New", this);
	layOutTop->addWidget(btnNew);
	QPushButton *btnOpen = new QPushButton("Open", this);
	layOutTop->addWidget(btnOpen);
	connect(btnOpen,&QPushButton::clicked,this,&Editor::Open);

	if(QHostInfo::localHostName() == "TKO3-130" && MyQDifferent::PathToExe().contains("C:/Work/C++/CAN"))
	{
		QPushButton *btnOpenPr1 = new QPushButton("Open БКЗ Внешний", this);
		layOutTop->addWidget(btnOpenPr1);
		connect(btnOpenPr1,&QPushButton::clicked,[this](){OpenFile("C:\\Work\\C++\\CAN\\ServisCreator\\БКЗ-27 Внешний.main");});

		QPushButton *btnOpenBKZ = new QPushButton("Open БКЗ Внутренний", this);
		layOutTop->addWidget(btnOpenBKZ);
		connect(btnOpenBKZ,&QPushButton::clicked,[this](){OpenFile("C:\\Work\\C++\\CAN\\ServisCreator\\БКЗ-27 Внутренний.main");});

		QPushButton *btnOpenAZT = new QPushButton("Open тест АЗТ", this);
		layOutTop->addWidget(btnOpenAZT);
		connect(btnOpenAZT,&QPushButton::clicked,[this](){OpenFile("C:\\Work\\C++\\CAN\\ServisCreator\\БКЗ-27 Тест АЗТ.main");});

		// button Test
		QPushButton *test = new QPushButton("Test", this);
		layOutTop->addWidget(test);
		connect(test,&QPushButton::clicked,[this](){
			auto clone = config.CloneConfig();
			clone.ResolveImportInConfig(config);

			for(auto &file:clone.files)
				MyQDialogs::ShowText(file.ExportToXMLTag(true).ToXMLCode(2));
		});

		QPushButton *btnShowRunnedXml = new QPushButton("ShowRunnedXml", this);
		layOutTop->addWidget(btnShowRunnedXml);
		connect(btnShowRunnedXml,&QPushButton::clicked, [this](){
			MyQDialogs::ShowText(xmlForRun.toString(2),1700,1000);
		});
	}
	else qdbg << "fast buttons hided. data:" << QHostInfo::localHostName() << MyQDifferent::PathToExe();

	QPushButton *btnSave = new QPushButton("Save", this);
	connect(btnSave,&QPushButton::clicked,this,&Editor::Save);
	layOutTop->addWidget(btnSave);
	QPushButton *btnAddinTree = new QPushButton("AddInTree", this);
	connect(btnAddinTree,&QPushButton::clicked,this,&Editor::AddInTree);
	layOutTop->addWidget(btnAddinTree);
	QPushButton *btnAddInTable = new QPushButton("AddInTable", this);
	connect(btnAddInTable,&QPushButton::clicked,this,&Editor::AddInTable);
	layOutTop->addWidget(btnAddInTable);

	QPushButton *btnOpenXMLEditor = new QPushButton("OpenXMLEditor", this);
	connect(btnOpenXMLEditor, &QPushButton::clicked, this, &Editor::SlotOpenXMLEditor);
	layOutTop->addWidget(btnOpenXMLEditor);

	QPushButton *btnOpenXMLEditorCols = new QPushButton("OpenXMLEditorCols", this);
	connect(btnOpenXMLEditorCols, &QPushButton::clicked, this, &Editor::SlotOpenXMLEditor);
	layOutTop->addWidget(btnOpenXMLEditorCols);

	QPushButton *btnRun = new QPushButton("Run", this);
	connect(btnRun,&QPushButton::clicked,this, &Editor::SlotRun);
	layOutTop->addWidget(btnRun);

	layOutTop->addStretch();

	// center left
	tree = new QTreeWidget(this);
	tree->setHeaderHidden(true);
	tree->setFont(basicFont);
	connect(tree, &QTreeWidget::itemActivated, this, &Editor::TreeItemActivated);
	splitterCenter->addWidget(tree);

	// center right
	QWidget *widgetCenterRight = new QWidget(this);
	splitterCenter->addWidget(widgetCenterRight);
	auto layCenterRight = new QVBoxLayout(widgetCenterRight);
	layCenterRight->setMargin(0);
	splitterRight = new QSplitter(Qt::Vertical, this);
	layCenterRight->addWidget(splitterRight);
	table.Create(basicFont);
	connect(table.ptr, &QTableWidget::itemSelectionChanged, this, &Editor::CellSelection);
	splitterRight->addWidget(table.ptr);
	textEdit.Create(basicFont);
	splitterRight->addWidget(textEdit.ptr);

	textEditOutput = new QTextEdit(this);
	textEditOutput->setFont(basicFont);
	splitterRight->addWidget(textEditOutput);
	Logs::SetTextBrowser(textEditOutput);

	QTimer::singleShot(0,this,[this]
	{
		move(10,10);
		resize(1870,675);
		LoadSettings();
	});
}

Editor::~Editor()
{
	Log("~Editor");
	Logs::SetTextBrowser(nullptr);

	if(!notDelete)		// нужно для ServisLoader-а, потому что там Editor уничтожается сразу
	{					// и нужно чтобы он не удалял Programm и CANInterfces
		if(programm) delete programm;
	}
}

void Editor::closeEvent(QCloseEvent * event)
{
	SaveSettings();
	event->accept();
}

void Editor::SaveSettings()
{
	QSettings settings(Editor_ns::SettingsFile(), QSettings::IniFormat);
	settings.setValue("geo", MyQDifferent::GetGeo(*this));
	settings.setValue("splitterCenterState", splitterCenter->saveState());
	settings.setValue("splitterRightState", splitterRight->saveState());

}

void Editor::LoadSettings()
{
	QSettings settings(Editor_ns::SettingsFile(), QSettings::IniFormat);
	MyQDifferent::SetGeo(settings.value("geo").toString(), *this);
	splitterCenter->restoreState(settings.value("splitterCenterState").toByteArray());
	splitterRight->restoreState(settings.value("splitterRightState").toByteArray());
}

void Editor::Open()
{
	QString file = QFileDialog::getOpenFileName(0, "Открыть", "", "*.main");
	if(file != "")
		OpenFile(file);
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
	IConfigItem *file = &configItem;
	while(!dynamic_cast<ConfigFile*>(file))
	{
		if(!file->parent)
		{
			file = nullptr;
			break;
		}
		file = file->parent;
	}

	if(file)
	{
		qdbg << file->ToStrForLog();
	}
	else Error("FindConfigItemInTable can't define file of configItem " + configItem.ToStrForLog());
}

IConfigItem* Editor::DefineTreeItemPtr(QTreeWidgetItem * item, bool checkExistance)
{
	if(checkExistance && !item)
	{
		Error("Editor::DefineItemPtr: QTreeWidgetItem * item is nullptr");
		return nullptr;
	}
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
	auto currentConfigItemPtr = table.ConfigItemOfCurrentRow();
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
	int col = table.ptr->currentColumn();
	int row = table.ptr->currentRow();
	auto scrollBarValueV = table.ptr->verticalScrollBar()->value();
	auto scrollBarValueH = table.ptr->horizontalScrollBar()->value();
	table.UpdateConfigItemsFromTable();
	for(auto &cFile:config.files)
		cFile.UpdateAllFromDefinitions();
	if(tree->currentItem())
	{
		TreeItemActivated(tree->currentItem());
		table.ptr->setCurrentCell(row,col);
		table.ptr->verticalScrollBar()->setValue(scrollBarValueV);
		table.ptr->horizontalScrollBar()->setValue(scrollBarValueH);
	}
}

void Editor::CellSelection()
{
	if(!table.changeRowsCountNow)
	{
		IConfigItem *configItemOfRow = table.ConfigItemOfCurrentRow();
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

void Editor::OpenFile(QString file)
{
	ReadConfig(file);
	PrintConfigTree();
}

void Editor::SlotRun()
{
	Run();
}

void Editor::SlotOpenXMLEditor()
{
	QString senderText = static_cast<QPushButton*>(sender())->text();

	auto defineItemPtr = DefineTreeItemPtr(tree->currentItem(),true);
	if(defineItemPtr)
	{
		if(auto configFilePtr = dynamic_cast<ConfigFile*>(defineItemPtr))
		{
			table.ConfigItemToTable(*defineItemPtr);
			for(int i=xmlEditors.size()-1; i>=0; i--)
				if(static_cast<XMLEditor*>(xmlEditors[i].get())->canDelete)
					xmlEditors.erase(xmlEditors.begin()+i);

			xmlEditors.push_back(make_unique<XMLEditor>(*configFilePtr, senderText, nullptr));
			xmlEditors.back()->show();
		}
		else Log("Текущий элемент не является файлом");
	}
	else Error("Editor::TreeItemActivated defineItemPtr is nullptr");
}

namespace forRunInEditor {
	Editor *editorPtr;
}

bool Editor::Run()
{
	if(!hasCorrectConfig)
	{
		Error("Конфигурация некорректна");
		return false;
	}

	if(programm) delete programm;

	xmlForRun = config.GetXMLForRun2();

	auto subElements = MyQDom::GetTopLevelElements(xmlForRun);
	if(subElements.size() == 1 && subElements[0].tagName() == Editor_ns::xmlTagBody)
	{
		forRunInEditor::editorPtr = this;
		auto programmDeleter = []()
		{

			if(forRunInEditor::editorPtr->programm)
				delete forRunInEditor::editorPtr->programm;
			else forRunInEditor::editorPtr->Error("programmDeleter executed, but programm is nullptr");
		};
		auto onDestructorEnds = []()
		{
			forRunInEditor::editorPtr->Log("onDestructorEnds");
			forRunInEditor::editorPtr->programm = nullptr;
			Logs::SetTextBrowser(forRunInEditor::editorPtr->textEditOutput);
		};
		Logs::SetTextBrowser(nullptr);
		programm = new Programm(subElements[0], config.ProjectName(), config.ProjectFile(), programmDeleter, onDestructorEnds);
		return true;
	}
	else Error("Wrong result config.GetXMLForRun(sortedDefs)");
	return false;
}

ConfigFile * Editor::FindConfigFile(const QString & fName, const QString & fPath)
{
	for(auto &file:config.files)
	{
		if(file.name == fName && file.path == fPath)
			return &file;
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
	auto mainFileRows = Code::TextToCommands(mainFileContent);
	bool noErrors = !mainFileRows.empty();
	for(auto &mainFileRow:mainFileRows)
	{
		QString path = QFileInfo(file.fileName()).path();
		QString fileName = path + "/" +FileRowInMainFile::FileNameFromMainFileRow(mainFileRow);
		file.setFileName(fileName);
		if(file.open(QFile::ReadOnly))
		{
			config.files.push_back(ConfigFile(fileName,file.readAll()));
			//config.files.emplace_back(fileName,file.readAll());
			file.close();
		}
		else
		{
			noErrors = false;
			Error("Editor::ReadConfig: can't open file [" + fileName + "]");
		}
	}
	if(noErrors) hasCorrectConfig = true;
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

IConfigItem * TableWidget::ConfigItemOfCurrentRow()
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
	if(configItem.CanHaveSubItemsInTable())
	{
		auto& subItems = configItem.SubItems();
		for(auto &subItem:subItems)
		{
			AddRow(*subItem.get(),subItem->FieldsValues());
			ConfigItemSubItemsToTableRecursion(*subItem.get());
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
