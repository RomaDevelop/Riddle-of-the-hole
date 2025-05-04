#include <QApplication>
#include <QFile>

#include "MyQShortings.h"
#include "MyQDifferent.h"

#include "editor.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QString fileProjectToLoad = MyQDifferent::PathToExe() + "/files/projectToLoad.prtl";
	QString fileProjectToLoadContent;
	QFile file(fileProjectToLoad);
	if(file.open(QFile::ReadOnly))
	{
		QString content = file.readAll();
		file.close();

		QString absoletePath = content;
		QString otnositPath = MyQDifferent::PathToExe() + "/" + content;
		QString inFilesPath = MyQDifferent::PathToExe() + "/files/" + content;
		if(QFileInfo::exists(absoletePath)) fileProjectToLoadContent = absoletePath;
		if(QFileInfo::exists(otnositPath)) fileProjectToLoadContent = otnositPath;
		if(QFileInfo::exists(inFilesPath)) fileProjectToLoadContent = inFilesPath;
	} 
	else
	{
		QMbc(nullptr,"Error","Can't open file "+fileProjectToLoad);
		return 0;
	}

	if(!fileProjectToLoadContent.size())
	{
		QMbc(nullptr,"Error","!fileProjectToLoadContent.size()");
		return 0;
	}

	if(fileProjectToLoadContent.size())
	{
		Editor e;
		e.OpenFile(fileProjectToLoadContent);
		if(e.Run()) { e.notDelete = true; }
		else
		{
			QMbc(nullptr,"Ошибка","Ошибка при загрузке приложения");
			return 0;
		}
	}

	return a.exec();
}
