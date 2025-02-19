
	// это уже исправлленное проблема была в том, вызовы Remove были ниже, и из  loInScrollArea, а они лежали то в oldLoInScrollArea
	// что я только не проблвал, даже таймер поставил, думал что должны какие-то слоты отработать чтобы Remove выполнилось

void WidgetCatalog::PlaceMultiselect()
{
	if(!widgetInScrollArea)
	{
		//multiselectWidget->setFixedWidth(350);
		QVBoxLayout *loOuter = new QVBoxLayout(multiselectWidget);
		loOuter->setContentsMargins(0,0,0,0);
		QScrollArea *scrollArea = new QScrollArea(multiselectWidget);
		loOuter->addWidget(scrollArea);
		widgetInScrollArea = new QWidget(scrollArea);
		widgetInScrollArea->setContentsMargins(10,10,0,0);
		scrollArea->setWidget(widgetInScrollArea);
		scrollArea->setWidgetResizable(true); // Позволяет внутреннему виджету изменять размер
	}




	// каждый раз создаем новый loInScrollArea чтобы после добавления виджетов растянуть последнюю пустую строку
	auto oldLoInScrollArea = loInScrollArea;
	// но перед этим унужно очистить и уничтожить старый
	if(oldLoInScrollArea)
	{
		for(auto &itmemPair:multiselectItemsThems) itmemPair.itemMultiselect.Remove(oldLoInScrollArea);
		for(auto &itmemPair:multiselectItemsTags) itmemPair.itemMultiselect.Remove(oldLoInScrollArea);
		for(auto &itmemPair:multiselectItemsActs) itmemPair.itemMultiselect.Remove(oldLoInScrollArea);

		delete oldLoInScrollArea;
	}

	loInScrollArea = new QGridLayout(widgetInScrollArea);
	loInScrollArea->setContentsMargins(0,0,0,0);

	// получаем все темы тэги актеры текущего списка фильмов
	QString sqlIN=main_sql_text;
	int begin=sqlIN.indexOf("[Код темы]\n");
	sqlIN=sqlIN.remove(0,begin+10);         // вырезается кусок из ADOQuery1->SQL->Text
	sqlIN.prepend("SELECT Фильмы.[Код фильма] FROM Фильмы");

	QString allThemsInCurFilms, allTegsInCurFilms, allActsInCurFilms;
	QSqlQuery query;
	if(!query.exec("SELECT DISTINCT [Код темы] FROM [Фильмы] WHERE [Код фильма] IN ("+sqlIN+")"))
		QMbError("Ошибка выполнения запроса:" + query.lastError().text());
	while(query.next()) allThemsInCurFilms.append(query.value(0).toString()).append(',');
	allThemsInCurFilms.chop(1);
	
	if(!query.exec("SELECT DISTINCT [Код тэга] FROM [Тэги фильмов] WHERE [Код фильма] IN ("+sqlIN+")"))
		QMbError("Ошибка выполнения запроса:" + query.lastError().text());
	while(query.next()) allTegsInCurFilms.append(query.value(0).toString()).append(',');
	allTegsInCurFilms.chop(1);
	
	if(!query.exec("SELECT DISTINCT [Код актера] FROM [Актеры фильмов] WHERE [Код фильма] IN ("+sqlIN+")"))
		QMbError("Ошибка выполнения запроса:" + query.lastError().text());
	while(query.next()) allActsInCurFilms.append(query.value(0).toString()).append(',');
	allActsInCurFilms.chop(1);
	
	MultiselectItem::col = 0;
	MultiselectItem::row = 0;
	
	auto check = [allThemsInCurFilms, allTegsInCurFilms, allActsInCurFilms](const MultiselectItem &item){
		if(0) {}
		else if(item.currentTable == thems	&&	allThemsInCurFilms.isEmpty()) return false;
		else if(item.currentTable == tags	&&	allTegsInCurFilms.isEmpty()) return false;
		else if(								allActsInCurFilms.isEmpty()) return false;
		QString sqlText;
		if(0) {}
		else if(item.currentTable == thems)	sqlText = "SELECT [Фильмы].[Код фильма] FROM Фильмы WHERE Фильмы.[Код темы]="+item.kod+" AND [Фильмы].[Код темы] IN ("+allThemsInCurFilms+")";
		else if(item.currentTable == tags)	sqlText = "SELECT [Тэги фильмов].[Код тэгов фильма] FROM [Тэги фильмов] WHERE [Тэги фильмов].[Код тэга]="+item.kod+" AND [Тэги фильмов].[Код тэга] IN ("+allTegsInCurFilms+")";
		else								sqlText = "SELECT [Актеры фильмов].[Код актеров фильма] FROM [Актеры фильмов] WHERE [Актеры фильмов].[Код актера]="+item.kod+" AND [Актеры фильмов].[Код актера] IN ("+allActsInCurFilms+")";
		QSqlQuery query;
		if(!query.exec(sqlText))
		{
			QMbError("Ошибка выполнения запроса:" + query.lastError().text());
			return false;
		}
		if(query.next()) return true;
		else return false;
	};
	
	for(auto &itmemPair:multiselectItemsThems)
		if(check(itmemPair.itemMultiselect)) itmemPair.itemMultiselect.Place(loInScrollArea);
	MultiselectItem::InsertSpacer(loInScrollArea);
	for(auto &itmemPair:multiselectItemsTags)
		if(check(itmemPair.itemMultiselect)) itmemPair.itemMultiselect.Place(loInScrollArea);
	MultiselectItem::InsertSpacer(loInScrollArea);
	for(auto &itmemPair:multiselectItemsActs)
		if(check(itmemPair.itemMultiselect)) itmemPair.itemMultiselect.Place(loInScrollArea);
	
	loInScrollArea->setColumnStretch(loInScrollArea->columnCount(),1);
	loInScrollArea->setRowStretch(loInScrollArea->rowCount(),1);
	
	// NOLINTNEXTLINE (тупой кланг не понимает что loInScrollArea назначен парент и он удалится)
}