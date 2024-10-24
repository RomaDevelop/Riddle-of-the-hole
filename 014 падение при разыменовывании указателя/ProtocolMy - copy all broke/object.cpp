#include <memory>

#include <QDebug>
#include <QGroupBox>

#include "MyQShowText.h"
#include "MyQExcel.h"
#include "MyQShortings.h"

#include "object.h"

using namespace std;

// WCount - words count. Including func name
namespace Functions
{					
	const QString FindParamInThisCategory		{"FindParamInThisCategory"};
	const int FindParamInThisCategoryWCount		{2};
	const QString findParamInModule				{"FindParamInModule"};
	const int findParamInModuleWCount			{3};
	const QString findParamInCategoryAndModule	{"FindParamInCategoryAndModule"};
	const int findParamInCategoryAndModuleWCount{4};
	const QString findParams					{"FindParams"};
	const int findParamsWCount					{2};
	const QString sendFrame						{"SendFrame"};
	const int sendFrameWCount					{2};
	const QString print							{"Print"};
	const int printLogWCount					{3};
	const QString if_							{"if"};
	const int ifWCount							{4};

	const QStringList functions{
		FindParamInThisCategory,
		findParamInModule,
				findParamInCategoryAndModule,
				findParams,
				sendFrame,
				print,
				if_
	};

	const std::vector<int> functionsWCounts{
		FindParamInThisCategoryWCount,
		findParamInModuleWCount,
				findParamInCategoryAndModuleWCount,
				findParamsWCount,
				sendFrameWCount,
				printLogWCount,
				ifWCount
	};

	const QString printKeyWordLog		{"Log"};
	const QString printKeyWordWarning	{"Warning"};
	const QString printKeyWordError		{"Error"};
}

Object::Object(const std::vector<QStringList> &paramsStrs,
			   QString Version, QString name_, int objTip_, Constants *constants_, int channel_):
	name {name_},
	objectTip {objTip_},
	constants{constants_},
	channel{channel_}
{
	if(Version == "001")
	{
		constantsNames = constants->NamesList();

		int paramsCount = paramsStrs.size();
		params.reserve(paramsCount);

		for(int i=0; i<paramsCount; i++)
		{
			params.push_back({paramsStrs[i], objectTip, constants, this});
		}

		for(int i=0; i<paramsCount; i++)
		{
			params[i].InitParamValue();
			/// params[i].InitParamValue() должно делаться после всех params.push_back(...)
			/// или параметры нужно хранить вектором указателй и создавать динамически
			/// поскольку InitParamValue передаёт адреса параметров в из value
			/// а во время push_back меняются адреса параметров
		}

		InitPeremennie();

		timerMsgsWorker = new QTimer;
		QObject::connect(timerMsgsWorker, &QTimer::timeout, [this](){ this->MsgsWorker(); });
		timerMsgsWorker->start(100);

		timerBlinking = new QTimer;
		QObject::connect(timerBlinking, &QTimer::timeout, [this](){ this->Blink(); });
		timerBlinking->start(500);
	}

	Log("created");
}

void Object::InitFilters()
{
	for(auto &param:params)
	{
		if(param.frameCell != "")
		{
			QStringList commands { Compiler::TextToCommands(param.frameCell) };
			bool idSet = false;
			for(auto &cmnd:commands)
			{
				if(cmnd.left(CommandsKeyWords::idFilter.length()) == CommandsKeyWords::idFilter)
				{
					if(idSet) Error("InitFilters: multiple ID setting. Param id: " + param.idCell + "; frame: " + param.frameCell);

					param.idFiltersStrs = { param.GenerateIDStrsList(cmnd) };
					for(int i=0; i<param.idFiltersStrs.size(); i++)
					{
						CANMsgIDFilter_t filter;
						filter.mask = 0xFFFFFFFF;
						filter.filter = param.ConstuctID(param.idFiltersStrs[i], constants);
						param.idFilters.push_back(filter);
					}
					if(param.idFilters.size())
					{
						idSet = true;
						param.outgoingID = param.idFilters[0].filter;
						param.outgoingIDStr = param.idFiltersStrs[0];
					}
				}

				if(cmnd.left(CommandsKeyWords::dataFilter.length()) == CommandsKeyWords::dataFilter)
				{
					auto words = Compiler::CommandToWords(cmnd);
					if(words.size() >= 4)
					{
						words.removeFirst();
						if(words[0].contains(CommandsKeyWords::dataOperand))
						{
							DataOperand::SetDataFilterMaskFromOperand(words[0],param.dataFilter);

							CANMsg_t frame;
							DoEverything(words.join(' '), &param, frame, DEretNullptr());

							for(uint i=0; i<frame.length; i++)
							{
								param.dataFilter.filter[i] = frame.data[i];
							}
						}
						else Error("InitFilters: wrong data filter command. Command: " + cmnd);
					}
					else Error("InitFilters: wrong words size in dataFilter command. Command: " + cmnd);
				}
			}

			if(!idSet) Error("InitFilters: ID not set. Param id: " + param.idCell + " frame " + param.frameCell);
		}
	}
}

