#ifndef WIDGET_H
#define WIDGET_H

#include <vector>

#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>

#include "value.h"
#include "param.h"
#include "object.h"
#include "protocol.h"

struct Transforms
{
	QString Undefined() const {return "undefined";}

	QString name {Undefined()};
	QStringList befors;
	QStringList afters;

	Transforms() = default;

	int FromQStringList(const QStringList &transformWords);
	int Add(Transforms transformsToAdd);
	QString ToStr() const;
	static bool Check(const QStringList &transformWords);

	static QString Transform(const QString &name, const Transforms &transforms, const QString &currText, int napravlenie, int &error);
	enum Napravlenie { in, out };
	enum Errors { noError, widgetNameDifferNameInTransform, notCheckedTransform, addDifferentName };
};

class WidgetColors
{
public:
	static QString clRed() {return "clRed";}
	static QString clGreen() {return "clGreen";}
	static QString clBlue() {return "clBlue";}

	static QStringList all() { return {clRed(), clGreen(), clBlue()}; }

	static QColor GetColor(QString colorStr);
};

namespace WidgetPartTypes
{
	const QString label {"QLabel"};
	const QString edit {"QLineEdit"};
	const QString button {"QPushButton"};
	const QString checkBox {"QCheckBox"};
	const QString groupBox {"QGroupBox"};

	const QStringList all { label, edit, button, checkBox, groupBox };

	const QString unnonwWidg {"unnonwWidgPart"};

	const QString commandNextRow {"NextRow"};
};

class WidgetPart: public LogedClass
{
	QWidget *widgPt {nullptr};
	QString name {""};
	QString type {""};

public:

	Transforms transforms;

	WidgetPart() = default;
	WidgetPart(QWidget *viewWidjPt_, QString name_, QString type_) {Set(viewWidjPt_, name_, type_);}
	void Set(QWidget *viewWidjPt_, QString name_, QString type_) {widgPt=viewWidjPt_; name=name_; type = type_; }

	inline const QString& GetName() const { return name; }
	inline const QString& GetType() const { return type; }

	void SetWidgetPartText(QString text);
	QString GetWidgPartCurrentText() const;

	void Create(QString name_, QString type_, QString caption, QString startValue);

	QWidget* GetPtQWidget();

	static bool isWidgPartType(const QString &str) { return WidgetPartTypes::all.contains(str); }
};

namespace WidgetTypes
{
	const QString chekBoxesNButton {"ГалочкиИКнопка"};
	const QString editButton {"EditButton"};
	const QString buttons {"Buttons"};

	const QString twoFieldBtn {"TwoFieldButton"};

	const QStringList all {chekBoxesNButton, editButton, buttons, twoFieldBtn};

	const QString undefined {"undefined"};

	const QString customCombinated {"customComb"};

	const QString whole {"whole"};
	const QString part {"part"};
};

namespace TwoFieldBtnNamesParts
{
	const QString groupBox {"groupBox"};
	const QString label1 {"label1"};
	const QString label2 {"label2"};

	const QString labelText1 {"labelText1"};
	const QString labelText2 {"labelText2"};
	const QString labelPressed {"labelPressed"};
	const QString label1On {"label1On"};
	const QString label2On {"label2On"};
	const QString label1Blink {"label1Blink"};
	const QString label2Blink {"label2Blink"};

	const QString labelCurrentBlink {"labelCurrentBlinkState"};
};

class Widget: public LogedClass
{
	QString type {WidgetTypes::undefined};
	QString name {WidgetTypes::undefined};

public:
	std::vector<WidgetPart> parts;
	QStringList obrabotchiki;
	std::vector<Transforms> transforms;

	Widget() = default;

	void SetName(const QString &name_);
	void SetType(const QString &type_);
	inline const QString& GetName() { return name; }
	inline const QString& GetType() { return type; }

	void CreateWidget(const QString &strWidgDescribtion);

	void CreateTwoFieldButton(const QStringList &words);

	void PlaceWidget(QGridLayout *layout, int row, int startCol, int &finishedCol);

	void Blink();

	void SetWidgetValue(const Value &value);
	Value GetWidgetValue() const;

	std::pair<int,int> GetWidgetWH();

	void AddWidgetPart(const WidgetPart &newPart);
	WidgetPart* FindWidgPartByName(const QString &name_);
	QWidget* GetQWidgPtByPartName(const QString &name_);

	void AddTransformsToParts();

	static bool isWidgType(const QString &str) { return WidgetTypes::all.contains(str); }
};

class WidgetPair: public LogedClass, public HaveClassName
{
public:
	Param *parentParam = nullptr;
	FrameWorker *parentFrameWorker = nullptr;

	Widget view;
	Widget ctrl;

	QString strIdToConnect;
	QString strParamDescribtion;
	QString strClassWindow;
	QString strCaption;
	QString strViewWidg;
	QString strCtrlWidg;
	QString strTabName;
	QString strTableName;

	int colInTable;
	int rowInTable;
	QTableWidget* tablePtr;

	QString ToStr() const { return strIdToConnect+"|"+strParamDescribtion+"|"+strClassWindow+"|"+strCaption+"|"+strViewWidg+"|"
				+strCtrlWidg+"|"+strTabName+"|"+strTableName; }

	void PlaceWidgetPairInGrLayout(QGridLayout *grLayout, bool createCaption);
	void PlaceWidgetPairInTable(QTableWidget *table);

	virtual QString GetClassName() const override { return ClassWidgetPair(); }
	virtual QString ToStrForLog() const override { return ToStr(); }
	static QString ClassWidgetPair() { return "WidgetPair"; }

};

struct Window: public LogedClass
{
	Object *parentObject = nullptr;
	Protocol *parentProtocol = nullptr;

	QString name;
	QString classWindow;
	QString connectObjectName;
	QString w = "150";
	QString h = "150";

	QWidget* windowPtQWidget;

	QTabWidget* tabWidget;
	std::vector<QWidget*> tabs;
	std::vector<QGridLayout*> tabsLOs;
	QStringList tabNames;

	std::vector<QTableWidget*> tables;

	std::vector<WidgetPair> widgets;

	void CreateTabs(QLayout *loPlace);
	QGridLayout* FindTabLayOut(const QString &tabName);

	QTableWidget* CreateTable(QString tableName, QGridLayout *grLayout);
	QTableWidget* FindTable(const QString &tableName, bool printLog);
};

#endif // WIDGET_H
