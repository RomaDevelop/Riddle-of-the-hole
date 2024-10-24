#include "protocol.h"

using namespace std;

#include <QObject>
#include <QTimer>

#include "MyQDom.h"
#include "MyCppDifferent.h"

#include "widget.h"

Protocol::Protocol(const QDomElement & domElement, Object *objectToConnect_, Constants *constants_)
{
	connectedObject = objectToConnect_;
	constants = constants_;

	QString typeStr;
	QString startSettings;
	auto attributes = MyQDom::Attributes(domElement);
	if((int)attributes.size() == Protocol_ns::count)
	{
		for(auto &attr:attributes)
		{
			if(     attr.first == Protocol_ns::name			) name = attr.second;
			else if(attr.first == Protocol_ns::type			) typeStr = attr.second;
			else if(attr.first == Protocol_ns::constants	) ;
			else if(attr.first == Protocol_ns::channel		) channelStr = attr.second;
			else if(attr.first == Protocol_ns::startSettings) startSettings = attr.second;
			else Logs::ErrorSt("Protocol::Protocol wrong attribute [" + QSn(attributes.size()) + "]");
		}
	}
	else Logs::ErrorSt("Protocol::Protocol wrong attributes.size() = " + QSn(attributes.size()));

	if(typeStr == CodeKeyWords::emulatorStr) type = CodeKeyWords::emulatorInt;
	else if(typeStr == CodeKeyWords::servisStr) type = CodeKeyWords::servisInt;
	else Error(GetClassName()+"(): wrong typeStr ["+typeStr+"]");

	auto subElements = MyQDom::GetTopLevelElements(domElement);
	workers.reserve(subElements.size());
	for(auto &subElement:subElements)
	{
		workers.push_back(make_unique<Worker>());
		workers.back()->InitWorker(subElement, constants, this, type);

		if(workers.back()->TypeCell().contains(FrameTypes::timer))
			timers.push_back(workers.back().get());
	}

	Category::FillCats(category, GetItemsListForFillCats());

	InitVariables();
	/// InitVariables() должно делаться после создания ВСЕХ params и frameWorkers
	/// поскольку variables могут ссылаться на параметры и обработчики

	InitFilters();

	timerMsgsWorker = new QTimer;
	QObject::connect(timerMsgsWorker, &QTimer::timeout, [this](){ this->WorkerIncommingFrames(); });
	timerMsgsWorker->start(100);

	InitTimers();

	Log("Protocol " + name + " created");
}

Protocol::~Protocol()
{
	if(timerMsgsWorker)
	{
		timerMsgsWorker->stop();
		delete timerMsgsWorker;
	}
}

void Protocol::InitTimers()
{
	for(auto &timer:timers)
	{
		auto commands = Code::TextToCommands(timer.frameWorkerTimer->TypeCell());
		Code::RemoveEmulatorServis(commands,type);

		if(commands.empty()) return;
		if(commands.size() == 1)
		{
			int interval = -1;
			bool check;
			QString timerInitStr = Code::GetInitialisationStr(commands[0],false);
			interval = timerInitStr.toInt(&check);

			if(check && interval != -1)
			{
				if(interval >= 10)
				{
					timer.qTimer = make_unique<QTimer>();
					QObject::connect(timer.qTimer.get(),&QTimer::timeout,[this, &timer](){
						CANMsg_t pustishka;
						for(auto &instruction:timer.frameWorkerTimer->Instructions())
							DoEverything(instruction, nullptr, timer.frameWorkerTimer, pustishka, nullptr, DEretNullptr());
					});
					timer.qTimer->start(interval);
				}
				else Error(GetClassName()+"::InitTimers to short interval ("+QSn(interval)+") by str ["+timer.frameWorkerTimer->TypeCell()+"]");
			}
			else Error(GetClassName()+"::InitTimers can't detect interval by str ["+timer.frameWorkerTimer->TypeCell()+"]");
		}
		else Error(GetClassName()+"::InitTimers wrong commands size ("+commands.size()+") TypeCell ["+timer.frameWorkerTimer->TypeCell()+"]");
	}
}

void Protocol::InitFilters()
{
	ClearFilters();
	for(auto &worker:workers)
	{
		if(FrameTypes::allFrames.contains(worker->TypeCell()))
		{
			QStringList commands { worker->Instructions() };
			bool idSet = false;
			for(auto &cmnd:commands)
			{
				if(cmnd.left(CodeKeyWords::idFilter.length()) == CodeKeyWords::idFilter)
				{
					if(idSet) Error("InitFilters: multiple ID setting. Param id: " + worker->IdCell() +
									"; frame: " + worker->FrameCell());

					QStringList idFiltersStrs = { worker->GenerateIDStrsList(cmnd) };
					for(int i=0; i<idFiltersStrs.size(); i++)
					{
						idSet = true;
						worker->AddIdFilter(worker->ConstuctID(idFiltersStrs[i], constants));
					}
				}

				if(cmnd.left(CodeKeyWords::dataFilter.length()) == CodeKeyWords::dataFilter)
				{
					auto words = Code::CommandToWords(cmnd);
					if(words.size() >= 4)
					{
						words.removeFirst();
						if(words[0].contains(CodeKeyWords::dataOperand))
						{
							int countWordsInDataOperand=0;
							QString strDataOperand = DataOperandFromWords(words, 0, countWordsInDataOperand, worker.get());

							CANMsg_t frame;
							std::array<u8, 8> mask {};
							DoEverything(words.join(' '), nullptr, worker.get(), frame, nullptr, DEretNullptr());
							DataOperand::SetDataFilterMaskFromOperand(strDataOperand, mask);
							worker->SetDataFilter(CANMsgDataFilter_t(frame.data,mask));
						}
						else Error("InitFilters: wrong data filter command. Command: " + cmnd);
					}
					else Error("InitFilters: wrong words size in dataFilter command. Command: " + cmnd);
				}
			}

			if(!idSet) Error("InitFilters: ID not set. IdCell: " + worker->IdCell() + " frame: " + worker->FrameCell());
		}

		if(defaultIdFilterFragmentsLengths.empty() && !worker->IdFilterFragmentsLengths().empty())
			defaultIdFilterFragmentsLengths = worker->IdFilterFragmentsLengths();
	}
}

void Protocol::ClearFilters()
{
	for(auto &frameWorker:workers)
	{
		if(FrameTypes::allFrames.contains(frameWorker->TypeCell()))
		{
			frameWorker->ClearFilters();
		}
	}
	defaultIdFilterFragmentsLengths.clear();
}

