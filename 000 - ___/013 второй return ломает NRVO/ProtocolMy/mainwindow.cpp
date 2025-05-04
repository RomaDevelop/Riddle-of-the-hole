#include "mainwindow.h"

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

#include "icanchannel.h"

#include "constants.h"
#include "object.h"
#include "programm.h"

Programm *programm;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QDir dirHashedFiles = mqd::GetPathToExe() +"/files";
	if(!dirHashedFiles.exists()) dirHashedFiles.mkdir(dirHashedFiles.path());

	MyQExcelHasher hasher;
	int res = hasher.InitFile1(dirHashedFiles.path(), "F:/C++/CAN/ProtocolMy/Протокол1.xlsx");
	if(res != MyQExcelHasher::noError) qdbg << "Error MyQExcelHasher.InitFile1. Error" << MyQExcelHasher::ErrCodeToStr(res);
	IExcelWorkbook *wbPt = hasher.Get();

	programm = new Programm(wbPt);
}

MainWindow::~MainWindow()
{
	delete programm;
}

