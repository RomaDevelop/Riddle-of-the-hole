#include "customwidget.h"

#include <QApplication>
#include <QDebug>
#include <QPushButton>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	CustomWidget *w = new CustomWidget;
	w->resize(800,600);
	w->show();

	QWidget *inner1 = new QWidget(w);
	inner1->setStyleSheet("background-color: #FFFDD0; border: 1px solid black; padding: 5px;");
	inner1->setGeometry(10,10,100,100);
	inner1->show();

	QWidget *inner2 = new CustomWidget(w);
	inner2->setParent(w); // не помогло
	inner2->setWindowFlags(inner1->windowFlags());  // не помогло
	inner2->setStyleSheet("background-color: #FFFDD0; border: 1px solid black; padding: 5px;");
	//auto btn = new QPushButton("sdsd",inner2);
	inner2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	inner2->setGeometry(120,10,100,100);
	inner2->setMinimumSize(100,100);
	inner2->show();

	w->setObjectName("inner1");

	qDebug() << inner2->parent()->objectName() << inner2->sizeHint() << inner2->size();

	return a.exec();
}