void Protocol::InitVariables()
{
	for(uint i=0; i<workers.size(); i++)
	{
		auto commandList {Code::TextToCommands(workers[i]->VariablesCell())};
		for(auto &command:commandList)
		{
			auto words {Code::CommandToWords(command)};
			if(words.size() >= 2)
			{
				Operator::CorrectVectorDefinition(words);
				QString tip = {words[0]};
				QString peremName {words[1]};
				if(Code::IsNumber(peremName)) Error("Protocol::InitVariables: wrong peremennaya name in command ["+command+"]");

				QString initStr = Code::GetInitialisationStr(command, false);

				if(ValuesTypes::all.contains(tip))
				{
					auto newVariable = std::make_shared<Value>(peremName,"",tip);
					workers[i]->AddVariable(newVariable,peremName);

					if(initStr != "")
					{
						if(ValuesTypes::vectorsNumeric.contains(tip) && Value::IsVectorNumeric(initStr))
						{
							newVariable->Set(initStr,tip);
						}
						else if(Code::IsNumber(initStr))
						{
							newVariable->Set(initStr,tip);
						}
						else
						{
							auto commads = Code::TextToCommands(initStr);
							if(commads.size() == 1)
							{
								CANMsg_t framePustishka;
								auto initResult = DoEverything(commads[0], nullptr, workers[i].get(), framePustishka, nullptr, DEretPtr());
								if(initResult)
								{
									newVariable->Set(initResult->IOGetValue(tip),tip);
								}
								else Error("Init str decoding result is nullptr");
							}
							else Error("not one command in initialisation str ["+initStr+"];\n\tvariable definition: ["+command+"]");
						}
					}
					else { /* nothing to do */ }
				}
				else if(tip == ParamPtr::InCofigCode())
				{
					if(initStr != "")
					{
						CANMsg_t framePustishka;
						auto ptr = DoEverything(initStr, nullptr, workers[i].get(),framePustishka, nullptr, DEretPtr());
						if(ptr)
						{
							if(ptr->GetClassName() == IOperand::classParam())
							{
								auto newVariable = make_shared<ParamPtr>((Param*)ptr,peremName);
								workers[i]->AddVariable(newVariable,peremName);
							}
							else if(ptr->GetClassName() == IOperand::classVectorParams())
							{
								auto vectParams = static_cast<VectorParams*>(ptr);
								if(vectParams->Get().size() == 1)
								{
									Param* ptrPramam0 = vectParams->Get()[0];
									auto newVariable = make_shared<ParamPtr>(ptrPramam0,peremName);
									workers[i]->AddVariable(newVariable,peremName);
								}
								else Error("Protocol::InitVariables wrong result init str: VectorParams size = " + QSn(vectParams->Get().size())
										   + "\n\tcommand in config file ["+command+"]");
							}
							else Error("Protocol::InitVariables unrealesed result init str [" + ptr->GetClassName() + "]");
						}
						else Error("Protocol::InitVariables can't find Param by initStr " + initStr);
					}
					else Error("Protocol::InitVariables initing " + tip + " whith empty initStr");
				}
				else if(tip == WorkerPtr::InCofigCode())
				{
					if(initStr != "")
					{
						CANMsg_t framePustishka;
						auto ptr = DoEverything(initStr, nullptr, workers[i].get(), framePustishka, nullptr, DEretPtr());
						if(ptr)
						{
							if(ptr->GetClassName() == IOperand::classWorker())
							{
								auto newVariable = make_shared<WorkerPtr>((Worker*)ptr,peremName);
								workers[i]->AddVariable(newVariable,peremName);
							}
							else Error("Protocol::InitVariables unrealesed result init str [" + ptr->GetClassName() + "]");
						}
						else Error("Protocol::InitVariables can't find Param by initStr " + initStr);
					}
					else Error("Protocol::InitVariables initing " + tip + " whith empty initStr");
				}
				else if(tip == VectorParams::inCofigCode)
				{
					if(initStr != "")
					{
						CANMsg_t framePustishka;
						auto ptr = DoEverything(initStr, nullptr, workers[i].get(), framePustishka, nullptr, DEretPtr());
						if(ptr && ptr->GetClassName() == IOperand::classVectorParams())
						{
							auto newVariable = make_shared<VectorParams>();
							workers[i]->AddVariable(newVariable,peremName);
							VectorParams *createdVectorParams = static_cast<VectorParams*>(newVariable.get());
							createdVectorParams->Set(static_cast<VectorParams *>(ptr)->Get(), peremName);
						}
						else Error("Protocol::InitVariables " + initStr + " result is not " + tip);
					}
					else Error("Protocol::InitVariables initing " + tip + " whith empty initStr");
				}
				else Error("Protocol::InitVariables unrealesed type variable. Command [" + command + "]");
			}
			else Error("Protocol::InitVariables wrong words count. Command [" + command + "]");
		} /* for */
	}/* for */
}

std::vector<Worker*> Protocol::FindWorkersInCategory(const QString &category_, const QString &nameFW, bool includeSubCats)
{
	std::vector<Worker*> findedFW;
	auto path = CategoryPath::PathFromQString(category_);
	auto findedCat = category.FindCategory(path,false);
	if(!findedCat) Warning("Protocol::FindWorkersInCategory category [" + category_ + "] doesn't exists");
	else
	{
		auto members = findedCat->GetMembers(includeSubCats);
		for(auto member:members)
		{
			Worker *fw = dynamic_cast<Worker*>(member);
			if(!fw) Error("Protocol::FindWorkersInCategory wrong member! It is " + member->GetClassName());
			else
			{
				if(fw->NameCell() == nameFW) findedFW.push_back(fw);
			}
		}
	}
	return findedFW;

//// старый поиск простым перебором. Пока не удаляю
//	std::vector<FrameWorker*> findedFW;
//	int size = frameWorkers.size();
//	for(int i=0; i<size; i++)
//	{
//		bool checked = false;
//		if(!includeSubCats && frameWorkers[i].categoryCell == category && frameWorkers[i].nameCell == nameFW)
//			checked = true;
//		else if(includeSubCats && frameWorkers[i].categoryCell.left(category.length()) == category && frameWorkers[i].nameCell == nameFW)
//			checked = true;

//		if(checked)
//			findedFW.push_back(&frameWorkers[i]);
//	}
//	return findedFW;
}

std::vector<Worker *> Protocol::FindWorkers(const QString & nameFW)
{
	std::vector<Worker*> findedFWs;
	int size = workers.size();
	for(int i=0; i<size; i++)
	{
		if(workers[i]->NameCell() == nameFW)
			findedFWs.push_back(workers[i].get());
	}
	return findedFWs;
}

