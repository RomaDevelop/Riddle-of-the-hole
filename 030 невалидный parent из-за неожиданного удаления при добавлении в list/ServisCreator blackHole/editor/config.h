#ifndef CONFIG_H
#define CONFIG_H

#include "IConfigItem.h"

struct FileRowInMainFile: public IConfigItem
{
	QString definition;

	~FileRowInMainFile() = default;
	FileRowInMainFile() { definition = Editor_ns::rowCaptionFileInMainFile + " \"\""; }
	FileRowInMainFile(QString definition_) { definition = definition_; }
	static QString FileNameFromMainFileRow(QString rowInMainFile);

	virtual QString GetClassName() const override { return classMainFileDefinition(); }
	virtual QString ToStrForLog() const override { return "["+definition+"]"; }

	virtual int FieldsCount() const override;
	virtual QStringList FieldsNames() const override;
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return false; }
	virtual bool CanHaveSubItemsInTable() const override { return false; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return Editor_ns::rowCaptionFileInMainFile; }
	static QString classMainFileDefinition() { return "MainFileDefinition"; }
};

struct ConfigFile: public LogedClass, public IConfigItem
{
	QString fileNameWithPath;
	QString path;
	QString name;
	QString suffix;
	QString strContent;

	ConfigFile() = default;
	ConfigFile(QString fileNameWithPath_, const QString & content_);
	ConfigFile(const ConfigFile & srcFile);
	ConfigFile(ConfigFile && srcFile) = default;
	~ConfigFile();

	void UpdateFileOnDiscFromStrContent();
	void UpdateDefinitionsFromStrContent();
	void UpdateAllFromDefinitions();
	void UpdateAllFromStrContent();
	QString ToStrForConfig() const;
	ConfigFile CloneFile();

	virtual QString GetClassName() const override { return classConfigFile; }
	virtual QString ToStrForLog() const override { return "["+path+"/"+name+"]"; }

	virtual int FieldsCount() const override;
	virtual QStringList FieldsNames() const override;
	virtual QStringList FieldsValues() const override;
	virtual void SetFieldValues(const QStringList &values) override;
	virtual void SetField(QString fieldName, QString newValue) override;
	virtual const QStringList* WhatCanBeAdded() const override;
	virtual bool CanHaveSubItems() const override { return true; }
	virtual bool CanHaveSubItemsInTable() const override { return true; }
	virtual TreeItemVals ToTreeItemVals() const override;
	virtual QString RowCaption() const override { return RowCaptionSt(); }

	static QString RowCaptionSt() { return classConfigFile; }
	inline static QString classConfigFile { "ConfigFile" };
};

struct SortedConfigItems
{
	std::vector<IConfigItem*> objectClasses;
	std::vector<IConfigItem*> protocolClasses;
	std::vector<IConfigItem*> windowClasses;
	std::vector<IConfigItem*> constants;
	std::vector<IConfigItem*> objects;
	std::vector<IConfigItem*> windows;
	std::vector<IConfigItem*> syncs;
	std::vector<IConfigItem*> channels;
	std::vector<IConfigItem*> other;

	std::vector<IConfigItem*> all;
};

class Config
{
public:
	std::list<ConfigFile> files;

	~Config() = default;

	QString ProjectName();
	QString ProjectFile();

	Config CloneConfig();

	std::vector<IConfigItem*> GetItemsIf(std::function<bool(IConfigItem*)> condition);
	std::vector<IConfigItem*> GetAllItems();
	SortedConfigItems GetDefinitionsForRun();
	QDomDocument GetXMLForRun(const SortedConfigItems &sortedConfigItems);
	void RemoveExtraDataFromXMLForRun(QDomDocument &xmlDoc);

	void ResolveAllImports(QDomDocument &xmlDoc, const SortedConfigItems &sortedConfigItems);
	bool ResolveCopyPreviousElement(QDomElement &importElement);
	bool ResolveMultiImport(QDomElement &importElement);
	bool ResolveOneImport(const SortedConfigItems &sortedConfigItems, QDomDocument &xmlDoc, QDomElement &importElement);
	void ImportModification(QDomElement &element, QString modification);

	void ResolveImportInConfig(Config &originalConfig);
	bool ResolveCopyPreviousElementInConfig(IConfigItem * importItem);
	void ImportModificationInCopyPreviousElement(std::vector<IConfigItem*> newItems, const QString &modification);
	bool ResolveMultiImportInConfig(IConfigItem * importItem);
	bool ResolveRegularImportInConfig();
	bool ResolveOneRegularImportInConfig(IConfigItem * importItem, Config &importFrom, bool deep);
	void ImportModificationInConfig(IConfigItem * item, QString modification);

	QDomDocument GetXMLForRun2();

	QString GetClassName() { return "Config"; }
};

#endif // CONFIG_H
