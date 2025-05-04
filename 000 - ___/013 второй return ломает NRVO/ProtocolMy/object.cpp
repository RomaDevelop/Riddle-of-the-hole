#include "object.h"

#include <memory>
using namespace std;

#include <QDebug>

#include "MyQShowText.h"
#include "MyQExcel.h"
#include "MyQShortings.h"


Object::Object(const std::vector<QStringList> &definedParams, const std::vector<QStringList> &definedWidgets, QString Version, QString name_, int objTip_, Constants *constants_, int channel_):
	name {name_},
	objectTip {objTip_},
	constants{constants_},
	channel{channel_}
{
	if(Version == "001")
	{
		int rowsSize = definedParams.size();
		int widgSize = definedWidgets.size();
		params.reserve(rowsSize);

		for(int i=0; i<rowsSize; i++)
		{
			params.push_back({definedParams[i], objectTip, constants});
		}

		InitPeremennie();

		for(int i=0; i<widgSize; i++)
		{
			Parametr *param = FindParamById(definedWidgets[i][1]);
			if(param) param->AddWidgetStrs(definedWidgets[i]);
			else Error("Protocol::Protocol Adiing widget, can't find param whith id" + definedWidgets[i][1] + " (widget row " + definedWidgets[i].join(' ') + ")");
		}

		if(params.size()) windowsCount =  params[0].strViewWidgs.size();

		timerMsgsWorker = new QTimer;
		QObject::connect(timerMsgsWorker, &QTimer::timeout, [this](){ this->MsgsWorker(); });
		timerMsgsWorker->start(100);
	}
}

void Object::InitFilters()
{
	for(auto &p:params)
		p.InitFilters();
}

void Object::InitPeremennie()
{
	for(uint i=0; i<params.size(); i++)
	{
		auto commandList {Compiler::TextToCommands(params[i].strPeremennie)};
		for(auto &command:commandList)
		{
			auto words {Compiler::CommandToWords(command)};
			if(words.size() >= 2)
			{
				QString tip {words[0]};
				QString peremName {words[1]};

				if(Compiler::IsNumber(peremName)) Error("InitPeremennie: wrong peremennaya name in command ["+command+"]");

				if(tip == ValuesTypes::paramPt)
				{
					params[i].peremennie.push_back(Value(peremName,nullptr));
					params[i].peremennieNames.push_back(peremName);
				}
				else if(ValuesTypes::all.contains(tip))
				{
					params[i].peremennie.push_back(Value(peremName,"",tip));
					params[i].peremennieNames.push_back(peremName);
				}
				else if(tip == ValuesTypes::vectorWord0)
				{
					tip += words[1] + words[2] + words[3];
					peremName = words[4];
					params[i].peremennie.push_back(Value(peremName,"",tip));
					params[i].peremennieNames.push_back(peremName);
				}
				else Error("InitPeremennie unrealesed type variable " + command);

				if(words.size() == 3)
				{
					void *ptr {nullptr};
					if(tip == ValuesTypes::paramPt)
					{
						ptr = FindParamByName(Compiler::GetInitialisationVal(command));
						params[i].peremennie.back().InitPt(peremName, ptr, ValuesTypes::paramPt);
					}
					else Error("InitPeremennie unrealesed initialisation " + command);
				}
			}
			else Error("InitPeremennie wrong words count " + command);
		}
	}
}

int Object::GetNumberFromNumberOperand(QString numberOperand, Parametr *param, const CANMsg_t &msg)
{
	QString name {Compiler::GetOperandName(numberOperand)};
	auto indexes {Compiler::GetAllIndexes(numberOperand)};


	if(indexes.secnd.empty())
	{
		if(name == "Data")
		{
			if(indexes.first.size() == 1)
				return msg.data[indexes.first[0]];
		}

		Value *peremennaya = param->FindPeremennaya(name,false);
		if(peremennaya != nullptr)
		{
			if(indexes.first.size() == 0)
			{
				return peremennaya->ToInt("");
			}
			if(indexes.first.size() == 1)
			{
				auto numbers { peremennaya->ToVectInt() };
				if((int)numbers.size() > indexes.first[0] && indexes.first[0] >= 0)
					return numbers[indexes.first[0]];
				else qdbg << "Protocol::GetNumberFromNumberOperand can't get number from " << numberOperand << "index = " << indexes.first[0] << "but value=" << peremennaya->IOGetValueAndTypeAndJoinPair();
			}
		}
	}

	qdbg << "Protocol::GetNumberFromNumberOperand can't get number from " << numberOperand;
	return 0;
}

std::vector<Parametr*> Object::FindParamsByName(const TextConstant &nameToFind)
{
	std::vector<Parametr*> findedParams;

	for(auto &p:params)
		if(p.name == nameToFind.Value()) findedParams.push_back(&p);

	if(findedParams.empty()) qdbg << "Protocol::FindParamsByName find nothing. Tryed to find" << nameToFind.Value();
	return findedParams;
}

Parametr *Object::FindParamById(const QString &id)
{
	Parametr* findedParam {nullptr};
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i].id == id)
		{ findedParam = &params[i]; break; }
	}

	return findedParam;
}

Parametr* Object::FindParamByName(const TextConstant &nameParametr)
{
	Parametr* findedParam {nullptr};
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i].name == nameParametr.Value())
		{ findedParam = &params[i]; break; }
	}

	return findedParam;
}

Parametr* Object::FindParamVModuleNomer(int nomerModule, const TextConstant &nameToFind)
{
	Parametr* findedParam {nullptr};

	int size = params.size();
	QString undmod  {"!!!undefinded!!!"};
	QString moduleName {undmod};
	for(int i=0; i<size; i++)
	{
		if(params[i].name == "Номер" && params[i].GetParamValue().ToInt("") == nomerModule)
			moduleName = params[i].module;
	}

	if(moduleName != undmod)
	{
		for(int i=0; i<size; i++)
		{
			if(params[i].name == nameToFind.Value() && params[i].module == moduleName)
			{ findedParam = &params[i]; break; }
		}
	}

	return findedParam;
}

