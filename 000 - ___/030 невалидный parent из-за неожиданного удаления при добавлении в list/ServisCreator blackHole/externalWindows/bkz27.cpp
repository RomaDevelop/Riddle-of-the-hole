#include "bkz27.h"

#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTableWidget>
#include <QSpacerItem>
#include <QSplitter>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>

using namespace std;

#include "code.h"

namespace Connect {
	const QString toObject {"toObject"};
	const QString toProtocol {"toProtocol"};

	const int typeObjToConnectIndex = 1;
	const int indexObjToConnectIndex = 2;
	const int catToConnectIndex = 3;
	const int nameToConnectIndex = 4;
	const int obrabotchikIndex = 5;

	const int wordsCount = 6;
}

BKZ27::BKZ27(QString connectString_, QString name_)
{
	name = name_;
	connectString = connectString_;

	window = make_unique<QWidget>();
	window->setWindowTitle("АРМ БКЗ-27");
	auto layOutMain = new QVBoxLayout(window.get());

	auto layOutHeader = new QHBoxLayout;
	layOutMain->addLayout(layOutHeader);
	ctrlElements.push_back(make_unique<QPushButton>("Рукопожатия Ок"));
	ctrlElements.back()->setObjectName(CreateStrConnectData(Connect::toProtocol,0, "БКЗ_27:1::Другое:6", "Рукопожатия Ок", "DoCommands(thisFrameWorker)"));
	layOutHeader->addWidget(ctrlElements.back().get());
	ctrlElements.push_back(make_unique<QPushButton>("Рукопожатия null"));
	ctrlElements.back()->setObjectName(CreateStrConnectData(Connect::toProtocol,0, "БКЗ_27:1::Другое:6", "Рукопожатия null", "DoCommands(thisFrameWorker)"));
	layOutHeader->addWidget(ctrlElements.back().get());
	layOutHeader->addStretch();

	tabWidget = make_unique<QTabWidget>();
	layOutMain->addWidget(tabWidget.get());

	QStringList tabNames { "МУП БКЗ", "МВДС 7-1", "МВДС 7-2", "МВДС 20-1", "МВДС 20-2", "МВДС 40" };

	QStringList cats1 {
		"БКЗ_27:1::МУП_БКЗ:0",
		"БКЗ_27:1::МВДС__7_1:1::МУП_МДК:1",
		"БКЗ_27:1::МВДС__7_2:2::МУП_МДК:1",
		"БКЗ_27:1::МВДС_20_1:3::МУП_МДК:1",
		"БКЗ_27:1::МВДС_20_2:4::МУП_МДК:1",
		"БКЗ_27:1::МВДС_40__:5::МУП_МДК:1"
	};
	QStringList cats2 {
		"",
		"БКЗ_27:1::МВДС__7_1:1::МДК:2",
		"БКЗ_27:1::МВДС__7_2:2::МДК:2",
		"БКЗ_27:1::МВДС_20_1:3::МДК:2",
		"БКЗ_27:1::МВДС_20_2:4::МДК:2",
		"БКЗ_27:1::МВДС_40__:5::МДК:2"
	};

	QStringList captionsSuffix1 {" МУП БКЗ",	" МУП МДК",	" МУП МДК",	" МУП МДК",	" МУП МДК",	" МУП МДК"};
	QStringList captionsSuffix2 {"",			" МДК",		" МДК",		" МДК",		" МДК",		" МДК"};

	QString captionRow;
	QString nameFrWorker;
	QString nameParam;
	QStringList btnCaptions;
	QStringList btnWorkers;

	for(int tabIndex=0; tabIndex<tabNames.size(); tabIndex++)
	{
		tabs.push_back(make_unique<QWidget>());
		QWidget *justCreatedTab = tabs.back().get();

		justCreatedTab->setObjectName(tabNames[tabIndex]);
		justCreatedTab->setAutoFillBackground(true);
		tabWidget->addTab(justCreatedTab,tabNames[tabIndex]);

		auto layOutTabMain = new QGridLayout(justCreatedTab);
		int row = layOutTabMain->rowCount();
		QStringList capts {"CAN 1 мк 1:", "CAN 1 мк 2:", "CAN 2 мк 1:", "CAN 2 мк 2:"};
		for(int i=0; i<4; i++)
		{
			layOutTabMain->addWidget(new QLabel(capts[i]),row,3+i,Qt::AlignCenter);
		}

		QString mdkLabels = "1010";
		QString labels = "1111";
		if(tabIndex==0) labels = "1001";

		captionRow = "Рукопожатие";
		nameFrWorker = "Рукопожатие запрос";
		nameParam = "Рукопожатие результат";
		btnCaptions = QStringList{"Запросить"};
		btnWorkers = QStringList{"SendFrame(thisFrameWorker)"};
						CreateRow(layOutTabMain, captionRow+captionsSuffix1[tabIndex], btnCaptions, btnWorkers, labels, cats1[tabIndex], nameFrWorker, nameParam);
		if(tabIndex>0)  CreateRow(layOutTabMain, captionRow+captionsSuffix2[tabIndex], btnCaptions, btnWorkers, mdkLabels, cats2[tabIndex], nameFrWorker, nameParam);

		captionRow = "Режим установить";
		nameFrWorker = "Режим установка";
		nameParam = "";
		btnCaptions = QStringList{"Сервис", "Загрузка ПО"};
		btnWorkers =	QStringList{"setRegim = режимСервис; SendFrame(thisFrameWorker)",
									"setRegim = режимЗагрузкаПО; SendFrame(thisFrameWorker)"};
						CreateRow(layOutTabMain, captionRow+captionsSuffix1[tabIndex], btnCaptions, btnWorkers, labels, cats1[tabIndex], nameFrWorker, nameParam);
		if(tabIndex>0)  CreateRow(layOutTabMain, captionRow+captionsSuffix2[tabIndex], btnCaptions, btnWorkers, mdkLabels, cats2[tabIndex], nameFrWorker, nameParam);

		captionRow = "Режим запросить";
		nameFrWorker = "Режим запрос";
		nameParam = "Режим";
		btnCaptions = QStringList{"Запросить"};
		btnWorkers = QStringList{"SendFrame(thisFrameWorker)"};
						CreateRow(layOutTabMain, captionRow+captionsSuffix1[tabIndex], btnCaptions, btnWorkers, labels, cats1[tabIndex], nameFrWorker, nameParam);
		if(tabIndex>0)  CreateRow(layOutTabMain, captionRow+captionsSuffix2[tabIndex], btnCaptions, btnWorkers, mdkLabels, cats2[tabIndex], nameFrWorker, nameParam);

		captionRow = "Номер блока";
		nameFrWorker = "Номер блока запрос";
		nameParam = "Номер блока";
		btnCaptions = QStringList{"Запросить"};
		btnWorkers = QStringList{"SendFrame(thisFrameWorker)"};
						CreateRow(layOutTabMain, captionRow+captionsSuffix1[tabIndex], btnCaptions, btnWorkers, labels, cats1[tabIndex], nameFrWorker, nameParam);

		captionRow = "Время наработки";
		nameFrWorker = "времяНаработкиЧтениеЗапрос";
		nameParam = "времяНаработки";
		btnCaptions = QStringList{"Запросить"};
		btnWorkers = QStringList{"SendFrame(thisFrameWorker)"};
		if(tabIndex==0)	CreateRow(layOutTabMain, captionRow+captionsSuffix1[tabIndex], btnCaptions, btnWorkers, labels, cats1[tabIndex], nameFrWorker, nameParam);

		captionRow = "Время наработки";
		nameFrWorker = "времяНаработкиСброс_Запрос";
		nameParam = "";
		btnCaptions = QStringList{" Отправить команду "};
		btnWorkers = QStringList{"SendFrame(thisFrameWorker)"};
		if(tabIndex==0)	CreateRow(layOutTabMain, captionRow+captionsSuffix1[tabIndex], btnCaptions, btnWorkers, labels, cats1[tabIndex], nameFrWorker, nameParam);

		layOutTabMain->setRowStretch(layOutTabMain->rowCount(),1);
	}
}

