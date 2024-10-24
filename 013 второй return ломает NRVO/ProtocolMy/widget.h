#ifndef WIDGET_H
#define WIDGET_H

#include <vector>

#include <QWidget>
#include <QTableWidget>

#include "value.h"

namespace WidgetsDefines
{
	const QString widget{"Виджет"};
};

namespace WidgPartTypes
{
	const QString label {"Label"};
	const QString edit {"Edit"};
	const QString button {"Button"};
	const QString checkBox {"Галочка"};
	const QString cell {"Ячейка"};

	const QString unnonwWidg {"unnonwWidgPart"};

	const QStringList all { label,edit,button,checkBox,cell };
	const QStringList needConnect { button };
};

class WidgPart: public IOperand
{
	QWidget *widgPt {nullptr};
	QTableWidgetItem *cellPt {nullptr};
	QString name {""};
	QString command {""};
	QString type {""};

public:
	WidgPart() = default;
	WidgPart(QWidget *viewWidjPt_,		QString name_, QString command_, QString type_) {Set(viewWidjPt_, name_, command_, type_);}
	WidgPart(QTableWidgetItem *cellPt_, QString name_, QString command_, QString type_) {Set(cellPt_,	  name_, command_, type_);}
	void Set(QWidget *viewWidjPt_,		QString name_, QString command_, QString type_) {widgPt=viewWidjPt_; name=name_; command=command_; type = type_; }
	void Set(QTableWidgetItem *cellPt_, QString name_, QString command_, QString type_) {cellPt=cellPt_;	 name=name_; command=command_; type = type_; }

	void Assign(IOperand *operand2) override
	{
		Log("mock WidgPart Assign this = " + operand2->IOGetValue(GetWidgPartValue().IOGetValueAndTypeAndJoinPair()));
	}
	virtual bool IsEqual(IOperand *operand2) override
	{
		Log("mock IsEqual " + operand2->IOGetValueAndTypeAndJoinPair());
		return false;
	}
	QString IOClassName() const override { return IOperand::classWidjetPt(); }
	std::pair<QString, QString> IOGetValueAndType()  const override
	{
		return GetWidgPartValue().IOGetValueAndType();
	}
	QString IOGetValue(QString outputValueType) const override
	{
		Value ret(GetWidgPartValue().ConvertToType(outputValueType));
		return ret.IOGetValueAndType().first;
	}
	QString IOGetType() const override { return GetWidgPartValue().IOGetType(); }

	inline const QString& GetName() const { return name; }
	inline const QString& GetType() const { return type; }

	Value GetWidgPartValue() const;
	void SetWidgetValue(QString value);

	void Create(QString name_, QString command_, QString type_, QString caption, QString startValue);

	QTableWidgetItem* GetTWItem() { return cellPt; }
	QWidget* GetPtQWidget() { return widgPt; }

	static bool isWidgPartType(const QString &str) { return WidgPartTypes::all.contains(str); }

	void Log(const QString &logStr) const   { qdbg << "Log:    WidgPart::" + logStr; }
	void Error(const QString &errStr) const { qdbg << "ERRROR: WidgPart::" + errStr; }
};

namespace WidgetTypes
{
	const QString chekBoxesNButton {"ГалочкиИКнопка"};
	const QString editButton {"EditButton"};
	const QString buttons {"Buttons"};
	const QString cell {"Cell"};

	const QString unnonwWidg {"unnonwWidget"};

	const QStringList all {chekBoxesNButton, editButton, buttons};

	const QString whole {"whole"};
	const QString part {"part"};
};

class Widget: public IOperand
{
	QString type {""};
	QString name {""};

public:
	std::vector<WidgPart> parts;
	QStringList obrabotchiki;

	Widget() = default;

	void Assign(IOperand *operand2) override
	{
		Log("mock Widget Assign this = " + operand2->IOGetValue(GetWidgetValue().IOGetValueAndTypeAndJoinPair()));
	}
	virtual bool IsEqual(IOperand *operand2) override
	{
		Log("mock IsEqual " + operand2->IOGetValueAndTypeAndJoinPair());
		return false;
	}
	QString IOClassName() const override { return IOperand::classWidjet(); }
	std::pair<QString, QString> IOGetValueAndType()  const override
	{
		return GetWidgetValue().IOGetValueAndType();
	}
	QString IOGetValue(QString outputValueType) const override
	{
		Value ret(GetWidgetValue().ConvertToType(outputValueType));
		return ret.IOGetValueAndType().first;
	}
	QString IOGetType() const override { return GetWidgetValue().IOGetType(); }

	void SetName(const QString &name_) { if(name != "") qdbg << "Widget::SetName set name not empty name" << name << name_; name = name_; }
	void SetType(const QString &type_) { if(type != "") qdbg << "Widget::SetType set type not empty type" << type << type_; type = type_; }
	inline const QString& GetName() { return name; }
	inline const QString& GetType() { return type; }

	void SetWidgetValue(QString value) { for(auto &p:parts) p.SetWidgetValue(value); }
	Value GetWidgetValue() const;

	void AddWidgetPart(const WidgPart &newPart);
	WidgPart* FindWidgPartByName(const QString &name);
	void DoObrabotchiki(const QString &partName);

	static bool isWidgType(const QString &str) { return WidgetTypes::all.contains(str); }

	void Log(const QString &logStr) const   { qdbg << "Log:    Widget::" + logStr; }
	void Error(const QString &errStr) const { qdbg << "ERRROR: Widget::" + errStr; }
};

#endif // WIDGET_H
