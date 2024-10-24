#include <QApplication>
#include <QDebug>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
	if(w.showMainWindow) w.show();

//#ifdef QT_NO_DEBUG
//	qDebug() << "QT_NO_DEBUG";
//#endif
//#ifdef QT_DEBUG
//	qDebug() << "QT_DEBUG";
//#endif

	return a.exec();
}
