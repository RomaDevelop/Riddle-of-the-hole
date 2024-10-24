#include "includeparser.h"
#include "ui_includeparser.h"

#include <memory>
#include <algorithm>
#include <vector>
using namespace std;

#include <QLineEdit>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QDateTime>
#include <QMessageBox>
#include <QCheckBox>
#include <QVBoxLayout>

#include "MyQShortings.h"
#include "MyQStr.h"

#include "checkboxdialog.h"

QString dateFormat = "yyyy.MM.dd hh:mm:ss:zzz";
QString backupPath;
QColor colorNew(146,208,80);
QColor colorOld(255,180,180);

class FileItem
{
public:
	QFileInfo info;
	QTableWidgetItem *itemFile {nullptr};
	QTableWidgetItem *itemModif {nullptr};
	FileItem(const QFileInfo &info): info {info} {}
};

class FilesNew
{
public:
	QString name;
	QTableWidgetItem *item {nullptr};
	vector<FileItem> filesItems;
	bool needUpdate {false};

	FilesNew(const QFileInfo &info)
	{
		name = info.fileName();
		filesItems.push_back(info);
	}
};

vector<FilesNew> vectFiles;

int IndexOf(QString name)
{
	for(uint i=0; i<vectFiles.size(); i++)
		if(vectFiles[i].name == name) return i;
	return -1;
}

bool Check(const QStringList &chekList, QString val)
{
	for(auto c:chekList)
	{
		c.replace("\\","/");
		if(c != "" && val.toLower().indexOf(c.toLower()) != -1) return false;
	}
	return true;
}


bool CheckExt(const QStringList &chekList, QString val)
{
	for(auto c:chekList)
	{
		c.remove(" ");
		c.remove("*.");
		if(c != "" && val.right(c.length()).toLower() == c) return true;
	}
	return false;
}

void ScanFiles(const QStringList &dirsToScan, const QStringList &exts, const QStringList &fnameExept, const QStringList &pathExept, bool hideOneFile)
{
	vectFiles.clear();
	for(auto &d:dirsToScan)
	{
		QDir dir(d);

		//QDirIterator it(dir.path(), /*exts*/QStringList() << "*.dll", QDir::NoFilter, QDirIterator::Subdirectories);
		QDirIterator it(dir.path(), QStringList(), QDir::NoFilter, QDirIterator::Subdirectories);
		while (it.hasNext())
		{
			QFileInfo f(it.next());

			if(f.filePath().indexOf("build-") == -1)
			{
				if(Check(fnameExept,f.fileName()) && Check(pathExept,f.filePath()) && CheckExt(exts,f.fileName()))
				{
					int ind = IndexOf(f.fileName());
					if(ind == -1) vectFiles.push_back(f);
					else vectFiles[ind].filesItems.push_back(f);
				}
			}
		}
	}

	if(hideOneFile)
		for(int i=vectFiles.size()-1; i>=0; i--)
			if(vectFiles[i].filesItems.size() < 2)
				vectFiles.erase(vectFiles.begin() + i);

	sort(vectFiles.begin(),vectFiles.end(),[](FilesNew a, FilesNew b){
		return a.name < b.name;
	});
}

void PrintVectFiles(QTableWidget *table)
{
	table->setRowCount(0);
	table->clear();
	table->setColumnCount(3);
	table->setColumnWidth(0, table->width()*0.30);
	table->setColumnWidth(1, table->width()*0.50);
	table->setColumnWidth(2, table->width()*0.15);
	//while(1000 >= table->rowCount()) table->insertRow(0);

	int index=0;
	int indexRight = 0;
	for(auto &vf:vectFiles)
	{
		while(index >= table->rowCount()) table->insertRow(index);
		table->setItem(index, 0, new QTableWidgetItem(vf.name));
		vf.item = table->item(index,0);
		indexRight = index;
		index++;
		for(auto &f:vf.filesItems)
		{
			while(indexRight >= table->rowCount()) table->insertRow(indexRight);
			table->setItem(indexRight, 1, new QTableWidgetItem(f.info.filePath()));
			table->setItem(indexRight, 2, new QTableWidgetItem(f.info.lastModified().toString(dateFormat)));
			f.itemFile  = table->item(indexRight,1);
			f.itemModif = table->item(indexRight,2);
			indexRight++;
		}
		if(indexRight > index) index = indexRight;
	}

	// выделение цветом
	for(uint i=0; i<vectFiles.size(); i++)
	{
		int MaxInd = 0;
		for(uint j=0; j<vectFiles[i].filesItems.size(); j++)
		{
			if(vectFiles[i].filesItems[j].info.lastModified() > vectFiles[i].filesItems[MaxInd].info.lastModified()) MaxInd = j;
		}

		for(uint j=0; j<vectFiles[i].filesItems.size(); j++)
		{
			if(vectFiles[i].filesItems[j].info.lastModified() == vectFiles[i].filesItems[MaxInd].info.lastModified())
			{
				vectFiles[i].filesItems[j].itemFile->setBackgroundColor(colorNew);
				vectFiles[i].filesItems[j].itemModif->setBackgroundColor(colorNew);
			}
			else if(vectFiles[i].filesItems[j].info.lastModified() < vectFiles[i].filesItems[MaxInd].info.lastModified())
			{
				vectFiles[i].needUpdate = true;
				vectFiles[i].filesItems[j].itemFile->setBackgroundColor(colorOld);
				vectFiles[i].filesItems[j].itemModif->setBackgroundColor(colorOld);
			}
			else QMb(nullptr,"","Error kod 50001");
		}
	}
}

