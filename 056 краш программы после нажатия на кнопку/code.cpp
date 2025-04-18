обработчик нажатия

void WidgetCatalog::SlotClickOnLabelCurrentThemTagAct(int, int kodTagOrAct, DataBase::tables currentTable)
{
	WriteAvarLog("SlotClickOnLabelCurrentThemTagAct start");
	MultiselectItem *msItem = MultiselectItemPair::FindItem(multiselect, currentTable, kodTagOrAct);
	if(!msItem) { QMbError("SlotClickOnLabelCurrentThemTagAct FindItem returned nullptr"); return; }
	MultiselectItemLeftMBClick(*msItem);
	WriteAvarLog("SlotClickOnLabelCurrentThemTagAct end");
}
все норм, все логи до конца пропечатывались

проблема была в том, что при зажатии вызывалась функция
и дело именно в // удаление если уже были созданы delete labels...[i]; delete btns...[i];
лабел только был нажат и еще не отработал, а его удаляли. Видимо из-за этого

добавил это:
template<class W>
void HideAndDel(std::vector<W*> widgets)
{
	for(auto &w:widgets)
	{
		w->hide();
		w->deleteLater();
	}
}

void WidgetCatalog::ViewThemeActsTagsOnFilm(QString kod_film)
{
	// удаление если уже были созданы
	static std::vector<ClickableQLabel*> labelsTags;
	static std::vector<ClickableQLabel*> labelsActs;
	static std::vector<QPushButton*> btnsTags;
	static std::vector<QPushButton*> btnsActs;
	static QLayoutItem *stretchTags = nullptr;
	static QLayoutItem *stretchActs = nullptr;

	//for(uint i=0; i<labelsTags.size(); i++) { delete labelsTags[i]; delete btnsTags[i]; }
	//for(uint i=0; i<labelsActs.size(); i++) { delete labelsActs[i]; delete btnsActs[i]; }
	HideAndDel(labelsTags);
	HideAndDel(labelsActs);
	HideAndDel(btnsTags);
	HideAndDel(btnsActs);

	labelsTags.clear();
	labelsActs.clear();
	btnsTags.clear();
	btnsActs.clear();
	if(stretchTags) { hloTagsOfCurrentFilm->removeItem(stretchTags); delete stretchTags; stretchTags = nullptr; }
	if(stretchActs) { hloActsOfCurrentFilm->removeItem(stretchActs); delete stretchActs; stretchActs = nullptr; }

	if(kod_film.isEmpty()) return;

	auto query = DataBase::DoSqlQuery("select * from Темы where [Код темы]=(select [Код темы] from Фильмы where [Код фильма]="+kod_film+")");
	query.next();
	labelThemOfCurrentFilm->setText("Тема: " + query.value(FieldsThems::nameFieldName).toString());

	// запрос тэгов фильма
	query = DataBase::DoSqlQuery("SELECT Тэги.Наименование, Тэги.[Код тэга] FROM Тэги "
								 "INNER JOIN [Тэги фильмов] ON Тэги.[Код тэга] = [Тэги фильмов].[Код тэга] "
								 "WHERE [Тэги фильмов].[Код фильма]="+kod_film+" order by Тэги.Наименование");
	int kodFilmInt = kod_film.toInt();

	while(query.next())
	{
		labelsTags.push_back(new ClickableQLabel(query.value(0).toString()));
		int kodTag = query.value(1).toInt();
		DataBase::tables currentTable = DataBase::tags;

		btnsTags.push_back(new QPushButton("X"));
		btnsTags.back()->setFixedSize(18,18);
		hloTagsOfCurrentFilm->addWidget(labelsTags.back());
		hloTagsOfCurrentFilm->addWidget(btnsTags.back());
		connect(labelsTags.back(), &ClickableQLabel::clicked, [this, kodFilmInt, kodTag, currentTable](){
			SlotClickOnLabelCurrentThemTagAct(kodFilmInt, kodTag, currentTable); });
		connect(btnsTags.back(), &QPushButton::clicked, [this, kodFilmInt, kodTag, currentTable](){
			SlotClickOnBtnXCurrentThemTagAct(kodFilmInt, kodTag, currentTable); });
	}
	hloTagsOfCurrentFilm->addStretch();
	stretchTags = hloTagsOfCurrentFilm->itemAt(hloTagsOfCurrentFilm->count()-1);

	// вывод актеров фильма
	query = DataBase::DoSqlQuery("SELECT Актеры.Имя, Актеры.[Код актера] FROM Актеры "
								 "INNER JOIN [Актеры фильмов] ON Актеры.[Код актера] = [Актеры фильмов].[Код актера] "
								 "WHERE [Актеры фильмов].[Код фильма]="+kod_film+" order by Актеры.Имя");
	while(query.next())
	{
		labelsActs.push_back(new ClickableQLabel(query.value(0).toString()));
		int kodAct = query.value(1).toInt();
		DataBase::tables currentTable = DataBase::acts;

		btnsActs.push_back(new QPushButton("X"));
		btnsActs.back()->setFixedSize(18,18);
		hloActsOfCurrentFilm->addWidget(labelsActs.back());
		hloActsOfCurrentFilm->addWidget(btnsActs.back());
		connect(labelsActs.back(), &ClickableQLabel::clicked, [this, kodFilmInt, kodAct, currentTable](){
			SlotClickOnLabelCurrentThemTagAct(kodFilmInt, kodAct, currentTable); });
		connect(btnsActs.back(), &QPushButton::clicked, [this, kodFilmInt, kodAct, currentTable](){
			SlotClickOnBtnXCurrentThemTagAct(kodFilmInt, kodAct, currentTable); });
	}
	hloActsOfCurrentFilm->addStretch();
	stretchActs = hloActsOfCurrentFilm->itemAt(hloActsOfCurrentFilm->count()-1);
}