void Object::InitPeremennie()
{
	for(uint i=0; i<params.size(); i++)
	{
		auto commandList {Compiler::TextToCommands(params[i].variablesCell)};
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
					params[i].variables.push_back(Value(peremName,nullptr));
					params[i].variablesNames.push_back(peremName);
				}
				else
					if(ValuesTypes::all.contains(tip))
					{
						params[i].variables.push_back(Value(peremName,"",tip));
						params[i].variablesNames.push_back(peremName);
					}
					else Error("InitPeremennie unrealesed type variable " + command);

				QString initStr = Compiler::GetInitialisationStr(command, false);
				if(initStr != "")
				{
					if(tip == ValuesTypes::paramPt)
					{
						CANMsg_t framePustishka;
						auto ptr = DoEverything(initStr,&params[i],framePustishka,DEretPtr());
						if(ptr && ptr->IOGetClassName() == IOperand::classParam())
							params[i].variables.back().InitPt(peremName, ptr, ValuesTypes::paramPt);
						else Error("InitPeremennie can't find Param by initStr" + initStr);

						qdbg << "Object::InitPeremennie()";
						qdbg << ptr << static_cast<Param*>(ptr);
						qdbg << static_cast<Param*>(ptr)->CellValuesToStr();
						qdbg << params[i].variables.back().GetPt() << static_cast<Param*>(params[i].variables.back().GetPt());
						qdbg << static_cast<Param*>(params[i].variables.back().GetPt())->CellValuesToStr();
					}
					else
					{
						if(ValuesTypes::vectorsNumeric.contains(tip))
						{
							initStr.remove('{');
							initStr.remove('}');
							auto vals = initStr.split(',',QString::SkipEmptyParts);
							params[i].variables.back().Set(vals.join(';'),tip);
						}
						else Error("InitPeremennie unrealesed initialisation " + command);
					}
				}
			}
			else Error("InitPeremennie wrong words count " + command);
		}
	}
}

std::vector<Param*> Object::FindParamsByName(const TextConstant &nameToFind)
{
	std::vector<Param*> findedParams;

	for(auto &p:params)
		if(p.nameCell == nameToFind.Value()) findedParams.push_back(&p);

	if(findedParams.empty()) Error("Protocol::FindParamsByName find nothing. Tryed to find " + nameToFind.Value());
	return findedParams;
}

Param *Object::FindParamById(const QString &id)
{
	Param* findedParam {nullptr};
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i].idCell == id)
		{ findedParam = &params[i]; break; }
	}

	return findedParam;
}

Param* Object::FindParamByName(const TextConstant &nameParam)
{
	Param* findedParam {nullptr};
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i].nameCell == nameParam.Value())
		{ findedParam = &params[i]; break; }
	}

	return findedParam;
}

Param *Object::FindParamInCategory(QString category, QString nameParam)
{
	Param* findedParam {nullptr};

	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i].categoryCell == category && params[i].nameCell == nameParam)
		{
			findedParam = &params[i];
			break;
		}
	}

	return findedParam;
}

Param* Object::FindParamInModuleNomer(int nomerModule, const TextConstant &nameParam)
{
	Param* findedParam {nullptr};

	QString nomerModuleStr = QSn(nomerModule);
	QString nameParamStr = nameParam.Value();
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i].moduleNumberCell == nomerModuleStr && params[i].nameCell == nameParamStr)
		{
			findedParam = &params[i];
			break;
		}
	}

	return findedParam;
}

Param *Object::FindParamInCategoryAndModule(int nomerModule, const TextConstant &nameParam, const TextConstant &nameCategory)
{
	Param* findedParam {nullptr};

	QString nomerModuleStr = QSn(nomerModule);
	QString nameParamStr = nameParam.Value();
	QString nameCatStr = nameCategory.Value();
	int size = params.size();
	for(int i=0; i<size; i++)
	{
		if(params[i].moduleNumberCell == nomerModuleStr && params[i].nameCell == nameParamStr && params[i].categoryCell == nameCatStr)
		{
			findedParam = &params[i];
			break;
		}
	}

	return findedParam;
}

