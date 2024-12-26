#include "widget.h"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

#include "compiler.h"

#include "MyQShortings.h"

void WidgPart::Create(QString name_, QString command_, QString type_, QString caption, QString startValue)
{
	if(widgPt != nullptr || cellPt != nullptr) qdbg << "Error. Try WidgPart::Create in existing WidgPart" << name << type;
	else
	{
		name = name_;
		command = command_;
		type = type_;

		if(type_ == WidgPartTypes::label)
			widgPt = new QLabel(startValue);

		else if(type_ == WidgPartTypes::edit)
			widgPt = new QLineEdit(startValue);

		else if(type_ == WidgPartTypes::button)
			widgPt = new QPushButton(caption);

		else if(type_ == WidgPartTypes::checkBox)
			widgPt = new QCheckBox(caption);

		else if(type_ == WidgPartTypes::cell)
			cellPt = new QTableWidgetItem(startValue);

		else qdbg << "Error. Try WidgPart::Create an unrealesed WidgPart" << type;

		if(type_ == WidgPartTypes::label || type_ == WidgPartTypes::edit || type_ == WidgPartTypes::cell)
			if(caption != "") qdbg << "Warning. WidgPart::Create" << type << "try set caption" << caption;

		if(type_ == WidgPartTypes::button || type_ == WidgPartTypes::checkBox)
			if(startValue != "") qdbg << "Warning. WidgPart::Create" << type << "try set startValue" << startValue;
	}
}

void WidgPart::SetWidgetValue(QString value)
{
	if(type == WidgPartTypes::label)
	{
		if(!widgPt) { qdbg << "SetWidgetValue try set nullptr QWidget"; return; }
		((QLabel*)widgPt)->setText(value);
	}
	else if(type == WidgPartTypes::cell)
	{
		if(!cellPt) { qdbg << "SetWidgetValue try set nullptr QTableWidgetItem"; return; }
		((QTableWidgetItem*)cellPt)->setText(value);
	}
	else qdbg << "SetWidgetValue: unnonw Widj:" << type;
}

Value WidgPart::GetWidgPartValue() const
{

	if(type == WidgPartTypes::checkBox)
	{
		if(((QCheckBox*)widgPt)->isChecked()) return Value("","1",ValuesTypes::bool_);
			else return Value("","0",ValuesTypes::bool_);
	}
	if(type == WidgPartTypes::edit)
	{
		QString text = ((QLineEdit*)widgPt)->text();
		if(text == "") text = "0";
		bool ok;
		text.toLongLong(&ok);
		if(ok) return Value("",text,ValuesTypes::llong);
		else { qdbg << "WidgPart::GetWidgValue: wrong value:" << text; }
	}
	else qdbg << "WidgPart::GetWidgValue: unrealesed type:" << type;
	return Value();
}

void Widget::AddWidgetPart(const WidgPart &newPart)
{
	parts.push_back(newPart);

//	int labelCount {0};
//	int editCount {0};
//	int editButtonCnt {0};
//	int buttonCount {0};
//	int checkBoxCount {0};
//	int cellCount {0};

//	for(auto &p:parts)
//	{
//		if(p.GetType() == WidgPartTypes::label) labelCount++;
//		if(p.GetType() == WidgPartTypes::edit) editCount++;
//		if(p.GetType() == WidgPartTypes::editButton) editButtonCnt++;
//		if(p.GetType() == WidgPartTypes::button) buttonCount++;
//		if(p.GetType() == WidgPartTypes::checkBox) checkBoxCount++;
//		if(p.GetType() == WidgPartTypes::cell) cellCount++;
//	}
}

WidgPart* Widget::FindWidgPartByName(const QString &name)
{
	int size = parts.size();
	for(int i=0; i<size; i++) if(parts[i].GetName() == name) return &parts[i];
	return nullptr;
}
void Widget::DoObrabotchiki(const QString &partName)
{
	int size = parts.size();
	for(int i=0; i<size; i++)
	{
		if(parts[i].GetName() == partName)
		{

		}
	}
}

Value Widget::GetWidgetValue() const
{
	if(type == WidgetTypes::chekBoxesNButton)
	{
		QString values;
		for(auto &p:parts)
			if(p.GetType() == WidgPartTypes::checkBox) values += p.GetWidgPartValue().ToStr() + ";";
		return Value("",values,ValuesTypes::vectorBool);
	}
	qdbg << "Widget::GetValue unrealesed WidgType" << type;
	return Value("","0;",ValuesTypes::vectorBool);
}
