#include <vector>
using namespace std;

#include <QMessageBox>
#include <QAxObject>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>

#include "MyQExcel.h"
#include "MyQShortings.h"
#include "MyQShowText.h"
#include "MyQDifferend.h"
#include "MyQFileDir.h"

#include "icanchannel.h"

#include "constants.h"
#include "object.h"
#include "programm.h"
#include "mainwindow.h"

Programm *programm;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QString file = "F:/C++/CAN/ProtocolMy/Протокол1.xlsx";

	QString pathFiles = mqd::GetPathToExe()+"/files";
	if(!MQFD::CreatePath(pathFiles))
		QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла настроек, невозможно сохранить настройки");

	MyQExcelHasher hasher;
	int res = hasher.InitFile(pathFiles, file);
	if(res == MyQExcelHasher::noError)
	{
		IExcelWorkbook *wbPt = hasher.Get();

		programm = new Programm(wbPt);
	}
	else
	{
		qdbg << "Error MyQExcelHasher.InitFile1. Error" << MyQExcelHasher::ErrCodeToStr(res);
		showMainWindow = true;
	}
}

MainWindow::~MainWindow()
{
	delete programm;
}

