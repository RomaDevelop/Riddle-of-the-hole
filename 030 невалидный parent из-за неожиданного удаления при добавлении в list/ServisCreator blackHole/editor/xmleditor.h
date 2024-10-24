#ifndef XMLEDITOR_H
#define XMLEDITOR_H

#include <QWidget>
#include <QSplitter>
#include <QCloseEvent>

#include "logs.h"
#include "config.h"

class XMLEditor: public QWidget, public LogedClass
{
	Q_OBJECT

	QFont basicFont;
	QTextCharFormat tagFormat;
	QTextCharFormat attributeFormat;
	QTextCharFormat valueFormat;

	QSplitter *splitterCenter = nullptr;
	QSplitter *splitterRight = nullptr;

	QTextEdit *textEditFile = nullptr;
	QTextEdit *textEditAttribute = nullptr;
	int posStart = 0;
	int posEnd = 0;
	bool editingAtTextEditAttributeNow = false;
	bool positionChangeAtTextEditFileNow = false;

	ConfigFile& configFile;

public:
	typedef void (*OnSaveFinished)(ConfigFile&);
private:
	OnSaveFinished onSaveFinished = nullptr;

public:
	XMLEditor(ConfigFile& configFile, QString senderText, OnSaveFinished onSaveFinished, QWidget *parent = nullptr);
	~XMLEditor() { }
	void closeEvent (QCloseEvent *event);
	void SaveSettings();
	void LoadSettings();

	bool canDelete = false;

	private slots: void SlotSave();

};

#endif // XMLEDITOR_H