IncludeParser::IncludeParser(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::IncludeParser)
{
	ui->setupUi(this);
	this->move(30,30);

	toSave.push_back(ui->textEditScan);
	toSave.push_back(ui->lineEditExts);
	toSave.push_back(ui->textEditExeptFName);
	toSave.push_back(ui->textEditExeptFPath);
	toSave.push_back(ui->checkBoxHideIfOne);

	QDir pathBackup(QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files");
	if(!pathBackup.exists()) pathBackup.mkdir(pathBackup.path());
	pathBackup.setPath(pathBackup.path() + "/backup");
	if(!pathBackup.exists()) pathBackup.mkdir(pathBackup.path());
	if(!pathBackup.exists())
	{
		QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла резервного копирования. Резервные копии не будут сохраняться!");
		return;
	}
	backupPath = pathBackup.path();

	QFile file(QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files/settings.stgs");
	if(!file.exists()) QMessageBox::information(this, "Ошибка", "Отсутствует файл настроек "+file.fileName()+" Установлены по умолчанию.");
	else
	{
		QString Settings;
		file.open(QIODevice::ReadOnly);
		Settings = file.readAll();
		QString save_version = Settings.left(Settings.indexOf("[endSetting]"));
		if(save_version.indexOf("save_version 002") != -1)
		{
			QString splOneSetting = "[endSetting]";
			QString splInSetting = "[s;]";
			int count = Settings.count(splOneSetting);
			for(int i=1; i<count; i++)
			{
				QString setting = Settings.split(splOneSetting)[i];
				QString name = setting.split(splInSetting)[1];
				QString className = setting.split(splInSetting)[2];
				QString value = setting.split(splInSetting)[3];

				QWidget *component = this->findChild<QWidget*>(name);
				if(component!=NULL)
				{
					if(className=="QLineEdit") ((QLineEdit*)component)->setText(value);
					if(className=="QTextEdit") ((QTextEdit*)component)->setPlainText(value);
					if(className=="QCheckBox") ((QCheckBox*)component)->setChecked(value.toInt());
				}
			}
		}
		else QMb(this,"Ошибка чтения настроек.", "Не известная версия сохранения");
	}
}

IncludeParser::~IncludeParser()
{
	QDir pathSave(QFileInfo(QCoreApplication::applicationFilePath()).path() + "/files");
	if(!pathSave.exists()) pathSave.mkdir(pathSave.path());
	if(!pathSave.exists())
	{
		QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла настроек, невозможно сохранить настройки");
		return;
	}
	QFile file(pathSave.path() + "/settings.stgs");
	QString Settings = "save_version 002[endSetting]\n";
	for(unsigned int i=0; i<toSave.size(); i++)
	{
		QString class_name=toSave[i]->metaObject()->className();
		QString name=toSave[i]->objectName();

		if		(class_name=="QCheckBox") Settings += "toSave[s;]"+name+"[s;]"+class_name+"[s;]"
				+QString::number(static_cast<QCheckBox*>(toSave[i])->isChecked())+"[s;]";
		else if (class_name=="QLineEdit") Settings += "toSave[s;]"+name+"[s;]"+class_name+"[s;]"
				+static_cast<QLineEdit*>(toSave[i])->text()+"[s;]";
		else if (class_name=="QTextEdit") Settings += "toSave[s;]"+name+"[s;]"+class_name+"[s;]"
				+static_cast<QTextEdit*>(toSave[i])->toPlainText()+"[s;]";
		else qdbg << "Сохранение этого объекта не реализовано";
		Settings += "[endSetting]";
	}
	file.open(QIODevice::WriteOnly);
	file.write(Settings.toUtf8());
	delete ui;
}

void IncludeParser::on_pushButtonScan_clicked()
{
	ScanFiles(ui->textEditScan->toPlainText().split("\n"),
			  ui->lineEditExts->text().split(";"),
			  ui->textEditExeptFName->toPlainText().split("\n"),
			  ui->textEditExeptFPath->toPlainText().split("\n"),
			  ui->checkBoxHideIfOne->isChecked());

	PrintVectFiles(ui->tableWidget);
}

void replaceFile(QFileInfo &src, QFileInfo &dst)
{
	QFile fileToReplace(dst.filePath());
	QString backupFile = backupPath + "/" + QDateTime::currentDateTime().toString(dateFormat).replace(':','.') + " " + dst.fileName();
	if(!fileToReplace.copy(backupFile)) QMb(nullptr,"Ошибка","Не удалось создать backup-файл" + backupFile);
	else
	{
		if(!fileToReplace.remove()) QMb(nullptr,"Ошибка","Не удалось удалить файл " + fileToReplace.fileName());
		if(!QFile::copy(src.filePath(),dst.filePath())) QMb(nullptr,"Ошибка","Не удалось создать файл " + dst.fileName());
	}
}

void IncludeParser::on_tableWidget_cellDoubleClicked(int row, int column)
{
	QTableWidgetItem *item = ui->tableWidget->item(row, column);

	FilesNew *filesFind {nullptr};
	FileItem *fileFind {nullptr};

	for(uint i=0; i<vectFiles.size(); i++)
	{
		for(uint j=0; j<vectFiles[i].filesItems.size(); j++)
			if(vectFiles[i].filesItems[j].itemFile == item || vectFiles[i].filesItems[j].itemModif == item)
			{
				filesFind = &vectFiles[i];
				fileFind = &vectFiles[i].filesItems[j];
			}
	}

	if(!filesFind || !fileFind) return;

	QMessageBox messageBox(QMessageBox::Question, "Замена файлов", "Вы кликнули на файл " + fileFind->info.filePath() + ".\n\nЧто нужно сделать?");
	if(fileFind->itemFile->backgroundColor() == colorNew) //порядок кнопок
	{
		messageBox.addButton("Заменить им все аналогичные",QMessageBox::YesRole);
		messageBox.addButton("Заменить его новейшим",QMessageBox::YesRole);
	}
	else
	{
		messageBox.addButton("Заменить его новейшим",QMessageBox::YesRole);
		messageBox.addButton("Заменить им все аналогичные",QMessageBox::YesRole);
	}
	messageBox.addButton("Ничего",QMessageBox::NoRole);
	int desision =  messageBox.exec();

	if(messageBox.buttons()[desision]->text() == "Заменить им все аналогичные") // кликнутый копируем на место всех
	{
		QFileInfoList files;
		for(auto &f:filesFind->filesItems)
			files += f.info;
		QFileInfoList filesToReplace;

		// берем все файлы кроме нашаего
		for(auto &f:files)
			if(f.filePath() != fileFind->info.filePath())
				filesToReplace += f;

		// формруем QString с именами файлов, которые будут заменены
		QString filestpReplaceStr;
		for(auto &f:filesToReplace)
		{
			QString fileStrPlus = f.filePath();
			filestpReplaceStr += fileStrPlus + "    (" + f.lastModified().toString(dateFormat) + ")\n";
		}

		if(!filesToReplace.empty())
		{
			QString replaceFileStr = fileFind->info.filePath() + "    (" + fileFind->itemModif->text() + ")";
			if(QMessageBox::question(this, "Замена файлов", "Заменить файлы:\n" + filestpReplaceStr + "\nфайлом:\n" + replaceFileStr
									 + "?\n(Резервные копии будут сохранены)",
									 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
			{
				for(auto &f:filesToReplace)
				{
					replaceFile(fileFind->info, f);
				}
			}
		}
	}
	else if(messageBox.buttons()[desision]->text() == "Заменить его новейшим") // кликнутый заменяем самым новым
	{
		QFileInfoList files;
		for(auto &f:filesFind->filesItems)
			files += f.info;

		QFileInfo newestLastModifFI = files[0];
		for(auto &f:files)
			if(f.lastModified() > newestLastModifFI.lastModified())
				newestLastModifFI = f;

		// если наш файл новее новейшего - неправльно определён новейший
		if(fileFind->info.lastModified() > newestLastModifFI.lastModified()) QMb(this,"Ошибка","Ошибка desision == 1. Код ошибки 505");

		if(fileFind->info.lastModified() == newestLastModifFI.lastModified()) QMb(this,"Замена файлов", "Данный файл является новейшим");
		else
		{
			if(QMessageBox::question(this, "Замена файла", "Заменить файл:\n" + fileFind->info.filePath() + "\n\nфайлом:\n"
									 + newestLastModifFI.filePath() + "?\n(Резервные копии будут сохранены)",
									 QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
			{
				replaceFile(newestLastModifFI, fileFind->info);
			}
		}
	}
	else if(messageBox.buttons()[desision]->text() == "Ничего") ;  // Ничего
	else QMb(this,"Error","Error code 50002");

	on_pushButtonScan_clicked();
}

void IncludeParser::on_pushButtonMassUpdate_clicked()
{
	CheckBoxDialog *chDial = new CheckBoxDialog;
	QStringList values;

	for(auto &f:vectFiles)
		if(f.needUpdate)
			values += f.name;

	chDial->Execute(values);

	qdbg << chDial->GetCheckedValues().join(";");

	delete chDial;
}