Value *Object::FindVariableVParametre(QString namePeremenn, const TextConstant &nameParam)
{
	Param* param = FindParamByName(nameParam);
	if(param) return param->FindVariable(namePeremenn,false);
	else return nullptr;
}

Window *Object::FindWindow(QString windowName)
{
	for(auto w:windows)
		if(w->name == windowName) return w;
	return nullptr;
}

void Object::SendFrames(std::vector<ParamAndFrame> &paramsAndFrames)
{
	for(auto &paf:paramsAndFrames)
	{
		Object *obj = (Object *)paf.param->objectParent;
		if(!obj) Error("Wrorng behavior objectParent is nullptr");
		else obj->SendFrame(paf.frame);

	}
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
		QString tipFrame;
		Param *parametr = CheckFilters(msgs.back(), tipFrame);

		if(parametr)
		{
			Log("get and passed filter: " + msgs.back().ToStrEx2(2));
			WorkIncommingFrame(msgs.back(), parametr);
		}
		else Log("get not passed filter: " + msgs.back().ToStrEx2(2));

		msgs.pop();
	}
}

void Object::SetDataFilterInFrame(CANMsg_t &frame, CANMsgDataFilter_t filter)
{
	EasyData ed(frame);
	for(int byte=0; byte<8; byte++)
	{
		bitset<8> mask = filter.mask[byte];
		bitset<8> filt = filter.filter[byte];
		for(int bit=0; bit<8; bit++)
			if(mask[bit]) ed.SetBitBiBi(byte,bit,filt[bit]);
	}
	ed.ToMsg(frame);
}

std::vector<ParamAndFrame> Object::ConstructFrames(Param *param)
{
	CANMsg_t msg;
	QStringList commandList {Compiler::TextToCommands(param->frameCell)};
	for(auto &command:commandList)
	{
		auto words {Compiler::CommandToWords(command)};

		if(words[0] == CommandsKeyWords::idFilter) msg.ID = param->outgoingID;
		else if(words[0] == CommandsKeyWords::dataFilter)
		{
			SetDataFilterInFrame(msg, param->dataFilter);
		}
		else DoEverything(command, param, msg, Object::DEretNullptr());
	}

	std::vector<ParamAndFrame> paramsAndFrames;
	paramsAndFrames.reserve(param->synchronParams.size()+1);
	paramsAndFrames.push_back({param,msg});
	for(auto syParam:param->synchronParams)
	{
		CANMsg_t syMsg(msg);
		syMsg.ID = syParam->outgoingID;
		SetDataFilterInFrame(syMsg,syParam->dataFilter);
		paramsAndFrames.push_back({syParam,std::move(syMsg)});
	}

	return paramsAndFrames;
}

void Object::WorkIncommingFrame(CANMsg_t &frame, Param *param)
{
	auto cmndList { Compiler::TextToCommands(param->frameCell) };
	for(auto &command:cmndList)
	{
		auto words {Compiler::CommandToWords(command)};
		if(words.empty()) { Error("WorkIncommingFrame words.empty()"); return; }

		if(command.left(CommandsKeyWords::idFilter.length()) == CommandsKeyWords::idFilter
				|| command.left(CommandsKeyWords::dataFilter.length()) == CommandsKeyWords::dataFilter) // если это команда содержащая опредедение ID
		{
			/* тут мы ничего не делаем. ID и Data фильтры были сформированы ранее и установлен фильтр */
		}
		else
		{
			DoEverything(command, param, frame, Object::DEretNullptr());
		}

	}
}

