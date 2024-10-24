#include "xmleditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTimer>
#include <QSettings>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

#include "MyQShellExecute.h"
#include "MyQShortings.h"
#include "MyQDifferent.h"
#include "SyntaxHighlighters.h"

#include "allConstants.h"

XMLEditor::XMLEditor(ConfigFile& configFile, QString senderText, OnSaveFinished onSaveFinished, QWidget *parent) :
	QWidget(parent),
	configFile {configFile},
	onSaveFinished{onSaveFinished}
{
	basicFont = QFont("Courier new",12);
	tagFormat.setForeground(Qt::blue);
	attributeFormat.setForeground(Qt::red);
	valueFormat.setForeground(Qt::darkMagenta);

	this->setWindowTitle(configFile.name + " ["+configFile.fileNameWithPath+"]");

	auto layOutMain = new QVBoxLayout(this);
	auto layOutTop = new QHBoxLayout();
	splitterCenter = new QSplitter(Qt::Horizontal);
	layOutMain->addLayout(layOutTop);
	layOutMain->addWidget(splitterCenter);

	// top
	QPushButton *btnSave = new QPushButton("Save",this);
	connect(btnSave,&QPushButton::clicked,this,&XMLEditor::SlotSave);
	layOutTop->addWidget(btnSave);
	layOutTop->addStretch();

	// center left
	auto tree = new QTreeWidget(this);
	tree->setHeaderHidden(true);
	tree->setFont(basicFont);
	connect(tree, &QTreeWidget::itemActivated, [](){qdbg << "mock 2";});
	splitterCenter->addWidget(tree);

	// center right
	QWidget *widgetCenterRight = new QWidget(this);
	splitterCenter->addWidget(widgetCenterRight);
	auto layCenterRight = new QVBoxLayout(widgetCenterRight);
	layCenterRight->setMargin(0);
	splitterRight = new QSplitter(Qt::Vertical,this);
	layCenterRight->addWidget(splitterRight);
	textEditFile = new QTextEdit(this);
	textEditFile->setFont(basicFont);
	textEditFile->setWordWrapMode(QTextOption::NoWrap);
	textEditAttribute = new QTextEdit(this);
	textEditAttribute->setFont(basicFont);
	splitterRight->addWidget(textEditFile);
	splitterRight->addWidget(textEditAttribute);

	connect(textEditFile,&QTextEdit::cursorPositionChanged,[this](){
		if(editingAtTextEditAttributeNow) return;
		positionChangeAtTextEditFileNow = true;
		posEnd = 0;
		textEditAttribute->clear();

		auto cursor = textEditFile->textCursor();
		cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,1);
		cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,2);
		auto format = ForSyntaxHighlighters::GetFormatUnderCursor(textEditFile);
		if(format.foreground().color() == valueFormat.foreground().color())
		{
			cursor = textEditFile->textCursor();
			int prevPos;
			while(1)
			{
				prevPos = cursor.position();
				cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor,1);
				if(prevPos == cursor.position()) break;
				if(cursor.selectedText().startsWith('"'))
				{
					cursor.setPosition(cursor.position(),QTextCursor::MoveAnchor);
					cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::MoveAnchor,1);
					posStart = cursor.position();
					while(1)
					{
						prevPos = cursor.position();
						cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,1);
						if(prevPos == cursor.position()) break;
						QString selectedText = cursor.selectedText();
						if(selectedText.endsWith('"'))
						{
							posEnd = posStart + selectedText.length()-1;
							selectedText.replace("&quot;","\"");
							selectedText.replace("&#xa;","\n");
							textEditAttribute->setPlainText(selectedText.left(selectedText.length()-1));
							break;
						}
					}
					break;
				}
			}
		}
		positionChangeAtTextEditFileNow = false;
	});

	connect(textEditAttribute,&QTextEdit::textChanged,[this](){
		if(positionChangeAtTextEditFileNow) return;
		if(posEnd == 0) return;
		editingAtTextEditAttributeNow = true;
		auto cursor = textEditFile->textCursor();
		cursor.setPosition(posStart, QTextCursor::MoveAnchor);
		cursor.setPosition(posEnd, QTextCursor::KeepAnchor);
		QString text = textEditAttribute->toPlainText();
		text.replace("\"","&quot;");
		text.replace("\n","&#xa;");
		posEnd = posStart + text.length();
		cursor.insertText(std::move(text));
		editingAtTextEditAttributeNow = false;
	});

	QString strContent = configFile.strContent;
	if(senderText == "OpenXMLEditor")
	{
		textEditFile->setPlainText(strContent);
	}
	else if(senderText == "OpenXMLEditorCols")
	{
		QString correctedContent;
		auto rows = strContent.split("\n");
		std::vector<int> wordsMaxLength;
		for(auto &row:rows)
		{
			row.replace('"','\'');
			row.replace("="," = ");
			row.replace("  =  "," = ");
			while(row.startsWith(' ')) row.remove(0,1);
			auto words = Code::CommandToWords(row);
			for(int i=0; i<words.size(); i++)
			{
				while(i >= (int)wordsMaxLength.size()) wordsMaxLength.push_back(0);

				if(words[i].size() > wordsMaxLength[i]) wordsMaxLength[i] = words[i].size();
			}
		}

		for(auto &row:rows)
		{
			row.replace('"','\'');
			row.replace('='," = ");
			row.replace("  =  "," = ");
			while(row.startsWith(' ')) row.remove(0,1);
			auto words = Code::CommandToWords(row);
			for(int i=0; i<words.size(); i++)
			{
				words[i].replace("'>","\">");
				words[i].replace("'/>","\">");
				if(words[i][0] == '\'') words[i][0] = '"';
				if(words[i][words[i].size()-1] == '\'') words[i][words[i].size()-1] = '"';

				while(words[i].size() < wordsMaxLength[i]+1)
					words[i] += ' ';
				correctedContent += words[i];
			}
			correctedContent += "\n";
		}

		QString fileName = MyQDifferent::PathToExe() + "/files/tmp_file.xml";
		QFile file(fileName);
		if(file.open(QFile::WriteOnly))
		{
			file.write(correctedContent.toUtf8());
			MyQShellExecute::ShellExecuteFile(fileName);
			QTimer::singleShot(0,[this](){ close(); });
		}
		else Error("Can't open file " + fileName);
	}
	else Error("wrong senderText");
	new XmlHighlighter(textEditFile->document(),&tagFormat,&attributeFormat,&valueFormat);

	QTimer::singleShot(0,this,[this]
	{
		move(10,10);
		resize(1870,675);
		LoadSettings();
	});
}

void XMLEditor::closeEvent(QCloseEvent * event)
{
	canDelete = true;
	SaveSettings();
	event->accept();
}

void XMLEditor::SaveSettings()
{
	QSettings settings(Editor_ns::SettingsFile(), QSettings::IniFormat);
	settings.beginGroup("XMLEditor");
	settings.setValue("geo", MyQDifferent::GetGeo(*this));
	settings.setValue("splitterCenterState", splitterCenter->saveState());
	settings.setValue("splitterRightState", splitterRight->saveState());
	settings.endGroup();
}

void XMLEditor::LoadSettings()
{
	QSettings settings(Editor_ns::SettingsFile(), QSettings::IniFormat);
	settings.beginGroup("XMLEditor");
	MyQDifferent::SetGeo(settings.value("geo").toString(), *this);
	splitterCenter->restoreState(settings.value("splitterCenterState").toByteArray());
	splitterRight->restoreState(settings.value("splitterRightState").toByteArray());
	settings.endGroup();
}

void XMLEditor::SlotSave()
{
	this->configFile.strContent = textEditFile->toPlainText();
	this->configFile.UpdateAllFromStrContent();
	if(this->onSaveFinished) this->onSaveFinished(this->configFile);
}
