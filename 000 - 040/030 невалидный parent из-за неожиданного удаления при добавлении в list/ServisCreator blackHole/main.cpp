#include <QApplication>
#include <QDebug>

#include <iostream>

#include "editor.h"
#include "tests.h"
#include "bkz27psi.h"

#include "MyCppDifferent.h"
#include "MyQShortings.h"
#include "MyQDifferent.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	Editor e;
	e.show();

	Tests::DoTests();

	return a.exec();
}