void Object::ConnectMake()
{
	for(uint i=0; i<params.size(); i++)
	{
		Param *param = &params[i];
		for(uint windowIndex=0; windowIndex<param->widgets.size(); windowIndex++)
		{
			WidgetPair* widgetOfCurrentParam = param->widgets[windowIndex];

			for(auto &wpt:widgetOfCurrentParam->ctrl.parts)
			{
				if(wpt.GetType() == WidgetPartTypes::button)
				{
					QPushButton *btn = (QPushButton *)wpt.GetPtQWidget();
					QString buttonName {wpt.GetName()};

					QObject::connect(btn, &QPushButton::clicked, [this, param, buttonName, windowIndex](){
						DoObrabotchiki(windowIndex, buttonName, param);
					});

					if(FrameTypes::all.contains(param->typeDefined))
					{
						if((objectTip == Param::servis && param->typeDefined == FrameTypes::inFrame)
								|| (objectTip == Param::emulator && param->typeDefined == FrameTypes::outFrame))
						{
							QObject::connect(btn, &QPushButton::clicked, [this, param](){
								auto paramsAndFrames { ConstructFrames(param) };
								SendFrames(paramsAndFrames);
							});
						}
						else Error("ConnectWidg wrong connect code 26021 param->typeDefined = "
								   + param->typeDefined + "; param->typeDefined = " + param->frameCell);
					}

					if(objectTip == Param::emulator)
					{
						if(!FrameTypes::all.contains(param->typeDefined))
						{
							QString currCtrlWidgetOfParamType = widgetOfCurrentParam->ctrl.GetType();
							vector<QWidget*> currPtQWidgCtrls;
							for(auto &wpt:widgetOfCurrentParam->ctrl.parts)
								currPtQWidgCtrls.push_back(wpt.GetPtQWidget());

							if(currCtrlWidgetOfParamType == WidgetTypes::buttons)
							{
								for(uint i=0; i<currPtQWidgCtrls.size(); i++)
								{
									QPushButton *btn = (QPushButton *)currPtQWidgCtrls[i];
									QString newValue = btn->text();
									QObject::connect(btn, &QPushButton::clicked, [param, newValue]()
									{
										param->value.Set(newValue,ValuesTypes::text);
									});
								}
							}
							else if(currCtrlWidgetOfParamType == WidgetTypes::editButton)
							{
								QPushButton *btn = (QPushButton *)currPtQWidgCtrls[1];
								QObject::connect(btn, &QPushButton::clicked, [param, widgetOfCurrentParam]()
								{
									param->value.Set(widgetOfCurrentParam->ctrl.GetWidgetValue());
								});
							}
							else if(currCtrlWidgetOfParamType == WidgetTypes::chekBoxesNButton)
							{
								QObject::connect(btn, &QPushButton::clicked, [param, widgetOfCurrentParam]()
								{
									param->value.Set(widgetOfCurrentParam->ctrl.GetWidgetValue());
								});
							}
							else if(widgetOfCurrentParam->ctrl.parts.size() == 1
									&& widgetOfCurrentParam->ctrl.parts[0].GetType() == WidgetPartTypes::checkBox)
							{
								QCheckBox *ch = (QCheckBox *)currPtQWidgCtrls[0];
								QObject::connect(ch, &QCheckBox::clicked, [param, widgetOfCurrentParam]()
								{
									param->value.Set(widgetOfCurrentParam->ctrl.GetWidgetValue());
								});
							}
						}
					}
				}
			}
		}
	}
}

Param* Object::CheckFilters(const CANMsg_t &msg, QString &tipFrame)
{
	Param *retParam = nullptr;
	for(uint i=0; i<params.size(); i++)
	{
		Param *p = &params[i];

		bool chFilts = false;
		int sizeFs = p->idFilters.size();
		// id filter check
		for(int i=0; i<sizeFs; i++)
			if(msg.CheckFilter(p->idFilters[i])) { chFilts = true; break; }
		// data filter check
		if(chFilts && p->dataFilter.HasMask())
		{
			chFilts = msg.CheckFilter(p->dataFilter);
		}

		if(chFilts)
		{
			if(objectTip == Param::emulator && p->typeDefined == FrameTypes::inFrame) { tipFrame=FrameTypes::inFrame; retParam = p; }
			if(objectTip == Param::servis && p->typeDefined == FrameTypes::outFrame) { tipFrame=FrameTypes::outFrame; retParam = p; }
		}
	}

	return retParam;
}

class Operation
{
public:
	IOperand *subj {nullptr};
	QString oper {Operators::undefined};
	IOperand *pred {nullptr};

	bool AddOperand(IOperand *operand)
	{
		if(!operand) { LogsSt::ErrorSt("Operation:AddOperand nullptr adding operand"); return false; }

		if(oper == Operators::undefined)
		{
			if(subj) { LogsSt::ErrorSt("Operation:AddOperand subject already seted"); return false; }
			subj = operand;
		}
		else
		{
			if(pred) { LogsSt::ErrorSt("Operation:AddOperand predicate already seted"); return false; }
			if(Operators::unarn.contains(oper)) { LogsSt::ErrorSt("AddOperand operator unarrn, but gived predicate"); return false; }
			pred = operand;
		}

		return true;
	}

	bool CanDoOperation(QString command, QString word0)
	{
		bool ret = false;

		if(!subj) LogsSt::ErrorSt("Operation::CanDoOperation subj is nullptr; command " + command + " word[0] = " + word0);
		else
		{
			if(oper == Operators::undefined)
			{
				ret = true;
			}
			else if(Operators::unarn.contains(oper))
			{
				if(pred) LogsSt::WarningSt("Operation::CanDoOperation unarn oper <" + oper + ">, but pred is not nullptr; command " + command + " word[0] = " + word0);
				ret = true;
			}
			else if(Operators::binarn.contains(oper))
			{
				if(pred) ret = true;
			}
			else LogsSt::ErrorSt("Operation::CanDoOperation oper <" + oper + "> unknown; command " + command + " word[0] = " + word0);
		}
		return ret;
	}