Value *Object::FindPeremennayaVParametre(QString namePeremenn, const TextConstant &nameParametr)
{
	Parametr* param = FindParamByName(nameParametr);
	if(param) return param->FindPeremennaya(namePeremenn,false);
	else return nullptr;
}

void Object::SendFrame(const CANMsg_t &msg)
{
	if(server.empty() || (server.size() == 1 && server[0] == this))
	{
		Log("Protocol::SendMsg нет соединений ни с кем");
		Log("tryed to send: " + msg.ToStrEx2(2));
	}

	for(auto pr:server)
		if(pr != this)
		{
			pr->msgs.push(msg);
			Log("send: " + msg.ToStrEx2(2));
		}
}

void Object::MsgsWorker()
{
	for(uint i=0; i<msgs.size(); i++)
	{
		Log(" get: " + msgs.back().ToStrEx2(2));

		QString tipFrame;
		Parametr *parametr = CheckFilters(msgs.back(), tipFrame);

		if(parametr)
		{
			//qdbg << "CheckFilters tipFrame=" << tipFrame << parametr->name;
			if(tipFrame == FrameTypes::ctrlKadr) EmulatorWorkIncommingFrame(msgs.back(), parametr);
			if(tipFrame == FrameTypes::rqstKadr) EmulatorWorkIncommingFrame(msgs.back(), parametr);
			if(tipFrame == FrameTypes::infoKadr) ServisWorkIncommingFrame(msgs.back(), parametr);
			if(tipFrame == FrameTypes::rspnsKadr) ServisWorkIncommingFrame(msgs.back(), parametr);
		}

		msgs.pop();
	}
}

CANMsg_t Object::EmulatorConstructFrame(Parametr *param, int windowIndex)
{
	CANMsg_t msg;
	QStringList commandList {Compiler::TextToCommands(param->frame)};
	for(auto &command:commandList)
	{
		auto words {Compiler::CommandToWords(command)};

		if(words[0] == "ID:") msg.ID = param->outgoingID;
		else DoEverything(command, param, windowIndex, msg, Object::DEretNullptr());
//		else if(words[0] == "Сервис") {}
//		else if(words[0] == "Эмулятор")
//		{
//			if(words.size() == 6 && words[1] == "Data" && words[2] == Operators::assign && words[3] == "ПоискПараметров")
//			{
//				QString paramName {words[4]};
//				auto findedParams = FindParamsByName(paramName);
//				if(!findedParams.size()) qdbg << "Protocol::ConstructEmulatorFrameActual ПоискПараметров can't find param; paramName =" << paramName;
//				else
//				{
//					QString values;
//					for(auto &p:findedParams) values += p->GetValue().ToDecStr() + ";";
//					if(words[5] == "Побитово")
//						Value("", values, ValuesTypes::vectorBool).ToDataPobit(msg);
//				}
//			}
//			else if(words.size() == 7)
//			{
//				QString dataOperand = words[1];
//				QString oper = words[2];
//				QString poisk = words[3];
//				QString paramName = words[4];
//				QString moduleOperand = words[5];
//				QString sposob = words[6];
//				if(oper == Operators::assign && poisk == "ПоискПараметраВМодулеНомер" && sposob == "Побитово")
//				{
//					int number = GetNumberFromNumberOperand(moduleOperand, param, CANMsg_t());
//					Parametr* findedParam = FindParamVModuleNomer(number,paramName);
//					if(!findedParam) qdbg << "Protocol::EmulatorConstructFrame FindParamVModuleNomer finded nothing";
//					else
//					{
//						findedParam->GetValue().ToDataPobit(msg, dataOperand);
//					}
//				}
//				else qdbg << "Protocol::ConstructEmulatorFrameActual: команда не опознана:" << poisk;
//			}
//			else qdbg << "Protocol::ConstructEmulatorFrameActual: команда не опознана:" << command;
//		}
//		else qdbg << "Protocol::ConstructEmulatorFrameActual: команда не опознана:" << command;
	}
	return msg;
}

CANMsg_t Object::ServisConstructFrame(Parametr *param, int windowIndex)
{
	CANMsg_t msg;
	QStringList commandList {Compiler::TextToCommands(param->frame)};
	for(auto &command:commandList)
	{
		auto words {Compiler::CommandToWords(command)};

		if(words[0] == "ID:") msg.ID = param->outgoingID;
		else DoEverything(command, param, windowIndex, msg, Object::DEretNullptr());
	}
	return msg;
}

