#include "bkz27psi.h"

#include <chrono>
using namespace std;

#include "MyCppDifferent.h"

#include <QGridLayout>

BKZ27_psi::BKZ27_psi(QString connectString_, QString name_, QWidget * parent):
	QWidget(parent)
{
	name = name_;
	connectString = connectString_;

	QFont tmpFont = font();
	tmpFont.setPointSize(10);
	setFont(tmpFont);

	setWindowTitle("БКЗ-27 ПСИ");
	setGeometry(50,50,1500,800);
	auto layOutMain = new QVBoxLayout(this);
	auto layOutRow1 = new QHBoxLayout;
	auto layOutRow2 = new QHBoxLayout;
	auto layOutRow3 = new QHBoxLayout;
	auto layOutRow4 = new QHBoxLayout;
	layOutMain->addLayout(layOutRow1);
	layOutMain->addLayout(layOutRow2);
	layOutMain->addLayout(layOutRow3);
	layOutMain->addLayout(layOutRow4);

	labelNomerBloka = new QLabel("1", this);
	layOutRow1->addWidget(new QLabel("Номер блока: ", this));
	layOutRow1->addWidget(labelNomerBloka);
	layOutRow1->addStretch();

	comboBoxNomerBloka = new QComboBox(this);
	comboBoxNomerBloka->addItems({"1","2","3","4","5","6","7","8","9","10","11"});
	QPushButton *btn = new QPushButton("Установить", this);
	connect(btn,&QPushButton::clicked,this,&BKZ27_psi::SlotChangeBlockNumber);
	layOutRow2->addWidget(new QLabel("Изменение номера блока: ", this));
	layOutRow2->addWidget(comboBoxNomerBloka);
	layOutRow2->addWidget(btn);
	layOutRow2->addStretch();

	connect(this,&BKZ27_psi::SignalPrint,this,&BKZ27_psi::SlotPrint);
	connect(this,&BKZ27_psi::SignalAnyCheckFinished,this,&BKZ27_psi::SlotAnyCheckFinished);

	btnStartCheckWorking = new QPushButton("Запустить проверку работоспособности", this);
	layOutRow3->addWidget(btnStartCheckWorking);
	connect(btnStartCheckWorking,&QPushButton::clicked,this,&BKZ27_psi::SlotCheckWorking);

	btnStartCheckLoading = new QPushButton("Контроль нагрузки", this);
	layOutRow3->addWidget(btnStartCheckLoading);
	connect(btnStartCheckLoading,&QPushButton::clicked,this,&BKZ27_psi::SlotCheckLoading);

	btnStartCheckPassport = new QPushButton("Считать паспорт", this);
	layOutRow3->addWidget(btnStartCheckPassport);
	connect(btnStartCheckPassport,&QPushButton::clicked,this,&BKZ27_psi::SlotCheckPassport);

	btnStartCheckBlNumberAndChRegims = new QPushButton("Проверка номера блока и режимов работы каналов", this);
	layOutRow3->addWidget(btnStartCheckBlNumberAndChRegims);
	connect(btnStartCheckBlNumberAndChRegims,&QPushButton::clicked,this,&BKZ27_psi::SlotCheckBlNumberAndChRegims);

	btnStartCheck8Hours = new QPushButton("Запустить 8-ми часовую проверку ", this);
	layOutRow3->addWidget(btnStartCheck8Hours);
	connect(btnStartCheck8Hours,&QPushButton::clicked,this,&BKZ27_psi::SlotCheckWorking8Hours);

	connect(this,&BKZ27_psi::SignalVklChan,this,&BKZ27_psi::SlotVklChan);
	connect(this,&BKZ27_psi::SignalCheckVklChan,this,&BKZ27_psi::SlotCheckVklChan);

	btnStopCheck = new QPushButton("Остановить проверку", this);
	btnStopCheck->setEnabled(false);
	layOutRow3->addWidget(btnStopCheck);
	QObject::connect(btnStopCheck,&QPushButton::clicked, this, &BKZ27_psi::SlotBtnStopCheck);

	layOutRow3->addStretch();

	textBrowser = new QTextBrowser(this);
	textBrowser->setFont(QFont("Courier new",12));
	layOutRow4->addWidget(textBrowser);
}

BKZ27_psi::~BKZ27_psi()
{
	if(StopTreadIfGoing())
		DeleteThreadIfExists();
}