void BKZ27::ConnectToObjects(IConnectExternalWidgets * programm, std::vector<Object *> allObjects, std::vector<Protocol *> allProtocols)
{
	objects.clear();
	protocols.clear();
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
	if(size == 4 && size == (int)objects.size() && size == (int)protocols.size()) {/* all ok */}
	else
	{
		Error(GetClassName() + "::ConnectToObjects bad names ["+namesToConnect.join(";")+"] or objects or protocols");
		objects.clear();
		protocols.clear();
		return;
	}

	for(auto &ctrlElement:ctrlElements)
	{
		ConnectData connectData = GetConnectDataFromStr(ctrlElement->objectName(), ConnectData::Codes::ctrlWidget, ctrlElement.get());
		int res = programm->ConnectExternalWidget(connectData);
		if(res != IConnectExternalWidgets::noError)
		{
			Error(GetClassName() + "::ConnectToObjects result ConnectExternalWidget error ["+IConnectExternalWidgets::ErrorCodeToStr(res)+
				  "] connectData:\n" + connectData.ToStrForLog());
		}
	}

	for(auto &viewElement:viewElements)
	{
		ConnectData connectData = GetConnectDataFromStr(viewElement->objectName(), ConnectData::Codes::viewWidget, viewElement.get());
		int res = programm->ConnectExternalWidget(connectData);
		if(res != IConnectExternalWidgets::noError)
		{
			Error(GetClassName() + "::ConnectToObjects result ConnectExternalWidget error ["+IConnectExternalWidgets::ErrorCodeToStr(res)+
				  "] connectData:\n" + connectData.ToStrForLog());
		}
	}
}

