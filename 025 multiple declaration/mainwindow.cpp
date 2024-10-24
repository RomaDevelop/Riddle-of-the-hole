#include <vector>
#include <thread>
using namespace std;

#include <QMessageBox>
#include <QAxObject>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QFileInfo>
#include <QFile>
#include <QFileDialog>
#include <QDir>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QDebug>
#include <QTimer>

#include "MyQExcel.h"
#include "MyQShortings.h"
#include "MyQDifferend.h"
#include "MyQFileDir.h"

#include "icanchannel.h"
#include "caninterfaces.h"

#include "constants.h"
#include "object.h"
#include "programm.h"
#include "tests\tests.h"
#include "mainwindow.h"

namespace MainWindow_ns
{
CANInterfaces *interfaces;
Programm *programm = nullptr;

MyQExcelHasher hasher;
QString pathFiles;
QString file;
QStringList lastFiles;
QFile fileSettings;

QTimer *timerReadExcelStarter;

QListWidget *listWidgetLast;

std::thread *threadReadExcel;
int resReadExcel = MyQExcelHasher::unknown;
bool finishReadExcel = false;
}

using namespace MainWindow_ns;

MainWindow::MainWindow(QWidget *parent)
	: QDialog(parent)
{
	auto layOutMain = new QVBoxLayout(this);
	auto layOutTop = new QHBoxLayout;
	auto layOutMid = new QVBoxLayout;
	layOutMain->addLayout(layOutTop);
	layOutMain->addLayout(layOutMid);

#ifdef QT_DEBUG
	{
		auto btnOpenDefault = new QPushButton("Протокол1.xlsx");
		layOutTop->addWidget(btnOpenDefault);
		connect(btnOpenDefault,&QPushButton::clicked,[](){
			file = "C:/Work/C++/CAN/ProtocolMy/Протокол1.xlsx";
			listWidgetLast->clear();
			listWidgetLast->addItem("Ожидайте, идет чтение файла");
			timerReadExcelStarter->start();
		});
	}
#endif

	auto btnOpen = new QPushButton("Выбрать файл");
	layOutTop->addWidget(btnOpen);
	connect(btnOpen,&QPushButton::clicked,[](){
		file = QFileDialog::getOpenFileName(0, "Открыть", "", "*.xlsx");
		listWidgetLast->clear();
		listWidgetLast->addItem("Ожидайте, идет чтение файла");
		timerReadExcelStarter->start();
	});

	layOutTop->addStretch();

	pathFiles = mqd::GetPathToExe()+"/files";
	fileSettings.setFileName(pathFiles+"/settings.stgs");
	if(fileSettings.exists())
	{
		if(fileSettings.open(QFile::ReadOnly))
		{
			lastFiles = QString(fileSettings.readAll()).split("\n",QString::SkipEmptyParts);
			fileSettings.close();
		}
		else Logs::ErrorSt("fileSettings.open(QFile::ReadOnly)");

		for(int i=lastFiles.size()-1; i>=0; i--)
		{
			if(!QFile::exists(lastFiles[i])) lastFiles.removeLast();
		}
	}

	timerReadExcelStarter = new QTimer;
	timerReadExcelStarter->setInterval(100);
	connect(timerReadExcelStarter, &QTimer::timeout,[this](){
			ReadExcel();
			timerReadExcelStarter->stop();
		});

	listWidgetLast = new QListWidget;
	layOutMid->addWidget(new QLabel("Недавние файлы"));
	layOutMid->addWidget(listWidgetLast);
	listWidgetLast->addItems(lastFiles);
	listWidgetLast->setMinimumWidth(810);
	connect(listWidgetLast, &QListWidget::itemDoubleClicked, [](QListWidgetItem *item){
		file = item->text();
		listWidgetLast->clear();
		listWidgetLast->addItem("Ожидайте, идет чтение файла");
		timerReadExcelStarter->start();
	});
}

MainWindow::~MainWindow()
{
	if(programm) delete programm;
	if(interfaces) delete interfaces;
}

void MainWindow::ReadExcel()
{
	if(QFile(file).exists())
	{
		lastFiles.insert(0,file);
		lastFiles.removeDuplicates();
		while(lastFiles.size() > 10) lastFiles.removeLast();
		if(fileSettings.open(QFile::WriteOnly))
		{
			fileSettings.write(lastFiles.join("\n").toUtf8());
			fileSettings.close();
		}
		else Logs::ErrorSt("fileSettings.open(QFile::WriteOnly)");

		if(!MQFD::CreatePath(pathFiles))
			QMessageBox::information(this, "Ошибка", "Ошибка создания директории для файла настроек, невозможно сохранить настройки");


		QString msgsStr;

		connect(this,&MainWindow::SignalReadExcelFinished, this, &MainWindow::SlotReadExcelFinished);

		resReadExcel = hasher.InitFile(pathFiles, file/*, textBrowWhaitDescribtion*/, this);

		if(resReadExcel == MyQExcelHasher::noError)
		{
			IExcelWorkbook *wbPt = hasher.Get();

			interfaces = new CANInterfaces;
			programm = new Programm(wbPt, interfaces);
		}
		else QMessageBox::information(this, "Ошибка", "Error MyQExcelHasher.InitFile. Error " + MyQExcelHasher::ErrCodeToStr(resReadExcel));

		bool doTests = false;
		if(doTests)
		{
			Logs::LogSt("start tests in MainWindow::MainWindow(QWidget *parent)");
			Tests::TestCategory();
			Logs::LogSt("end tests in MainWindow::MainWindow(QWidget *parent)");
		}
	}

	this->close();

	if(!programm || programm->IsProgrammShowsNothing())
	{
		QWidget *errMsg = new QWidget;
		auto lo = new QHBoxLayout(errMsg);
		lo->addWidget(new QLabel("Ничего не было выбрано"));
		errMsg->show();
	}
}

void MainWindow::SlotReadExcelFinished()
{

}