void Object::ServisWorkIncommingFrame(CANMsg_t &frame, Parametr *param)
{
	auto cmndList { Compiler::TextToCommands(param->frame) };
	for(auto &command:cmndList)
	{
		auto words {Compiler::CommandToWords(command)};
		if(words.empty()) { qdbg<<"Protocol::ServisWorkIncommingFrame words.empty()"; return; }

		if(command.indexOf("ID:") == 0) { /* тут мы ничаво не делаем */ }
		else if(words.size() && words[0] == "Эмулятор") { /* тут мы ничаво не делаем */ }
		else if(words.size() && words[0] == "Сервис")
		{
			DoEverything(command, param, noWindow, frame, Object::DEretNullptr());

//			if(words.size()>1 && words[1] == "ПоискПараметров")
//			{
//				if(words.size()==6 && words[3] == Operators::assign && words[4] == "Data" && words[5] == "Побитово")
//				{
//					QString newValue1 = words[5];

//					auto findedParams = FindParamsByName(words[2]);
//					if(!findedParams.size()) qdbg << "Protocol::ServisWorkIncommingFrame Установить ПоискПараметров can't find param; paramName =" << words[2];
//					else
//					{
//						std::vector<Value> values(findedParams.size());
//						Value::FromDataPobit(frame,values);
//						SetVectorParams(findedParams,values);
//					}
//				}
//				else qdbg << "Protocol::ServisWorkIncommingFrame wrong operation" << command;
//			}
//			else if(words.size() == 7 && words[1] == "Переменная" && words[3] == Operators::assign  && words[4] == "ПеременнаяВПараметре")
//			{
//				Value *peremToSet = param->FindPeremennaya(words[2],false);
//				Value *peremFrom { FindPeremennayaVParametre(words[5],words[6])};
//				if(!peremToSet) qdbg << "Protocol::ServisWorkIncommingFrame can't find FindPeremennaya in command " << command;
//				else if(!peremFrom) qdbg << "Protocol::ServisWorkIncommingFrame can't find FindPeremennayaVParametre in command " << command;
//				else peremToSet->Assign(*peremFrom);
//			}
//			else if(words.size() == 7 && words[1] == "ПоискПараметраВМодулеНомер" && words[4] == Operators::assign  && words[6] == "Побайтово")
//			{
//				QString nameParam = words[2];
//				QString numberOperand = words[3];
//				QString oper = words[4];
//				QString dataOperand = words[5];
//				QString sposob = words[6];

//				int number = GetNumberFromNumberOperand(numberOperand, param, frame);

//				Parametr *paramToSet = FindParamVModuleNomer(number, nameParam);
//				if(paramToSet != nullptr)
//				{
//					Value tmp;
//					tmp.FromDataUniversal(frame, dataOperand);
//					paramToSet->SetValue(tmp);
//				}
//				else qdbg << "Protocol::ServisWorkIncommingFrame FindParamVModuleNomer can't find" << command;
//			}
//			else qdbg << "Protocol::ServisWorkIncommingFrame wrong command" << command;
		}
		else qdbg << "Protocol::ServisWorkIncommingFrame wrong command" << command;
	}
}

void Object::EmulatorWorkIncommingFrame(CANMsg_t &frame, Parametr *parametrInicialiser)
{
	auto cmndList { Compiler::TextToCommands(parametrInicialiser->frame) };
	for(auto &command:cmndList)
	{
		auto words {Compiler::CommandToWords(command)};
		if(words.empty()) { qdbg<<"Protocol::EmulatorWorkIncommingFrame words.empty()"; return; }

		if(command.indexOf("ID:") == 0) { /* тут мы ничаво не делаем */ }
		else if(words[0] == "Сервис") { /* тут мы ничаво не делаем */ }
		else if(words[0] == "Эмулятор")
		{
			DoEverything(command, parametrInicialiser, noWindow, frame, Object::DEretNullptr());

//			didSometh = true;
//			if(words.size()>1 && words[1] == "ПоискПараметраВМодулеНомер")
//			{
//				if(words.size()==6 &&  words[4] == Operators::assign)
//				{
//					QString paramName = words[2];
//					int nomerModule = GetNumberFromNumberOperand(words[3],parametrInicialiser,frame);
//					int newValue1 = Value().FromDataPobyte(frame,words[5]).ToInt();

//					Parametr* findedParam = FindParamVModuleNomer(nomerModule,paramName);
//					if(!findedParam) qdbg << "Protocol::EmulatorWorkIncommingFrame ПоискПараметраВМодулеНомер can't find param; nomer module =" << nomerModule << "paramName =" << paramName;
//					else findedParam->SetValue(QSn(newValue1), ValuesTypes::binCode);
//				}
//				else qdbg << "Protocol::EmulatorWorkIncommingFrame wrong operation" << command;
//			}
//			else if(words.size()>1 && words[1] == "ПоискПараметров")
//			{
//				if(words.size()==6 && words[3] == Operators::assign && words[4] == "Data" && words[5] == "Побитово")
//				{
//					QString newValue1 = words[5];

//					auto findedParams = FindParamsByName(words[2]);
//					if(!findedParams.size()) qdbg << "Protocol::EmulatorWorkIncommingFrame Установить ПоискПараметров can't find param; paramName =" << words[2];
//					else
//					{
//						std::vector<Value> values(findedParams.size());
//						Value::FromDataPobit(frame,values);
//						SetVectorParams(findedParams,values);
//					}
//				}
//				else qdbg << "Protocol::EmulatorWorkIncommingFrame wrong operation" << command;
//			}
//			else if(words.size()>1 && words[1] == "ПеременнаяВПараметре")
//			{
//				if(words.size() == 7)
//				{
//					QString perememmayaName = words[2];
//					QString parametrName = words[3];
//					QString oper = words[4];
//					QString dataOperand = words[5];
//					QString sposob = words[6];

//					Value *valToSet { FindPeremennayaVParametre(perememmayaName,parametrName) };

//					if(valToSet)
//					{
//						if(oper == Operators::assign && sposob == "Побайтово")
//						{
//							valToSet->FromDataPobyte(frame, dataOperand);
//						}
//						else qdbg << "Protocol::EmulatorWorkIncommingFrame wrong command" << command;
//					}
//					else qdbg << "Protocol::EmulatorWorkIncommingFrame can't find param or peremennaya" << command;
//				}
//				else qdbg << "Protocol::EmulatorWorkIncommingFrame wrong word count in command" << command;
//			}
//			else if(words.size() == 3 && words[1] == "ОтправитьКадр")
//			{
//				Parametr* param = FindParamByName(words[2]);
//				if(param)
//				{
//					CANMsg_t msg { EmulatorConstructFrame(param, noWindow)};
//					SendMsg(msg);
//				}
//				else qdbg << "Protocol::EmulatorWorkIncommingFrame ОтправитьКадр can't find param" << command;
//			}
//			else qdbg << "Protocol::EmulatorWorkIncommingFrame wrong command" << command;
		}
		else qdbg << "Protocol::EmulatorWorkIncommingFrame wrong command" << command;
	}
}

