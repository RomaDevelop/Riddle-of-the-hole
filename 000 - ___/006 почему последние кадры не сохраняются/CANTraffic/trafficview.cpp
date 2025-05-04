#include "trafficview.h"
#include "ui_trafficview.h"

#include <mutex>
#include <chrono>
#include <thread>
#include <memory>
#include <algorithm>
using namespace std;

#include <QMessageBox>
#include <QCheckBox>
#include <QTextBlock>
#include <QLayout>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

#include "MyQShortings.h"
#include "MyQDifferend.h"

#include "cleanerthread.h"

namespace nsTrafficView
{
	vector<int> VectorChannelNumbers;

	struct FilterID
	{
		unsigned long filter {0};
		unsigned long mask {0};
		bool enabled {false};
	} FilterID;

	CANMsgDataFilter_t FilterData;
	bool DataEnabled = false;

	QString chCheckBoxPreffix {"ch_"};

	TrafficView *thisObject {nullptr}; // указатель на самого себя для развязки статики
	CleanerThread *cleanerThread1 {nullptr};

	vector<QWidget*> notActive;
}
using namespace nsTrafficView;

TrafficView::TrafficView(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TrafficView)
{
	ui->setupUi(this);
	thisObject = this;
	move(900,30);

	notActive.push_back(ui->groupBoxCommon);
	notActive.push_back(ui->groupBoxThresholds);

	connect(this,&TrafficView::SignalMessage,this,&TrafficView::SlotMessage);

	connect(ui->lineEditFilterID,&QLineEdit::textChanged,this,&TrafficView::SlotIDFilterSet);
	connect(ui->lineEditMaskID,&QLineEdit::textChanged,this,&TrafficView::SlotIDFilterSet);
	connect(ui->checkBoxFilterID,&QCheckBox::clicked,this,&TrafficView::SlotIDFilterSet);

	connect(ui->pushButtonFilterID0,&QPushButton::clicked,this,&TrafficView::SlotIDFilterEditsSet);
	connect(ui->pushButtonFilterID1,&QPushButton::clicked,this,&TrafficView::SlotIDFilterEditsSet);
	connect(ui->pushButtonMaskID0,&QPushButton::clicked,this,&TrafficView::SlotIDFilterEditsSet);
	connect(ui->pushButtonMaskID1,&QPushButton::clicked,this,&TrafficView::SlotIDFilterEditsSet);

	connect(ui->radioButtonFilterID2,&QRadioButton::clicked,this,&TrafficView::SlotIDFilterConvert);
	connect(ui->radioButtonFilterID10,&QRadioButton::clicked,this,&TrafficView::SlotIDFilterConvert);
	connect(ui->radioButtonFilterID16,&QRadioButton::clicked,this,&TrafficView::SlotIDFilterConvert);

	connect(ui->lineEditDataFilter,&QLineEdit::textChanged,this,&TrafficView::SlotDataFilterSet);
	connect(ui->lineEditDataMask,&QLineEdit::textChanged,this,&TrafficView::SlotDataFilterSet);
	connect(ui->checkBoxDataFilter,&QCheckBox::clicked,this,&TrafficView::SlotDataFilterSet);

	connect(ui->pushButtonDataFilter0,&QPushButton::clicked,this,&TrafficView::SlotDataFilterEditsSet);
	connect(ui->pushButtonDataFilter1,&QPushButton::clicked,this,&TrafficView::SlotDataFilterEditsSet);
	connect(ui->pushButtonDataMask0,&QPushButton::clicked,this,&TrafficView::SlotDataFilterEditsSet);
	connect(ui->pushButtonDataMask1,&QPushButton::clicked,this,&TrafficView::SlotDataFilterEditsSet);

	connect(ui->radioButtonDataFilter2,&QRadioButton::clicked,this,&TrafficView::SlotDataFilterConvert);
	connect(ui->radioButtonDataFilter10,&QRadioButton::clicked,this,&TrafficView::SlotDataFilterConvert);
	connect(ui->radioButtonDataFilter16,&QRadioButton::clicked,this,&TrafficView::SlotDataFilterConvert);

	getTraffic = true;
}

TrafficView::~TrafficView()
{
	if(cleanerThread1) delete cleanerThread1;
	delete ui;
}

