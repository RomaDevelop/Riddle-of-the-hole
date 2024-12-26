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

#include "MyQDialogs.h"

#include "configconstants.h"
#include "code.h"
#include "ioperand.h"
#include "object.h"

struct MainFileDefinition: public HaveClassName
{
	QString definition;

	~MainFileDefinition() = default;
	MainFileDefinition() { definition = Editor_ns::FileInMain + " \"\""; }
	MainFileDefinition(QString definition_) { definition = definition_; }
	virtual QString GetClassName() const override { return classMainFileDefinition(); }
	QString ToStrForLog() const override { return "["+definition+"]"; }

	static QString classMainFileDefinition() { return "MainFileDefinition"; }
};

struct ConfigFile: public LogedClass, public HaveClassName
{
	QString path;
	QString name;
	QString suffix;
	QString content;

	std::vector<std::shared_ptr<HaveClassName>> definitions;

	ConfigFile(QString fileNameWithPath, QString content_);
	~ConfigFile() = default;

#error
	/// 1:	запускаем, Open, открываем xml файл, делаем Add ObjectClass
	///		закрываем, при закрытии вылетаем ошибка
	/// 2:	добавляем ~ConfigFile() = default;
	///		не компилируется, странная ошибка use of deleted function 'std::unique_ptr<_Tp, _Dp>::unique_ptr(const std::unique_ptr<_Tp, _Dp>&) ...
	/// 3:	меняем на shared_ptr std::vector<std::unique_ptr<HaveClassName>> definitions;
	///		и всё работает (даже несмотря на то, что в shared_ptr создается make_unique)

	virtual QString GetClassName() const override { return classConfigFile(); }
	QString ToStrForLog() const override { return "["+path+"/"+name+"]"; }

	static QString classConfigFile() { return "ConfigFile"; }

};

class Config
{
public:
	~Config() = default;
	std::vector<ConfigFile> files;

	static QString PtrToStr(HaveClassName* ptr)
	{
		quint64 ptrVal = (quint64)ptr;
		return QString::number(ptrVal,16);
	}
	static HaveClassName* PtrFromStr(QString strPtr)
	{
		bool ok;
		quint64 ptrVal = strPtr.toULongLong(&ok,16);
		if(!ok) Logs::ErrorSt("PtrFromStr: wrong strPtr ["+strPtr+"]");
		return (HaveClassName*)ptrVal;
	}
};

class Editor: public QWidget, public LogedClass
{
	Q_OBJECT

	QTreeWidget *tree = nullptr;
	QTableWidget *table = nullptr;
	Config config;

	QFont basicFont;

public:
	explicit Editor(QWidget *parent = nullptr);
	~Editor() = default;

private slots:
	void Open();
	void PrintConfigTree();
	void TreeItemActivated(QTreeWidgetItem *item);
	void PrintConfigFile(ConfigFile& configFile);
	void PrintXMLFile(ConfigFile& configFile);

	void Add();
	void AddInXMLFile(ConfigFile& configFile, QString whatAdding);

	HaveClassName* DefineItemPtr(QTreeWidgetItem *item, bool checkExistance = true);
	const QStringList* WhatCanBeAdded(HaveClassName &configItem)
	{
		if(ConfigFile *configFile {dynamic_cast<ConfigFile*>(&configItem)})
		{
			if(configFile->suffix == Editor_ns::Extentions::main_ext)
				return &Editor_ns::canBeAddedAtFileMain;
			if(configFile->suffix == Editor_ns::Extentions::xml)
				return &Editor_ns::canBeAddedAtFileXML;
			else Logs::ErrorSt("WhatCanBeAdded wrong configFile type ["+configFile->suffix+"]");
		}
		else Logs::ErrorSt("WhatCanBeAdded unrealesed class ["+configItem.GetClassName()+"]");
		return nullptr;
	}

	void ReadConfig(QString fileName);
	ConfigFile* FindConfigFile(const QString &fName, const QString &fPath);
};

#endif // EDITOR_H