Worker* Protocol::FindWorkerById(const QString &id)
{
	Worker* findedFW {nullptr};
	int size = workers.size();
	for(int i=0; i<size; i++)
	{
		if(workers[i]->IdCell() == id)
		{ findedFW = workers[i].get(); break; }
	}

	return findedFW;
}

void Protocol::SendFrames(std::vector<WorkerAndFrame> &toSend)
{
	for(auto &frame:toSend)
	{
		Protocol *prot = dynamic_cast<Protocol *>(frame.worker->ProtocolParent());
		if(!prot) Error("Wrorng behavior. frame.worker->ProtocolParent() = ["+QSn((int)frame.worker->ProtocolParent())+
						"] and it's dynamic cast result is nullptr");
		else prot->SendFrame(frame.frame,*frame.worker);
	}
}

QString FrameToStrForTVRS(const CANMsg_t &frame)
{
	QString ret = frame.ToStrExt2(2);
	ret.insert(44,' ');
	ret.insert(39,' ');
	ret.insert(34,' ');
	ret.insert(30,' ');
	ret.insert(27,' ');
	ret.insert(20,' ');
	ret.insert(17,' ');
	return ret;
}

QString FrameToStrForEveryThing(const CANMsg_t &frame)
{
	QString ret = frame.ToStrExt2(2);
	ret.insert(46,' ');
	ret.insert(38,' ');
	ret.insert(30,' ');
	ret.insert(22,' ');
	return ret;
}

QString FrameToStrSplitting(const CANMsg_t &frame, const std::vector<u8>& idFragments)
{
	QString ret = frame.ToStrExt2(2);
	int splitAt = 46; // последний символ ID в frame.ToStrExt2(2)
	for(int i=idFragments.size()-1; i>=0; i--)
	{
		splitAt -= idFragments[i];
		ret.insert(splitAt,' ');
	}
	return ret;
}

void Protocol::SendFrame(const CANMsg_t &msg, const Worker &worker)
{
	if(!muteOutgoing)
	{
		if(internalChanPtr)
		{
			int res = internalChanPtr->WriteMsg(msg,this);
			if(res == ICANChannel::NO_ERRORS)
			{
				if(Settings::logFrameSent)
					Log("worker "+worker.CategoryCell()+"::"+worker.NameCell() + " send frame\n\t"
						+ FrameToStrSplitting(msg, worker.IdFilterFragmentsLengths()));
			}
			else Error("Protocol::SendFrame WriteMsg error " + ICANChannel::ErrorCodeToStr(res));
		}
		else Error("Protocol::SendFrame chanPtr is nullptr");
	}
}

void Protocol::GiveClientMsg(const CANMsg_t & msg)
{
	if(!ignoreIncoming)
		msgs.push(msg);
}

void Protocol::WorkerIncommingFrames()
{
	for(uint i=0; i<msgs.size(); i++)
	{
		Worker *worker = CheckFilters(msgs.front());

		if(worker)
		{
			if(Settings::logFrameGet)
			{
				Log("frame passed filter "+worker->CategoryCell()+"::"+worker->NameCell()
					+ "\n\t" + FrameToStrSplitting(msgs.front(),worker->IdFilterFragmentsLengths()));
			}
			WorkerFramePassedFilter(msgs.front(), worker);
		}
		else
		{
			if(Settings::warningNotPassedFilterFrames)
				Warning("frame not passed filter: " + FrameToStrSplitting(msgs.front(),defaultIdFilterFragmentsLengths));
		}

		msgs.pop();
	}
}

void Protocol::SetDataFilterInFrame(CANMsg_t &frame, CANMsgDataFilter_t filter)
{
	EasyData ed(frame);
	for(int byte=0; byte<8; byte++)
	{
		bitset<8> mask = filter.Mask()[byte];
		bitset<8> filt = filter.Filter()[byte];
		for(int bit=0; bit<8; bit++)
			if(mask[bit]) ed.SetBitBiBi(byte,bit,filt[bit]);
	}
	ed.ToMsg(frame);
}

std::vector<WorkerAndFrame> Protocol::ConstructFrames(Worker *frameWorker, std::vector<Value> *vidgetValues)
{
	CANMsg_t msg;
	msg.regime = 0b100;
	QStringList commandList { frameWorker->Instructions()};
	for(auto &command:commandList)
	{
		auto words {Code::CommandToWords(command)};

		if(words[0] == CodeKeyWords::idFilter) msg.ID = frameWorker->OutgoingID();
		else if(words[0] == CodeKeyWords::dataFilter)
		{
			SetDataFilterInFrame(msg, frameWorker->DataFilter());
		}
		else DoEverything(command, nullptr, frameWorker, msg, vidgetValues, Protocol::DEretNullptr());
	}

	std::vector<WorkerAndFrame> framesForSynch;
	framesForSynch.reserve(frameWorker->synchronWorkers.size()+1);
	framesForSynch.push_back({frameWorker,msg});
	for(auto syParam:frameWorker->synchronWorkers)
	{
		CANMsg_t syMsg(msg);
		syMsg.ID = syParam->OutgoingID();
		SetDataFilterInFrame(syMsg,syParam->DataFilter());
		framesForSynch.push_back({syParam,std::move(syMsg)});
	}

	return framesForSynch;
}

void Protocol::WorkerFramePassedFilter(CANMsg_t &frame, Worker *frameWorker)
{
	auto cmndList { frameWorker->Instructions() };
	for(auto &command:cmndList)
	{
		auto words {Code::CommandToWords(command)};
		if(words.empty()) { Error("WorkIncommingFrame words.empty()"); return; }

		if(command.left(CodeKeyWords::idFilter.length()) == CodeKeyWords::idFilter
				|| command.left(CodeKeyWords::dataFilter.length()) == CodeKeyWords::dataFilter) // если это команда содержащая опредедение ID
		{
			/* тут мы ничего не делаем. ID и Data фильтры были сформированы ранее и установлен фильтр */
		}
		else
		{
			DoEverything(command, nullptr, frameWorker, frame, nullptr, Protocol::DEretNullptr());
		}
	}
}