	/// DoOperation не проверяет валидность subj, pred, задан ли оператор, поэтому
	/// перед вызвовом DoOperation необходим вызов CanDoOperation,
	/// т.е. проверка возможности выполнения операции
	void DoOperation(QString command, bool localSubject, std::vector<std::shared_ptr<IOperand>> &createdIOperands)
	{
		// Assign
		if(oper == Operators::assign)
		{
			subj->Assign(pred);
			pred = nullptr;
			oper = Operators::undefined;
		}
		// PlusAssign
		else if(oper == Operators::plusAssign)
		{
			subj->PlusAssign(pred);
			pred = nullptr;
			oper = Operators::undefined;
		}
		// Index
		else if(oper == Operators::index1)
		{
			auto element = subj->Index(pred->IOGetValue(ValuesTypes::sLong).toInt());
			createdIOperands.push_back(element);
			subj = element.get();
			pred = nullptr;
			oper = Operators::undefined;
		}
		// Clear
		else if(oper == Operators::clear)
		{
			subj->Clear();
			oper = Operators::undefined;
		}
		// no operator, getter or dider
		else if(oper == Operators::undefined)
		{
			if(localSubject)
			{
				if(subj && subj->IOGetClassName() == IOperand::classDataOperand())
				{
					std::shared_ptr<DataOperand> newDO = make_shared<DataOperand>();
					createdIOperands.push_back(newDO);
					newDO->frame = ((DataOperand*)subj)->frame;
					newDO->strDataOperand = ((DataOperand*)subj)->strDataOperand;

					subj = newDO.get();
				}
				else if(subj && subj->IOGetClassName() == IOperand::classValue())
				{
					std::shared_ptr<Value> newVal = make_shared<Value>();
					createdIOperands.push_back(newVal);
					newVal->Set(*static_cast<Value*>(subj));

					subj = newVal.get();
				}
				else if(subj && subj->IOGetClassName() == IOperand::classSubValue())
				{
					SubValue* subValue = static_cast<SubValue*>(subj);
					std::shared_ptr<SubValue> newSubVal = make_shared<SubValue>(*subValue);
					createdIOperands.push_back(newSubVal);
					subj = newSubVal.get();
				}
				else LogsSt::ErrorSt("DoOperation: try return unrealesed local subject in command: " + command);
			}
			else
			{
//				if(subj) retOperand = subj;
//				else LogedStClass::ErrorSt("DoOperation: nullptr subj in command: " + command);
				if(!subj) LogsSt::ErrorSt("DoOperation: nullptr subj in command: " + command);
			}
		}
		else LogsSt::ErrorSt("DoOperation: unrealesed operation <" + oper + "> in command: " + command);
	}
};

