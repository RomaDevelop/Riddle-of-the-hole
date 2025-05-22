void MyQTableWidget::CreateContextMenu()
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	menu = new QMenu(this);
	QAction *cutAction = menu->addAction("Cut");
	QAction *copyAction = menu->addAction("Copy");
	QAction *pasteAction = menu->addAction("Paste");
	QAction *pasteActionFromOsClip = menu->addAction("Paste from OS clipboard");

	connect(cutAction, &QAction::triggered, this, &MyQTableWidget::Cut);
	connect(copyAction, &QAction::triggered, this, &MyQTableWidget::Copy);
	connect(pasteAction, &QAction::triggered, this, &MyQTableWidget::Paste);
	connect(pasteActionFromOsClip, &QAction::triggered, this, &MyQTableWidget::PasteFromOsClip);

	cutAction->setShortcut(QKeySequence::Cut);
	copyAction->setShortcut(QKeySequence::Copy);
	pasteAction->setShortcut(QKeySequence::Paste);
	
	// вот без этого при CustomContextMenu не работают, ии предлагал keyPressEvent переопределять
	//addAction(cutAction);
	//addAction(copyAction);
	//addAction(pasteAction);

	connect(this, &QWidget::customContextMenuRequested, [this](const QPoint &pos){ menu->exec(mapToGlobal(pos)); });
}