Worker* Protocol::CheckFilters(const CANMsg_t &msg)
{
	vector<Worker*> passedFW;
	for(uint i=0; i<workers.size(); i++)
	{
		Worker *fw = workers[i].get();
		if((type == CodeKeyWords::emulatorInt && fw->TypeCell() == FrameTypes::inFrame)
				|| (type == CodeKeyWords::servisInt && fw->TypeCell() == FrameTypes::outFrame))
		{
			bool chFilts = false;
			int sizeFs = fw->IdFilters().size();
			// id filter check
			for(int i=0; i<sizeFs; i++)
				if(msg.CheckFilter(fw->IdFilters()[i])) { chFilts = true; break; }
			// data filter check
			if(chFilts && fw->DataFilter().HasMask())
			{
				chFilts = msg.CheckFilter(fw->DataFilter());
			}

			if(chFilts) passedFW.push_back(fw);
		}
	}

	if(passedFW.size() > 1)
	{
		QString fwStrs = "Protocol::CheckFilters filter passed not on one frameWorker!!!";
		for(auto fw:passedFW) fwStrs += "\n\tpassed frameWorker " + fw->ToStrForLog();
		Error(fwStrs);
	}

	if(passedFW.size() == 1) return passedFW[0];
	else return nullptr;
}

IOperand* Protocol::DoFunction(QString command, Function* function, vector<IOperand*> params, Worker *frameWorker)
{
	ClearOldCreatedIOperands();

	QString startLogText = "DoFunction "+function->name;
	QString infoAddForLog = "\n\tcommand [" + command + "]";
	int paramsCountMin = -1;
	int paramsCountMax = -1;
	int paramsCountFact = params.size();
	Functions::Check::ParamsCount(function->name,paramsCountMin,paramsCountMax);
	if(paramsCountFact >= paramsCountMin && paramsCountFact <= paramsCountMax)
	{
		bool nullptrParams = false;
		for(int i=0; i<paramsCountFact; i++)
			if(!params[i])
			{
				Error(startLogText+" function param["+i+"] is nullptr; nullptr returned" + infoAddForLog);
				nullptrParams = true;
				if(0) qdbg << CodeMarkers::toDo;
				// в этом цикле проверяется наличие nullptr параметров функции. Нужно будет удалить отдельные проверки почти в каждой ветке
				return nullptr;
			}
		if(nullptrParams) return nullptr;

		// print
		if(function->name == Functions::print.name)
		{
			QString text;
			QString logTypeInCode = params[0]->IOGetValue(ValuesTypes::text);
			QString logType = logTypeInCode;
			for(int i=1; i<paramsCountFact; i++)
			{
				text += params[i]->IOGetValue(ValuesTypes::text) + " ";
			}

			if(text != "")
			{
				if(logType == Functions::printKeyWordLog) Log(Functions::print.name + " " + logType + ": " + text);
				else if(logType == Functions::printKeyWordWarning) Warning(Functions::print.name + " " + logType + ": " + text);
				else if(logType == Functions::printKeyWordError) Error(Functions::print.name + " " + logType + ": " + text);
				else
				{
					Error(startLogText+" unrealesed print keyword " + logTypeInCode + infoAddForLog);
					Warning(Functions::print.name +": " + text);
				}
			}
			else Error(startLogText+" print nothing" + infoAddForLog);

			return nullptr;
		}
		// sendFrame
		else if(function->name == Functions::sendFrame.name)
		{
			// sendFrame нелья перенести в PointAccess Worker-а:
			if(0) qdbg << CodeMarkers::note;	// нельзя создать Worker PointAccess SendFrame потому что при создании фрейма
												//вызывается Protocol::DoEverything

			if(params[0])
			{
				Worker* workerToSend = nullptr;
				if(Worker* tmpPtr {dynamic_cast<Worker*>(params[0])}) workerToSend = tmpPtr;
				if(WorkerPtr* tmpPtr {dynamic_cast<WorkerPtr*>(params[0])}) workerToSend = tmpPtr->GetWorkerPtr();

				if(workerToSend)
				{
					if(0) { qdbg << CodeMarkers::note << "Не нужно сюда пытаться добавить получение Value виджетов из виджетов"
														 "через создание в Worker указателя на дочерние виджеты"
														 "потому что у одного Worker-а может быть много виджетов"
														 "и не известно у какого именно виджет вытаскивать Value"
														 "создаем в Worker-е переменную и меняем её значение через обработчик в виджете"; }
					auto workersAndFrames { ConstructFrames(workerToSend, nullptr) };
					SendFrames(workersAndFrames);
				}
				else Error(startLogText+" param[0] is unrealesed class [" + params[0]->GetClassName() + "]" + infoAddForLog);
			}
			else Error(startLogText+" param[0] is nullptr");

			return nullptr;
		}
		// doCommands
		else if(function->name == Functions::doCommands.name)
		{
			// doCommands нелья перенести в PointAccess FrameWorker-а:
			if(0) 			qdbg << CodeMarkers::note;	// нельзя создать FrameWorker PointAccess SendFrame потому что при создании фрейма
														//вызывается Protocol::DoEverything;

			if(params[0])
			{
				Worker* fwToDoCommands = nullptr;
				if(Worker* tmpPtr {dynamic_cast<Worker*>(params[0])}) fwToDoCommands = tmpPtr;
				if(WorkerPtr* tmpPtr {dynamic_cast<WorkerPtr*>(params[0])}) fwToDoCommands = tmpPtr->GetWorkerPtr();

				if(fwToDoCommands)
				{
					auto commands = fwToDoCommands->Instructions();
					CANMsg_t pustishka;
					for(auto &command:commands)
					{
						this->DoEverything(command,nullptr,fwToDoCommands,pustishka,nullptr,DEretNullptr());
					}
				}
				else Error(startLogText+" param[0] is unrealesed class [" + params[0]->GetClassName() + "]" + infoAddForLog);
			}
			else Error(startLogText+" param[0] is nullptr");

			return nullptr;
		}
		// Stop
		else if(function->name == Functions::stop.name)
		{
			Error(startLogText+" stop "+CodeMarkers::mock);
			return nullptr;
		}
		// if_
		else if(function->name == Functions::if_.name)
		{
			if(!params[0]) Error(startLogText+" result is nullptr" + infoAddForLog);
			else
			{
				if(params[0]->GetClassName() == IOperand::classValue())
				{
					QString res = static_cast<Value*>(params[0])->IOGetValue(ValuesTypes::text);
					shared_ptr<Value> resValue;
					if(res == Operators::cmpResultFalse)
					{
						resValue = make_shared<Value>(Operators::ifResultFalse, ValuesTypes::text);
					}
					else if(res == Operators::cmpResultTrue)
					{
						resValue = make_shared<Value>(Operators::ifResultTrue, ValuesTypes::text);
					}
					else
					{
						Error(startLogText+" result is " + res + infoAddForLog);
						resValue = make_shared<Value>(ValuesTypes::undefined, ValuesTypes::text);
					}

					createdIOperands.push_back(resValue);
					return resValue.get();
				}
				else Error(startLogText+" result class name is " + params[0]->GetClassName() + infoAddForLog);
			}

			return nullptr;
		}
		// FindParamsInThisCategory
		else if(function->name == Functions::FindParamsInThisCategory.name)
		{
			if(!frameWorker)
			{
				Error(startLogText+" frameWorker is nullptr " + infoAddForLog);
				return nullptr;
			}

			QString paramName = params[0]->IOGetValue(ValuesTypes::text);
			QString categoryName = frameWorker->CategoryCell();
			bool includeSubCategories = false;
			if(paramsCountFact == 2)
			{
				QString subCatStrMarker = params[1]->IOGetValue(ValuesTypes::text);
				if(subCatStrMarker == Functions::keyWordIncludeSubCats) includeSubCategories = true;
				else Error(startLogText + " wrong keyword " + subCatStrMarker);
			}

			auto findedParams = connectedObject->FindParamsInCategory(categoryName,paramName,includeSubCategories);
			shared_ptr<VectorParams> vectParams = make_shared<VectorParams>();
			createdIOperands.push_back(vectParams);

			if(!findedParams.empty())
				vectParams->Set(findedParams, Functions::FindParamsInThisCategory.name+"("+categoryName+", "+paramName+")");
			else Warning(startLogText+" can't find params in" + infoAddForLog);

			return vectParams.get();
		}
		// FindParamsInCategory
		else if(function->name == Functions::FindParamsInCategory.name)
		{
			if(!frameWorker)
			{
				Error(startLogText+" frameWorker is nullptr " + infoAddForLog);
				return nullptr;
			}

			QString categoryName = params[0]->IOGetValue(ValuesTypes::text);
			QString paramName = params[1]->IOGetValue(ValuesTypes::text);

			bool includeSubCategories = false;
			if(paramsCountFact == 3)
			{
				QString subCatStrMarker = params[2]->IOGetValue(ValuesTypes::text);
				if(subCatStrMarker == Functions::keyWordIncludeSubCats) includeSubCategories = true;
				else Error(startLogText + " wrong keyword " + subCatStrMarker);
			}

			auto findedParams = connectedObject->FindParamsInCategory(categoryName,paramName,includeSubCategories);
			shared_ptr<VectorParams> vectParams = make_shared<VectorParams>();
			createdIOperands.push_back(vectParams);

			if(!findedParams.empty())
				vectParams->Set(findedParams, Functions::FindParamsInCategory.name+"("+categoryName+", "+paramName+")");
			else Warning(startLogText+" can't find params in" + infoAddForLog);

			return vectParams.get();
		}
		// FindFWInThisCategory
		else if(function->name == Functions::FindWorkerInThisCategory.name)
		{
			if(!frameWorker)
			{
				Error(startLogText+" frameWorker is nullptr " + infoAddForLog);
				return nullptr;
			}

			QString paramName = params[0]->IOGetValue(ValuesTypes::text);
			QString categoryName = frameWorker->CategoryCell();
			bool includeSubCategories = false;
			if(paramsCountFact == 2)
			{
				QString subCatStrMarker = params[1]->IOGetValue(ValuesTypes::text);
				if(subCatStrMarker == Functions::keyWordIncludeSubCats) includeSubCategories = true;
				else Error(startLogText + " wrong keyword " + subCatStrMarker);
			}

			auto findedFWs = FindWorkersInCategory(categoryName,paramName,includeSubCategories);

			if(findedFWs.size())
			{
				if(findedFWs.size() > 1) Error(startLogText+" find more 1 " + infoAddForLog);
				return findedFWs[0];
			}
			else Error(startLogText+" find nothing" + infoAddForLog);
			return nullptr;
		}
		// FindParams
		else if(function->name == Functions::FindParams.name)
		{
			QString paramName(params[0]->IOGetValue(ValuesTypes::text));

			auto findedParams = connectedObject->FindParamsByName(paramName);
			shared_ptr<VectorParams> vectParams = make_shared<VectorParams>();
			createdIOperands.push_back(vectParams);

			if(!findedParams.empty())
				vectParams->Set(findedParams, Functions::FindParams.name+"("+paramName+")");
			else Warning(startLogText+" find nothing" + infoAddForLog);

			return vectParams.get();
		}
		// FindWorker
		else if(function->name == Functions::FindWorker.name)
		{
			QString frameWorkerName(params[0]->IOGetValue(ValuesTypes::text));

			auto findedFrameWorkers = FindWorkers(frameWorkerName);
			if(findedFrameWorkers.size() == 1)
			{
				return findedFrameWorkers[0];
			}
			else Error(startLogText+" FindWorkers result wrong size ("+findedFrameWorkers.size()+")" + infoAddForLog);
			return nullptr;
		}
		else Error(startLogText+" unrealesed function" + infoAddForLog);
	}
	else Error(startLogText+" wrong params count. Expected min=" + QSn(paramsCountMin) + " max=" + QSn(paramsCountMax)
			   +" but get "+QSn(paramsCountFact) + infoAddForLog);

	return nullptr;
}

