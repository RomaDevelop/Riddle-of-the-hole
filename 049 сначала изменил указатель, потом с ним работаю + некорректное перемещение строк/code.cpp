void TableWidget::MoveItemAndRow(bool up)
{
	auto item = ConfigItemOfCurrentRow();
	qdbg << item->ToStrForLog() << item->PreviousSibling();
	qdbg << item->PreviousSibling()->ToStrForLog();
	qdbg << item->PreviousSibling()->SubItemCount(true);
	if(!item) return;

	if(up && !item->PreviousSibling()) return;
	if(!up && !item->NextSibling()) return;

	if(!item->MoveMe(up ? true : false)) // происходит перемещение item-а в дереве и его PreviousSibling NextSibling могут стать nullptr
	{
		Logs::ErrorSt("MoveItemAndRow MoveMe false result");
		return;
	}

	int row = tableWidgetPtr->currentRow();
	int distance = (up ? item->PreviousSibling()->SubItemCount(true) + 1 : item->NextSibling()->SubItemCount(true) + 1); // извлекаю PreviousSibling или NextSibling и сразу с ними что-то делаю
	int place = (up ? row - distance : row + distance);
	int countRowsToMove = item->SubItemCount(true) + 1;
	for(int i = 0; i < countRowsToMove; i++)
	{
		MoveRow(row, place);
		place++;
		row++;
	}
	GoToItem(*item);
}

// а еще перемещение строк не работало корректно, для корректной потребовалось совсем по другому:

void TableWidget::MoveItemAndRow(bool up)
{
	auto item = ConfigItemOfCurrentRow();
	if(!item) return;

	if(up && !item->PreviousSibling()) return;
	if(!up && !item->NextSibling()) return;

	int thisRow = tableWidgetPtr->currentRow();
	int thisRowsCount = item->SubItemCount(true) + 1;
	int siblRowsCount = (up ? item->PreviousSibling()->SubItemCount(true) + 1 : item->NextSibling()->SubItemCount(true) + 1);
	int siblRow = (up ? thisRow - siblRowsCount : thisRow + thisRowsCount);

	if(!item->MoveMe(up ? true : false))
	{
		Logs::ErrorSt("MoveItemAndRow MoveMe false result");
		return;
	}


	RowsBuffer bufThis = TakeRows(thisRow, thisRowsCount);
	RowsBuffer bufSibl = TakeRows(siblRow, siblRowsCount);

	PasteRows(bufThis, (up ? siblRow : thisRow + siblRowsCount));
	PasteRows(bufSibl, (up ? siblRow + thisRowsCount : thisRow));

	GoToItem(*item);
}