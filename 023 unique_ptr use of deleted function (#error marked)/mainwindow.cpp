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
#include "MyQDifferend.h"
#include "MyQFileDir.h"

#include "icanchannel.h"

#include "constants.h"
#include "object.h"
#include "programm.h"
#include "tests\tests.h"
#include "mainwindow.h"

Programm *programm = nullptr;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QString file = "C:/Work/C++/CAN/ProtocolMy/Протокол1.xlsx";

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
	else QMessageBox::information(this, "Ошибка", "Error MyQExcelHasher.InitFile. Error " + MyQExcelHasher::ErrCodeToStr(res));

	if(!programm || programm->IsProgrammShowsNothing())
	{
		CreateErrorLabel("");
		showMainWindow = true;
	}

	bool doTests = false;
	if(doTests)
	{
		Logs::LogSt("start tests in MainWindow::MainWindow(QWidget *parent)");
		Tests::TestCategory();
		Logs::LogSt("end tests in MainWindow::MainWindow(QWidget *parent)");
	}
}

MainWindow::~MainWindow()
{
	if(programm) delete programm;
}