void TrafficView::CreateChannelCheckBoxes()
{
	if(!ui->groupBox->layout()) ui->groupBox->setLayout(new QHBoxLayout(ui->groupBox));
	else
	{
		int count = ui->groupBox->layout()->count();
		for(int i=0; i<count; i++)
			ui->groupBox->layout()->itemAt(i)->widget()->deleteLater();  // удаление если были ранее созданные QCheckBox-ы
	}

	vector<int> chs { traffic->GetChNumbers() };
	for(auto ch:chs)
	{
		QCheckBox *chBox = new QCheckBox();
		chBox->setObjectName(chCheckBoxPreffix + QSn(ch));
		chBox->setText(chBox->objectName());
		chBox->setChecked(true);
		connect(chBox,&QCheckBox::clicked,this,&TrafficView::SlotChannelFilterSet);
		ui->groupBox->layout()->addWidget(chBox);
	}
}

bool TrafficView::CheckFilters(const nsTraffic::TrafficMsg &msg)
{
	if(!VectorChannelNumbers.empty())
	{
		bool filterCheck = false;
		for(auto f:VectorChannelNumbers)
			if(msg.chNumberAbsolut == f)
			{
				filterCheck = true;
				break;
			}
		if(!filterCheck) return false;
	}

	if(FilterID.enabled)
		if(!msg.CheckFilter(FilterID.filter,FilterID.mask))
			return false;

	if(DataEnabled)
		if(!msg.CheckFilter(FilterData))
			return false;

	return true;
}

void TrafficView::SetupViewMsgs(nsTraffic::Traffic *traffic_, unsigned int maxRowsCount)
{
	traffic = traffic_;

	if(cleanerThread1) delete cleanerThread1;
	cleanerThread1 = new CleanerThread(ui->textBrowser, maxRowsCount);

	CreateChannelCheckBoxes();
	ui->lineEditThreshView->setText(QSn(cleanerThread1->GetLinesThreshold()));
	ui->lineEditThreshWr->setText(QSn(traffic->GetThresholdWriting()));

	traffic->SetViewerCB([](const nsTraffic::TrafficMsg &msg)
	{
		if(thisObject->getTraffic)
		{
			if(thisObject->CheckFilters(msg))
				emit thisObject->SignalMessage(msg.ToStrTraffic());
		}
	});
}

void TrafficView::StopView()
{
	traffic->SetViewerCB(nullptr);
}

void TrafficView::SlotMessage(const QString &msg_str)
{
	ui->textBrowser->append(msg_str);
//	if(ui->checkBoxAutoScrolling->isChecked())
//	{
//		QTextCursor cursor = ui->textBrowser->textCursor();
//		cursor.movePosition(QTextCursor::End);
//		cursor.movePosition(QTextCursor::StartOfLine);
//		ui->textBrowser->setTextCursor(cursor);
//	}
}

void TrafficView::SuspendGetTraffic()
{
	getTraffic = false;
}
void TrafficView::ContinueGetTraffic()
{
	getTraffic = true;
}

void TrafficView::SlotChannelFilterSet()
{
	VectorChannelNumbers.clear();
	bool allChecked = true;

	int count = ui->groupBox->layout()->count();
	for(int i=0; i<count; i++)
	{
		QCheckBox *chBox = (QCheckBox*)ui->groupBox->layout()->itemAt(i)->widget();
		if(chBox->isChecked())
			VectorChannelNumbers.push_back(chBox->objectName().remove(0,chCheckBoxPreffix.length()).toInt());
		else allChecked = false;
	}
	if(allChecked) VectorChannelNumbers.clear();
}

void TrafficView::SlotIDFilterSet()
{
	FilterID.enabled = false;

	int base = 2;
	if(ui->radioButtonFilterID10->isChecked()) base = 10;
	else if(ui->radioButtonFilterID16->isChecked()) base = 16;

	QString filter = ui->lineEditFilterID->text();
	QString mask = ui->lineEditMaskID->text();
	filter.replace(" ","");
	mask.replace(" ","");

	bool resF, resM;
	FilterID.filter = filter.toULong(&resF, base);
	FilterID.mask = mask.toULong(&resM, 2);

	if(sender()->objectName() != ui->checkBoxFilterID->objectName()) ui->checkBoxFilterID->setCheckState(Qt::Unchecked);
	else if(ui->checkBoxFilterID->isChecked())
	{
		if(!(resF && resM))
		{
			ui->checkBoxFilterID->setCheckState(Qt::Unchecked);
			if(!resF) QMbw(this,"Ошибка","Недопустимые символы в фильтре!");
			if(!resM) QMbw(this,"Ошибка","Недопустимые символы в маске! Маска должна быть всегда в бинарном коде!");
		}
		else FilterID.enabled = true;
	}
}