void BKZ27::CreateRow(QGridLayout * layOutPlace, QString captionRow, QStringList btnCaptions, QStringList btnWorkers, QString labelsHasVals,
					  QString category, QString nameFrWorker, QString nameParam)
{
	int row = layOutPlace->rowCount();
	layOutPlace->addWidget(new QLabel(captionRow),row,0);

	if(btnCaptions.size() != btnWorkers.size())
	{
		Error(GetClassName() + "::CreateRow btnCaptions.size() != btnWorkers.size()");
		return;
	}
	if(labelsHasVals.size() != 4)
	{
		Error(GetClassName() + "::CreateRow labelsStartVals.size() != 4");
		return;
	}

	QStringList labelsStartVals;
	for(auto c:labelsHasVals)
	{
		if(c=='0') labelsStartVals += "";
		else if(c=='1') labelsStartVals += "null";
		else Error(GetClassName() + "::CreateRow wrong symbol in labelsHasVals ["+labelsHasVals+"]");
	}

	for(int i=0; i<btnCaptions.size(); i++)
	{
		ctrlElements.push_back(make_unique<QPushButton>(btnCaptions[i]));
		layOutPlace->addWidget(ctrlElements.back().get(),row,1+i);
		ctrlElements.back()->setObjectName(CreateStrConnectData(Connect::toProtocol,0,category,nameFrWorker,btnWorkers[i]));
	}

	if(nameParam.size())
	{
		for(int index=0; index<4; index++)
		{
			viewElements.push_back(make_unique<QLabel>(labelsStartVals[index]));
			viewElements.back()->setObjectName(CreateStrConnectData(Connect::toObject,index,category,nameParam,""));
			layOutPlace->addWidget(viewElements.back().get(),row,index+3,Qt::AlignCenter);
		}
	}
}



QString BKZ27::SaveSettings()
{
	QString str;
	auto geo = window->geometry();
	str = QSn(geo.x()) + ";" + QSn(geo.y()) + ";" + QSn(geo.width()) + ";" + QSn(geo.height()) + ";";
	return str;
}

void BKZ27::LoadSettings(const QString & str)
{
	QStringList stgs = str.split(";", QString::SkipEmptyParts);
	if(stgs.size() == 4)
	{
		window->setGeometry(stgs[0].toInt(),stgs[1].toInt(),stgs[2].toInt(),stgs[3].toInt());
	}
	else Error(GetClassName() + "::SetSettings wrong setting ["+str+"] size ("+QSn(stgs.size())+")");
}

QString BKZ27::CreateStrConnectData(QString objectOrProtocol, int index, QString cat, QString name, QString obrabotch)
{
	QString ret = "connect "+objectOrProtocol+" "+QSn(index)+" "
			+ CodeKeyWords::textConstantSplitter + cat + CodeKeyWords::textConstantSplitter + " "
			+ CodeKeyWords::textConstantSplitter + name + CodeKeyWords::textConstantSplitter + " "
			+ CodeKeyWords::textConstantSplitter + obrabotch + CodeKeyWords::textConstantSplitter;
	return ret;
}

ConnectData BKZ27::GetConnectDataFromStr(const QString &widgetObjectName, int widgetCat, QWidget *widgetPtr)
{
	ConnectData connectData;
	connectData.widgetCat = widgetCat;
	connectData.widgetPtr = widgetPtr;

	QString typeObjToConnectIndex;
	int index = -1;
	QString nameToConnect;
	auto commands = Code::TextToCommands(widgetObjectName);
	if(commands.size() == 1)
	{
		auto words = Code::CommandToWords(commands[0]);
		if(words.size() == Connect::wordsCount)
		{
			connectData.elementInObjectCat = TextConstant::GetTextConstVal(words[Connect::catToConnectIndex]);
			connectData.elementInObjectName = TextConstant::GetTextConstVal(words[Connect::nameToConnectIndex]);
			connectData.obrabotchik = TextConstant::GetTextConstVal(words[Connect::obrabotchikIndex]);

			typeObjToConnectIndex = words[Connect::typeObjToConnectIndex];
			bool check;
			index = words[Connect::indexObjToConnectIndex].toInt(&check);
			if(!check) Error(GetClassName() + "::GetConnectDataFromStr error getting index from str ["+widgetObjectName+"]");

			if(typeObjToConnectIndex == Connect::toObject)
			{
				if(index < (int)objects.size())
				{
					nameToConnect = objects[index]->name;
					connectData.objectPtr = objects[index];
				}
				else Error(GetClassName() + "::GetConnectDataFromStr bad index ("+QSn(index)+") from str ["+widgetObjectName+"]");
			}
			else if(typeObjToConnectIndex == Connect::toProtocol)
			{
				if(index < (int)protocols.size())
				{
					nameToConnect = protocols[index]->name;
					connectData.objectPtr = protocols[index];
				}
				else Error(GetClassName() + "::GetConnectDataFromStr bad index ("+QSn(index)+") from str ["+widgetObjectName+"]");
			}
			else Error(GetClassName() + "::GetConnectDataFromStr wrond typeObjToConnectIndex ["
					   +typeObjToConnectIndex+"] from str ["+widgetObjectName+"]");
		}
		else Error(GetClassName() + "::GetConnectDataFromStr wrong words count ("+QSn(words.size())+") from str ["+widgetObjectName+"]");
	}
	else Error(GetClassName() + "::GetConnectDataFromStr wrong commands count ("+QSn(commands.size())+") from str ["+widgetObjectName+"]");

	return connectData;
}