Parametr* Object::CheckFilters(const CANMsg_t &msg, QString &tipFrame)
{
	for(uint i=0; i<params.size(); i++)
	{
		Parametr *p = &params[i];

		bool chFilts = false;
		int sizeFs = p->incomingFilters.size();
		for(int i=0; i<sizeFs; i++)
			if(msg.CheckFilter(p->incomingFilters[i])) { chFilts = true; break; }

		if(chFilts)
		{
			if(objectTip == Parametr::emulator && p->strType == FrameTypes::ctrlKadr) { tipFrame=FrameTypes::ctrlKadr; return p; }
			if(objectTip == Parametr::emulator && p->strType == FrameTypes::rqstKadr) { tipFrame=FrameTypes::rqstKadr; return p; }
			if(objectTip == Parametr::servis && p->strType == FrameTypes::infoKadr) { tipFrame=FrameTypes::infoKadr; return p; }
			if(objectTip == Parametr::servis && p->strType == FrameTypes::rspnsKadr) { tipFrame=FrameTypes::rspnsKadr; return p; }
		}
	}
	return nullptr;
}

void Object::CreateViewWidg(Parametr *param, int windowIndex)
{
	QStringList commands { Compiler::TextToCommands(param->strViewWidgs[windowIndex]) };
	for(auto cmnd:commands)
	{
		auto words { Compiler::CommandToWords(cmnd) };
		QString viewWidjType = words[0];

		if(viewWidjType == WidgPartTypes::label)
		{
			QLabel *lbl = new QLabel(param->GetParamValue().ToStr());
			param->viewWidgets[windowIndex].AddWidgetPart({lbl, "", cmnd, viewWidjType});
		}

		if(viewWidjType == WidgPartTypes::cell)
		{
			QTableWidgetItem *item = new QTableWidgetItem(param->GetParamValue().ToStr());
			param->viewWidgets[windowIndex].AddWidgetPart({item, "", "", WidgPartTypes::cell});
		}
	}
}

void Object::EmulatorConnectWidg(Parametr *param, int windowIndex)
{
	if(Compiler::TextToCommands(param->strCtrlWidgs[windowIndex]).empty()) return;

	Widget &curWidg = param->ctrlWidgs[windowIndex];

	if(curWidg.GetType() == WidgetTypes::chekBoxesNButton)
		qdbg << "Protocol::ConnectEmulatorWidg2 connection type unrealesed" << curWidg.GetType();
	else if(curWidg.GetType() == WidgetTypes::editButton)
	{
		QLineEdit *edit = (QLineEdit *)curWidg.parts[0].GetPtQWidget();
		QPushButton *btn = (QPushButton *)curWidg.parts[1].GetPtQWidget();
		QString senderName { curWidg.parts[1].GetName() };
		QObject::connect(btn, &QPushButton::clicked, [param, edit, windowIndex, senderName]()
		{
			param->DoObrabotchiki(windowIndex, senderName);

			param->SetValue(edit->text(),ValuesTypes::int_);
		});
	}
	else if(curWidg.GetType() == WidgetTypes::buttons)
	{
		for(uint i=0; i<curWidg.parts.size(); i++)
		{
			QPushButton *btn = (QPushButton *)curWidg.parts[i].GetPtQWidget();
			QString senderName { curWidg.parts[i].GetName() };
			QString newValue = btn->text();
			QObject::connect(btn, &QPushButton::clicked, [param, newValue, windowIndex, senderName]()
			{
				param->DoObrabotchiki(windowIndex, senderName);

				param->SetValue(newValue,ValuesTypes::int_);
			});
		}
	}
	else if(curWidg.parts.size() == 1 && curWidg.parts[0].GetType() == WidgPartTypes::button)
	{
		QPushButton *btn = (QPushButton *)curWidg.parts[0].GetPtQWidget();
		QString senderName { curWidg.parts[0].GetName() };
		QObject::connect(btn, &QPushButton::clicked, [this, param, windowIndex, senderName](){
			if(param->strType.contains(FrameTypes::infoKadr))
			{
				param->DoObrabotchiki(windowIndex, senderName);

				CANMsg_t msg { EmulatorConstructFrame(param, windowIndex) };
				SendFrame(msg);
			}
			else if(param->strType.contains(FrameTypes::rqstKadr)) {qdbg <<"mock Protocol::ConnectEmulatorWidg FrameTypes::rqstKadr";}
			else qdbg << "Protocol::ConnectEmulatorWidg2 wrong behavior";
		});
	}
	else if(curWidg.parts.size() == 1 && curWidg.parts[0].GetType() == WidgPartTypes::checkBox)
	{
		QCheckBox *ch = (QCheckBox *)curWidg.parts[0].GetPtQWidget();
		QString senderName { curWidg.parts[0].GetName() };

		QObject::connect(ch, &QCheckBox::clicked, [param, ch, windowIndex, senderName](){
			param->DoObrabotchiki(windowIndex, senderName);

			param->SetValue(ch->isChecked() ? "1" : "0", ValuesTypes::bool_);
		});
	}
	else
	{
		qdbg << "Protocol::ConnectEmulatorWidg2 connection unrealesed, type ="
			 << curWidg.GetType() << "(param " << param->module << param->name << ")";
	}
}

class Operation
{
public:
	IOperand *subj {nullptr};
	QString oper {Operators::undefined};
	IOperand *pred {nullptr};

