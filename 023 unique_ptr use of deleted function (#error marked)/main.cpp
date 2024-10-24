#include <QApplication>
#include <QDebug>

#include "mainwindow.h"
#include "editor.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	bool oldCompile = true;
	oldCompile = false;
	if(oldCompile)
	{
		MainWindow w;
		if(w.showMainWindow) w.show();
	}

	Editor e;
	e.show();

	return a.exec();
}
