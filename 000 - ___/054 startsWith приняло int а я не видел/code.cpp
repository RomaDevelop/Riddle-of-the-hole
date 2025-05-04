строка 41 сволочь

void Worker::InitWorker(const QDomElement &workerElement, Constants *constants, HaveClassName *aProtocolParent, int aProtocolType)
{
	auto attrs = MyQDom::Attributes(workerElement);
	if((int)attrs.size() == Worker_ns::count)
	{
		for(auto &attr:attrs)
		{
			if(attr.first == Worker_ns::category) categoryCell = attr.second;
			else if(attr.first == Worker_ns::name) nameCell = attr.second;
			else if(attr.first == Worker_ns::type) typeCell = attr.second;
			else if(attr.first == Worker_ns::variables) variablesCell = attr.second;
			else if(attr.first == Worker_ns::instructions) instructionsCell = attr.second;
			else Error(GetClassName()+"::InitWorker wrong param attribute ["+attr.first+"]");
		}
	}
	else Error(GetClassName()+"::InitWorker wrong attrs size ("+QSn(attrs.size())+")");

	auto subElements = MyQDom::GetTopLevelElements(workerElement);
	if(subElements.size()) Error(GetClassName()+"::InitWorker wrong subElements size ("+QSn(subElements.size())+")");

	this->constants = constants;
	protocolParent = aProtocolParent;
	protocolType = aProtocolType;
	addProperties.Init(addPropsCell);
	statements = Code::TextToStatements(instructionsCell);

	if(nameCell == "w_out_1")
		QMbError("");

	for(int i=statements.size()-1; i>=0; i--)
	{
		bool needErase = false;
		if(statements[i].header.startsWith(CodeKeyWords::emulatorStr))
		{
			if(protocolType == CodeKeyWords::emulatorInt)
				statements[i].header.remove(0, CodeKeyWords::emulatorStr.size()+1);
			else needErase = true;
		}
		else if(statements[i].header.startsWith(CodeKeyWords::servisInt))
		{
			if(protocolType == CodeKeyWords::servisInt)
				statements[i].header.remove(0, CodeKeyWords::servisStr.size()+1);
			else needErase = true;
		}

		if(nameCell == "w_out_1") qdbg << CodeKeyWords::TypeToStr(protocolType)<< statements[i].header << needErase;
		if(needErase)
		{
			statements.erase(statements.begin()+i);
			continue;
		}

		Code::RemoveEmulatorServis(statements[i].blockInstructions, protocolType);

		for(int instrIndex=0; instrIndex<statements[i].blockInstructions.size(); instrIndex++)
		{
			if(statements[i].blockInstructions[instrIndex].left(CodeKeyWords::idFilter.length()) == CodeKeyWords::idFilter)
			{
				if(!idFilterStr.isEmpty()) { Error("multiple id commands forbidden"); continue; }

				idFilterStr = statements[i].blockInstructions[instrIndex];
				statements[i].blockInstructions.removeAt(instrIndex);
				continue;
			}
			if(statements[i].blockInstructions[instrIndex].left(CodeKeyWords::dataFilter.length()) == CodeKeyWords::dataFilter)
			{
				dataFilterStrs += statements[i].blockInstructions[instrIndex];
				statements[i].blockInstructions.removeAt(instrIndex);
				continue;
			}
		}
	}

	if(nameCell == "w_out_1")
	{
		MyQDialogs::ShowText(CodeKeyWords::TypeToStr(protocolType) + "\n" + Statement::PrintStatements(statements));
	}
}