	bool AddOperand(IOperand *operand)
	{
		if(!operand) { Error("AddOperand nullptr adding operand"); return false; }

		if(oper == Operators::undefined)
		{
			if(subj) { Error("AddOperand subject already seted"); return false; }
			subj = operand;
		}
		else
		{
			if(pred) { Error("AddOperand predicate already seted"); return false; }
			if(Operators::unarn.contains(oper)) { Error("AddOperand operator unarrn, but gived predicate"); return false; }
			pred = operand;
		}

		return true;
	}

	void Error(const QString &errStr) { qdbg << "ERRROR: Operation::" + errStr; }
};




Object::DEReturn Object::DoEverything(QString command, Parametr *param, int windowIndex, CANMsg_t &frame, QString retType)
{
	// эта функция должна принимать команду и список указателей на всё всё, которые унаследованы от абстрактного класса

	//Log("DoEverything start do command "+ command);

//	QString pointStopCommand {"tokOtvet.numbers = Data[0,1,2,3]"};
//	if(command.contains(pointStopCommand))
//		int a = 5;

	QString dataOperandKeyWord {"Data"};
	QString servisKeyWord {"Сервис"};
	QString emulatorKeyWord {"Эмулятор"};

	auto words { Compiler::CommandToWords(command) };

	Operation oper;
	Value number;
	DataOperand dataOperand;
	VectorParams vectParams;
	bool localSubject = false;

	int prevSize = words.size();
	while(words.size())
	{
		// Если это предназначено только для Сервиса или только для эмулятора
		if(words[0] == servisKeyWord || words[0] == emulatorKeyWord)
		{
			// если команда нам не соответсвует - выходим из функции
			//if(words[0] == servisKeyWord && this->objectTip == Parametr::Types::emulator) return DEReturn();
			//if(words[0] == emulatorKeyWord && this->objectTip == Parametr::Types::servis) return DEReturn();

			// если не вышли - значит команда нам соответсвует, удаляем первое слово
			words.removeAt(0);
		}
		// Function
		else
		if(functions.contains(words[0]))
		{
			int funcIndex = functions.indexOf(words[0]);
			int fwCount = functionsWCounts[funcIndex];
			if(words.size() >= fwCount)
			{
				// funcLog
//				if(words.size() && words[0] == funcLog)
//				{
//					CANMsg_t msgPustishka;
//					DEReturn smth { DoEverything(words[1], param, noWindow, msgPustishka, Object::DEretPtr()) };
//					if(smth.ptr) Log("DoEverything "+ funcLog +": " + smth.ptr->IOGetValue(ValuesTypes::any));
//					else Error("DoEverything unrealesed behavior function "+funcLog+". Command [" + command + "]");
//				}
//				// funcSendFrame
//				else if(words.size() && words[0] == funcSendFrame)
//				{
//					CANMsg_t msgPustishka;
//					DEReturn smth { DoEverything(words[1], param, noWindow, msgPustishka, Object::DEretPtr()) };
//					Parametr *frameSender {nullptr};

//					if(smth.ptr && smth.ptr->IOClassName() == IOperand::classParametr())
//					{
//						frameSender = (Parametr*)smth.ptr;
//						CANMsg_t msg { EmulatorConstructFrame(frameSender, noWindow)};
//						SendFrame(msg);
//					}
//					else Error("DoEverything unrealesed behavior function "+funcSendFrame+". ["+words[0]+"] is not "+IOperand::classParametr()+". Command [" + command + "]");
//				}
//				// funcFindParamInModule
//				else if(words.size() && words[0] == funcFindParamInModule)
//				{
//					Parametr *findedParam {nullptr};
//					QString paramName = words[1];
//					QString numberOperand = words[2];
//					int nomerModule = -1;

//					if(Compiler::GetOperandName(numberOperand) == dataOperandKeyWord)
//					{
//						Value valFromData("",ValuesTypes::vectorByte);
//						valFromData.FromDataUniversal(frame, numberOperand);
//						nomerModule = valFromData.ToInt("");
//					}
//					else
//					{
//						DEReturn nModuleSmth { DoEverything(numberOperand, param, windowIndex, frame, Object::DEretPtr()) };
//						if(nModuleSmth.ptr && nModuleSmth.ptr->IOClassName() == IOperand::classValue())
//						{
//							Value* valuePtParametr = (Value*)nModuleSmth.ptr;
//							nomerModule = valuePtParametr->ToInt(numberOperand);
//						}
//						else Error("DoEverything unrealesed behavior function "+funcFindParamInModule+". Command [" + command + "]");
//					}

//					if(nomerModule != -1)
//					{
//						findedParam = FindParamVModuleNomer(nomerModule,paramName);
//					}
//					else Error("DoEverything can't define numberOperand function "+funcFindParamInModule+". Command [" + command + "]");

//					if(findedParam)
//					{
//						if(!oper.AddOperand(findedParam))
//							Error("DoEverything error AddOperand. Command [" + command + "]");
//					}
//					else Error("DoEverything can't find param in funcFindParamInModule. Command [" + command + "]");
//				}
//				// funcFindParams
//				else if(words.size() && words[0] == funcFindParams)
//				{
//					vectParams.Set(FindParamsByName(words[1]));
//					if(!oper.subj) localSubject = true;
//					oper.AddOperand(&vectParams);
//				}
				// if
				/*else*/ if(words.size() && words[0] == funcIf)
				{
					bool resIf = false;
					DEReturn obj1 { DoEverything(words[2], param, windowIndex, frame, Object::DEretPtr()) };
					DEReturn obj2 = DoEverything(words[4], param, windowIndex, frame, Object::DEretPtr());

					if(words[3] == Operators::cmpEqual)
					{
						//resIf = obj1.ptr->IsEqual(obj2.ptr);
					}
					else Error("DoEverything unrealesed if condition. Command [" + command + "]");

					if(!resIf) fwCount = words.size(); // если проверка условия не пройдена будут удалены все последующие слова из команды
					else {/* выше задано fwCount = functionsWCounts[funcIndex] */}
					qdbg << "if DEReturn: ~DEReturn( bfore this ERROR!!!";
				}
				else Error("DoEverything unrealesed function. Command [" + command + "]");
			}
			else Error("DoEverything: wrong function words count. Command [" + command + "]");

			int countRemove = fwCount;
			if(countRemove > words.size()) { countRemove = words.size(); Error("DoEverything: wrong behavior. countRemove > words.size(). Command [" + command + "]"); }
			for(int i=0; i<fwCount; i++)
				words.removeAt(0);
		}
//		// Number
//		else if(Compiler::IsNumber(words[0]))
//		{
//			if(Compiler::IsInteger(words[0]))
//			{
//				number.Set(words[0],Value::GetTypeByStrValue(words[0]));
//				if(!oper.subj)
//				{
//					Error("DoEverything error. Number can't be subject [" + command + "]");
//					oper.AddOperand(nullptr);
//				}
//				else
//				{
//				if(!oper.AddOperand(&number))
//					Error("DoEverything error AddOperand. Command [" + command + "]");
//				}
//			}
//			else Error("DoEverything: floating numbers unrealesed. Command [" + command + "]");

//			words.removeAt(0);
//		}
//		// Variable
//		else if(param->peremennieNames.contains(Compiler::GetOperandName(words[0])))
//		{
//			Value *findedPeremennaya = param->FindPeremennaya(Compiler::GetOperandName(words[0]),true);
//			IOperand *newOperand {nullptr};

//			if(findedPeremennaya)
//			{
//				if(findedPeremennaya->IOGetType() == ValuesTypes::paramPt)
//				{
//					Parametr *peremennayaParam = (Parametr*)findedPeremennaya->GetPt();
//					if(Compiler::GetFirstSubObjectName(words[0]) != "")
//					{
//						newOperand = peremennayaParam->FindPeremennaya(Compiler::GetFirstSubObjectName(words[0]),true);
//					}
//					else
//					{
//						newOperand = peremennayaParam;
//					}
//				}
//				else newOperand = findedPeremennaya;
//			}

//			if(newOperand)
//			{
//				if(!oper.AddOperand(newOperand))
//					Error("DoEverything error AddOperand. Command [" + command + "]");
//			}
//			else Error("DoEverything: Can't find variable. Command [" + command + "]");

//			words.removeAt(0);
//		}
//		// Widget
//		else if(param->ctrlWidgsNames.contains(words[0]))
//		{
//			if(windowIndex != noWindow)
//			{
//				auto resFind {param->FindWidget(words[0],windowIndex) };
//				IOperand *findedW {nullptr};
//				if(resFind.first && resFind.second == WidgetTypes::whole) findedW = (Widget*)resFind.first;
//				else if(resFind.first && resFind.second == WidgetTypes::part) findedW = (WidgPart*)resFind.first;
//				else Error("DoEverything error finding. Command [" + command + "]");

//				if(!oper.AddOperand(findedW))
//					Error("DoEverything error AddOperand. Command [" + command + "]");
//			}
//			else Error("DoEverything get window ingex noWindow, but frame contains links to widgets! Wrong behavior!");

//			words.removeAt(0);
//		}
//		// Operator
//		else if(Operators::all.contains(words[0]))
//		{
//			if(!oper.subj) Error("DoCommand: subject can't be nullptr. Command: "+ command);
//			if(oper.oper != Operators::undefined) Error("DoEverything: operator alredy set. Too much opers in command. Command: "+ command);
//			else oper.oper = words[0];

//			words.removeAt(0);
//		}
//		// Data operand
//		else if(Compiler::GetOperandName(words[0]) == dataOperandKeyWord)
//		{
//			dataOperand.frame = &frame;
//			dataOperand.strDataOperand = words[0];

//			if(!oper.subj) localSubject = true;
//			if(!oper.AddOperand(&dataOperand))
//				Error("DoEverything error AddOperand. Command [" + command + "]");

//			words.removeAt(0);
//		}
//		else Error("DoEverything: unknown word [" + words[0] + "] in command [" + command + "] in param ["+ param->name +"]");

		if(prevSize == words.size())
		{
			Error("DoEverything: while doing command (" + command + ") size not changed. Breaking cycle.");
			break;
		}

		prevSize = words.size();
	}

//	QString pointStopCommand2 {"ПоискПараметраВМодулеНомер"};
//	if(command.contains(pointStopCommand2))
//		int a = 5;

	// выполнение операции
	// выполнение операции
	// выполнение операции

	DEReturn retOperand;
//	if(oper.oper == Operators::assign)
//	{
//		if(retType == Object::DEretNullptr())
//		{
//			if(oper.subj && oper.pred)
//				oper.subj->Assign(oper.pred);
//			else Error("DoEverything: nullptr operands in command: " + command);
//		}
//		else Error("DoEverything: unrealesed behavior (assign but ret != DEretNullptr): " + command);
//	}
//	// no operator, getter or dider
//	else if(oper.oper == Operators::undefined)
//	{
//		if(retType == Object::DEretPtr())
//		{
//			if(localSubject)
//			{
//				if(oper.subj && oper.subj->IOClassName() == IOperand::classDataOperand())
//				{
//					DataOperand *newDO = new DataOperand;
//					newDO->frame = &frame;
//					newDO->strDataOperand = ((DataOperand*)oper.subj)->strDataOperand;

//					retOperand.ptr = newDO;
//					retOperand.needDelete = true;
//				}
//				else Error("DoEverything: try return unrealesed local subject in command: " + command);
//			}
//			else
//			{
//				if(oper.subj) retOperand.ptr = oper.subj;
//				else Error("DoEverything: nullptr subj in command: " + command);
//			}
//		}
//		else if(retType == Object::DEretNullptr()) { /* ret type void, retOperand {nullptr} default */ }
//		else Error("DoEverything: NoOperCommand unrealesed return ["+retType+"] command: " + command);
//	}
//	else Error("DoEverything: unrealesed operation ("+oper.subj->IOClassName()+" "+oper.oper+" "+oper.pred->IOClassName()+") command: " + command);

//	// return values check
//	if(retType == Object::DEretPtr())
//	{
//		if(!retOperand.ptr) Error("DoEverything: wrong or unrealesed retOperand value, need "+ retType +" but ret nullptr ");
//	}

//	if(retType == Object::DEretNullptr() && retOperand.ptr)
//		Error("DoEverything: wrong ret value, need "+ retType +" but ret is not nullptr");

	return retOperand;
}


