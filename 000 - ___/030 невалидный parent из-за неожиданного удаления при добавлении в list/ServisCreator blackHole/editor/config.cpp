#include "config.h"

#include <map>

#include <QFile>
#include <QFileInfo>

#include "MyQDialogs.h"
#include "MyQDom.h"

#include "protocolConfig.h"
#include "objectConfig.h"
#include "constantsConfig.h"
#include "windowConfig.h"
#include "sync.h"
#include "channelConfig.h"
#include "otherConfig.h"

#include "configItemFabric.h"

namespace ImportModification
{
	const int modifTypeIndex = 0;

	namespace Remove {
		const int wordCount = 2;
		const QString commandWordRemoveName { "removeName" };
		const QString attributeName { Common::name };
		const int indexValueToRemove = 1;
	}

	namespace Rename {
		const int wordCount = 3;
		const QString commandWord { "rename" };
		const QString attributeName { Common::name };
		const int indexNameToRename = 1;
		const int indexNewName = 2;
	}

	namespace Set {
		const int wordCount = 3;
		const QString commandWord { "set" };
		const int indexAttribute = 1;
		const int indexNewValue = 2;
	}

	namespace Replace {
		const int wordCount = 3;
		const QString commandWord { "replace" };
		const int indexCurrentText = 1;
		const int indexNewText = 2;
	}

	namespace Count {
		const int countCommandsMustBe { 2 };
		const int commandWordIndex = 0;

		const QString commandWordCount { "count" };
		const int commandCountIndex { 0 };
		const int countIndex { 1 };
		const int wordsCountInCommandCount { 2 };

		const QString commandWordReplace { Replace::commandWord };
		const int commandReplaceIndex { 1 };
		const int replaceWordsCountPlusToCount = 2;
		const int replaceFromIndex { 1 };
	}

	namespace copyPreviousElement {

		const QString commandWord { "copyPreviousElement" };
		const int commandCountIndex = 0;
		const int commandReplaceIndex = 1;
		const int copyPreviousElementMinCommandsSize = 2;

		const QString countKeyWord { Count::commandWordCount };
		const int countKeyWordIndex = 0;
		const int countIndex = 1;
		const int countSize = 2;

		const QString replaceKeyWord { Replace::commandWord };
		const int replaceKeyWordIndex = 0;
		const int replaceWhatIndex = 1;
		const int replaceWordsCountPlusToCount = 2;
	}

	const QStringList allCommandWords { Remove::commandWordRemoveName, Rename::commandWord, Set::commandWord, Replace::commandWord };
	const std::vector<int> wordCounts { Remove::wordCount, Rename::wordCount, Set::wordCount, Replace::wordCount };
}

QString FileRowInMainFile::FileNameFromMainFileRow(QString rowInMainFile)
{
	QString fileName;
	auto words = Code::CommandToWords(rowInMainFile);
	if(words.size() == 2)
	{
		if(words[0] == Editor_ns::rowCaptionFileInMainFile)
		{
			if(TextConstant::IsItTextConstant(words[1],false))
			{
				fileName = TextConstant::GetTextConstVal(words[1]);
			}
			else Logs::ErrorSt("Editor::ReadConfig: wrong word[1] (not text constant) in command ["
							   + rowInMainFile + "]");
		}
		else Logs::ErrorSt("Editor::ReadConfig: wrong word[0] in command [" + rowInMainFile + "]");
	}
	else Logs::ErrorSt("Editor::ReadConfig: wrong words count in command [" + rowInMainFile + "]");
	return fileName;
}

int FileRowInMainFile::FieldsCount() const
{
	return Editor_ns::colsCountInMainFile;
}

QStringList FileRowInMainFile::FieldsNames() const
{
	return Editor_ns::headerMain;
}

QStringList FileRowInMainFile::FieldsValues() const
{
	return {definition};
}

void FileRowInMainFile::SetFieldValues(const QStringList & values)
{
	if(values.size() >= FieldsCount())
		definition = values[Editor_ns::mainFileFileIndex];
	else Logs::ErrorSt("MainFileDefinition::SetFieldValues values.size() < FieldsCount()");
}

void FileRowInMainFile::SetField(QString fieldName, QString newValue)
{
	Logs::ErrorSt("MainFileDefinition::SetField forbidden " + fieldName + " " + newValue);
}

const QStringList * FileRowInMainFile::WhatCanBeAdded() const
{
	Logs::ErrorSt("MainFileDefinition::WhatCanBeAdded forbidden");
	return nullptr;
}

TreeItemVals FileRowInMainFile::ToTreeItemVals() const
{
	Logs::ErrorSt("MainFileDefinition::ToTreeItemVals forbidden");
	return {};
}

ConfigFile::ConfigFile(QString fileNameWithPath_, const QString &content_)
{
	QFileInfo fi(fileNameWithPath_);
	fileNameWithPath = fileNameWithPath_;
	path = fi.path();
	name = fi.fileName();
	suffix = fi.suffix();
	strContent = content_;
	strContent.replace("&quot;","'");

	UpdateDefinitionsFromStrContent();
}

ConfigFile::ConfigFile(const ConfigFile &srcFile):
	fileNameWithPath { srcFile.fileNameWithPath	},
	path			 { srcFile.path			    },
	name			 { srcFile.name			    },
	suffix			 { srcFile.suffix			},
	strContent		 { srcFile.strContent		}
{
	Warning("copy constructor ConfigFile executed " + fileNameWithPath);
	for(auto &srcSubItem:srcFile.SubItems(true))
	{
		AddSubItem(srcSubItem->CloneItem(true));
	}
}

ConfigFile::~ConfigFile()
{
	//qdbg << "~ConfigFile()";
}

void ConfigFile::UpdateFileOnDiscFromStrContent()
{
	QFile file(fileNameWithPath);
	if(file.open(QFile::WriteOnly))
	{
		file.write(strContent.toUtf8());
		file.close();
	}
	else Error(GetClassName() + "::SaveFile can't open file ["+fileNameWithPath+"]");
}

