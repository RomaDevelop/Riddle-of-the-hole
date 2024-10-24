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

#include "MyQDialogs.h"

#include "configconstants.h"
#include "code.h"
#include "ioperand.h"
#include "iconfigitem.h"

struct MainFileDefinition: public IConfigItem
{
	QString definition;

	~MainFileDefinition() = default;
	MainFileDefinition() { definition = Editor_ns::FileInMain + " \"\""; }
	MainFileDefinition(QString definition_) { definition = definition_; }
	static QString FileNameFromMainFileRow(QString rowInMainFile);

	virtual QString GetClassName() const override { return classMainFileDefinition(); }
	QString ToStrForLog() const override { return "["+definition+"]"; }

	virtual int FieldsCount() const override;
	virtual QStringList FieldsNames() const override;
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void Set(const QDomElement &element) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual void AddSubItem(const QString &subItemType) override;
	virtual bool HaveSubDefinitionsInTable() const override { return false; }
	virtual std::vector<IConfigItem*> SubDefinitions() override;
	virtual QString ToStrForConfig() const override;
	virtual TreeItemVals ToTreeItemVals() const override;

	virtual QString RowCaption() const override { return Editor_ns::FileInMain; }

	static QString classMainFileDefinition() { return "MainFileDefinition"; }
};

struct ConfigFile: public LogedClass, public IConfigItem
{
	QString fileNameWithPath;
	QString path;
	QString name;
	QString suffix;
	QString strContent;

	std::vector<std::shared_ptr<IConfigItem>> definitions;

	ConfigFile() = delete;
	ConfigFile(QString fileNameWithPath_, QString content_);
	~ConfigFile() = default;

	void UpdateFileFromDefinitions();

	virtual QString GetClassName() const override { return classConfigFile(); }
	QString ToStrForLog() const override { return "["+path+"/"+name+"]"; }

	virtual int FieldsCount() const override;
	virtual QStringList FieldsNames() const override;
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual void Set(const QDomElement &element) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual void AddSubItem(const QString &subItemType) override;
	virtual bool HaveSubDefinitionsInTable() const override { return true; }
	virtual std::vector<IConfigItem*> SubDefinitions() override;
	virtual QString ToStrForConfig() const override;
	virtual TreeItemVals ToTreeItemVals() const override;

	virtual QString RowCaption() const override { return classConfigFile(); }

	static QString classConfigFile() { return "ConfigFile"; }
};

class Config
{
public:
	std::vector<ConfigFile> files;

	~Config() = default;
};

class TableWidget: public QWidget
{
	Q_OBJECT
public:
	bool changeRowsCountNow = false;

	QTableWidget *ptr = nullptr;
	QTextEdit *textEdit = nullptr;
	IConfigItem *configItemOfTable = nullptr;
	std::vector<IConfigItem*> tableRowsItemsPtrs;

	void Create(const QFont &font);
	void Clear();
	void AddRow(IConfigItem &tableRowItem, const QStringList &itemValues);
	IConfigItem* ConfigItemOfRow();

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
	Config config;

	QFont basicFont;

public:
	explicit Editor(QWidget *parent = nullptr);
	~Editor() = default;

private slots:
	void Open();
	void AddInTree();
	void AddInTable();
	void Save();

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