IOperand* Protocol::DoExpression(QString command, Expression &expression, Worker *frameWorker, QString retType)
{
	ClearOldCreatedIOperands();

	if(!expression.CheckNullptr())
	{
		Error("DoExpression: Expression contains nullptr!" + expression.ToStrForLog2(command));
		return nullptr;
	}

	if(expression.Size() == 1)
	{
		auto className = expression.Word(0).ptr->GetClassName();
		if(IOperand::allIOClassNames().contains(className))
			return static_cast<IOperand*>(expression.Word(0).ptr);
		else
		{
			Error("DoExpression: Expression of 1 word, but word ["+className+"] is not IOperand" + expression.ToStrForLog2(command));
			return nullptr;
		}
	}

	Expression expressionBefore = expression;
	while(expression.Size() > 1)
	{
		expressionBefore = expression;
		if(!expression.CheckNullptr())
		{
			Error("DoExpression: Expression contains nullptr" + expression.ToStrForLog2(command));
			return nullptr;
		}
		int min = Operator::udefinedPriority;
		const int undefIndex = -1;
		int indexMaxPrior = undefIndex;
		for(uint i=0; i<expression.Size(); i++)
		{
			if(expression.Word(i).ptr->GetClassName() == Operator::classOperator())
			{
				int priorI = static_cast<Operator*>(expression.Word(i).ptr)->priority;
				if(min == Operator::udefinedPriority || priorI < min)
				{
					min = priorI;
					indexMaxPrior = i;
				}
			}
		}

		if(indexMaxPrior != undefIndex)
		{
			int leftOperandIndex = indexMaxPrior-1;
			int operatorIndex = indexMaxPrior;
			int rghtOperandIndex = indexMaxPrior+1;
			Operator* oper = static_cast<Operator*>(expression.Word(operatorIndex).ptr);

			int countErase = 0;
			int eraseIndex = operatorIndex;

			if(oper->type == Operator::unarn)
			{
				if(leftOperandIndex >= 0 && leftOperandIndex < (int)expression.Size())
				{
					IOperand* leftOperand = static_cast<IOperand*>(expression.Word(leftOperandIndex).ptr);
					if(0)
					{
						Log(leftOperand->ToStrForLog());
					}
					else Error("DoExpression unrealesed unarn operator " + oper->oper + expression.ToStrForLog2(command));
				}
				else Error("DoExpression wrong leftOperandIndex (" + QSn(leftOperandIndex)
						   + ") (size="+QSn(expression.Size())+")" + expression.ToStrForLog2(command));
			}
			else if(oper->type == Operator::binarn)
			{
				if(leftOperandIndex >= 0 && leftOperandIndex < (int)expression.Size()
						&& rghtOperandIndex > 0 && rghtOperandIndex < (int)expression.Size())
				{
					IOperand* leftOperand = static_cast<IOperand*>(expression.Word(leftOperandIndex).ptr);
					IOperand* rghtOperand = static_cast<IOperand*>(expression.Word(rghtOperandIndex).ptr);
					if(oper->oper == Operators::point)
					{
						IOperand* IOResPointAccess = leftOperand->PointAccess(rghtOperand->IOGetValue(ValuesTypes::text));
						expression.SetWord(leftOperandIndex,IOResPointAccess,expression.Word(leftOperandIndex).str
										   + expression.Word(operatorIndex).str + expression.Word(rghtOperandIndex).str);
						countErase = 2;
					}
					else if(Operators::cmpOperators.contains(oper->oper))
					{
						bool res = leftOperand->CmpOperation(rghtOperand, oper->oper);
						QString strRes = Operators::cmpResultTrue;
						if(!res) strRes = Operators::cmpResultFalse;
						std::shared_ptr<IOperand> cmpRes = make_shared<Value>(strRes,ValuesTypes::text);
						createdIOperands.push_back(cmpRes);
						expression.SetWord(leftOperandIndex,cmpRes.get(),expression.Word(leftOperandIndex).str
										   + expression.Word(operatorIndex).str + expression.Word(rghtOperandIndex).str);
						countErase = 2;
					}
					else if(oper->oper == Operators::assign)
					{
						leftOperand->Assign(rghtOperand);
						expression.ChangeStrWord(leftOperandIndex, expression.Word(leftOperandIndex).str
												 + expression.Word(operatorIndex).str + expression.Word(rghtOperandIndex).str);
						countErase = 2;
					}
					else if(oper->oper == Operators::minus)
					{
						createdIOperands.push_back(leftOperand->Minus(rghtOperand));
						expression.SetWord(leftOperandIndex,createdIOperands.back().get(),expression.Word(leftOperandIndex).str
										   + expression.Word(operatorIndex).str + expression.Word(rghtOperandIndex).str);
						countErase = 2;
					}
					else if(oper->oper == Operators::plusAssign)
					{
						leftOperand->PlusAssign(rghtOperand);
						expression.ChangeStrWord(leftOperandIndex, expression.Word(leftOperandIndex).str
												 + expression.Word(operatorIndex).str + expression.Word(rghtOperandIndex).str);
						countErase = 2;
					}
					else Error("DoExpression unrealesed binarn operator " + oper->oper + expression.ToStrForLog2(command));
				}
				else Error("DoExpression wrong leftOperandIndex (" + QSn(leftOperandIndex)
						   + ") or rghtOperandIndex (" + QSn(rghtOperandIndex)
						   + ") (size="+QSn(expression.Size())+")" + expression.ToStrForLog2(command));
			}
			else if(oper->type == Operator::canHaveNestedExpr)
			{
				if(oper->oper == Operators::index1)
				{
					if(leftOperandIndex >= 0 && leftOperandIndex < (int)expression.Size())
					{
						auto leftOperandIO = static_cast<IOperand*>(expression.Word(leftOperandIndex).ptr);
						int index = 0;
						Expression nestedExpession = expression.GetNestedWords(operatorIndex);
						IOperand *nestedExpessionResult = DoExpression(command, nestedExpession, frameWorker, DEretPtr());
						if(nestedExpessionResult) index = nestedExpessionResult->IOGetValue(ValuesTypes::sLong).toLongLong();
						else Error("DoExpression nestedExpessionResult is nullptr");
						std::shared_ptr<IOperand> itemOnIndex = leftOperandIO->Index(index);
						createdIOperands.push_back(itemOnIndex);
						expression.SetWord(leftOperandIndex, itemOnIndex.get(),
										   expression.Word(leftOperandIndex).str+Operators::index1+QSn(index)+Operators::index2);

						countErase = nestedExpession.Size() + 2;
					}
					else Error("DoExpression wrong leftOperandIndex (" + QSn(leftOperandIndex)
							   + ") (size="+QSn(expression.Size())+")" + expression.ToStrForLog2(command));
				}
				else if(oper->oper == Operators::roundBracer1)
				{
					if(leftOperandIndex >= 0 && leftOperandIndex < (int)expression.Size())
					{
						if(expression.Word(leftOperandIndex).ptr->GetClassName() == Function::classFunction())
						{
							auto functionOperand = static_cast<Function*>(expression.Word(leftOperandIndex).ptr);
							vector<IOperand*> params;
							auto nestedExpession = expression.GetNestedWords(operatorIndex);

							if(nestedExpession.Size())
							{
								auto paramsExpressions = Expression::SplitWordsAsFunctionParams(nestedExpession);
								for(auto &expr:paramsExpressions)
									params.push_back(DoExpression("nested expression <"+expr.CommandFromStrWords()+">",
																  expr, frameWorker, DEretPtr()));
							}

							auto funcRes = DoFunction(command, functionOperand, params, frameWorker);
							expression.SetWord(leftOperandIndex, funcRes, functionOperand->name+" "+nestedExpession.CommandFromStrWords());
							countErase = nestedExpession.Size() + 2;

							if(functionOperand->name == Functions::if_.name)
							{
								if(leftOperandIndex == 0)
								{
									if(funcRes)
									{
										if(funcRes->GetClassName() == IOperand::classValue())
										{
											Value *resValue = static_cast<Value*>(funcRes);
											QString res = resValue->IOGetValue(ValuesTypes::text);
											if(res == Operators::ifResultFalse)
											{
												countErase = expression.Size() -1;
												eraseIndex = 1;
											}
											else if(res == Operators::ifResultTrue)
											{
												countErase = nestedExpession.Size() + 3;
												eraseIndex = 0;
												if(countErase > (int)expression.Size())
													Error("DoExpression function if result is " + res
														  + " but erase all expression words!" + expression.ToStrForLog2(command));
											}
											else Error("DoExpression function if result is " + res + expression.ToStrForLog2(command));
										}
										else Error("DoExpression wrong if result classname: " +
												   funcRes->GetClassName() + expression.ToStrForLog2(command));
									}
									else Error("DoExpression wrong if result is nullptr" + expression.ToStrForLog2(command));
								}
								else Error("DoExpression function if is not 1st operand" + expression.ToStrForLog2(command));
							}
						}
						else Error("DoExpression oper is roundBracer1 but leftOperand ["+expression.Word(leftOperandIndex).ptr->GetClassName()+
								   "] is not " + Function::classFunction() + expression.ToStrForLog2(command));
					}
					else Error("DoExpression wrong leftOperandIndex (" + QSn(leftOperandIndex)
							   + ") (size="+QSn(expression.Size())+")" + expression.ToStrForLog2(command));
				}
				else Error("DoExpression canHaveNestedExpr: wrong operator " + oper->oper + expression.ToStrForLog2(command));
			}
			else Error("DoExpression wrong Operator type [" + QSn(oper->type) + "][" + Operator::TypeToStr(oper->type) + "]"
					   + expression.ToStrForLog2(command));

			for(int i=0; i<countErase; i++)
			{
				if(eraseIndex >= 0 && eraseIndex < (int)expression.Size())
					expression.Erase(eraseIndex);
				else Error("DoExpression wrong eraseIndex ["+QSn(eraseIndex)+"] (expression.Size() = "+QSn(expression.Size())+")"
						   + expression.ToStrForLog2(command));
			}
		}
		else Error("DoExpression в выражении не обнаружены операции" + expression.ToStrForLog2(command));

		if(expression == expressionBefore)
		{
			Error("DoExpression expression not changed, break cycle" + expression.ToStrForLog2(command));
			break;
		}
	}

	IOperand *retIO = nullptr;
	if(expression.Size() == 1)
	{
		if(retType == Protocol::DEretNullptr())
		{
			expression.SetWord(0, nullptr, expression.Word(0).str);
		}
		else if(retType == Protocol::DEretPtr())
		{
			if(expression.Word(0).ptr) retIO = static_cast<IOperand*>(expression.Word(0).ptr);
			else Error("DoExpression: retType is " + retType + " but ret is nullptr" + expression.ToStrForLog2(command));
		}
		else Error("DoEverything2: unrealesed retType " + retType + expression.ToStrForLog2(command));
	}
	else
	{
		Error("DoExpression: doing expression finished, but word count = "+QSn(expression.Size())
			  +", but must be 1" + expression.ToStrForLog2(command));
		return nullptr;
	}
	return retIO;
}