void ConfigFile::UpdateDefinitionsFromStrContent()
{
	ClearSubItems();
	if(suffix == Editor_ns::Extentions::main_ext)
	{
		auto commands = Code::TextToCommands(strContent);
		for(int i=0; i<commands.size(); i++)
		{
			auto addres = AddSubItem(Editor_ns::rowCaptionFileInMainFile);
			if(addres)
			{
				auto addedFileRow = dynamic_cast<FileRowInMainFile*>(addres);
				if(addedFileRow) { addedFileRow->definition = commands[i]; }
				else Error(GetClassName()+"::UpdateDefinitionsFromStrContent dynamic_cast<FileRowInMainFile*>(addres) nullptr result");
			}
			else Error(GetClassName()+"::UpdateDefinitionsFromStrContent AddSubItem(Editor_ns::FileInMain) nullptr result");
		}
	}
	else if(suffix == Editor_ns::Extentions::xml)
	{
		QDomDocument xml;
		QString errMsg;
		int errLine, errCol;
		if(xml.setContent(strContent, &errMsg, &errLine, &errCol))
		{
			auto elements = MyQDom::GetTopLevelElements(xml);
			if(elements.size() == 1 && elements[0].tagName() == Editor_ns::xmlTagBody)
			{
				CreateSubElementsIfCanAndHave(elements[0]);
			}
			else Error(GetClassName() + "::UpdateDefinitionsFromStrContent wrong size or top level tag (must me 1 and "+Editor_ns::xmlTagBody+")");
		}
		else
		{
			Error(GetClassName() + "::UpdateDefinitionsFromStrContent error parsing file " + ToStrForLog());
			Error(GetClassName() + "::UpdateDefinitionsFromStrContent errMsg: " + errMsg);
			Error(GetClassName() + "::UpdateDefinitionsFromStrContent errLine: " + QSn(errLine));
			Error(GetClassName() + "::UpdateDefinitionsFromStrContent errCol: " + QSn(errCol));
		}
	}
	else Error(GetClassName() + "::UpdateDefinitionsFromStrContent: wrong suffix " + suffix);
}

void ConfigFile::UpdateAllFromDefinitions()
{
	strContent = ToStrForConfig();
	UpdateFileOnDiscFromStrContent();
	UpdateDefinitionsFromStrContent();
}

