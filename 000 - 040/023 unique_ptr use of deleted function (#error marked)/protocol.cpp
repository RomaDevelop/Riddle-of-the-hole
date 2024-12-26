#include <QObject>
#include <QTimer>

#include "configconstants.h"
#include "settings.h"
#include "protocol.h"

using namespace std;

Protocol::Protocol(const std::vector<QStringList> &frameWorkersStrs, Object *objectToConnect_, QString version, QString name_, int type_, Constants *constants_, int channel_):
	name {name_},
	connectedObject {objectToConnect_},
	constants{constants_},
	channel{channel_},
	type {type_}
{
	if(version == "001")
	{
		constantsNames = constants->NamesList();

		int frWorkersCount = frameWorkersStrs.size();
		frameWorkers.resize(frWorkersCount);
		for(int i=0; i<frWorkersCount; i++)
		{
			frameWorkers[i] = make_unique<FrameWorker>();
			frameWorkers[i]->InitFrameWorker(frameWorkersStrs[i], this);
		}

		if(frWorkersCount)
		{
			auto headCat = CategoryPath::FirstCatFromPath(frameWorkers[0]->CategoryCell());
			category.Set(headCat.name, headCat.number, nullptr);
		}
		if(category.GetNameNumber().name == CategoryKeywords::undefined)
			Error("Protocol::Protocol undefined head category");
		for(int i=0; i<frWorkersCount; i++)
		{
			auto catPath = CategoryPath::PathFromQString(frameWorkers[i]->CategoryCell());
			category.InsertMember(*frameWorkers[i].get(), catPath, true);
		}

		InitVariables();
		/// InitVariables() должно делаться после создания ВСЕХ params и frameWorkers
		/// поскольку variables могут ссылаться на параметры и обработчики

		timerMsgsWorker = new QTimer;
		QObject::connect(timerMsgsWorker, &QTimer::timeout, [this](){ this->WorkerIncommingFrames(); });
		timerMsgsWorker->start(100);
	}

	Log("Protocol " + name + " created");
}

void Protocol::InitFilters()
{
	for(auto &frameWorker:frameWorkers)
	{
		if(frameWorker->FrameCell() != "")
		{
			QStringList commands { Code::TextToCommands(frameWorker->FrameCell()) };
			bool idSet = false;
			for(auto &cmnd:commands)
			{
				if(cmnd.left(CommandsKeyWords::idFilter.length()) == CommandsKeyWords::idFilter)
				{
					if(idSet) Error("InitFilters: multiple ID setting. Param id: " + frameWorker->IdCell() + "; frame: " + frameWorker->FrameCell());

					QStringList idFiltersStrs = { frameWorker->GenerateIDStrsList(cmnd) };
					for(int i=0; i<idFiltersStrs.size(); i++)
					{
						idSet = true;
						CANMsgIDFilter_t filter;
						filter.mask = 0xFFFFFFFF;
						filter.filter = frameWorker->ConstuctID(idFiltersStrs[i], constants);
						frameWorker->AddIdFilter(filter);
					}
				}

				if(cmnd.left(CommandsKeyWords::dataFilter.length()) == CommandsKeyWords::dataFilter)
				{
					auto words = Code::CommandToWords(cmnd);
					if(words.size() >= 4)
					{
						words.removeFirst();
						if(words[0].contains(CommandsKeyWords::dataOperand))
						{
							int countWordsInDataOperand=0;
							QString strDataOperand = DataOperand::DataOperandFromWords(words, 0, countWordsInDataOperand);

							CANMsgDataFilter_t newFilter;
							CANMsg_t frame;
							DataOperand::SetDataFilterMaskFromOperand(strDataOperand,newFilter);
							DoEverything(words.join(' '), frameWorker.get(), frame, DEretNullptr());
							for(uint i=0; i<frame.length; i++)
								newFilter.filter[i] = frame.data[i];
							frameWorker->SetDataFilter(newFilter);
						}
						else Error("InitFilters: wrong data filter command. Command: " + cmnd);
					}
					else Error("InitFilters: wrong words size in dataFilter command. Command: " + cmnd);
				}
			}

			if(!idSet) Error("InitFilters: ID not set. Param id: " + frameWorker->IdCell() + " frame " + frameWorker->FrameCell());
		}
	}
}

void Protocol::InitVariables()
{
	for(uint i=0; i<frameWorkers.size(); i++)
	{
		auto commandList {Code::TextToCommands(frameWorkers[i]->VariablesCell())};
		for(auto &command:commandList)
		{
			auto words {Code::CommandToWords(command)};
			if(words.size() >= 2)
			{
				Operator::CorrectVectorDefinition(words);
				QString tip = {words[0]};
				QString peremName {words[1]};
				if(Code::IsNumber(peremName)) Error("Object::InitVariables: wrong peremennaya name in command ["+command+"]");

				QString initStr = Code::GetInitialisationStr(command, false);

				if(ValuesTypes::all.contains(tip))
				{
					frameWorkers[i]->variables.push_back(std::make_shared<Value>(peremName,"",tip));
					frameWorkers[i]->variablesNames.push_back(peremName);
					Value *createdVariable = static_cast<Value *>(frameWorkers[i]->variables.back().get());

					if(initStr != "")
					{
						createdVariable->Set(initStr,tip);
					}
					else { /* nothing to do */ }
				}
				else if(tip == ParamPtr::InCofigCode())
				{
					if(initStr != "")
					{
						CANMsg_t framePustishka;
						auto ptr = DoEverything(initStr,frameWorkers[i].get(),framePustishka,DEretPtr());
						if(ptr)
						{
							if(ptr->GetClassName() == IOperand::classParam())
							{
								frameWorkers[i]->variables.push_back(make_shared<ParamPtr>((Param*)ptr,peremName));
								frameWorkers[i]->variablesNames.push_back(peremName);
							}
							else if(ptr->GetClassName() == IOperand::classVectorParams())
							{
								auto vectParams = static_cast<VectorParams*>(ptr);
								if(vectParams->Get().size() == 1)
								{
									Param* ptrPramam0 = vectParams->Get()[0];
									frameWorkers[i]->variables.push_back(make_shared<ParamPtr>(ptrPramam0,peremName));
									frameWorkers[i]->variablesNames.push_back(peremName);
								}
								else Error("Object::InitVariables wrong result init str: VectorParams with size " + QSn(vectParams->Get().size()));
							}
							else Error("Object::InitVariables unrealesed result init str [" + ptr->GetClassName() + "]");
						}
						else Error("Object::InitVariables can't find Param by initStr " + initStr);
					}
					else Error("Object::InitVariables initing " + tip + " whith empty initStr");
				}
				else if(tip == FrameWorkerPtr::InCofigCode())
				{
					if(initStr != "")
					{
						CANMsg_t framePustishka;
						auto ptr = DoEverything(initStr,frameWorkers[i].get(),framePustishka,DEretPtr());
						if(ptr)
						{
							if(ptr->GetClassName() == IOperand::classFrameWorker())
							{
								frameWorkers[i]->variables.push_back(make_shared<FrameWorkerPtr>((FrameWorker*)ptr,peremName));
								frameWorkers[i]->variablesNames.push_back(peremName);
							}
							else Error("Object::InitVariables unrealesed result init str [" + ptr->GetClassName() + "]");
						}
						else Error("Object::InitVariables can't find Param by initStr " + initStr);
					}
					else Error("Object::InitVariables initing " + tip + " whith empty initStr");
				}
				else if(tip == VectorParams::InCofigCode())
				{
					if(initStr != "")
					{
						CANMsg_t framePustishka;
						auto ptr = DoEverything(initStr,frameWorkers[i].get(),framePustishka,DEretPtr());
						if(ptr && ptr->GetClassName() == IOperand::classVectorParams())
						{
							frameWorkers[i]->variables.push_back(make_shared<VectorParams>());
							frameWorkers[i]->variablesNames.push_back(peremName);
							VectorParams *createdVectorParams = static_cast<VectorParams*>(frameWorkers[i]->variables.back().get());
							createdVectorParams->Set(static_cast<VectorParams *>(ptr)->Get(), peremName);
						}
						else Error("Object::InitVariables " + initStr + " result is not " + tip);
					}
					else Error("Object::InitVariables initing " + tip + " whith empty initStr");
				}
				else Error("Object::InitVariables unrealesed type variable " + command);
			}
			else Error("Object::InitVariables wrong words count " + command);
		} /* for */
	}/* for */
}

std::vector<FrameWorker*> Protocol::FindFrameWorkersInCategory(const QString &category_, const QString &nameFW, bool includeSubCats)
{
	std::vector<FrameWorker*> findedFW;
	auto path = CategoryPath::PathFromQString(category_);
	auto findedCat = category.FindCategory(path,false);
	if(!findedCat) Warning("Protocol::FindFrameWorkersInCategory category [" + category_ + "] doesn't exists");
	else
	{
		auto members = findedCat->GetMembers(includeSubCats);
		for(auto member:members)
		{
			FrameWorker *fw = dynamic_cast<FrameWorker*>(member);
			if(!fw) Error("Protocol::FindFrameWorkersInCategory wrong member! It is " + member->GetClassName());
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

FrameWorker* Protocol::FindFrameWorkerById(const QString &id)
{
	FrameWorker* findedFW {nullptr};
	int size = frameWorkers.size();
	for(int i=0; i<size; i++)
	{
		if(frameWorkers[i]->IdCell() == id)
		{ findedFW = frameWorkers[i].get(); break; }
	}

	return findedFW;
}

void Protocol::SendFrames(std::vector<FrameWorkerAndFrame> &toSend)
{
	for(auto &frame:toSend)
	{
		Protocol *prot = (Protocol *)frame.frameWorker->ProtocolParent();
		if(!prot) Error("Wrorng behavior objectParent is nullptr");
		else prot->SendFrame(frame.frame);

	}
}

void Protocol::SendFrame(const CANMsg_t &msg)
{
	if(network.empty() || (network.size() == 1 && network[0] == this))
	{
		Log("Protocol::SendMsg нет соединений ни с кем");
		Log("tryed to send: " + msg.ToStrEx2(2));
	}

	for(auto pr:network)
		if(pr != this)
		{
			pr->msgs.push(msg);
			Log("send: " + msg.ToStrEx2(2));
		}
}

void Protocol::WorkerIncommingFrames()
{
	for(uint i=0; i<msgs.size(); i++)
	{
		FrameWorker *frW = CheckFilters(msgs.back());

		if(frW)
		{
			Log("frame passed filter: " + msgs.back().ToStrEx2(2));
			WorkerFramePassedFilter(msgs.back(), frW);
		}
		else Warning("frame not passed filter: " + msgs.back().ToStrEx2(2));

		msgs.pop();
	}
}

void Protocol::SetDataFilterInFrame(CANMsg_t &frame, CANMsgDataFilter_t filter)
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

std::vector<FrameWorkerAndFrame> Protocol::ConstructFrames(FrameWorker *frameWorker)
{
	CANMsg_t msg;
	QStringList commandList {Code::TextToCommands(frameWorker->FrameCell())};
	for(auto &command:commandList)
	{
		auto words {Code::CommandToWords(command)};

		if(words[0] == CommandsKeyWords::idFilter) msg.ID = frameWorker->OutgoingID();
		else if(words[0] == CommandsKeyWords::dataFilter)
		{
			SetDataFilterInFrame(msg, frameWorker->DataFilter());
		}
		else DoEverything(command, frameWorker, msg, Protocol::DEretNullptr());
	}

	std::vector<FrameWorkerAndFrame> framesForSynch;
	framesForSynch.reserve(frameWorker->synchronFWs.size()+1);
	framesForSynch.push_back({frameWorker,msg});
	for(auto syParam:frameWorker->synchronFWs)
	{
		CANMsg_t syMsg(msg);
		syMsg.ID = syParam->OutgoingID();
		SetDataFilterInFrame(syMsg,syParam->DataFilter());
		framesForSynch.push_back({syParam,std::move(syMsg)});
	}

	return framesForSynch;
}

void Protocol::WorkerFramePassedFilter(CANMsg_t &frame, FrameWorker *frameWorker)
{
	auto cmndList { Code::TextToCommands(frameWorker->FrameCell()) };
	for(auto &command:cmndList)
	{
		auto words {Code::CommandToWords(command)};
		if(words.empty()) { Error("WorkIncommingFrame words.empty()"); return; }

		if(command.left(CommandsKeyWords::idFilter.length()) == CommandsKeyWords::idFilter
				|| command.left(CommandsKeyWords::dataFilter.length()) == CommandsKeyWords::dataFilter) // если это команда содержащая опредедение ID
		{
			/* тут мы ничего не делаем. ID и Data фильтры были сформированы ранее и установлен фильтр */
		}
		else
		{
			DoEverything(command, frameWorker, frame, Protocol::DEretNullptr());
		}
	}
}

FrameWorker* Protocol::CheckFilters(const CANMsg_t &msg)
{
	FrameWorker *retFW = nullptr;
	for(uint i=0; i<frameWorkers.size(); i++)
	{
		FrameWorker *fw = frameWorkers[i].get();
		if((type == emulator && fw->TypeCell() == FrameTypes::inFrame)
				|| (type == servis && fw->TypeCell() == FrameTypes::outFrame))
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

			if(chFilts)
			{
				if(retFW) Error("Object::CheckFilters filter passed not on one param!!!");
				retFW = fw;
			}
		}
	}

	return retFW;
}

IOperand* Protocol::DoFunction(QString command, Function* function, vector<IOperand*> params, FrameWorker *frameWorker)
{
	QString startLogText = "DoFunction "+function->name;
	QString infoAddForLog = "\n\tcommand [" + command + "]";
	int paramsCountMin = -1;
	int paramsCountMax = -1;
	int paramsCountFact = params.size();
	Functions::Check::ParamsCount(function->name,paramsCountMin,paramsCountMax);
	if(paramsCountFact >= paramsCountMin && paramsCountFact <= paramsCountMax)
	{
		// print
		if(function->name == Functions::print.name)
		{
			QString text;
			QString logTypeInCode = params[0]->IOGetValue(ValuesTypes::text);
			QString logType = TextConstant::GetTextConstVal(logTypeInCode);
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
		// sendFrame // можно будет перенести в pointAccess FrameWorker-a когда ConstructFrame и DoEveryting уйдет из Object-a
		else if(function->name == Functions::sendFrame.name)
		{
			if(params[0] && params[0]->GetClassName() == IOperand::classFrameWorkerPtr())
			{
				FrameWorker* fwToSend = static_cast<FrameWorkerPtr*>(params[0])->GetFrameWorkerPtr();
				auto paramsAndFrames { ConstructFrames(fwToSend) };
				SendFrames(paramsAndFrames);
			}
			else Error(startLogText+" param[0] is not " + IOperand::classParam() + ", it is " + params[0]->GetClassName() + infoAddForLog);

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
		// findParamInThisCategory
//		else if(function->name == Functions::FindNameInThisCategory.name)
//		{
//			QString paramName = TextConstant::GetTextConstVal(params[0]->IOGetValue(ValuesTypes::text));
//			QString categoryName = frameWorker->categoryCell;

//			auto findedParams = FindParamsInCategory(categoryName,paramName);
//			uint size = findedParams.size();
//			if(size)
//			{
//				if(size > 1)
//				{
//					QString str = "DoFunction find more 1 param. Categoty [" + categoryName + "]" + infoAddForLog;
//					for(auto p:findedParams) str += "\n\t param: " + p->CellValuesToStrForLog();
//					Warning(str);
//				}

//				return findedParams[0];
//			}
//			else
//			{
//				Error(startLogText+" can't find param in" + infoAddForLog);
//				return nullptr;
//			}
//		}
		// FindParamsInThisCategory
		else if(function->name == Functions::FindParamsInThisCategory.name)
		{
			QString paramName = TextConstant::GetTextConstVal(params[0]->IOGetValue(ValuesTypes::text));
			QString categoryName = frameWorker->CategoryCell();
			bool includeSubCategories = false;
			if(paramsCountFact == 2)
			{
				QString subCatStrMarker = TextConstant::GetTextConstVal(params[1]->IOGetValue(ValuesTypes::text));
				if(subCatStrMarker == Functions::keyWordIncludeSubCats) includeSubCategories = true;
				else Error(startLogText + " wrong keyword " + subCatStrMarker);
			}

			auto findedParams = connectedObject->FindParamsInCategory(categoryName,paramName,includeSubCategories);
			shared_ptr<VectorParams> vectParams = make_shared<VectorParams>();
			createdIOperands.push_back(vectParams);
			vectParams->Set(findedParams, "");

			if(findedParams.empty())
				Warning(startLogText+" can't find params in" + infoAddForLog);

			return vectParams.get();
		}
		// FindFWInThisCategory
		else if(function->name == Functions::FindFWInThisCategory.name)
		{
			QString paramName = TextConstant::GetTextConstVal(params[0]->IOGetValue(ValuesTypes::text));
			QString categoryName = frameWorker->CategoryCell();
			bool includeSubCategories = false;
			if(paramsCountFact == 2)
			{
				QString subCatStrMarker = TextConstant::GetTextConstVal(params[1]->IOGetValue(ValuesTypes::text));
				if(subCatStrMarker == Functions::keyWordIncludeSubCats) includeSubCategories = true;
				else Error(startLogText + " wrong keyword " + subCatStrMarker);
			}

			auto findedFWs = FindFrameWorkersInCategory(categoryName,paramName,includeSubCategories);

			if(findedFWs.size())
			{
				if(findedFWs.size() > 1) Error(startLogText+" find more 1 " + infoAddForLog);
				return findedFWs[0];
			}
			else Error(startLogText+" find nothing" + infoAddForLog);
			return nullptr;
		}
		// findParamInCategoryNumber
//		else if(function->name == Functions::findParamInCategoryNumber)
//		{
//			QString paramName = TextConstant::GetTextConstVal(params[0]->IOGetValue(ValuesTypes::text));
//			QString categoryNumber = params[1]->IOGetValue(ValuesTypes::sLLong);
//			Param *findedParam = FindParamInCategoryNumber(paramName,categoryNumber);

//			if(!findedParam) Error(startLogText+" can't find param in defined categoryNumber [" +
//								   categoryNumber + "]" + infoAddForLog);
//			return findedParam;
//		}
		// findParamInModule
//		else if(function->name == Functions::findParamInModule)
//		{
//			Param *findedParam {nullptr};
//			QString paramName = params[0]->IOGetValue(ValuesTypes::text);
//			int nomerModule = params[1]->IOGetValue(ValuesTypes::sLLong).toInt();

//			findedParam = FindParamInModuleNomer(nomerModule,paramName);

//			if(!findedParam) Error(startLogText+" can't find param" + infoAddForLog);
//			return findedParam;
//		}
		// findParamInCategoryAndModule
//		else if(function->name == Functions::findParamInCategoryAndModule)
//		{
//			Param *findedParam {nullptr};
//			QString paramName = params[0]->IOGetValue(ValuesTypes::text);
//			QString categoryName = params[1]->IOGetValue(ValuesTypes::text);
//			int nomerModule = params[1]->IOGetValue(ValuesTypes::sLLong).toInt();

//			findedParam = FindParamInCategoryAndModule(nomerModule,paramName,categoryName);

//			if(!findedParam) Error(startLogText+" can't find param " + infoAddForLog);
//			return findedParam;
//		}
		// findParams
//		else if(function->name == Functions::findParams)
//		{
//			shared_ptr<VectorParams> vectParams = make_shared<VectorParams>();
//			createdIOperands.push_back(vectParams);
//			vectParams->Set(FindParamsByName(params[0]->IOGetValue(ValuesTypes::text)),"");
//			return vectParams.get();
//		}
		else Error(startLogText+" unrealesed function" + infoAddForLog);
	}
	else Error(startLogText+" wrong params count. Expected min=" + QSn(paramsCountMin) + " max=" + QSn(paramsCountMax)
			   +" but get "+QSn(paramsCountFact) + infoAddForLog);

	return nullptr;
}

IOperand* Protocol::DoExpression(QString command, Expression &expression, FrameWorker *frameWorker, QString retType)
{
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
					else if(oper->oper == Operators::cmpEqual)
					{
						bool res = leftOperand->IsEqual(rghtOperand);
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
						   + "or rghtOperandIndex (" + QSn(rghtOperandIndex)
						   + ") (size="+QSn(expression.Size())+")" + expression.ToStrForLog2(command));
			}
			else if(oper->type == Operator::canHaveNestedExpr)
			{
				if(oper->oper == Operators::index1)
				{
					if(leftOperandIndex >= 0 && leftOperandIndex < (int)expression.Size())
					{
						auto leftOperandIO = static_cast<IOperand*>(expression.Word(leftOperandIndex).ptr);
						Expression nestedExpession = expression.GetNestedWords(operatorIndex);
						IOperand *nestedExpessionResult = DoExpression(command, nestedExpession, frameWorker, DEretPtr());
						QString strIndex = nestedExpessionResult->IOGetValue(ValuesTypes::sLong);
						int index = strIndex.toInt();
						std::shared_ptr<IOperand> itemOnIndex = leftOperandIO->Index(index);
						createdIOperands.push_back(itemOnIndex);
						expression.SetWord(leftOperandIndex, itemOnIndex.get(),
										   expression.Word(leftOperandIndex).str+Operators::index1+strIndex+Operators::index2);

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
							auto nestedExpession = expression.GetNestedWords(operatorIndex);

							auto paramsExpressions = Expression::SplitWordsAsFunctionParams(nestedExpession);
							vector<IOperand*> params;
							for(auto &expr:paramsExpressions)
								params.push_back(DoExpression("nested expression <"+expr.CommandFromStrWords()+">", expr, frameWorker, DEretPtr()));

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
										else Error("DoExpression wrong if result classname: " + funcRes->GetClassName() + expression.ToStrForLog2(command));
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

IOperand* Protocol::DoEverything(QString command, FrameWorker *frameWorker, CANMsg_t &frame, QString retType)
{
	QString commandForLog = "\n\tcommand [" + command + "]";
	if(Settings::logDoEvrythingCommands) Log("DoEverything2 start" + commandForLog);

//	QString debugStopper = "Сервис FindParamInCategoryNumber ( \"КС уставок в ПЗУ\",Data [ 2 ] ) [ Data [ 1 ] ] = Data [ 3,4 ]";
//	if(command.contains(debugStopper))
//		int debug = 5;

	auto words { Code::CommandToWords(command) };
	if(words.empty())
	{
		Error("DoEverything2 words.empty()" + commandForLog);
		return nullptr;
	}

	// Servis or Emulator
	bool servisOrEmulator = false;
	bool servisBool = false;
	bool emulatorBool = false;
	// если команда нам не соответсвует - выходим из функции
	if(words[0] == CommandsKeyWords::servis) { servisOrEmulator = true; servisBool = true; }
	if(words[0] == CommandsKeyWords::emulator) { servisOrEmulator = true; emulatorBool = true; }
	if(servisBool && this->type == emulator) return nullptr;
	if(emulatorBool && this->type == servis) return nullptr;
	if(servisOrEmulator) words.removeAt(0);

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
			tmpObjects.push_back(make_shared<Value>(words[i],ValuesTypes::text));
			expression.AddWord(tmpObjects.back().get(),words[i]);
		}
		// Constant
		else if(constantsNames.contains(words[i]))
		{
			auto constant = constants->FindConstant(words[i]);
			if(constant)
			{
				auto constEncValue = constant->value;
				tmpObjects.push_back(make_shared<Value>(constEncValue.ToBinStr(),ValuesTypes::binCode));
				expression.AddWord(tmpObjects.back().get(),words[i]);
			}
			else Error("DoEverything: can't find constant [" + words[i] + "]" + commandForLog);
		}
		// Variable
		else if(frameWorker->variablesNames.contains(words[i]))
		{
			IOperand *findedPeremennayaIO = frameWorker->FindVariable(words[i],true);
			if(findedPeremennayaIO)
			{
				expression.AddWord(findedPeremennayaIO,words[i]);
			}
			else Error("DoEverything: Can't find variable [" + words[i] + "]" + commandForLog);
		}
		// Widget
//		else if(param->ctrlWidgetsNames.contains(words[i]))
//		{
//			auto resFind {param->FindWidget(words[i]) };
//			if(resFind.first && resFind.second == WidgetTypes::whole)
//			{
//				tmpObjects.push_back(make_shared<Value>(static_cast<Widget*>(resFind.first)->GetWidgetValue()));
//				expression.AddWord(tmpObjects.back().get(),words[i]);
//			}
//			else if(resFind.first && resFind.second == WidgetTypes::part)
//			{
//				tmpObjects.push_back(make_shared<Value>(static_cast<WidgetPart*>(resFind.first)->GetWidgPartCurrentText(),ValuesTypes::text));
//				expression.AddWord(tmpObjects.back().get(),words[i]);
//			}
//			else Error("DoEverything error defining widget [" + words[i] + "]" + commandForLog);
//		}
		// Data operand
		else if(words[i] == CommandsKeyWords::dataOperand)
		{
			tmpObjects.push_back(make_shared<DataOperand>());
			expression.AddWord(tmpObjects.back().get(),words[i]);

			DataOperand *dataOperandPt = static_cast<DataOperand*>(tmpObjects.back().get());
			dataOperandPt->frame = &frame;
			int countWordsInDataOperand = 0;
			dataOperandPt->strDataOperand = DataOperand::DataOperandFromWords(words, i, countWordsInDataOperand);
			i += countWordsInDataOperand;
		}
		else Error("DoEverything: unknown word [" + words[i] + "]" + commandForLog);
	}

	//	QString debugStopper = "Сервис FindParamInCategoryNumber ( \"КС уставок в ПЗУ\",Data [ 2 ] ) [ Data [ 1 ] ] = Data [ 3,4 ]";
	//	if(command.contains(debugStopper))
	//		int debug = 5;

	retOperand = DoExpression(command, expression, frameWorker, retType);

	if(retType == Protocol::DEretNullptr()) retOperand = nullptr;
	else if(retType == Protocol::DEretPtr())
	{
		if(!retOperand) Error("DoEverythin: unrealesed retType is " + retType + " but ret is nullptr" + commandForLog);
	}
	else Error("DoEverything: unrealesed retType " + retType + commandForLog);

	if(Settings::logDoEvrythingCommands) Log("DoEverything end" + commandForLog);

	return retOperand;
}

QString Protocol::TypeToStr()
{
	if(type == emulator) return "emulator";
	if(type == servis) return "servis";
	if(type == undefined) return "undefined";
	Error("TypeToStr unnown type" + QSn(type));
	return "unnown type";
}
