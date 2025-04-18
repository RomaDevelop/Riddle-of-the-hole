а вот никак их и не извлечь, хоть усрись

	static QTableWidget tmpParent;
	static QTableWidget *parent = &tmpParent;
	parent->show();
	parent->setRowCount(0);
	parent->setColumnCount(3);
	for(int row=0; row<table->rowCount(); row++)
	{
		table->takeItem(row, ColIndexes::name);

		auto w1 = table->cellWidget(row, ColIndexes::chBox);
		auto w2 = table->cellWidget(row, ColIndexes::notifyDTedit);
		auto w3 = table->cellWidget(row, ColIndexes::postponeDTedit);

		table->setCellWidget(row, ColIndexes::chBox, nullptr);
		table->setCellWidget(row, ColIndexes::notifyDTedit, nullptr);
		table->setCellWidget(row, ColIndexes::postponeDTedit, nullptr);

		parent->setRowCount(parent->rowCount()+1);
		parent->setCellWidget(parent->rowCount()-1, 0, w1);
		parent->setCellWidget(parent->rowCount()-1, 1, w2);
		parent->setCellWidget(parent->rowCount()-1, 2, w3);

		//table->removeCellWidget(row, ColIndexes::chBox);
		//table->removeCellWidget(row, ColIndexes::notifyDTedit);
		//table->removeCellWidget(row, ColIndexes::postponeDTedit);
	}
	qdbg << "take did";

	table->setRowCount(0);