void ConfigFile::UpdateAllFromStrContent()
{
	UpdateFileOnDiscFromStrContent();
	UpdateDefinitionsFromStrContent();
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

QString ConfigFile::ToStrForConfig() const
{
	QString ret;
	if(suffix == Editor_ns::Extentions::xml)
	{
		auto &subItems = SubItems(true);
		for(auto &def:subItems)
		{
			ret += def->ExportToXMLTag(true).ToXMLCode(2);
		}

		ret = "<"+Editor_ns::xmlTagBody+">\n\n" + ret + "</"+Editor_ns::xmlTagBody+">";
	}
	else if(suffix == Editor_ns::Extentions::main_ext)
	{
		auto &subItems = SubItems();
		for(auto &def:subItems)
		{
			FileRowInMainFile* fileRow = dynamic_cast<FileRowInMainFile*>(def.get());
			if(fileRow)
			{
				ret += fileRow->definition + CodeKeyWords::commandSplitter + "\n";
			}
			else Logs::ErrorSt(GetClassName() + "::ToStrForConfig: nullptr dynamic_cast<FileRowInMainFile*>(def.get())");
		}
	}
	else Logs::ErrorSt(GetClassName() + "::ToStrForConfig: Wrong suffix ["+suffix+"]");

	return ret;
}

ConfigFile ConfigFile::CloneFile()
{
	ConfigFile clone;
	clone.fileNameWithPath	= fileNameWithPath	;
	clone.path				= path			    ;
	clone.name				= name			    ;
	clone.suffix			= suffix			;
	clone.strContent		= strContent		;

	for(auto &subItem:SubItems(true))
		clone.AddSubItem(subItem->CloneItem(true));

	return clone;
}

TreeItemVals ConfigFile::ToTreeItemVals() const
{
	TreeItemVals tree;
	tree.caption = GetClassName() + " " + name;
	tree.ptr = PtrToStr(this);
	if(suffix == Editor_ns::Extentions::xml)
	{
		auto &subItems = SubItems();
		for(auto &def:subItems)
		{
			tree.subVals.push_back(def->ToTreeItemVals());
		}
	}
	return tree;
}

QString Config::ProjectName()
{
	QString ret;
	if(files.empty()) Logs::ErrorSt("Config::ProjectName files.empty()");
	else
	{
		if(files.front().suffix == Editor_ns::Extentions::main_ext)
		{
			ret = files.front().name;
		}
		else Logs::ErrorSt("Config::ProjectName files[0] is not main file");
	}
	return ret;
}

QString Config::ProjectFile()
{
	QString ret;
	if(files.empty()) Logs::ErrorSt("Config::ProjectName files.empty()");
	else
	{
		if(files.front().suffix == Editor_ns::Extentions::main_ext)
		{
			ret = files.front().fileNameWithPath;
		}
		else Logs::ErrorSt("Config::ProjectName files[0] is not main file");
	}
	return ret;
}

Config Config::CloneConfig()
{
	Config clone;
	for(auto &file:files)
		clone.files.push_back(file.CloneFile());

	return clone;
}

std::vector<IConfigItem *> Config::GetItemsIf(std::function<bool(IConfigItem*)> condition)
{
	std::vector<IConfigItem *> allItems;
	for(auto &file:files)
	{
		auto allItemsOfFile = file.SubItemPtrs(true);
		allItems.insert(allItems.end(), allItemsOfFile.begin(), allItemsOfFile.end());
	}
	for(int i=allItems.size()-1; i>=0; i--)
	{
		if(!condition(allItems[i]))
			allItems.erase(allItems.begin() + i);
	}
	return allItems;
}

std::vector<IConfigItem *> Config::GetAllItems()
{
	std::vector<IConfigItem *> allItems;
	for(auto &file:files)
	{
		auto allItemsOfFile = file.SubItemPtrs(true);
		allItems.insert(allItems.end(), allItemsOfFile.begin(), allItemsOfFile.end());
	}
	return allItems;
}

SortedConfigItems Config::GetDefinitionsForRun()
{
	SortedConfigItems sortedConfigItems;
	for(auto &file:files)
	{
		if(file.suffix == Editor_ns::Extentions::main_ext)
		{
			// nothing to do
		}
		else if(file.suffix == Editor_ns::Extentions::xml)
		{
			auto &subItemsOfFile = file.SubItems();
			for(auto &subItemOfFile:subItemsOfFile)
			{
				bool error = false;
				if(dynamic_cast<ObjectClass*>(subItemOfFile.get()))
					sortedConfigItems.objectClasses.push_back(subItemOfFile.get());
				else if(dynamic_cast<ProtocolClass*>(subItemOfFile.get()))
					sortedConfigItems.protocolClasses.push_back(subItemOfFile.get());
				else if(dynamic_cast<WindowClass*>(subItemOfFile.get()))
					sortedConfigItems.windowClasses.push_back(subItemOfFile.get());
				else if(dynamic_cast<ConstantsConfigItem*>(subItemOfFile.get()))
					sortedConfigItems.constants.push_back(subItemOfFile.get());
				else if(dynamic_cast<ObjectConfigItem*>(subItemOfFile.get()))
					sortedConfigItems.objects.push_back(subItemOfFile.get());
				else if(dynamic_cast<WindowConfigItem*>(subItemOfFile.get()))
					sortedConfigItems.windows.push_back(subItemOfFile.get());
				else if(dynamic_cast<SyncConfigItem*>(subItemOfFile.get()))
					sortedConfigItems.syncs.push_back(subItemOfFile.get());
				else if(dynamic_cast<ChannelConfigItem*>(subItemOfFile.get()))
					sortedConfigItems.channels.push_back(subItemOfFile.get());
				else if(dynamic_cast<OtherConfigItem*>(subItemOfFile.get()))
					sortedConfigItems.channels.push_back(subItemOfFile.get());
				else
				{
					error = true;
					if(dynamic_cast<HaveClassName*>(subItemOfFile.get()))
						Logs::ErrorSt("Config::GetDefinitionsForRun(): wrong class [" + subItemOfFile->GetClassName() + "]");
					else Logs::ErrorSt("Config::GetDefinitionsForRun(): wrong class unknown");
				}
				if(!error) sortedConfigItems.all.push_back(subItemOfFile.get());
			}
		}
		else Logs::ErrorSt("Config::GetDefinitionsForRun(): wrong suffix [" + file.suffix + "]");
	}

	return sortedConfigItems;
}

QDomDocument Config::GetXMLForRun(const SortedConfigItems & sortedConfigItems)
{
	QDomDocument xmlDoc;
	QDomElement body = xmlDoc.createElement(Editor_ns::xmlTagBody);
	xmlDoc.appendChild(body);
	for(auto &channel:sortedConfigItems.channels)
	{
		channel->ExportToDomDocument(xmlDoc,body);
	}
	for(auto &constants:sortedConfigItems.constants)
	{
		constants->ExportToDomDocument(xmlDoc,body);
	}
	for(auto &other:sortedConfigItems.other)
	{
		other->ExportToDomDocument(xmlDoc,body);
	}
	for(auto &objectClass:sortedConfigItems.objectClasses)
	{
		objectClass->ExportToDomDocument(xmlDoc,body);
	}
	for(auto &protClass:sortedConfigItems.protocolClasses)
	{
		protClass->ExportToDomDocument(xmlDoc,body);
	}
	for(auto &windowClass:sortedConfigItems.windowClasses)
	{
		windowClass->ExportToDomDocument(xmlDoc,body);
	}
	for(auto &object:sortedConfigItems.objects)
	{
		auto fields = object->FieldsValues();
		QString name = fields[ObjectConfigItem_ns::nameIndex];
		QString type = fields[ObjectConfigItem_ns::typeIndex];
		QString objectClassStr = fields[ObjectConfigItem_ns::objectClassIndex];
		QString protocolClassStr = fields[ObjectConfigItem_ns::protocolClassIndex];
		QString constants = fields[ObjectConfigItem_ns::constantsIndex];
		QString channel = fields[ObjectConfigItem_ns::channelIndex];
		QString startSettings = fields[ObjectConfigItem_ns::startSettingsIndex];

		QDomElement objectElement = xmlDoc.createElement(ObjectConfigItem_ns::rowCaption);
		body.appendChild(objectElement);
		objectElement.setAttribute(Object_ns::name,name);
		objectElement.setAttribute(Object_ns::type,type);
		ObjectClass *objectClass = nullptr;
		for(auto &iter:sortedConfigItems.objectClasses)
		{
			ObjectClass* objectClassIter = dynamic_cast<ObjectClass*>(iter);
			if(objectClassIter)
			{
				if(objectClassIter->objectClassName == objectClassStr)
					objectClass = objectClassIter;
			}
			else Logs::ErrorSt("GetXMLForRun: dynamic_cast<ObjectClass*>(iter) nullptr");
		}
		if(objectClass)
		{
			auto &subItems = objectClass->SubItems();
			for(auto &subItem:subItems)
			{
				objectElement.appendChild(subItem->ToDomElement(xmlDoc));
			}
		}
		else Logs::ErrorSt("GetXMLForRun: can't find objectClass by objectClassName ["+objectClassStr
						   +"] for "+object->RowCaption()+" ["+name+"]");

		QDomElement protElement = xmlDoc.createElement(Protocol_ns::Protocol);
		body.appendChild(protElement);
		protElement.setAttribute(Protocol_ns::name,name);
		protElement.setAttribute(Protocol_ns::type,type);
		protElement.setAttribute(Protocol_ns::constants,constants);
		protElement.setAttribute(Protocol_ns::channel,channel);
		protElement.setAttribute(Protocol_ns::startSettings,startSettings);
		ProtocolClass *protClass = nullptr;
		for(auto &iter:sortedConfigItems.protocolClasses)
		{
			ProtocolClass* protClassIter = dynamic_cast<ProtocolClass*>(iter);
			if(protClassIter)
			{
				if(protClassIter->protocolClassName == protocolClassStr)
					protClass = protClassIter;
			}
			else Logs::ErrorSt("GetXMLForRun: dynamic_cast<ProtocolClass*>(iter) nullptr");
		}
		if(protClass)
		{
			auto& subItems = protClass->SubItems();
			for(auto &subItem:subItems)
			{
				protElement.appendChild(subItem->ToDomElement(xmlDoc));
			}
		}
		else Logs::ErrorSt("GetXMLForRun: can't find protocolClass by protocolClassName ["+protocolClassStr
						   +"] for "+object->RowCaption()+" ["+name+"]");
	}
	for(auto &window:sortedConfigItems.windows)
	{
		namespace this_ns = WindowConfigItem_ns::Fields;
		auto fields = window->FieldsValues();
		QString name = fields[this_ns::nameIndex];
		QString windowClassStr = fields[this_ns::windowClassIndex];
		QString objectToConnect = fields[this_ns::objectToConnectIndex];
		QString other = fields[this_ns::OtherIndex];
		QDomElement windowElement = xmlDoc.createElement(this_ns::rowCaption);
		body.appendChild(windowElement);
		windowElement.setAttribute(this_ns::name,name);
		windowElement.setAttribute(this_ns::windowClass,windowClassStr);
		windowElement.setAttribute(this_ns::objectToConnect,objectToConnect);
		windowElement.setAttribute(this_ns::Other,other);
		if(!windowClassStr.contains(this_ns::external))
		{
			WindowClass *windowClass = nullptr;
			for(auto &iter:sortedConfigItems.windowClasses)
			{
				WindowClass* windowClassIter = dynamic_cast<WindowClass*>(iter);
				if(windowClassIter)
				{
					if(windowClassIter->name == windowClassStr)
						windowClass = windowClassIter;
				}
				else Logs::ErrorSt("GetXMLForRun: dynamic_cast<WindowClass*>(iter) nullptr");
			}
			if(windowClass)
			{
				auto& subItems = windowClass->SubItems();
				for(auto &subItem:subItems)
				{
					windowElement.appendChild(subItem->ToDomElement(xmlDoc));
				}
			}
			else Logs::ErrorSt("GetXMLForRun: can't find windowClass by windowClassStr ["+windowClassStr
							   +"] for "+window->RowCaption()+" ["+name+"]");
		}
		else { /* nothing to do */ }
	}
	for(auto &sync:sortedConfigItems.syncs)
	{
		sync->ExportToDomDocument(xmlDoc,body);
	}

	RemoveExtraDataFromXMLForRun(xmlDoc);
	return xmlDoc;
}

namespace ImportLog {
	QStringList content;
	//QStringList namesToLog { "эмулятор_каналы" };
	QStringList namesToLog { };


	int before = 1;
	int after = 2;

	int doImportCount = 0;

	void Log(QDomElement &importElement, QDomElement &importParent, int code)
	{
		if(doImportCount > 0 || namesToLog.contains(importElement.attribute("name")))
		{
			if(code == before) content += "before: ";
			if(code == after) content += "after: ";
			content += MyQDom::ToString(importParent);
			if(code == before) content += "----------------------------------------------------------------------------";
			if(code == after) content += "============================================================================";

			doImportCount--;
		}
	}
}

IConfigItem * findByNameAndType(std::vector<IConfigItem *> items, QString name, QString type)
{
	IConfigItem * ret = nullptr;
	for(auto &item:items)
	{
		if(item->RowCaption() == type)
		{
			if(item->FieldsNames().contains(Import_ns::name) && item->Field(Import_ns::name) == name)
			{
				if(!ret) ret = item;
				else Logs::ErrorSt("findByNameAndType found note one importing item ["+type+":"+name+"]");
			}
		}
	}
	return ret;
}

void Config::ResolveImportInConfig(Config &originalConfig)
{
	if(this == &originalConfig) { Logs::ErrorSt("ResolveImportInConfig this == &originalConfig"); return; }

	{// noDeepImport
		auto conditionNoDeepImports = [](IConfigItem* item){
			if(item->RowCaption() == Import_ns::rowCaption)
				return item->Field(Import_ns::policy) == Import_ns::Policies::noDeepImport;
			else return false;
		};

		std::vector<IConfigItem *> noDeepImports = GetItemsIf(conditionNoDeepImports);
		while(!noDeepImports.empty())
		{
			for(auto &import:noDeepImports)
			{
				if(!ResolveOneRegularImportInConfig(import,originalConfig,false))
					Logs::ErrorSt("while ResolveOneRegularImportInConfig error");
			}

			noDeepImports = GetItemsIf(conditionNoDeepImports);
		}
	}

	{// мульти импорт
		std::vector<IConfigItem *> multiImports = GetItemsIf([](IConfigItem* item){
				if(item->RowCaption() == Import_ns::rowCaption
				   && item->Field(Import_ns::type) != ImportModification::copyPreviousElement::commandWord)
					return item->Field(Import_ns::modification).startsWith(ImportModification::Count::commandWordCount);
				else return false;
		});
		for(auto &import:multiImports)
		{
			if(!ResolveMultiImportInConfig(import))
				Logs::ErrorSt("while ResolveMultiImportInConfig error");
		}
	}


	{// копирование предыдущих строк
		std::vector<IConfigItem *> copyPrevRowsImpors = GetItemsIf([](IConfigItem* item){
				if(item->RowCaption() == Import_ns::rowCaption)
				return item->Field(Import_ns::type) == ImportModification::copyPreviousElement::commandWord;
				else return false;
		});
		for(auto &import:copyPrevRowsImpors)
		{
			if(!ResolveCopyPreviousElementInConfig(import))
				Logs::ErrorSt("while ResolveCopyPreviousElementInConfig error");
		}
	}

	{// обычный импорт
		if(!ResolveRegularImportInConfig())
			Logs::ErrorSt("while ResolveRegularImportInConfig error");
	}
}

void Config::ResolveAllImports(QDomDocument &xmlDoc, const SortedConfigItems &sortedConfigItems)
{
	Logs::LogSt("Начало импорта " + QTime::currentTime().toString("hh:mm:ss.zzz"));
	int i=0;
	while(1)
	{
		i++;
		// берем первый импорт
		QDomElement import = MyQDom::FirstChildIncludeSubChilds(xmlDoc,Import_ns::rowCaption);
		if(import.isNull()) break;

		auto importParent = import.parentNode().toElement();

		if(!ResolveOneImport(sortedConfigItems, xmlDoc,import))
		{
			Logs::ErrorSt("Во время импортирования возникли ошибки, импортирование прервано");
			break;
		}

		if(i%100 == 0)
			Logs::LogSt("Выполнено итераций импорта " + QSn(i));
	}
	Logs::LogSt("Окончание импорта " + QTime::currentTime().toString("hh:mm:ss.zzz"));

	if(!ImportLog::content.empty()) MyQDialogs::ShowText(ImportLog::content.join('\n'),1500,800);
	ImportLog::content.clear();
}

bool Config::ResolveOneImport(const SortedConfigItems &sortedConfigItems, QDomDocument &xmlDoc, QDomElement &importElement)
{
	if(importElement.attribute("name") == "...")
		ImportLog::doImportCount = 10;

	auto importParent = importElement.parentNode().toElement();
	ImportLog::Log(importElement, importParent, ImportLog::before);

	if(importElement.attribute(Import_ns::type) == ImportModification::copyPreviousElement::commandWord)
	{
		bool res = ResolveCopyPreviousElement(importElement);
		ImportLog::Log(importElement, importParent, ImportLog::after);
		return res;
	}
	else if(importElement.attribute(Import_ns::modification).startsWith(ImportModification::Count::commandWordCount))
	{
		bool res = ResolveMultiImport(importElement);
		ImportLog::Log(importElement, importParent, ImportLog::after);
		return res;
	}

	QString type = importElement.attribute(Import_ns::type);
	QString name = importElement.attribute(Import_ns::name);
	QString modification = importElement.attribute(Import_ns::modification);
	QString policy = importElement.attribute(Import_ns::policy);

	if(policy == Import_ns::Policies::empty || policy == Import_ns::Policies::deepImport)
	{
		auto allElements = MyQDom::GetAllLevelElements(xmlDoc);
		QDomElement *importingElementPtr = nullptr;

		for(auto &element:allElements)
			if(type == element.tagName() && name == element.attribute(Import_ns::name))
			{
				if(importingElementPtr) Logs::ErrorSt(GetClassName() + "::ResolveStandartImport found note one importing item ["+type+":"+name+"]");
				else importingElementPtr = &element;
			}

		if(!importingElementPtr)
		{
			Logs::ErrorSt(GetClassName() + "::ResolveStandartImport can't find importing ["+type+":"+name+"]");
			return false;
		}

		auto allSubs = MyQDom::GetAllLevelElements(*importingElementPtr);
		for(auto &sub:allSubs)
		{
			if(sub.tagName() == Import_ns::rowCaption)
			{
				if(!ResolveOneImport(sortedConfigItems, xmlDoc, sub))
				{
					Logs::ErrorSt("Возникла ошибка при импортировании вложенного импорта");
				}
			}
		}

		QDomElement importingElementClone = importingElementPtr->cloneNode(true).toElement();

		if(!importingElementClone.isNull())
		{
			ImportModification(importingElementClone, modification);

			auto subElements = MyQDom::GetTopLevelElements(importingElementClone);
			for(auto &subElement:subElements)
			{
				importElement.parentNode().insertBefore(subElement,importElement);
			}

			importElement.parentNode().removeChild(importElement);
			ImportLog::Log(importElement, importParent, ImportLog::after);
			return true;
		}
		else Logs::ErrorSt(GetClassName() + "::ResolveStandartImport err cloning ["+MyQDom::ToString(*importingElementPtr) +"]");
	}
	else if(policy == Import_ns::Policies::noDeepImport)
	{
		IConfigItem *importingItem = nullptr;
		for(auto &item:sortedConfigItems.all)
			if(type == item->RowCaption() && name == item->Field(Import_ns::name))
			{
				if(importingItem) Logs::ErrorSt(GetClassName() + "::ResolveImport found note one importing item ["+type+":"+name+"]");
				else importingItem = item;
			}
		if(importingItem)
		{
			QDomElement importingElement = importingItem->ToDomElement(xmlDoc);
			ImportModification(importingElement, modification);
			auto subElements = MyQDom::GetTopLevelElements(importingElement);
			for(auto &subElement:subElements)
			{
				importElement.parentNode().insertBefore(subElement,importElement);
			}
			importElement.parentNode().removeChild(importElement);
			ImportLog::Log(importElement, importParent, ImportLog::after);
			return true;
		}
		else Logs::ErrorSt(GetClassName() + "::ResolveImport can't find importing ["+type+":"+name+"]");
	}
	else Logs::ErrorSt("Неизвестная политика импорта " + importElement.attribute(Import_ns::policy));

	ImportLog::Log(importElement, importParent, ImportLog::after);
	return false;
}

bool Config::ResolveRegularImportInConfig()
{
//	auto allItems = GetAllItems();
//	int count = 0;
//	for(auto removedItem:IConfigItem::removedItems)
//	{
//		for(auto item:allItems)
//		{
//			if(item == removedItem)
//			{
//				qdbg << "idiocraty" << IConfigItem::PtrToStr(item);
//				MyQDialogs::ShowText(item->ExportToXMLTag(1).ToXMLCode(2) + "\n\n"
//									 + item->parent->ExportToXMLTag(1).ToXMLCode(2));
//				count++;
//			}
//		}
//	}
//	if(count) return false;

	while (1)
	{
		// ищем импорт
		auto allItems = GetAllItems();
		IConfigItem * importElement = nullptr;
		for(auto &item:allItems)
		{
			if(item->RowCaption() == Import_ns::rowCaption)
			{
				if(item->Field(Import_ns::policy) == Import_ns::Policies::empty || item->Field(Import_ns::policy) == Import_ns::Policies::deepImport)
				{
					importElement = item;
					break;
				}
				else Logs::ErrorSt("Wrong import policy ["+item->Field(Import_ns::policy)+"] for ResolveRegularImport");
			}
		}

		if(!importElement) break;

		// производим импорт
		if(!ResolveOneRegularImportInConfig(importElement, *this, true)) return false;
	}

	return true;
}

namespace forRORIIC { int showsCount = 0; QString funcName = "ResolveOneRegularImportInConfig\n"; }

bool Config::ResolveOneRegularImportInConfig(IConfigItem * importElement, Config &importFrom, bool deep)
{
	QString before;
	auto parentForLog = importElement->parent;
	int &counterLog = forRORIIC::showsCount;
	counterLog--;
	if(counterLog > 0) before = forRORIIC::funcName + importElement->parent->ExportToXMLTag(true).ToXMLCode(2);

	QString type = importElement->Field(Import_ns::type);
	QString name = importElement->Field(Import_ns::name);
	QString modification = importElement->Field(Import_ns::modification);
	QString policy = importElement->Field(Import_ns::policy);

	// ищем импортируемый элемент
	IConfigItem * importingElementPtr = nullptr;
	auto allItems = importFrom.GetAllItems();
	for(auto &element:allItems)
		if(type == element->RowCaption() && name == element->Field(Import_ns::name))
		{
			if(importingElementPtr) Logs::ErrorSt(GetClassName() + "::ResolveStandartImport found note one importing item ["+type+":"+name+"]");
			else importingElementPtr = element;
		}

	if(!importingElementPtr)
	{
		Logs::ErrorSt(GetClassName() + "::ResolveStandartImport can't find importing ["+type+":"+name+"]");
		if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
		return false;
	}

	// проверяем на вложенный импорт
	if(deep)
	{
		auto allSubs = importingElementPtr->SubItemPtrs(true);
		for(auto &sub:allSubs)
		{
			if(sub->RowCaption() == Import_ns::rowCaption)
			{
				return ResolveOneRegularImportInConfig(sub, importFrom, deep);
			}
		}
	}

	// клонируем импортируемый итем,
	// производим модификацию,
	// вставляем его содержимое до импорта,
	// удаляем импорт
	auto importingElementClone = importingElementPtr->CloneItem(true);
	if(importingElementClone)
	{
		ImportModificationInConfig(importingElementClone.get(), modification);

		auto subElements = importingElementClone->SubItems(true);
		for(auto &subElement:subElements)
		{
			if(!importElement->InsertSibling(subElement,IConfigItem::beforeMe))
			{
				Logs::ErrorSt(GetClassName() + "::ResolveStandartImport err inserting");
				if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
				return false;
			}
		}

		importElement->RemoveMe();
		if(counterLog > 0) MyQDialogs::ShowText(before + "\n\n" + parentForLog->ExportToXMLTag(true).ToXMLCode(2));
		return true;
	}
	else Logs::ErrorSt(GetClassName() + "::ResolveStandartImport err cloning ["+importingElementPtr->ExportToXMLTag(false).ToXMLCode(2)+"]");

	if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
	return false;
}

namespace forIMIC { int showsCount = 0; QString funcName = "ImportModificationInConfig\n"; }

void Config::ImportModificationInConfig(IConfigItem * item, QString modification)
{
	if(modification.isEmpty()) return;

	QString before;
	int &counterLog = forIMIC::showsCount;
	counterLog--;
	if(counterLog > 0) before = forIMIC::funcName + "\nmodification:\n" + modification + "\n\n" + item->ExportToXMLTag(true).ToXMLCode(2);

	namespace thisNs = ImportModification;
	QStringList commands = Code::TextToCommands(modification);
	for(auto &command:commands)
	{
		auto words = Code::CommandToWords(command);
		int wordsCountFakt = words.size();
		if(wordsCountFakt)
		{
			bool check = false;
			int index = thisNs::allCommandWords.indexOf(words[thisNs::modifTypeIndex]);
			int wordsCountMustBe = -1;
			if(index != -1)
			{
				wordsCountMustBe = thisNs::wordCounts[index];
				if(wordsCountFakt == wordsCountMustBe) check = true;
				else Logs::ErrorSt(GetClassName() + "::ImportModification wrong wordsSize ["+QSn(wordsCountFakt)+"] for word ["+words[0]
						+"]\n\tcommand ["+command+"]");
			}
			else Logs::ErrorSt(GetClassName() + "::ImportModification unknown word[0] = ["+words[thisNs::modifTypeIndex]
					+"]\n\tcommand ["+command+"]");

			if(check)
			{
				if(words[thisNs::modifTypeIndex] == thisNs::Remove::commandWordRemoveName)
				{
					namespace thisNs = thisNs::Remove;
					QString nameToRemove = TextConstant::GetTextConstVal(words[thisNs::indexValueToRemove]);
					auto subItems = item->SubItemPtrs(true);
					for(auto &subItem:subItems)
					{
						if(subItem->FieldsNames().contains(Common::name) && subItem->Field(Common::name) == nameToRemove)
						{
							subItem->RemoveMe();
						}
					}
				}
				else if(words[thisNs::modifTypeIndex] == thisNs::Rename::commandWord)
				{
					namespace thisNs = thisNs::Rename;
					QString nameToRename = TextConstant::GetTextConstVal(words[thisNs::indexNameToRename]);
					QString newName = TextConstant::GetTextConstVal(words[thisNs::indexNewName]);
					auto subItems = item->SubItemPtrs(true);
					for(auto &subItem:subItems)
					{
						if(subItem->FieldsNames().contains(Common::name) && subItem->Field(Common::name) == nameToRename)
							subItem->SetField(Common::name, newName);
					}
				}
				else if(words[thisNs::modifTypeIndex] == thisNs::Set::commandWord)
				{
					namespace thisNs = thisNs::Set;
					QString attribute { words[thisNs::indexAttribute] };
					QString newValue { TextConstant::GetTextConstVal(words[thisNs::indexNewValue]) };
					auto subItems = item->SubItemPtrs(true);
					for(auto &subItem:subItems)
					{
						if(subItem->FieldsNames().contains(attribute))
							subItem->SetField(attribute, newValue);
					}
				}
				else if(words[thisNs::modifTypeIndex] == thisNs::Replace::commandWord)
				{
					namespace thisNs = thisNs::Replace;
					QString replaceWhat { TextConstant::GetTextConstVal(words[thisNs::indexCurrentText]) };
					QString replaceTo { TextConstant::GetTextConstVal(words[thisNs::indexNewText]) };
					auto subItems = item->SubItemPtrs(true);
					for(auto &subItem:subItems)
					{
						subItem->ReplaceInFields(replaceWhat,replaceTo);
					}
				}
				else Logs::ErrorSt(GetClassName() + "::ImportModification unrealesed word ["+words[thisNs::modifTypeIndex]+"]");
			}
			else Logs::ErrorSt(GetClassName() + "::ImportModification command ["+command+"] not passed check");
		}
		else Logs::ErrorSt(GetClassName() + "::ImportModification words.empty()");
	}

	if(counterLog > 0) MyQDialogs::ShowText(before + "\n\n" + item->ExportToXMLTag(true).ToXMLCode(2));
}

QDomDocument Config::GetXMLForRun2()
{
	auto clone = CloneConfig();
	clone.ResolveImportInConfig(*this);
	SortedConfigItems sortedDefs = clone.GetDefinitionsForRun();
	return clone.GetXMLForRun(sortedDefs);
}

bool Config::ResolveMultiImport(QDomElement & importElement)
{
	namespace thisNs = ImportModification::Count;
	QString modification = importElement.attribute(Import_ns::modification);
	auto commands = Code::TextToCommands(modification);
	if(commands.size() == thisNs::countCommandsMustBe)
	{
		auto wordsCount = Code::CommandToWords(commands[thisNs::commandCountIndex]);
		auto wordsReplace = Code::CommandToWords(commands[thisNs::commandReplaceIndex]);
		QString error;
		if(error.isEmpty() && wordsCount.size() != thisNs::wordsCountInCommandCount)
			error += "wrong words count in command Count "+QSn(wordsCount.size())+", must be"
					+ QSn(thisNs::wordsCountInCommandCount);
		if(error.isEmpty() && wordsCount[thisNs::commandWordIndex] != thisNs::commandWordCount)
			error += "wrong word("+wordsCount[thisNs::commandWordIndex]+") in command Count";
		if(error.isEmpty() && wordsReplace.size() != wordsCount[thisNs::countIndex].toInt() + thisNs::replaceWordsCountPlusToCount)
			error += "wrong words count in command Replace "+QSn(wordsReplace.size())+", must be "
					+QSn(wordsCount[thisNs::countIndex].toInt() + thisNs::replaceWordsCountPlusToCount);
		if(error.isEmpty() && wordsReplace[thisNs::commandWordIndex] != thisNs::commandWordReplace)
			error += "wrong word("+wordsReplace[thisNs::commandWordIndex]+") in command Replace";

		if(error.isEmpty())
		{
			int count = wordsCount[thisNs::countIndex].toInt();
			for(int i=0; i<count; i++)
			{
				QString newModification = thisNs::commandWordReplace + " " + wordsReplace[thisNs::replaceFromIndex] + " ";
				newModification += wordsReplace[i + thisNs::replaceWordsCountPlusToCount];

				QDomElement newElement = importElement.cloneNode(true).toElement();
				if(!newElement.isNull())
				{
					QDomElement newElementInsetred =
							importElement.parentNode().insertBefore(newElement,importElement).toElement();
					if(!newElementInsetred.isNull())
					{
						newElementInsetred.setAttribute(Import_ns::modification, newModification);
					}
					else Logs::ErrorSt(GetClassName() + "::ResolveMultiImport error 4");
				}
				else Logs::ErrorSt(GetClassName() + "::ResolveMultiImport error 3");
			}

			importElement.parentNode().removeChild(importElement);
			return true;
		}
		else Logs::ErrorSt(GetClassName() + "::ResolveMultiImport error 2\n\terror ["+error+"]\n\tmodification ["+modification+"]");
	}
	else Logs::ErrorSt(GetClassName() + "::ResolveMultiImport error 1\n\tmodification ["+modification+"]");

	return false;
}

namespace forRMIIC { int showsCount = 0; QString funcName = "ResolveMultiImportInConfig\n"; }

bool Config::ResolveMultiImportInConfig(IConfigItem * importItem)
{
	QString before;
	auto parentForLog = importItem->parent;
	int &counterLog = forRMIIC::showsCount;
	counterLog--;
	if(counterLog > 0) before = forRMIIC::funcName + parentForLog->ExportToXMLTag(true).ToXMLCode(2);

	namespace thisNs = ImportModification::Count;
	QString modification = importItem->Field(Import_ns::modification);
	auto commands = Code::TextToCommands(modification);
	if(commands.size() == thisNs::countCommandsMustBe)
	{
		auto wordsCount = Code::CommandToWords(commands[thisNs::commandCountIndex]);
		auto wordsReplace = Code::CommandToWords(commands[thisNs::commandReplaceIndex]);
		QString error;
		if(error.isEmpty() && wordsCount.size() != thisNs::wordsCountInCommandCount)
			error += "wrong words count in command Count "+QSn(wordsCount.size())+", must be"
					+ QSn(thisNs::wordsCountInCommandCount);
		if(error.isEmpty() && wordsCount[thisNs::commandWordIndex] != thisNs::commandWordCount)
			error += "wrong word("+wordsCount[thisNs::commandWordIndex]+") in command Count";
		if(error.isEmpty() && wordsReplace.size() != wordsCount[thisNs::countIndex].toInt() + thisNs::replaceWordsCountPlusToCount)
			error += "wrong words count in command Replace "+QSn(wordsReplace.size())+", must be "
					+QSn(wordsCount[thisNs::countIndex].toInt() + thisNs::replaceWordsCountPlusToCount);
		if(error.isEmpty() && wordsReplace[thisNs::commandWordIndex] != thisNs::commandWordReplace)
			error += "wrong word("+wordsReplace[thisNs::commandWordIndex]+") in command Replace";

		if(error.isEmpty())
		{
			int count = wordsCount[thisNs::countIndex].toInt();
			for(int i=0; i<count; i++)
			{
				QString newModification = thisNs::commandWordReplace + " " + wordsReplace[thisNs::replaceFromIndex] + " ";
				newModification += wordsReplace[i + thisNs::replaceWordsCountPlusToCount];

				std::shared_ptr<IConfigItem> newItem = importItem->CloneItem(true);
				if(newItem)
				{

					IConfigItem* newItemInsetred = importItem->InsertSibling(newItem,IConfigItem::beforeMe);
					if(newItemInsetred)
					{
						newItemInsetred->SetField(Import_ns::modification, newModification);
					}
					else Logs::ErrorSt(GetClassName() + "::ResolveMultiImport error 4");
				}
				else Logs::ErrorSt(GetClassName() + "::ResolveMultiImport error 3");
			}

			importItem->RemoveMe();
			if(counterLog > 0) MyQDialogs::ShowText(before + "\n\n" + parentForLog->ExportToXMLTag(true).ToXMLCode(2));
			return true;
		}
		else Logs::ErrorSt(GetClassName() + "::ResolveMultiImport error 2\n\terror ["+error+"]\n\tmodification ["+modification+"]");
	}
	else Logs::ErrorSt(GetClassName() + "::ResolveMultiImport error 1\n\tmodification ["+modification+"]");

	if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
	return false;
}

void Config::ImportModification(QDomElement & element, QString modification)
{
	namespace thisNs = ImportModification;
	QStringList commands = Code::TextToCommands(modification);
	for(auto &command:commands)
	{
		auto words = Code::CommandToWords(command);
		int wordsCountFakt = words.size();
		if(wordsCountFakt)
		{
			bool check = false;
			int index = thisNs::allCommandWords.indexOf(words[thisNs::modifTypeIndex]);
			int wordsCountMustBe = -1;
			if(index != -1)
			{
				wordsCountMustBe = thisNs::wordCounts[index];
				if(wordsCountFakt == wordsCountMustBe) check = true;
				else Logs::ErrorSt(GetClassName() + "::ImportModification wrong wordsSize ["+QSn(wordsCountFakt)+"] for word ["+words[0]
						+"]\n\tcommand ["+command+"]");
			}
			else Logs::ErrorSt(GetClassName() + "::ImportModification unknown word[0] = ["+words[thisNs::modifTypeIndex]
					+"]\n\tcommand ["+command+"]");

			if(check)
			{
				if(words[thisNs::modifTypeIndex] == thisNs::Remove::commandWordRemoveName)
				{
					namespace thisNs = thisNs::Remove;
					QStringPair attr {thisNs::attributeName, TextConstant::GetTextConstVal(words[thisNs::indexValueToRemove])};
					auto elementToRemove = MyQDom::FirstChildIncludeSubChilds(element, attr);
					if(!elementToRemove.isNull())
						elementToRemove.parentNode().removeChild(elementToRemove);
				}
				else if(words[thisNs::modifTypeIndex] == thisNs::Rename::commandWord)
				{
					namespace thisNs = thisNs::Rename;
					QStringPair attrName {thisNs::attributeName, TextConstant::GetTextConstVal(words[thisNs::indexNameToRename])};
					QString newName {TextConstant::GetTextConstVal(words[thisNs::indexNewName])};
					auto elementToRename = MyQDom::FirstChildIncludeSubChilds(element, attrName);
					if(!elementToRename.isNull())
						elementToRename.setAttribute(attrName.first,newName);
				}
				else if(words[thisNs::modifTypeIndex] == thisNs::Set::commandWord)
				{
					namespace thisNs = thisNs::Set;
					QString attribute { words[thisNs::indexAttribute] };
					QString newValue { TextConstant::GetTextConstVal(words[thisNs::indexNewValue]) };
					auto elements = MyQDom::GetAllLevelElements(element);
					for(auto &element:elements)
					{
						if(element.hasAttribute(attribute))
							element.setAttribute(attribute,newValue);
					}
				}
				else if(words[thisNs::modifTypeIndex] == thisNs::Replace::commandWord)
				{
					namespace thisNs = thisNs::Replace;
					QString currentText { TextConstant::GetTextConstVal(words[thisNs::indexCurrentText]) };
					QString newText { TextConstant::GetTextConstVal(words[thisNs::indexNewText]) };
					auto elements = MyQDom::GetAllLevelElements(element);
					for(auto &element:elements)
					{
						MyQDom::ReplaceInAttributes(element, currentText, newText);
					}
				}
				else Logs::ErrorSt(GetClassName() + "::ImportModification unrealesed word ["+words[thisNs::modifTypeIndex]+"]");
			}
			else Logs::ErrorSt(GetClassName() + "::ImportModification command ["+command+"] not passed check");
		}
		else Logs::ErrorSt(GetClassName() + "::ImportModification words.empty()");
	}
}

bool Config::ResolveCopyPreviousElement(QDomElement & importElement)
{
	namespace thisNs = ImportModification::copyPreviousElement;
	QString modification = importElement.attribute(Import_ns::modification);
	auto commands = Code::TextToCommands(modification);
	if(commands.size() == thisNs::copyPreviousElementMinCommandsSize)
	{
		auto countWords = Code::CommandToWords(commands[thisNs::commandCountIndex]);
		auto replaceWords = Code::CommandToWords(commands[thisNs::commandReplaceIndex]);
		if(countWords[thisNs::countKeyWordIndex] == thisNs::countKeyWord  && countWords.size() == thisNs::countSize
				&& replaceWords[thisNs::replaceKeyWordIndex] == thisNs::replaceKeyWord)
		{
			int count = Code::CommandToWords(commands[thisNs::commandCountIndex])[thisNs::countIndex].toInt();
			if(count > 0)
			{
				if(replaceWords.size() == count + thisNs::replaceWordsCountPlusToCount)
				{
					auto replaceWords = Code::CommandToWords(commands[1]);
					QString replaceWhat = TextConstant::GetTextConstVal(replaceWords[thisNs::replaceWhatIndex]);
					QStringList replaceTo = replaceWords;
					for(int i=0; i<thisNs::replaceWordsCountPlusToCount; i++)
						replaceTo.removeFirst();
					for(auto &str:replaceTo) str = TextConstant::GetTextConstVal(str);

					if(replaceTo.size() == count)
					{
						QDomElement importingElement = importElement.previousSibling().toElement();
						if(!importingElement.isNull())
						{
							for(int coping_i=0; coping_i<count; coping_i++)
							{
								QDomElement newElement = importingElement.cloneNode(true).toElement();
								if(!newElement.isNull())
								{
									QDomElement newElementInsetred =
											importingElement.parentNode().insertBefore(newElement,importElement).toElement();
									if(!newElementInsetred.isNull())
									{
										MyQDom::ReplaceInAttributes(newElementInsetred, replaceWhat,replaceTo[coping_i]);
									}
									else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement err 1.6 ["+modification +"]");
								}
								else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement err 1.5 ["+modification +"]");
							}
							importElement.parentNode().removeChild(importElement);
							return true;
						}
						else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification 1.4 ["+modification +"]");
					}
					else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification 1.3 ["+modification +"]");
				}
				else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification 1.2 ["+modification +"]");
			}
			else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification 1.1 ["+modification +"]");
		}
		else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification 1 ["+modification +"]");

	}
	else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification ["+modification
					   +"] (count > 0) not passed");

	return false;
}

namespace forRCPEIC { int showsCount = 0; QString funcName = "ResolveCopyPreviousElementInConfig\n"; }

bool Config::ResolveCopyPreviousElementInConfig(IConfigItem * importElement)
{
	QString before;
	int &counterLog = forRCPEIC::showsCount;
	auto parentForLog = importElement->parent;
	counterLog--;
	if(counterLog > 0) before = forRCPEIC::funcName + parentForLog->ExportToXMLTag(true).ToXMLCode(2);

	namespace thisNs = ImportModification::copyPreviousElement;
	QString modification = importElement->Field(Import_ns::modification);
	QString policy = importElement->Field(Import_ns::policy);
	if(!policy.isEmpty() && policy != Import_ns::Policies::removeAfterCopying)
		Logs::WarningSt("wrong copyPreviousElement policy ["+policy+"]");
	bool removeAfterCopying = policy.contains(Import_ns::Policies::removeAfterCopying);

	auto commands = Code::TextToCommands(modification);
	if(commands.size() >= thisNs::copyPreviousElementMinCommandsSize)
	{
		auto commandCountWords = Code::CommandToWords(commands[thisNs::commandCountIndex]);
		if(commandCountWords[thisNs::countKeyWordIndex] == thisNs::countKeyWord  && commandCountWords.size() == thisNs::countSize)
		{
			int count = Code::CommandToWords(commands[thisNs::commandCountIndex])[thisNs::countIndex].toInt();
			if(count > 0)
			{
				auto importingElement = importElement->PreviousSibling();
				if(importingElement)
				{
					std::vector<IConfigItem*> newItemsInserted;
					for(int coping_i=0; coping_i<count; coping_i++)
					{
						std::shared_ptr<IConfigItem> newElement = importingElement->CloneItem(true);
						if(newElement)
						{
							IConfigItem* newElementInsetred = importElement->InsertSibling(newElement,IConfigItem::beforeMe);
							if(newElementInsetred)
							{
								newItemsInserted.push_back(newElementInsetred);
							}
							else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement err 1.6 ["+modification +"]");
						}
						else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement err 1.5 ["+modification +"]");
					}

					if((int)newItemsInserted.size() == count)
						ImportModificationInCopyPreviousElement(newItemsInserted, modification);
					else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement err 1.4.1 ["+modification +"]");

					if(removeAfterCopying) importingElement->RemoveMe();
					importElement->RemoveMe();
					if(counterLog > 0) MyQDialogs::ShowText(before + "\n\n" + parentForLog->ExportToXMLTag(true).ToXMLCode(2));
					return true;
				}
				else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification 1.4 ["+modification +"]");

			}
			else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification 1.1 ["+modification +"]");
		}
		else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement wrong modification 1 ["+modification +"]");

	}
	else Logs::ErrorSt(GetClassName() + "::ResolveCopyPreviousElement (commands.size() >= thisNs::copyPreviousElementMinCommandsSize) not passed;"
					   + " modification ["+modification+"]");

	if(counterLog > 0) MyQDialogs::ShowText(before + "\n\nafter nothing, error was");
	return false;
}

void Config::ImportModificationInCopyPreviousElement(std::vector<IConfigItem*> newItems, const QString &modification)
{
	namespace thisNs = ImportModification::copyPreviousElement;

	auto copyModificationCommands = Code::TextToCommands(modification);
	copyModificationCommands.removeAt(thisNs::countKeyWordIndex);

	for(auto &command:copyModificationCommands)
	{
		auto words = Code::CommandToWords(command);
		if(words.empty()) { Logs::ErrorSt("ImportModificationInCopyPreviousElement words.size() == 0"); return; }

		if(words[0] == thisNs::replaceKeyWord)
		{
			int sizeMustBe = newItems.size() + thisNs::replaceWordsCountPlusToCount;
			if(words.size() != sizeMustBe)
			{
				Logs::ErrorSt("ImportModificationInCopyPreviousElement wrong words size: "+QSn(words.size())+", but must me "
								+ QSn(sizeMustBe) + ";\nmodification: " + modification);
				return;
			}

			QString replaceWhat = TextConstant::GetTextConstVal(words[thisNs::replaceWhatIndex]);
			QStringList replaceTo = words;
			for(int i=0; i<thisNs::replaceWordsCountPlusToCount; i++)
				replaceTo.removeFirst();
			for(auto &str:replaceTo) str = TextConstant::GetTextConstVal(str);

			if(replaceTo.size() != (int)newItems.size())
			{
				Logs::ErrorSt(GetClassName() + "::ImportModificationInCopyPreviousElement wrong modification ["
							  + modification +"]");
				return;
			}

			for(uint i=0; i<newItems.size(); i++)
			{
				newItems[i]->ReplaceInFields(replaceWhat,replaceTo[i]);
			}
		}
		else Logs::ErrorSt("ImportModificationInCopyPreviousElement wrong modification ["+words[0]+"] "
				+ modification);
	}
}

void Config::RemoveExtraDataFromXMLForRun(QDomDocument & xmlDoc)
{
	std::vector<QDomElement> elements = MyQDom::GetAllLevelElements(xmlDoc);

	for(int i=(int)elements.size()-1; i>=0; i--)
	{
		QString tagName = elements[i].tagName();
		QString attributeName = elements[i].attribute(Common::name);
		if(attributeName.startsWith("//"))
			elements[i].parentNode().removeChild(elements[i]);	// удаление элементов у которых атрибут имя начинается с //
		else if(tagName == ObjectClass_ns::ObjectClass)
			elements[i].parentNode().removeChild(elements[i]);	// удаление ObjectClass элементов
		else if(tagName == ProtocolClass_ns::ProtocolClass)
			elements[i].parentNode().removeChild(elements[i]);	// удаление ProtocolClass элементов
		else if(tagName == WindowClass_ns::rowCaption)
			elements[i].parentNode().removeChild(elements[i]);	// удаление WindowClass элементов
		else {/* nothing to do */}
	}
}

