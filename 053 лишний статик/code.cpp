// а все потому что скопировал из прототипа и захватил лишнее
static std::vector<Statement2> TextToStatements(const QString &text);

std::vector<Statement2> Code::TextToStatements(const QString &text)
{
	static std::vector<Statement2> statements; // тут он!!! из-за него при каждом вызове добавлялось от предыдущего
	Statement2* currentStatement = nullptr;
	bool quats = false;
	bool block = false;
	bool commented = false;
	QChar currQuats = CodeKeyWords::quatsSymbol1;
	QString current;
	int sizeText = text.size();
	for(int i = 0; i<sizeText; i++)
	{
		// логика игнорирования комментария
		if(commented)  // если сейчас комментарий
		{
			if(text[i] == '\n') // если встретили перенос на сл.строку - прекращаем игнорировать
				commented = false;
			else
				continue;  // продолжаем игнорировать
		}
		// определяем, что встретили маркер закомментированного
		else if(!quats && text[i] == CodeKeyWords::commentMarker[0] && i+1 < sizeText && text[i+1] == CodeKeyWords::commentMarker[1])
		{
			commented = true;
			continue;
		}
		// конец логики игнорирования комментария

		// начало блока
		if(!quats && text[i] == CodeKeyWords::blockOpener)
		{
			if(block) { Logs::ErrorSt("nested blocks are forbidden, text: " + text); current.clear(); break; }

			block = true;
			Normalize(current);
			currentStatement = &statements.emplace_back();
			currentStatement->header = std::move(current);
			current.clear();
			continue;
		}

		// завершение одиночной команды без блока
		if(!quats && !block && text[i] == CodeKeyWords::commandSplitter)
		{
			Normalize(current);
			statements.emplace_back();
			statements.back().blockInstructions += std::move(current);
			current.clear();
			continue;
		}

		// завершение блока
		if(!quats && block && text[i] == CodeKeyWords::blockCloser)
		{
			currentStatement->blockInstructions = Code::TextToCommands(current);
			currentStatement = nullptr;
			current.clear();

			block = false;
			continue;
		}

		current += text[i];

		if(!quats && TextConstant::IsItQuatsSybol(text[i])) { quats = true; currQuats = text[i]; continue; }
		if(text[i] == currQuats && quats) { quats = false; continue; }
	}

	if(current.size())
	{
		if(block) Logs::ErrorSt("missing ending symbol; text: " + text);
		else
		{
			Normalize(current);
			statements.emplace_back();
			statements.back().blockInstructions += std::move(current);
		}
	}

	return statements;
}