QString Protocol::DataOperandFromWords(QStringList words, int DataOperandIndex, int & countWordsInDataOperand, Worker *worker)
{
	countWordsInDataOperand = 0;
	QString dataOperand = CodeKeyWords::dataOperand;
	int startFrom = DataOperandIndex+1;
	for(int twice=0; twice<2; twice++)
	{
		int countNestedBracers = 0;
		if(startFrom < words.size() && words[startFrom] == Operators::index1)
			for(int i=startFrom; i<words.size(); i++)
			{
				if(words[i] == Operators::index1) countNestedBracers++;
				if(words[i] == Operators::index2) countNestedBracers--;

				if(Code::IsInteger(words[i])
						|| words[i] == Operators::index1
						|| words[i] == Operators::index2
						|| words[i] == Operators::minus
						|| words[i] == Operators::comma)
					dataOperand += words[i];
				else
				{
					CANMsg_t dummy;
					auto result = DoEverything(words[i], nullptr, worker, dummy, nullptr, DEretPtr());
					if(result)
					{
						bool ok;
						QString index = result->IOGetValue(ValuesTypes::uShort);
						index.toUShort(&ok);
						if(ok) dataOperand += index;
						else
						{
							dataOperand += words[i];
							Error("Bad data operand index definition ["+index+"]; result of decoding word ["
								  +words[i]+"] is ["+result->ToStrForLog()+"]");
						}
					}
					else
					{
						Error("nullptr result of decoding word ["+words[i]+"]");
					}
				}

				countWordsInDataOperand++;

				if(countNestedBracers == 0 && words[i] == Operators::index2)
				{
					startFrom = i+1;
					break;
				}
			}
	}

	return dataOperand;
}