void TrafficView::SlotIDFilterEditsSet()
{
	QString filter0 = "0000 0000 0000 0000 0000 0000 0000 0000";
	if(ui->radioButtonFilterID10->isChecked()) filter0 = "0 0 0 0 0 0 0 0 0 0";
	else if(ui->radioButtonFilterID16->isChecked()) filter0 = "0 0 0 0 0 0 0 0";

	QString filter1 = filter0;
	filter1.replace("0", "1");

	QString mask0 = "0000 0000 0000 0000 0000 0000 0000 0000";
	QString mask1 = mask0;
	mask1.replace("0", "1");

	if(sender()->objectName() == ui->pushButtonFilterID0->objectName()) ui->lineEditFilterID->setText(filter0);
	else if(sender()->objectName() == ui->pushButtonFilterID1->objectName()) ui->lineEditFilterID->setText(filter1);
	else if(sender()->objectName() == ui->pushButtonMaskID0->objectName()) ui->lineEditMaskID->setText(mask0);
	else if(sender()->objectName() == ui->pushButtonMaskID1->objectName()) ui->lineEditMaskID->setText(mask1);
	// при изменении lineEdid вызовется событие, которое запишет данные в trafficViewFilterID
}

void TrafficView::SlotIDFilterConvert()
{
	int newBase = 2;
	if(ui->radioButtonFilterID10->isChecked()) newBase = 10;
	else if(ui->radioButtonFilterID16->isChecked()) newBase = 16;

	QString newFilter = QSn(FilterID.filter,newBase);
	if(newBase == 2)  while(newFilter.length()<32) newFilter = "0" + newFilter;
	if(newBase == 10) while(newFilter.length()<10) newFilter = "0" + newFilter;
	if(newBase == 16) while(newFilter.length()<8)  newFilter = "0" + newFilter;

											// i>=1 специально, чтобы не вставлял в нулевой
	if(newBase == 2)  for(int i=newFilter.length()-1; i>=1; i--) if(i % 4 == 0) newFilter.insert(i," ");
	if(newBase == 10) for(int i=newFilter.length()-1; i>=1; i--) newFilter.insert(i," ");
	if(newBase == 16) for(int i=newFilter.length()-1; i>=1; i--) newFilter.insert(i," ");

	ui->lineEditFilterID->setText(newFilter);
	// при изменении lineEdid вызовется событие, которое запишет данные в trafficViewFilterID
}

void TrafficView::SlotDataFilterSet()
{
	DataEnabled = false;

	int base = 2;
	if(ui->radioButtonDataFilter10->isChecked()) base = 10;
	else if(ui->radioButtonDataFilter16->isChecked()) base = 16;

	QString filter = ui->lineEditDataFilter->text();
	QString mask = ui->lineEditDataMask->text();
	filter.replace(" ","");
	mask.replace(" ","");

	bool res = FilterData.FromStr(filter,base,mask);

	if(sender()->objectName() != ui->checkBoxDataFilter->objectName()) ui->checkBoxDataFilter->setCheckState(Qt::Unchecked);
	else if(ui->checkBoxDataFilter->isChecked())
	{
		if(!res)
		{
			ui->checkBoxDataFilter->setCheckState(Qt::Unchecked);
			if(!res) QMbw(this,"Ошибка","Недопустимые символы в фильтре или маске!");
		}
		else DataEnabled = true;
	}
}

void TrafficView::SlotDataFilterEditsSet()
{
	QString filter0 = "00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000";
	if(ui->radioButtonDataFilter10->isChecked()) filter0 = "000 000 000 000 000 000 000 000";
	else if(ui->radioButtonDataFilter16->isChecked()) filter0 = "00 00 00 00 00 00 00 00";

	QString filter1 = filter0;
	filter1.replace("0", "1");

	QString mask0 = "00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000";
	QString mask1 = mask0;
	mask1.replace("0", "1");

	if(sender()->objectName() == ui->pushButtonDataFilter0->objectName()) ui->lineEditDataFilter->setText(filter0);
	else if(sender()->objectName() == ui->pushButtonDataFilter1->objectName()) ui->lineEditDataFilter->setText(filter1);
	else if(sender()->objectName() == ui->pushButtonDataMask0->objectName()) ui->lineEditDataMask->setText(mask0);
	else if(sender()->objectName() == ui->pushButtonDataMask1->objectName()) ui->lineEditDataMask->setText(mask1);
	// при изменении lineEdid вызовется событие, которое запишет данные в trafficViewFilterData
}

