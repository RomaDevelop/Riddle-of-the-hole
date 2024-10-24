#ifndef EDITOR_H
#define EDITOR_H

#include <memory>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTableWidget>
#include <QSpacerItem>
#include <QSplitter>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QDomDocument>
#include <QCloseEvent>

#include "MyQDialogs.h"

#include "config.h"

#include "programm.h"

class TableWidget: public QWidget
{
	Q_OBJECT
public:
	bool changeRowsCountNow = false;

	QTableWidget *ptr = nullptr;
	QTextEdit *textEditEditor = nullptr;
	IConfigItem *configItemOfTable = nullptr;
	std::vector<IConfigItem*> tableRowsItemsPtrs;

	void Create(const QFont &font);
	void Clear();
	void AddRow(IConfigItem &tableRowItem, const QStringList &itemValues);
	IConfigItem* ConfigItemOfCurrentRow();

	void ConfigItemToTable(IConfigItem &configItem, uint selectRow = 0);
	void ConfigItemSubItemsToTableRecursion(IConfigItem & configItem);
	void UpdateConfigItemsFromTable();
};

struct TextEdit
{
	QTextEdit *ptr = nullptr;
	QTableWidgetItem *tableItem = nullptr;
	IConfigItem *configItem = nullptr;
	QString fieldName;

	void Create(const QFont &font);
	static inline int a =0;
	void ConnectToItem(QTableWidgetItem *item, IConfigItem *configItem_, QString fieldName_);
};

class Editor: public QWidget, public LogedClass
{
	Q_OBJECT

	QTreeWidget *tree = nullptr;
	TableWidget table;
	TextEdit textEdit;
	QTextEdit *textEditOutput = nullptr;
	Config config;
	QDomDocument xmlForRun;

	QSplitter *splitterCenter = nullptr;
	QSplitter *splitterRight = nullptr;

	QFont basicFont;

	Programm *programm = nullptr;

	bool hasCorrectConfig = false;

	std::vector<std::unique_ptr<QWidget>> xmlEditors;

public:
	explicit Editor(QWidget *parent = nullptr);
	~Editor();
	void closeEvent (QCloseEvent *event);

	QString GetTreeW() { return QSn(tree->width()); }
	void SaveSettings();
	void LoadSettings();

	void OpenFile(QString file);
	bool Run();
	bool notDelete = false;		// нужно для ServisLoader-а, потому что там Editor уничтожается сразу
								// и нужно чтобы он не удалял Programm и CANInterfces

	private slots: void Open();
	private slots: void AddInTree();
	private slots: void AddInTable();
	private slots: void Save();
	public slots: void SlotRun();
	private slots: void SlotOpenXMLEditor();

	void CellSelection();

private:
	void PrintConfigTree();
	void TreeItemActivated(QTreeWidgetItem *item);
	void FindConfigItemInTable(IConfigItem &configItem);

	IConfigItem* DefineTreeItemPtr(QTreeWidgetItem *item, bool checkExistance = true);

	void ReadConfig(QString fileName);
	ConfigFile* FindConfigFile(const QString &fName, const QString &fPath);
};

#endif // EDITOR_H