IOperand* Protocol::DoEverything(QString command, Param *thisParam, Worker *worker, CANMsg_t &frame,
								 std::vector<Value> *WidgetValues, QString retType)
{
	ClearOldCreatedIOperands();

	QString commandForLog = "\n\tcommand [" + command + "]";
	if(Settings::logDoEvrythingCommands) Log("DoEverything start" + commandForLog);

	auto words { Code::CommandToWords(command) };
	if(words.empty())
	{
		Error("DoEverything2 words.empty()" + commandForLog);
		return nullptr;
	}

	for(auto &word:words)
	{
		if(!TextConstant::IsItTextConstant(word, false)
				&& word.contains(CodeKeyWords::wrongTextConstantSplitter))
		{
			Error("DoEverything: word [" + word + "] contains "+CodeKeyWords::wrongTextConstantSplitter
				  +" symbol. If your mean text constant you must use "+CodeKeyWords::textConstantSplitter+ commandForLog);
			return nullptr;
		}
	}

	if(0) { qdbg << CodeMarkers::toDo << "где ещё в DoEverything сделать мапы?"; }

	QStringList WidgetValuesNames;
	if(WidgetValues) for(auto &widgValue:*WidgetValues) WidgetValuesNames += widgValue.GetName();
	int widgetValueIndex = -1;
	QString prevWidgValue;

	IOperand* retOperand = nullptr;
	Expression expression;
	vector<shared_ptr<HaveClassName>> tmpObjects;
	bool prevWordIsOperatorPoint = false;
	for(int i=0; i<words.size(); i++)
	{
		prevWordIsOperatorPoint = false;
		if(expression.Size())
		{
			HaveClassName *prevWord = expression.Word(expression.Size()-1).ptr;
			if(prevWord && prevWord->GetClassName() == Operator::classOperator() && static_cast<Operator*>(prevWord)->oper == Operators::point)
				prevWordIsOperatorPoint = true;
		}

		// Operator
		if(Operators::all.contains(words[i]))
		{
			tmpObjects.push_back(make_shared<Operator>(words[i]));
			expression.AddWord(tmpObjects.back().get(),words[i]);
		}
		// PointAccess predicate
		else if(prevWordIsOperatorPoint)
		{
			tmpObjects.push_back(make_shared<Value>(words[i],ValuesTypes::text));
			expression.AddWord(tmpObjects.back().get(),words[i]);
		}
		// Function
		else if(Functions::Check::IsFunction(words[i]))
		{
			tmpObjects.push_back(make_shared<Function>(words[i]));
			expression.AddWord(tmpObjects.back().get(),words[i]);
		}
		// Number
		else if(Code::IsNumber(words[i]))
		{
			if(Code::IsInteger(words[i]))
			{
				tmpObjects.push_back(make_shared<Value>(words[i],Value::GetTypeByStrValue(words[i])));
				expression.AddWord(tmpObjects.back().get(),words[i]);
			}
			else Error("DoEverything: floating numbers [" + words[i] + "] unrealesed" + commandForLog);
		}
		// TextConstant
		else if(TextConstant::IsItTextConstant(words[i],false))
		{
			QString tmpStr(words[i]);
			tmpStr.replace("\\n","\n");
			tmpStr.replace("\\t","\t");
			tmpObjects.push_back(make_shared<Value>(std::move(tmpStr),ValuesTypes::textConstant));
			// если в конфигурации в текстовой константе встречается \n оно преобразуется в два отдельных символа \ и n
			// но нужно что бы программа воспринимала это как символ переноса строки
			expression.AddWord(tmpObjects.back().get(),words[i]);
		}
		// Constant
		else if(auto constant = constants->FindConstant(words[i]))
		{
			auto constEncValue = constant->value;
			tmpObjects.push_back(make_shared<Value>(constEncValue.ToBinStr(),ValuesTypes::binCode));
			expression.AddWord(tmpObjects.back().get(),words[i]);
		}
		// Variable
		else if(worker && worker->FindVariable(words[i], false))
		{
			IOperand *findedPeremennayaIO = worker->FindVariable(words[i],true);
			if(0) { qdbg << CodeMarkers::canBeOptimized << "тут дважды выполняется поиск FindVariable"
														   "из-за того, что нужно сначала проверить валидность указателя"
														   "нельзя внутри if сделать написать auto res = worker->FindVariable(words[i], false)"
														   "можно в Worker добавить GetLastFindVariableRes"; }
			if(findedPeremennayaIO)
			{
				expression.AddWord(findedPeremennayaIO,words[i]);
			}
			else Error("DoEverything: Can't find variable [" + words[i] + "]" + commandForLog);
		}
		// WidgetValues
		else if(int index = WidgetValuesNames.indexOf(words[i]); index != -1)
		{
			widgetValueIndex = index;
			prevWidgValue = WidgetValues->at(index).ToStrForLog();
			expression.AddWord(&WidgetValues->at(index),words[i]);
		}
		// Data operand
		else if(words[i] == CodeKeyWords::dataOperand)
		{
			int countWordsInDataOperand = 0;
			QString strDataOperand = DataOperandFromWords(words, i, countWordsInDataOperand, worker);
			i += countWordsInDataOperand;

			tmpObjects.push_back(make_shared<DataOperand>(frame,strDataOperand));
			expression.AddWord(tmpObjects.back().get(),words[i]);
		}
		// thisParam
		else if(words[i] ==  CodeKeyWords::thisParam)
		{
			if(thisParam)
				expression.AddWord(thisParam,words[i]);
			else Error("DoEverything: thisParam is nullptr" + commandForLog);
		}
		// thisFrameWorker
		else if(words[i] ==  CodeKeyWords::thisFrameWorker)
		{
			if(worker)
				expression.AddWord(worker,words[i]);
			else Error("DoEverything: thisFrameWorker is nullptr" + commandForLog);
		}
		else
		{
			Error("DoEverything: unknown word [" + words[i] + "]" + commandForLog);
			return nullptr;
		}
	}

	//	QString debugStopper = "Сервис FindParamInCategoryNumber ( \"КС уставок в ПЗУ\",Data [ 2 ] ) [ Data [ 1 ] ] = Data [ 3,4 ]";
	//	if(command.contains(debugStopper))
	//		int debug = 5;

	retOperand = DoExpression(command, expression, worker, retType);

	if(widgetValueIndex != -1)
	{
		if(prevWidgValue != WidgetValues->at(widgetValueIndex).ToStrForLog())
			Warning("DoEverything: WidgetValue was changed, but it is senselessly");
	}

	if(retType == Protocol::DEretNullptr()) retOperand = nullptr;
	else if(retType == Protocol::DEretPtr())
	{
		if(!retOperand)
			Error("DoEverythin: retType is " + retType + " but ret is nullptr" + commandForLog);
		else
		{
			for(auto &tmpObj:tmpObjects)
			{
				// если возвращаем объект из tmpObjects, то сохраняем его в createdHaveClassNames
				// чтобы автомато не удалился
				if(retOperand == dynamic_cast<decltype(retOperand)>(tmpObj.get()))
					createdHaveClassNames.push_back(tmpObj);
			}
		}
	}
	else Error("DoEverything: unrealesed retType " + retType + commandForLog);

	if(Settings::logDoEvrythingCommands) Log("DoEverything end" + commandForLog);

	return retOperand;
}

void Protocol::ClearOldCreatedIOperands()
{
	while(createdIOperands.size() >= 1000)
		createdIOperands.pop_front();

	while(createdHaveClassNames.size() >= 1000)
		createdHaveClassNames.pop_front();

	while(Param::createdIOperands.size() >= 5000)
		Param::createdIOperands.pop_front();
}

ItemsListForFillCats Protocol::GetItemsListForFillCats()
{
	ItemsListForFillCats ret;
	for(auto &fw:workers)
	{
		ret.items.push_back(fw.get());
		ret.categoryStrVals.push_back(fw->CategoryCell());
	}
	return ret;
}