void Object::ServisConnectWidg(std::vector<WidgPart> &doConnect, Parametr *param, int windowIndex)
{
	for(auto &wpt:doConnect)
	{
		if(wpt.GetType() == WidgPartTypes::button)
		{
			QPushButton *btn = (QPushButton *)wpt.GetPtQWidget();
			QString buttonName {wpt.GetName()};
			QObject::connect(btn, &QPushButton::clicked, [this, param, windowIndex, buttonName](){

				param->DoObrabotchiki(windowIndex, buttonName);

				if(param->strType.contains(FrameTypes::ctrlKadr))
				{
					CANMsg_t msg { ServisConstructFrame(param, windowIndex) };
					SendFrame(msg);
				}
				if(param->strType.contains(FrameTypes::rqstKadr))
				{
					CANMsg_t msg { ServisConstructFrame(param, windowIndex) };
					SendFrame(msg);
				}
			});
		}
		else if(wpt.GetType() == WidgPartTypes::checkBox) { /* стандартный чекбокс ничего не делает */  }
		else qdbg << "Protocol::ConnectServiceWidg connection type unrealesed" << wpt.GetType();
	}
}

void Object::CreateCtrlWidg(Parametr *param, int windowIndex, vector<WidgPart> &doConnect)
{
	doConnect.clear();
	QStringList cmndList { Compiler::TextToCommands(param->strCtrlWidgs[windowIndex]) };
	for(auto cmnd:cmndList)
	{
		auto words { Compiler::CommandToWords(cmnd) };
		int size = words.size();
		if(!size) qdbg << "Protocol::CreateCtrlWidg Wrong widget command words.empty()" << cmnd;
		else if(words[0] == CrWidgCmnds::obrabotchik)
		{
			param->ctrlWidgs[param->ctrlWidgs.size()-1].obrabotchiki.push_back(cmnd);
		}
		else if(Widget::isWidgType(words[0]))
		{
			if(words[0] == WidgetTypes::chekBoxesNButton)
			{
				if(size >= 4)
				{
					param->ctrlWidgs[windowIndex].SetType(WidgetTypes::chekBoxesNButton);
					param->ctrlWidgs[windowIndex].SetName(words[1]);

					int btnIndex = words.size()-1;
					for(int i=2; i<words.size(); i++)
					{
						WidgPart wpt;
						QString caption { TextConstant::GetTextConstVal(words[i]) };
						if(i != btnIndex) wpt.Create("", "", WidgPartTypes::checkBox, caption, "");
						if(i == btnIndex) wpt.Create("", "", WidgPartTypes::button, caption, "");
						if(i == btnIndex) doConnect.push_back(wpt);
						param->ctrlWidgs[windowIndex].AddWidgetPart(wpt);
					}
				}
				else qdbg << "Protocol::CreateCtrlWidg Wrong words size (" +QSn(size)+ ") in command" << cmnd;
			}
			else if(words[0] == WidgetTypes::buttons)
			{
				if(size >= 3)
				{
					param->ctrlWidgs[windowIndex].SetType(WidgetTypes::buttons);
					param->ctrlWidgs[windowIndex].SetName(words[1]);

					for(int i=2; i<words.size(); i++)
					{
						WidgPart wpt;
						QString caption { TextConstant::GetTextConstVal(words[i]) };
						wpt.Create("", "", WidgPartTypes::button, caption, "");
						doConnect.push_back(wpt);
						param->ctrlWidgs[windowIndex].AddWidgetPart(wpt);
					}
				}
				else qdbg << "Protocol::CreateCtrlWidg Wrong word count (" +QSn(size)+ ") in command" << cmnd;
			}
			else if(words[0] == WidgetTypes::editButton)
			{
				if(size == 3)
				{
					param->ctrlWidgs[windowIndex].SetType(WidgetTypes::editButton);
					param->ctrlWidgs[windowIndex].SetName(words[1]);

					QString caption { TextConstant::GetTextConstVal(words[2]) };

					WidgPart wptEdit;
					wptEdit.Create("", "", WidgPartTypes::edit, "", "");
					param->ctrlWidgs[windowIndex].AddWidgetPart(wptEdit);

					WidgPart wptBtn;
					wptBtn.Create("", "", WidgPartTypes::button, caption, "");
					doConnect.push_back(wptBtn);
					param->ctrlWidgs[windowIndex].AddWidgetPart(wptBtn);
				}
				else qdbg << "Protocol::CreateCtrlWidg Wrong word count (" +QSn(size)+ ") in command" << cmnd;
			}
			else qdbg << "Protocol::CreateCtrlWidg unrealesed Widget" << words[0];
		}
		else if(WidgPart::isWidgPartType(words[0]))
		{
			QString tip = words[0];

			QString name = "";
			if(size >= 2) name = words[1];

			QString caption = "";
			if(size >= 3) caption = TextConstant::GetTextConstVal(words[2]);

			QString startVal = "";
			if(size == 4) startVal = TextConstant::GetTextConstVal(words[3]);

			WidgPart wpt;
			wpt.Create(name, "", tip, caption, startVal);
			param->ctrlWidgs[windowIndex].AddWidgetPart(wpt);

			if(tip == WidgPartTypes::button || tip == WidgPartTypes::checkBox) doConnect.push_back(wpt);
		}
		else qdbg << "Protocol::CreateCtrlWidg unnown command" << cmnd;
	}
}

