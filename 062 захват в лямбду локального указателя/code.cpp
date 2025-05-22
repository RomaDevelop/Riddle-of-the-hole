из

Protocol *protocol = widgetPair->parentProtocol;
Worker *worker = widgetPair->parentWorker;
int protType = protocol->type;
for(auto &wpt:widgetPair->ctrl.parts)
{
	// обработчики
	if(!wpt->statements.front().blockInstructions.empty())
	{
		auto doObrabotchiki = [protocol, &widgetPair, &wpt]()
		{
			CANMsg_t msgPustishka;
			protocol->DoStatements(wpt->statements,
								   widgetPair->parentParam, widgetPair->parentWorker, widgetPair->parentVariable,
								   msgPustishka);
		};

		if(auto btn = dynamic_cast<QPushButton *>(wpt->GetPtQWidget()))
		{
			QObject::connect(btn, &QPushButton::clicked, doObrabotchiki);
		}
		if(auto chBox = dynamic_cast<QCheckBox *>(wpt->GetPtQWidget()))
		{
			QObject::connect(chBox, &QCheckBox::clicked, doObrabotchiki);
		}
		if(auto castedPtr = dynamic_cast<QComboBox *>(wpt->GetPtQWidget()))
		{
			QObject::connect(castedPtr, &QComboBox::currentTextChanged, doObrabotchiki);
		}
	}
}
			
сделал

widgetPair->MakeQConnectAll();

void WidgetPair::MakeQConnectAll()
{
	for(auto &wpt:ctrl.parts)
	{
		MakeQConnectObrabotchiki(wpt.get());
	}
}

void WidgetPair::MakeQConnectObrabotchiki(WidgetPart *wpt)
{
	if(!wpt->statements.front().blockInstructions.empty())
	{
		auto doObrabotchiki = [this, &wpt]()
		{
			CANMsg_t msgPustishka;
			parentProtocol->DoStatements(wpt->statements,
								   parentParam, parentWorker, parentVariable,
								   msgPustishka);
		};

		if(auto btn = dynamic_cast<QPushButton *>(wpt->GetPtQWidget()))
		{
			QObject::connect(btn, &QPushButton::clicked, doObrabotchiki);
		}
		if(auto chBox = dynamic_cast<QCheckBox *>(wpt->GetPtQWidget()))
		{
			QObject::connect(chBox, &QCheckBox::clicked, doObrabotchiki);
		}
		if(auto castedPtr = dynamic_cast<QComboBox *>(wpt->GetPtQWidget()))
		{
			QObject::connect(castedPtr, &QComboBox::currentTextChanged, doObrabotchiki);
		}
	}
}

забыл убрать &wpt в
auto doObrabotchiki = [this, &wpt]

ведь там это была ссылка на умный указатель
а тут это стала ссылка на обычный указатель которая стала мертвой по завершению работы MakeQConnectObrabotchiki