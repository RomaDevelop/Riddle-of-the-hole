#include <QApplication>
#include <QDebug>

#include "mainwindow.h"
#include "editor/editor.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//MainWindow w;
	//w.show();

	Editor e;
	e.show();

	return a.exec();
}