void Object::CreateWidgets()
{
	for(int window=0; window<windowsCount; window++)
	{
		int pCount = params.size();
		for(int p=0; p<pCount; p++)
		{
			Parametr *param = &params[p];
			param->viewWidgets.push_back(Widget());
			param->ctrlWidgs.push_back(Widget());

			if(param->strViewWidgs[window] != "" || param->strCtrlWidgs[window] != "")
			{
				// viewWidj
				CreateViewWidg(param,window);

				// ctrlWidj
				vector<WidgPart> doConnect;
				CreateCtrlWidg(param, window, doConnect);

				if(objectTip == Parametr::servis) ServisConnectWidg(doConnect, param, window);
				else EmulatorConnectWidg(param, window);
			}
		}
	}

	// добавление имен виджетов в ctrlWidgsNames
	int pCount = params.size();
	for(int p=0; p<pCount; p++)
	{
		Parametr *param = &params[p];
		for(uint w=0; w<param->ctrlWidgs.size(); w++)
		{
			if(!param->ctrlWidgsNames.contains(param->ctrlWidgs[w].GetName()))
				param->ctrlWidgsNames+=param->ctrlWidgs[w].GetName();

			auto &parts {param->ctrlWidgs[w].parts};
			for(uint wpi=0; wpi<parts.size(); wpi++)
			{
				if(!param->ctrlWidgsNames.contains(parts[wpi].GetName()))
					param->ctrlWidgsNames+=parts[wpi].GetName();
			}
		}

		for(auto &wname:param->ctrlWidgsNames)
			if(Compiler::IsNumber(wname)) Error("CreateWidgets: wrong widget name ["+wname+"] in param ["+param->name+"]");
	}
}