void TrafficView::SlotDataFilterConvert()
{
	int newBase = 2;
	if(ui->radioButtonDataFilter10->isChecked()) newBase = 10;
	else if(ui->radioButtonDataFilter16->isChecked()) newBase = 16;

	QString allFilter;
	int count = 8;
	for(int i=0; i<count; i++)
	{
		QString oneFilter = QSn(FilterData.filter[i],newBase);
		if(newBase == 2)  while(oneFilter.length()<8) oneFilter = "0" + oneFilter;
		if(newBase == 10) while(oneFilter.length()<3) oneFilter = "0" + oneFilter;
		if(newBase == 16) while(oneFilter.length()<2) oneFilter = "0" + oneFilter;
		if(i != count-1) oneFilter += " ";

		allFilter += oneFilter;
	}

	ui->lineEditDataFilter->setText(allFilter);
	// при изменении lineEdid вызовется событие, которое запишет данные в trafficViewFilterData
}

void TrafficView::on_checkBoxGetTraffic_clicked()
{
	if(((QCheckBox*)sender())->isChecked())
		ContinueGetTraffic();
	else SuspendGetTraffic();
}

void TrafficView::on_pushButtonThreshold_clicked()
{
	if(ui->lineEditThreshView->text().toUInt() && ui->lineEditThreshWr->text().toUInt())
	{
	cleanerThread1->SetLinesThreshold(ui->lineEditThreshView->text().toUInt());
	traffic->SetThresholdWriting(ui->lineEditThreshWr->text().toUInt());
	}
	else QMb(this,"Ошибка","Не верно заполнены данные!");
}

void TrafficView::on_checkBoxAutolean_clicked()
{
	if(((QCheckBox*)sender())->isChecked())
		cleanerThread1->ContinueClean();
	else cleanerThread1->SuspendClean();
}

class TrafficViewFile
{
public:
	shared_ptr<QFile> file;
	int version {0};
	int channel {-1};
	nsTraffic::TrafficMsg lastReadedMsg;

	TrafficViewFile(QString file_str): file {make_shared<QFile>(file_str)}
	{
		if(file->open(QIODevice::ReadOnly))
		{
			//определение версии
			QString version_str = file->readLine();
			if(version_str.indexOf("TrafficChannel::WriteMessagesToFile v001") != -1)
				version = 1;
			else
			{
				QMbw(nullptr, "Ошибка", "TrafficViewFile: Ошибка! Не удалось определить версию файла!");
				file->close();
			}

			// определение канала
			if(version == 1)
			{
				qdbg << "file->open == true" << "version == 1";
				qdbg << "";
				QString fileName = QFileInfo(file_str).fileName();
				fileName.remove(0,23);
				fileName.remove(fileName.length()-(4),4);
				channel = fileName.toInt();
			}
		}
		else QMbw(nullptr, "TrafficViewFile", "TrafficViewFile: Ошибка! Не удалось открыть файл для чтения траффика!");
	}
	~TrafficViewFile() { if(file->isOpen()) file->close(); };

	void ReadMsg() { lastReadedMsg.FromStrTraffic(file->readLine()); }
};

void TrafficView::ReadMsgsFromFiles(vector<QString> files_str)
{
	vector<nsTraffic::TrafficMsg> messages;

	vector<TrafficViewFile> files;
	for(const auto &f:files_str)
		files.push_back(f);

	vector<TrafficViewFile*> emptyFiles;
	vector<TrafficViewFile*> filesToRead;
	for(auto &f:files)
	{
		filesToRead.push_back(&f);
		f.ReadMsg();
	}

	while(!filesToRead.empty())
	{
		uint oldestMsg = 0;
		uint oldestMsgInd = 0;
		for(uint i=0; i<filesToRead.size(); i++)
		{
			if(oldestMsg >= filesToRead[i]->lastReadedMsg.msgNumber)
			{
				oldestMsg = filesToRead[i]->lastReadedMsg.msgNumber;
				oldestMsgInd = i;
			}
		}

		messages.push_back(filesToRead[oldestMsgInd]->lastReadedMsg); // записываем самое старое сообщение
		filesToRead[oldestMsgInd]->ReadMsg(); // читаем следующее
		if(filesToRead[oldestMsgInd]->file->atEnd())
		{
			filesToRead[oldestMsgInd]->file->close();
			filesToRead.erase(filesToRead.begin() + oldestMsgInd); // удаляем файл из списка если всё прочитали
		}
	}
}

void TrafficView::on_pushButtonOpen_clicked()
{
	QStringList files = QFileDialog::getOpenFileNames(
							this,
							"Select one or more files to open",
							mqd::GetPathToExe() + "/files/traffic",
							"Text files (*.txt)");
	ReadMsgsFromFiles(files.toVector().toStdVector());
}