IOperand* Object::DoEverything(QString command, Param *param, CANMsg_t &frame, QString retType)
{// эта функция должна принимать команду и список указателей на всё всё, которые унаследованы от абстрактного класса
	auto words { Compiler::CommandToWords(command) };

	for(int i=0; i<words.size(); i++)
	{
		if(words[i].left(1) == Operators::index1 && words[i].right(1) == Operators::index2)
		{
			words[i] = words[i].mid(1,words[i].length()-2);
			words.insert(i, Operators::index1);
		}
	}

	QString pointStopCommand2 {"mesto = Data[0]"};
	if(command.contains(pointStopCommand2))
		bool stop = true;

	Operation operation;
	Value number;
	Value widgetValue;
	Value constantValue;
	shared_ptr<SubValue> subValue;
	DataOperand dataOperand;
	VectorParams vectParams;
	bool localSubject = false;

	IOperand* retOperand = nullptr;

	int prevSize = words.size();
	while(words.size())
	{
		bool nothingToDo = false;
		int wordsCountToRemove = 0;
		// Если это предназначено только для Сервиса или только для эмулятора
		if(words[0] == CommandsKeyWords::servis || words[0] == CommandsKeyWords::emulator)
		{
			nothingToDo = true;

			// если команда нам не соответсвует - выходим из функции
			if(words[0] == CommandsKeyWords::servis && this->objectTip == Param::Types::emulator) return nullptr;
			if(words[0] == CommandsKeyWords::emulator && this->objectTip == Param::Types::servis) return nullptr;

			// если return не было сделано - значит команда нам соответсвует, удаляем первое слово
			wordsCountToRemove = 1;
		}
		// Operator
		else if(Operators::all.contains(words[0]))
		{
			if(!operation.subj) Error("DoCommand: subject can't be nullptr. Command: "+ command);
			else
			{
				if(operation.oper != Operators::undefined)
					Error("DoEverything: operator alredy set. Too much opers in command. Command: "+ command);
				else
				{
					operation.oper = words[0];
				}
			}

			wordsCountToRemove = 1;
		}
		// Functions
		else if(Functions::functions.contains(words[0]))
		{
			int funcIndex = Functions::functions.indexOf(words[0]);
			int fwCountMin = Functions::functionsWCounts[funcIndex];
			int fwCountFakt = fwCountMin;
			if(words.size() >= fwCountMin)
			{
				// funcLog
				if(words.size() && words[0] == Functions::print)
				{
					nothingToDo = true;
					fwCountFakt = words.size();
					QString text;
					for(int i=2; i<fwCountFakt; i++)
					{
						if(TextConstant::IsItTextConstant(words[i], false))
						{
							text = TextConstant::GetTextConstVal(words[i]);
						}
						else
						{
							IOperand* smth { DoEverything(words[i], param, frame, Object::DEretPtr()) };
							if(smth) text += smth->IOGetValue(ValuesTypes::text);
							else Error("DoEverything unrealesed behavior function "+Functions::print+". Command [" + command + "]");
						}
						text += " ";
					}

					if(text != "")
					{
						if(words[1] == Functions::printKeyWordLog) Log("DoEverything "+ Functions::print +": " + text);
						else if(words[1] == Functions::printKeyWordWarning) Warning("DoEverything "+ Functions::print +": " + text);
						else if(words[1] == Functions::printKeyWordError) Error("DoEverything "+ Functions::print +": " + text);
						else
						{
							Warning("DoEverything "+ Functions::print +" unrealesed print keyword " + words[1]);
							Log("DoEverything "+ Functions::print +": " + text);
						}
					}
					else Error("DoEverything "+Functions::print+" print nothing. Command [" + command + "]");
				}
				// funcSendFrame
				else if(words.size() && words[0] == Functions::sendFrame)
				{
					nothingToDo = true;
					CANMsg_t msgPustishka;
					IOperand* smth { DoEverything(words[1], param, msgPustishka, Object::DEretPtr()) };
					Param *frameSender {nullptr};

					if(smth && smth->IOGetClassName() == IOperand::classParam())
					{
						frameSender = (Param*)smth;

						auto paramsAndFrames { ConstructFrames(frameSender) };
						SendFrames(paramsAndFrames);
					}
					else Error("DoEverything unrealesed behavior function "+Functions::sendFrame
							   +". ["+words[0]+"] is not "+IOperand::classParam()+". Command [" + command + "]");
				}
				// if
				else if(words.size() && words[0] == Functions::if_)
				{
					nothingToDo = true;
					bool resIf = false;
					IOperand* obj1 { DoEverything(words[1], param, frame, Object::DEretPtr()) };
					IOperand* obj2 = DoEverything(words[3], param, frame, Object::DEretPtr());

					if(!(obj1 && obj2)) Error("DoEverything if param is nullptr");
					else
					{
						if(words[2] == Operators::cmpEqual)
						{
							resIf = obj1->IsEqual(obj2);
						}
						else Error("DoEverything unrealesed if condition. Command [" + command + "]");

						if(!resIf) fwCountMin = words.size(); // если проверка условия не пройдена будут удалены все последующие слова из команды
						else {/* если проверка условия пройдена, ничего делать не надо, выше fwCount fwCount = Functions::functionsWCounts[funcIndex] */}
					}
				}
				// funcFindParamInCategory
				else if(words.size() && words[0] == Functions::FindParamInThisCategory)
				{
					QString paramName = TextConstant::GetTextConstVal(words[1]);
					QString categoryName = param->categoryCell;

					Param *findedParam = FindParamInCategory(categoryName,paramName);

					if(findedParam)
					{
						if(!operation.AddOperand(findedParam))
							Error("DoEverything error AddOperand. Command [" + command + "]");
					}
					else Error("DoEverything can't find param in funcFindParamInModule. Command [" + command + "]");
				}
				// funcFindParamInModule
				else if(words.size() && words[0] == Functions::findParamInModule)
				{
					Param *findedParam {nullptr};
					QString paramName = words[1];
					QString numberOperand = words[2];
					int nomerModule = -1;

					IOperand* nModuleSmth { DoEverything(numberOperand, param, frame, Object::DEretPtr()) };
					if(nModuleSmth) nomerModule = nModuleSmth->IOGetValue(ValuesTypes::uLong).toULong();
					else Error("DoEverything unrealesed behavior function "+Functions::findParamInModule+". Command [" + command + "]");

					if(nomerModule != -1)
					{
						findedParam = FindParamInModuleNomer(nomerModule,paramName);
					}
					else Error("DoEverything can't define numberOperand function "+Functions::findParamInModule+". Command [" + command + "]");

					if(findedParam)
					{
						if(!operation.AddOperand(findedParam))
							Error("DoEverything error AddOperand. Command [" + command + "]");
					}
					else Error("DoEverything can't find param in funcFindParamInModule. Command [" + command + "]");
				}
				// funcFindParamInCategoryAndModule
				else if(words.size() && words[0] == Functions::findParamInCategoryAndModule)
				{
					Param *findedParam {nullptr};
					QString paramName = words[1];
					QString categoryName = words[2];
					QString numberOperand = words[3];
					int nomerModule = -1;

					IOperand* nModuleSmth { DoEverything(numberOperand, param, frame, Object::DEretPtr()) };
					if(nModuleSmth) nomerModule = nModuleSmth->IOGetValue(ValuesTypes::uLong).toULong();
					else Error("DoEverything unrealesed behavior function "+Functions::findParamInCategoryAndModule+". Command [" + command + "]");

					if(nomerModule != -1)
					{
						findedParam = FindParamInCategoryAndModule(nomerModule,paramName,categoryName);
					}
					else Error("DoEverything can't define numberOperand function "+Functions::findParamInCategoryAndModule
							   +". Command [" + command + "]");

					if(findedParam)
					{
						if(!operation.AddOperand(findedParam))
							Error("DoEverything error AddOperand. Command [" + command + "]");
					}
					else Error("DoEverything can't find param in "+Functions::findParamInCategoryAndModule+". Command [" + command + "]");
				}
				// funcFindParams
				else if(words.size() && words[0] == Functions::findParams)
				{
					vectParams.Set(FindParamsByName(words[1]));
					if(!operation.subj) localSubject = true;
					operation.AddOperand(&vectParams);
				}
				else Error("DoEverything unrealesed function. Command [" + command + "]");
			}
			else Error("DoEverything: wrong function words count. Command [" + command + "]");

			wordsCountToRemove = fwCountMin;
			if(wordsCountToRemove > words.size())
			{
				wordsCountToRemove = words.size();
				Error("DoEverything: wrong behavior. countRemove > words.size(). Command [" + command + "]");
			}
		}
		// Number
		else if(Compiler::IsNumber(words[0]))
		{
			if(Compiler::IsInteger(words[0]))
			{
				number.Set(words[0],Value::GetTypeByStrValue(words[0]));
				if(!operation.subj) localSubject = true;
				if(!operation.AddOperand(&number))
					Error("DoEverything error AddOperand. Command [" + command + "]");
			}
			else Error("DoEverything: floating numbers unrealesed. Command [" + command + "]");

			wordsCountToRemove = 1;
		}
		// Constants
		else if(constantsNames.contains(words[0]))
		{
			auto constant = constants->FindConstant(words[0]);
			if(!constant) Error("DoEverything: can't find constand. Command [" + command + "]");
			else
			{
				auto constEncValue = constant->value;
				constantValue.Init("",constEncValue.ToBinStr(),ValuesTypes::binCode);
				if(!operation.subj) localSubject = true;
				if(!operation.AddOperand(&constantValue))
					Error("DoEverything error AddOperand. Command [" + command + "]");
			}

			wordsCountToRemove = 1;
		}
		// Variable
		else if(param->variablesNames.contains(Compiler::GetOperandName(words[0])))
		{
			Value *findedPeremennaya = param->FindVariable(Compiler::GetOperandName(words[0]),true);
			IOperand *newOperand {nullptr};

			if(findedPeremennaya)
			{
				if(findedPeremennaya->GetType() == ValuesTypes::paramPt)
				{
					Param *peremennayaParam = (Param*)findedPeremennaya->GetPt();
					if(Compiler::GetFirstSubObjectName(words[0]) != "")
					{
						newOperand = peremennayaParam->FindVariable(Compiler::GetFirstSubObjectName(words[0]),true);
					}
					else
					{
						newOperand = peremennayaParam;
						qdbg << peremennayaParam->IOGetValueAndType();
						qdbg << newOperand->IOGetValueAndType();
					}
				}
				else newOperand = findedPeremennaya;

				QString indexes = Compiler::GetIdexesText(words[0]);
				if(indexes != "")
				{
					IOperand* indexIO = DoEverything(indexes,param,frame,DEretPtr());
					if(!indexIO) Error("DoEverything: can't define index in word " + words[0] + " command " + command);
					else
					{
						Value indexValue("0",ValuesTypes::uLong);
						indexValue.Assign(indexIO);
						if(findedPeremennaya->GetType() == ValuesTypes::paramPt)
							Error("DoEverything: unrealesed behavior. Indexes from type " + findedPeremennaya->GetType());
						else
						{
							if(!operation.subj) localSubject = true;
							subValue = make_shared<SubValue>(*findedPeremennaya,indexValue.ToNumber());
							newOperand = subValue.get();
						}
					}
				}
				//	else not need. Indexes not must be

				if(newOperand)
				{
					if(!operation.AddOperand(newOperand))
						Error("DoEverything error AddOperand. Command [" + command + "]");
				}
				else Error("DoEverything: Can't define newOperand. Command [" + command + "]");
			}
			else Error("DoEverything: Can't find variable. Command [" + command + "]");

			wordsCountToRemove = 1;
		}
		// Widget
		else if(param->ctrlWidgetsNames.contains(words[0]))
		{
			auto resFind {param->FindWidget(words[0]) };
			if(!operation.subj) localSubject = true;
			if(resFind.first && resFind.second == WidgetTypes::whole)
			{
				widgetValue.Set(((Widget*)resFind.first)->GetWidgetValue());
			}
			else if(resFind.first && resFind.second == WidgetTypes::part)
			{
				widgetValue.Set(((WidgetPart*)resFind.first)->GetWidgPartCurrentText(),ValuesTypes::text);
			}
			else Error("DoEverything error finding. Command [" + command + "]");

			if(!operation.AddOperand(&widgetValue))
				Error("DoEverything error AddOperand. Command [" + command + "]");

			wordsCountToRemove = 1;
		}
		// Data operand
		else if(Compiler::GetOperandName(words[0]) == CommandsKeyWords::dataOperand)
		{
			dataOperand.frame = &frame;
			dataOperand.strDataOperand = words[0];

			if(!operation.subj) localSubject = true;
			if(!operation.AddOperand(&dataOperand))
				Error("DoEverything error AddOperand. Command [" + command + "]");

			wordsCountToRemove = 1;
		}
		else Error("DoEverything: unknown word [" + words[0] + "] in command [" + command + "] in param ["+ param->nameCell +"]");

		if(nothingToDo == false)
			if(operation.CanDoOperation(command, words[0]))
				operation.DoOperation(command, localSubject, createdIOperands);

		for(int i=0; i<wordsCountToRemove; i++)
			words.removeAt(0);

		if(prevSize == words.size())
		{
			Error("DoEverything: while doing command (" + command + ") size not changed. Breaking cycle.");
			break;
		}
		prevSize = words.size();
	}

//	QString pointStopCommand2 {"mestoNumber"};
//	if(command.contains(pointStopCommand2))
//		bool stop = true;

	// выполнение операции
	// выполнение операции
	// выполнение операции

	//if(!retOperand)
		//retOperand = operation.DoOperation(command, retType, localSubject, createdIOperands);


	// return values check
//	if(retType == Object::DEretPtr())
//	{
//		if(!retOperand) Error("DoEverything: wrong or unrealesed retOperand value, need "+ retType +" but ret nullptr ");
//	}

//	if(retType == Object::DEretNullptr() && retOperand)
//		Error("DoEverything: wrong ret value, need "+ retType +" but ret is not nullptr");

	if(retType == Object::DEretPtr()) retOperand = operation.subj;

	return retOperand;
}

void Object::DoObrabotchik(QString obrabotchik, QString senderNameFakt, Param *senderParam)
{
	auto words = Compiler::CommandToWords(obrabotchik);
	if(words.size() < 3) Error("DoObrabotchik: wrong widget command obrabotchik not enougth words " + obrabotchik);
	else
	{
		QString senderNameMustBe = words[1];

		if(senderNameMustBe == senderNameFakt)
		{
			words.removeAt(1); // removing senderName
			words.removeAt(0); // removing obrabotchik keyword

			CANMsg_t msgPustishka;
			DoEverything(words.join(" "), senderParam, msgPustishka, Object::DEretNullptr());
		}
	}
}

void Object::DoObrabotchiki(int windowIndex, QString senderName, Param *senderParam)
{
	int size = senderParam->widgets[windowIndex]->ctrl.obrabotchiki.size();
	for(int i=0; i<size; i++)
	{
		DoObrabotchik(senderParam->widgets[windowIndex]->ctrl.obrabotchiki[i], senderName, senderParam);
	}
}