void Object::PlaceWidgets(int startX, int startY, QString PrefixName)
{
	for(int window=0; window<windowsCount; window++)
	{
		windows.push_back(new QWidget);
		windowTables.push_back(nullptr);

		QVBoxLayout *all  = new QVBoxLayout(windows[window]);
		all->setObjectName("QVBoxLayout_all");

		int pCount = params.size();
		for(int p=0; p<pCount; p++)
		{
			Parametr *param = &params[p];

			if(param->strViewWidgs[window] != "" || param->strCtrlWidgs[window] != "")
			{
				QHBoxLayout *paramLO = new QHBoxLayout();
				all->addLayout(paramLO);

				if(param->viewWidgets[window].GetType() != WidgetTypes::cell)
				{
					if(param->viewWidgets[window].parts.size() || param->ctrlWidgs[window].parts.size())
					{
						// param->name
						QLabel *lblName = new QLabel(param->name);
						paramLO->addWidget(lblName);

						// viewWidj
						for(auto &wpt:param->viewWidgets[window].parts)
							paramLO->addWidget(wpt.GetPtQWidget());

						// ctrlWidj
						for(auto &wpt:param->ctrlWidgs[window].parts)
							paramLO->addWidget(wpt.GetPtQWidget());
					}
				}
			}
		}

		windows[window]->setWindowTitle(PrefixName + " "+QSn(window+1));
		MinResize();
		PlaceWindows(startX, startY);
		windows[window]->show();
	}

	Object::FillTables();
}

void Object::FillTables()
{
	int notSetIndex = -100;
	int colInd = notSetIndex;
	int rowInd = notSetIndex;
	QString rowName;
	QString firstRow;
	QString colName;
	QString rowNames;

	int pCount = params.size();
	for(int i=0; i<pCount; i++)
	{
		Parametr *param = &params[i];

		bool tableAdded = false;
		int wCount = param->viewWidgets.size();

		for(int w=0; w<wCount; w++)
		{
			QTableWidgetItem *item {nullptr};

			if(param->viewWidgets[w].parts.size())
				item = param->viewWidgets[w].parts[0].GetTWItem();

			if(item != nullptr)
			{
				QTableWidget *table = windowTables[windowTables.size()-1];
				if(!table)
				{
					table = new QTableWidget;
					windowTables[windowTables.size()-1] = table;
					table->setColumnCount(1);
					table->setRowCount(1);
				}

				if(colName != param->module)
				{
					colName = param->module;
					int newIndex = table->columnCount();
					table->insertColumn(newIndex);
					table->setItem(0, newIndex, new QTableWidgetItem(colName));

					if(colInd == notSetIndex) colInd = 1;
					else colInd++;
				}
				if(rowName != param->name)
				{
					rowName = param->name;
					int newIndex = table->rowCount();
					if(!rowNames.contains(rowName))
					{
						table->insertRow(newIndex);
						table->setItem(newIndex, 0, new QTableWidgetItem(rowName));
					}
					rowNames += "[" + rowName + "]";
				}
				if(firstRow == "") firstRow = param->name;
				if(firstRow == param->name) rowInd = 1;

				if(colInd == notSetIndex || rowInd == notSetIndex)
					qdbg << "CreateServisVidgets notSetIndex! colInd =" << colInd << "rowInd =" << rowInd;

				table->setItem(rowInd, colInd, item);
				rowInd++;


				QLayout *LO = windows[w]->findChild<QLayout*>("QVBoxLayout_all");
				if(!tableAdded) LO->addWidget(table);
				tableAdded = true;
			}
		}
	}
}

void Object::PlaceWindows(int x, int y)
{
	for(auto w:windows)
	{
		w->move(x,y);
		x = x + w->width() + 30;
	}
}

void Object::MinResize()
{
	for(auto w:windows)
	{
		w->resize(150,150);
	}
}