bool BKZ27_psi::StopTreadIfGoing()
{
	stopThreadFlag = true;
	for(int i=0; i<30; i++)
	{
		if(!threadGoing) return true;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	Error(GetClassName()+"::StopTread timeout whait thread stop");
	return false;
}

bool BKZ27_psi::DeleteThreadIfExists()
{
	bool ret = true;
	if(treadPtr)
	{
		if(threadGoing)
		{
			Error(GetClassName()+"::DeleteThreadIfExists tryed delete going thread");
			ret = false;
		}
		else
		{
			delete treadPtr;
			treadPtr = nullptr;
		}
	}
	return ret;
}

void BKZ27_psi::Delay(int milliSec, int count, bool printPoint)
{
	for(int i=0; i<count; i++)
	{
		if(stopThreadFlag) { return; }
		this_thread::sleep_for(std::chrono::milliseconds(milliSec));
		if(stopThreadFlag) { return; }
		if(printPoint) emit SignalPrint(".",psiTextBrowserAddInLastRow);
	}
}

void BKZ27_psi::SlotCheckWorking()
{
	DisableStartBtnsEnableStopBtn();
	textBrowser->clear();
	textBrowser->append("Проверка работоспособности");

	if(!StopTreadIfGoing()) return;
	if(!DeleteThreadIfExists()) return;

	stopThreadFlag = false;
	treadPtr = new std::thread([this]
	{
		threadGoing = true;
		for(uint i=0; i<chansCountMustBe; i++)
		{
			if(stopThreadFlag) { threadGoing = false; return; }
			emit SignalVklChan(i);
			emit SignalPrint("\tОжидание результата",psiTextBrowser);
			Delay(300,5,true);
			if(stopThreadFlag) { threadGoing = false; return; }
			emit SignalCheckVklChan(i);
			this_thread::sleep_for(std::chrono::milliseconds(200));
			emit SignalPrint("",psiTextBrowser);
		}
		if(stopThreadFlag) { threadGoing = false; return; }
		emit SignalVklChan(-1);
		emit SignalPrint("\tОжидание результата",psiTextBrowser);
		Delay(300,5,true);
		if(stopThreadFlag) { threadGoing = false; return; }
		emit SignalCheckVklChan(-1);
		this_thread::sleep_for(std::chrono::milliseconds(200));
		if(stopThreadFlag) { threadGoing = false; return; }
		emit SignalPrint("",psiTextBrowser);
		emit SignalAnyCheckFinished();
		threadGoing = false;
	});
	treadPtr->detach();
}

string SecondsCountToStr(int seconds)
{
	string h, m, s;
	h = std::to_string(seconds/3600);
	m = std::to_string((seconds%3600) / 60);
	s = std::to_string(seconds%60);
	if(h.size() < 2) h = "0" + h;
	if(m.size() < 2) m = "0" + m;
	if(s.size() < 2) s = "0" + s;
	return h+":"+m+":"+s;
}

void BKZ27_psi::SlotCheckWorking8Hours()
{
	DisableStartBtnsEnableStopBtn();
	textBrowser->clear();
	textBrowser->append("Проверка работоспособности (8 часов)");

	if(!StopTreadIfGoing()) return;
	if(!DeleteThreadIfExists()) return;
	stopThreadFlag = false;
	treadPtr = new std::thread([this]
	{
		threadGoing = true;
		auto startTime = chrono::steady_clock::now();
		auto endTime = startTime + chrono::hours(8);
		while(endTime > chrono::steady_clock::now())
		{
			for(uint i=0; i<chansCountMustBe && endTime > chrono::steady_clock::now(); i++)
			{
				if(stopThreadFlag) { threadGoing = false; return; }
				emit SignalVklChan(i);
				emit SignalPrint("\tОжидание результата",psiTextBrowser);
				Delay(300,5,true);
				if(stopThreadFlag) { threadGoing = false; return; }
				emit SignalCheckVklChan(i);

				auto remainSeconds = std::chrono::duration_cast<std::chrono::seconds>(endTime - chrono::steady_clock::now()).count();
				QString timeRemain = SecondsCountToStr(remainSeconds).c_str();
				if(remainSeconds > 0) emit SignalPrint("\tОсталось " + timeRemain + "\n\n.", psiTextBrowser);

				Delay(500,15,true);
			}
		}
		if(stopThreadFlag) { threadGoing = false; return; }
		emit SignalVklChan(-1);
		emit SignalPrint("\tОжидание результата",psiTextBrowser);
		Delay(300,5,true);
		if(stopThreadFlag) { threadGoing = false; return; }
		emit SignalCheckVklChan(-1);
		this_thread::sleep_for(std::chrono::milliseconds(200));
		if(stopThreadFlag) { threadGoing = false; return; }
		emit SignalPrint("",psiTextBrowser);
		emit SignalAnyCheckFinished();
		threadGoing = false;
	});
	treadPtr->detach();
}

void BKZ27_psi::SlotAnyCheckFinished()
{
	textBrowser->append("Проверка завершена");
	EnableStartBtnsDisableStopBtn();
}

bool BKZ27_psi::DefineParamPtrs()
{
	auto tmpVectParamsSostUstr1 = objects[0]->FindParamsByName("Состояние устройства");
	auto tmpVectParamsSostUstr2 = objects[1]->FindParamsByName("Состояние устройства");

	auto tmpVectParamsNumber1 = objects[0]->FindParamsByName("Заводской номер");
	auto tmpVectParamsNumber2 = objects[1]->FindParamsByName("Заводской номер");
	auto tmpVectParamsVersion1 = objects[0]->FindParamsByName("Версия ПО");
	auto tmpVectParamsVersion2 = objects[1]->FindParamsByName("Версия ПО");

	auto paramsChansVkls1 = objects[0]->FindParamsByName("ВКЛ/ВЫКЛ");
	auto paramsChansVkls2 = objects[1]->FindParamsByName("ВКЛ/ВЫКЛ");

	auto paramsChansIspr1 = objects[0]->FindParamsByName("Исправность");
	auto paramsChansIspr2 = objects[1]->FindParamsByName("Исправность");

	auto paramsChansErrLoad1 = objects[0]->FindParamsByName("Отказ нагрузки");
	auto paramsChansErrLoad2 = objects[1]->FindParamsByName("Отказ нагрузки");

	auto tmpVectFramesVkls1 = protocols[0]->FindWorkers("вкл упр апериодический");
	auto tmpVectFramesVkls2 = protocols[1]->FindWorkers("вкл упр апериодический");

	auto tmpVectFramesDannIzd1 = protocols[0]->FindWorkers("Данные об изделии запрос");
	auto tmpVectFramesDannIzd2 = protocols[1]->FindWorkers("Данные об изделии запрос");

	if(tmpVectParamsSostUstr1.size() == 1 && tmpVectParamsSostUstr2.size() == 1
			&& tmpVectParamsNumber1.size() == 1 && tmpVectParamsNumber2.size() == 1
			&& tmpVectParamsVersion1.size() == 1 && tmpVectParamsVersion2.size() == 1
			&& paramsChansVkls1.size() == chansCountMustBe && paramsChansVkls2.size() == chansCountMustBe
			&& paramsChansIspr1.size() == chansCountMustBe && paramsChansIspr2.size() == chansCountMustBe
			&& paramsChansErrLoad1.size() == chansCountMustBe && paramsChansErrLoad2.size() == chansCountMustBe
			&& tmpVectFramesVkls1.size() == 1 && tmpVectFramesVkls2.size() == 1
			&& tmpVectFramesDannIzd1.size() == 1 && tmpVectFramesDannIzd2.size() == 1)
	{
		paramSostYstrCh1 = tmpVectParamsSostUstr1[0];
		paramSostYstrCh2 = tmpVectParamsSostUstr2[0];
		paramFabricNumber1 = tmpVectParamsNumber1[0];
		paramFabricNumber2 = tmpVectParamsNumber2[0];
		paramSoftVersion1 = tmpVectParamsVersion1[0];
		paramSoftVersion2 = tmpVectParamsVersion2[0];
		channelsVkls1 = paramsChansVkls1;
		channelsVkls2 = paramsChansVkls2;
		channelsIspr1 = paramsChansIspr1;
		channelsIspr2 = paramsChansIspr2;
		channelsErrorLoading1 = paramsChansErrLoad1;
		channelsErrorLoading2 = paramsChansErrLoad2;
		frWorkerUprChans1 = tmpVectFramesVkls1[0];
		frWorkerUprChans2 = tmpVectFramesVkls2[0];
		frWorkerZaprosDannixObIsdelii1 = tmpVectFramesDannIzd1[0];
		frWorkerZaprosDannixObIsdelii2 = tmpVectFramesDannIzd2[0];
		return true;
	}
	return false;
}

bool BKZ27_psi::ClearParamsValues(std::vector<Param *> params)
{
	vector<bool> ignores(protocols.size());
	vector<bool> mutes(protocols.size());
	for(uint i=0; i<protocols.size(); i++)
	{
		ignores[i] = protocols[i]->ignoreIncoming;
		mutes[i] = protocols[i]->muteOutgoing;
		protocols[i]->ignoreIncoming = true;
		protocols[i]->muteOutgoing = true;
	}

	for(uint i=0; i<params.size(); i++)
	{
		params[i]->SetParamValue(Value("",params[i]->GetParamValue().GetType()));
	}

	bool correct = true;
	for(uint i=0; i<params.size(); i++)
	{
		if(params[i]->GetParamValue().GetValue() != "")
			correct = false;
	}

	for(uint i=0; i<protocols.size(); i++)
	{
		protocols[i]->ignoreIncoming = ignores[i];
		protocols[i]->muteOutgoing = mutes[i];
	}

	return correct;
}

void BKZ27_psi::ConnectToObjects(IConnectExternalWidgets * programm_, std::vector<Object *> allObjects, std::vector<Protocol *> allProtocols)
{
	objects.clear();
	protocols.clear();
	programm = programm_;
	QStringList namesToConnect = Code::TextToCommands(connectString);
	for(auto &name:namesToConnect)
	{
		for(auto &obj:allObjects)
		{
			if(obj->name == name)
			{
				objects.push_back(obj);
				break;
			}
		}

		for(auto &prot:allProtocols)
		{
			if(prot->name == name)
			{
				protocols.push_back(prot);
				break;
			}
		}
	}

	int size = namesToConnect.size();
	if(size == 2 && size == (int)objects.size() && size == (int)protocols.size()) {/* all ok */}
	else
	{
		Error(GetClassName() + "::ConnectToObjects bad names ["+namesToConnect.join(";")+"] or objects or protocols");
		objects.clear();
		protocols.clear();
		return;
	}

	for(auto &ctrlElement:ctrlElements)
	{
		int res = programm->ConnectExternalWidget(ctrlElement.second);
		if(res != IConnectExternalWidgets::noError)
		{
			Error(GetClassName() + "::ConnectToObjects result ConnectExternalWidget error ["+IConnectExternalWidgets::ErrorCodeToStr(res)+
				  "] connectData:\n" + ctrlElement.second.ToStrForLog());
		}
	}

	for(auto &viewElement:viewElements)
	{
		int res = programm->ConnectExternalWidget(viewElement.second);
		if(res != IConnectExternalWidgets::noError)
		{
			Error(GetClassName() + "::ConnectToObjects result ConnectExternalWidget error ["+IConnectExternalWidgets::ErrorCodeToStr(res)+
				  "] connectData:\n" + viewElement.second.ToStrForLog());
		}
	}

	if(!DefineParamPtrs())
	{
		textBrowser->append("Ошибка определения параметров - проверки невозможны");
		DisableStartBtnsEnableStopBtn();
		btnStopCheck->setEnabled(false);
	}
}

void BKZ27_psi::closeEvent(QCloseEvent * event)
{
	windowGeo = QSn(x()) + " " + QSn(y()) + " " + QSn(width()) + " " + QSn(height());
	event->accept();
}

QString BKZ27_psi::SaveSettings()
{
	if(!windowGeo.size()) windowGeo = QSn(x()) + " " + QSn(y()) + " " + QSn(width()) + " " + QSn(height());
	return windowGeo;
}

void BKZ27_psi::LoadSettings(const QString & str)
{
	auto strList = str.split(" ",QString::SkipEmptyParts);
	if(strList.size() == 4)
	{
		move(strList[0].toInt(),strList[1].toInt());
		resize(strList[2].toInt(),strList[3].toInt());
	}
	else Warning("БКЗ-27 ПСИ не может установить настройки, установлены по умолчанию.");
}

void BKZ27_psi::DisableStartBtnsEnableStopBtn()
{
	btnStartCheckWorking->setEnabled(false);
	btnStartCheckLoading->setEnabled(false);
	btnStartCheckPassport->setEnabled(false);
	btnStartCheckBlNumberAndChRegims->setEnabled(false);
	btnStartCheck8Hours->setEnabled(false);
	btnStopCheck->setEnabled(true);
}

void BKZ27_psi::EnableStartBtnsDisableStopBtn()
{
	btnStartCheckWorking->setEnabled(true);
	btnStartCheckLoading->setEnabled(true);
	btnStartCheckPassport->setEnabled(true);
	btnStartCheckBlNumberAndChRegims->setEnabled(true);
	btnStartCheck8Hours->setEnabled(true);
	btnStopCheck->setEnabled(false);
}

void BKZ27_psi::SlotPrint(QString text, int type)
{
	if(0) {}
	else if(type == psiTextBrowser) textBrowser->append(text);
	else if(type == psiTextBrowserAddInLastRow)
	{
		auto cursor = textBrowser->textCursor();
		cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
		cursor.insertText(text);
	}
	else if(type == log) Log(text);
	else if(type == warning) Warning(text);
	else if(type == error) Error(text);
}

void BKZ27_psi::SlotChangeBlockNumber()
{
	textBrowser->clear();
	for(auto &prot:protocols)
	{
		QString constantName = "UNCurr";
		auto constant = prot->constants->FindConstant(constantName);
		if(constant)
		{
			if(constant->value.GetEnc() == Encodings::bin)
			{
				QString newNumberStr;
				bool check;
				uint newNumberUInt = comboBoxNomerBloka->currentText().toUInt(&check);
				if(check)
				{
					newNumberStr = QSn(newNumberUInt,2);
					while(newNumberStr.length() < (int)constant->value.GetBitLength())
						newNumberStr = "0" + newNumberStr;
					if(newNumberStr.length() <= (int)constant->value.GetBitLength())
					{
						prot->constants->SetConstant(constantName,newNumberStr);
						prot->InitFilters();
						textBrowser->append("Номер блока установен");
					}
					else textBrowser->append("Слишком большое значение в поле выбора");
				}
				else textBrowser->append("Ошибочные данные в поле выбора");
			}
			else textBrowser->append("Непредвиденная кодировка");

		}
		else textBrowser->append("Не найдена константа");
	}
	labelNomerBloka->setText(comboBoxNomerBloka->currentText());
}

void BKZ27_psi::SlotBtnStopCheck()
{
	textBrowser->append("\nОтавновка проверки\n");
	if(StopTreadIfGoing()) SlotAnyCheckFinished();
	else
	{
		textBrowser->append("Ошибка при остановке проверки");
		Error(GetClassName()+"::StopTreadIfGoing StopTreadIfGoing() returned false");
	}
}

void BKZ27_psi::SlotVklChan(int channelIndex)
{
	CANMsg_t frame;
	if(channelIndex >= 0 && channelIndex < (int)chansCountMustBe)
	{
		textBrowser->append("Отправлена команда управления каналами ("+QSn(channelIndex+1)+" - вкл, остальные - выкл)");
		EasyData ed(frame);
		ed.SetBitBi(channelIndex,1);
		ed.ToMsg(frame);
	}
	else
	{
		textBrowser->append("Отправлена команда управления каналами (все - выкл)");
	}
	auto commands = Code::TextToCommands((QString)
										 "thisFrameWorker.новыеСостояния = Data[][0-"+QSn(chansCountMustBe-1)+"];"+
										 "SendFrame(thisFrameWorker)");
	for(auto &command:commands)
	{
		protocols[0]->DoEverything(command, nullptr, frWorkerUprChans1, frame, nullptr, Protocol::DEretNullptr());
		protocols[1]->DoEverything(command, nullptr, frWorkerUprChans2, frame, nullptr, Protocol::DEretNullptr());
	}
}

QString SostUstr(QString str)
{
	if(str == "0") return "готов";
	if(str == "1") return "ошибка подключения";
	if(str == "3") return "не готов";
	return "непредвиденные данные ["+str+"]";
}

void BKZ27_psi::SlotCheckVklChan(int channelIndexMustBeVkl)
{
	QStringList errors;

	QString sostUstrItog;
	auto sostUstr1 = paramSostYstrCh1->GetParamValue().GetValue();
	auto sostUstr2 = paramSostYstrCh2->GetParamValue().GetValue();
	sostUstr1 = SostUstr(sostUstr1);
	sostUstr2 = SostUstr(sostUstr2);
	if(sostUstr1 != "готов") errors += "Состояние устройства по can 1 " + sostUstr1;
	if(sostUstr2 != "готов") errors += "Состояние устройства по can 2 " + sostUstr1;
	sostUstrItog += "Состояние устройства: can 1 - " + sostUstr1 + "; can 2 - " + sostUstr2;

	std::vector<int> wrongParamValues1, wrongParamValues2;
	std::vector<int> chansOn1, chansOff1,  chansOn2, chansOff2;
	std::vector<int> chansNeIspr1, chansNeIspr2;
	for(uint i=0; i<chansCountMustBe; i++)
	{
		auto paramValue1 = channelsVkls1.at(i)->GetParamValue().GetValue();
		if(paramValue1 == "1") chansOn1.push_back(i+1);
		else if(paramValue1 == "0") chansOff1.push_back(i+1);
		else wrongParamValues1.push_back(i+1);

		auto paramValue2 = channelsVkls2.at(i)->GetParamValue().GetValue();
		if(paramValue2 == "1") chansOn2.push_back(i+1);
		else if(paramValue2 == "0") chansOff2.push_back(i+1);
		else wrongParamValues2.push_back(i+1);

		QString isprVal = channelsIspr1.at(i)->GetParamValue().GetValue();
		if(isprVal != "1") chansNeIspr1.push_back(i+1);
		isprVal = channelsIspr2.at(i)->GetParamValue().GetValue();
		if(isprVal != "1") chansNeIspr2.push_back(i+1);
	}
	using mcd = MyCppDifferent;
	if(wrongParamValues1.size()) errors += QString("Непредвиденные состояния параметров can 1: ") + mcd::ToDiapasons(wrongParamValues1).c_str();
	if(wrongParamValues2.size()) errors += QString("Непредвиденные состояния параметров can 2: ") + mcd::ToDiapasons(wrongParamValues2).c_str();

	QString chansIsprItog;
	if(chansNeIspr1.size() || chansNeIspr2.size()) chansIsprItog = "Не все каналы исправны";
	else chansIsprItog = "Все каналы исправны";
	if(chansNeIspr1.size()) errors += QString("Не исправны каналы can 1: ") + mcd::ToDiapasons(chansNeIspr1).c_str();
	if(chansNeIspr2.size()) errors += QString("Не исправны каналы can 2: ") + mcd::ToDiapasons(chansNeIspr2).c_str();

	QString resOn1 = mcd::ToDiapasons(chansOn1).c_str();
	QString resOff1 = mcd::ToDiapasons(chansOff1).c_str();

	QString resOn2 = mcd::ToDiapasons(chansOn2).c_str();
	QString resOff2 = mcd::ToDiapasons(chansOff2).c_str();

	QString vklRes1 = "По данным от can 1: канал "+resOn1+" включен; каналы " + resOff1 + " выключены";
	QString vklRes2 = "По данным от can 2: канал "+resOn2+" включен; каналы " + resOff2 + " выключены";
	if(resOn1 == "") vklRes1.remove("канал  включен; ");
	if(resOn2 == "") vklRes2.remove("канал  включен; ");
	QString vklResItog;
	if(channelIndexMustBeVkl == -1)
	{
		if(true
				&& chansOn1.size() == 0
				&& chansOn2.size() == 0
				&& chansOff1.size() == chansCountMustBe
				&& chansOff2.size() == chansCountMustBe)
			vklResItog = "Состояние каналов соответсвует требованию";
		else
		{
			vklResItog = "Состояние каналов не соответсвует требованию";
			errors += vklResItog;
		}
	}
	else
	{
		if(true
				&& chansOn1.size() == 1
				&& chansOn2.size() == 1
				&& chansOn1[0] == channelIndexMustBeVkl+1
				&& chansOn2[0] == channelIndexMustBeVkl+1
				&& chansOff1.size() == chansCountMustBe-1
				&& chansOff2.size() == chansCountMustBe-1)
			vklResItog = "Состояние каналов соответсвует требованию";
		else
		{
			vklResItog = "Состояние каналов не соответсвует требованию";
			errors += vklResItog;
		}
	}

	textBrowser->append("\tРезультат:\n\t"+sostUstrItog+"\n\t"+chansIsprItog+"\n\t"+vklRes1+"\n\t"+vklRes2+"\n\t"+vklResItog);
	if(errors.empty()) textBrowser->append("\tОшибки отсутствуют");
	else
	{
		textBrowser->append("\tОбнаружены ошибки:");
		for(auto &error:errors) textBrowser->append("\t\t"+error);
	}
}

void BKZ27_psi::SlotCheckLoading()
{
	textBrowser->clear();
	textBrowser->append("Проверка контроля подключения нагрузки:");
	if(!ClearParamsValues(channelsErrorLoading1) || !ClearParamsValues(channelsErrorLoading2))
	{
		textBrowser->append("Ошибка установки параметров");
		SlotAnyCheckFinished();
		return;
	}

	QTimer::singleShot(1500,this,[this](){
		std::vector<int> wrongParamValues1, wrongParamValues2;
		std::vector<int> hasErr1, noErr1, hasErr2, noErr2;
		for(uint i=0; i<chansCountMustBe; i++)
		{
			auto paramValue1 = channelsErrorLoading1[i]->GetParamValue().GetValue();
			if(paramValue1 == "1") hasErr1.push_back(i+1);
			else if(paramValue1 == "0") noErr1.push_back(i+1);
			else wrongParamValues1.push_back(i+1);

			auto paramValue2 = channelsErrorLoading2[i]->GetParamValue().GetValue();
			if(paramValue2 == "1") hasErr2.push_back(i+1);
			else if(paramValue2 == "0") noErr2.push_back(i+1);
			else wrongParamValues2.push_back(i+1);
		}

		QString hasErrStr1 = MyCppDifferent::ToDiapasons(hasErr1).c_str();
		QString hasErrStr2 = MyCppDifferent::ToDiapasons(hasErr2).c_str();
		QString noErrStr1 = MyCppDifferent::ToDiapasons(noErr1).c_str();
		QString noErrStr2 = MyCppDifferent::ToDiapasons(noErr2).c_str();

		QString res;
		res += "\tПо данным основного канала: ";
		if(hasErrStr1.size()) res+= "\n\t\tимеется отказ нагрузки на канахах: " + hasErrStr1;
		if(noErrStr1.size()) res+= "\n\t\tотсутствует отказ нагрузки на канахах: " + noErrStr1;
		if(wrongParamValues1.size())
			res+= "\n\t\tОшибка! Неверное значение параетров: " + (QString)MyCppDifferent::ToDiapasons(wrongParamValues1).c_str();

		res += "\n\tПо данным резервного канала: ";
		if(hasErrStr2.size()) res+= "\n\t\tимеется отказ нагрузки на канахах: " + hasErrStr2;
		if(noErrStr2.size()) res+= "\n\t\tотсутствует отказ нагрузки на канахах: " + noErrStr2;
		if(wrongParamValues2.size())
			res+= "\n\t\tОшибка! Неверное значение параетров: " + (QString)MyCppDifferent::ToDiapasons(wrongParamValues2).c_str();

		textBrowser->append(res);
		textBrowser->append("");
		SlotAnyCheckFinished();
	});
}

void BKZ27_psi::SlotCheckPassport()
{
	DisableStartBtnsEnableStopBtn();
	textBrowser->clear();
	textBrowser->append("Отправка запросов данных об изделии");
	if(!ClearParamsValues({paramFabricNumber1}) || !ClearParamsValues({paramFabricNumber2}) ||
			!ClearParamsValues({paramSoftVersion1}) || !ClearParamsValues({paramSoftVersion2}))
	{
		textBrowser->append("Ошибка установки параметров");
		SlotAnyCheckFinished();
		return;
	}

	QTimer::singleShot(0,this,[this](){
		auto commands = Code::TextToCommands((QString)
											 "thisFrameWorker.запросНомера = 1;"+
											 "thisFrameWorker.запросВерсии = 0;"+
											 "SendFrame(thisFrameWorker);");
		for(auto &command:commands)
		{
			CANMsg_t framePustishka;
			protocols[0]->DoEverything(command, nullptr, frWorkerZaprosDannixObIsdelii1, framePustishka, nullptr, Protocol::DEretNullptr());
			protocols[1]->DoEverything(command, nullptr, frWorkerZaprosDannixObIsdelii2, framePustishka, nullptr, Protocol::DEretNullptr());
		}

		QTimer::singleShot(500,this,[this](){
			auto commands = Code::TextToCommands((QString)
											"thisFrameWorker.запросНомера = 0;"+
											"thisFrameWorker.запросВерсии = 1;"+
											"SendFrame(thisFrameWorker);");
			for(auto &command:commands)
			{
				CANMsg_t framePustishka;
				protocols[0]->DoEverything(command, nullptr, frWorkerZaprosDannixObIsdelii1, framePustishka, nullptr, Protocol::DEretNullptr());
				protocols[1]->DoEverything(command, nullptr, frWorkerZaprosDannixObIsdelii2, framePustishka, nullptr, Protocol::DEretNullptr());
			}

			textBrowser->append("Запросы данных об изделии отправлены, ожидание ответа");

			QTimer::singleShot(1500,this,[this](){
				textBrowser->append("Проверка результата:");
				QString number1 = paramFabricNumber1->GetParamValue().ToStr();
				QString number2 = paramFabricNumber2->GetParamValue().ToStr();
				QString versio1 = paramSoftVersion1->GetParamValue().ToStr();
				QString versio2 = paramSoftVersion2->GetParamValue().ToStr();
				if(number1 == "") number1 = "не определен";
				if(number2 == "") number2 = "не определен";
				if(versio1 == "") versio1 = "не определена";
				if(versio2 == "") versio2 = "не определена";
				textBrowser->append("\tпо данных основного  канала заводской номер " + number1);
				textBrowser->append("\tпо данным резервного канала заводской номер " + number2);
				textBrowser->append("\tпо данных основного  канала версия ПО " + versio1);
				textBrowser->append("\tпо данным резервного канала версия ПО " + versio2);
				textBrowser->append("");
				SlotAnyCheckFinished();
			});
		});
	});
}

void BKZ27_psi::SlotCheckBlNumberAndChRegims()
{
	DisableStartBtnsEnableStopBtn();
	textBrowser->clear();
	textBrowser->append("Проверка номера блока");
	if(!ClearParamsValues({paramSostYstrCh1}) || !ClearParamsValues({paramSostYstrCh2}))
	{
		textBrowser->append("Ошибка установки параметров");
		SlotAnyCheckFinished();
		return;
	}

	QTimer::singleShot(1500,this,[this](){
		if(paramSostYstrCh1->GetParamValue().GetValue() != "")
			textBrowser->append("\tНа основном  канале номер блока соответсвует выбранному (" + labelNomerBloka->text() + ")");
		else textBrowser->append("\tНа основном  канале номер блока не определён");
		if(paramSostYstrCh2->GetParamValue().GetValue() != "")
			textBrowser->append("\tНа резервном канале номер блока соответсвует выбранному (" + labelNomerBloka->text() + ")");
		else textBrowser->append("\tНа резервном канале номер блока не определён");

		textBrowser->append("\nПроверка режима работы каналов");
		QTimer::singleShot(100,this,[this](){
			std::vector<int> chansOn1, chansOff1,  chansOn2, chansOff2;
			for(uint i=0; i<chansCountMustBe; i++)
			{
				auto paramValue1 = channelsVkls1.at(i)->GetParamValue().GetValue();
				if(paramValue1 == "1") chansOn1.push_back(i+1);
				else if(paramValue1 == "0") chansOff1.push_back(i+1);

				auto paramValue2 = channelsVkls2.at(i)->GetParamValue().GetValue();
				if(paramValue2 == "1") chansOn2.push_back(i+1);
				else if(paramValue2 == "0") chansOff2.push_back(i+1);
			}
			QString resOn1 = MyCppDifferent::ToDiapasons(chansOn1).c_str();
			QString resOff1 = MyCppDifferent::ToDiapasons(chansOff1).c_str();

			QString resOn2 = MyCppDifferent::ToDiapasons(chansOn2).c_str();
			QString resOff2 = MyCppDifferent::ToDiapasons(chansOff2).c_str();

			QString vklRes1 = "\tНа основном  канале: канал "+resOn1+" включен; каналы " + resOff1 + " выключены;";
			QString vklRes2 = "\tНа резервном канале: канал "+resOn2+" включен; каналы " + resOff2 + " выключены;";
			if(resOn1 == "") vklRes1.remove("канал  включен; ");
			if(resOn2 == "") vklRes2.remove("канал  включен; ");
			if(resOff1 == "") vklRes1.remove("каналы  выключены;");
			if(resOff2 == "") vklRes2.remove("каналы  выключены;");
			if(resOn1 == "" && resOff1 == "") vklRes1 += "состояние не определено";
			if(resOn2 == "" && resOff2 == "") vklRes2 += "состояние не определено";
			textBrowser->append(vklRes1);
			textBrowser->append(vklRes2);
			textBrowser->append("");
			SlotAnyCheckFinished();
		});
	});
}

















