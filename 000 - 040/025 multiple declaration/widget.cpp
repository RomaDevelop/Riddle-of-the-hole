#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>

#include "MyQShortings.h"

#include "code.h"
#include "configconstants.h"
#include "widget.h"

using namespace std;

void WidgetPart::SetWidgetPartText(QString text)
{
	int errTr;
	text = Transforms::Transform(name, transforms, text, Transforms::in, errTr);
	if(errTr != Transforms::noError)
		Error("Transform: widget name ["+name+"] differ name in transform ["+transforms.ToStr()+"]");

	if(widgPt)
	{
		if(type == widgPt->metaObject()->className())
		{
			if(type == WidgetPartTypes::label)
			{
				((QLabel*)widgPt)->setText(text);
			}
			else Error("SetWidgetPartText: unrealesed WidgPart type: " + type);
		}
		else Error("SetWidgetPartText: widgPt type is " + type + " but metaObject()->className() is " + widgPt->metaObject()->className());
	}
	else Error("SetWidgetPartText: widgPt is nullptr");
}

QString WidgetPart::GetWidgPartCurrentText() const
{
	QString ret;
	if(widgPt)
	{
		if(type == widgPt->metaObject()->className())
		{
			if(type == WidgetPartTypes::checkBox)
				ret = QSn(((QCheckBox*)widgPt)->isChecked());
			else if(type == WidgetPartTypes::edit)
				ret = ((QLineEdit*)widgPt)->text();
			else if(type == WidgetPartTypes::label)
				ret = ((QLabel*)widgPt)->text();
			else if(type == WidgetPartTypes::button)
				ret = ((QPushButton*)widgPt)->text();
			else Error("GetWidgPartCurrentText: unrealesed WidgPart type: " + type);
		}
		else Error("GetWidgPartCurrentText: widgPt type is " + type + " but metaObject()->className() is " + widgPt->metaObject()->className());
	}
	else Error("GetWidgPartCurrentText: widgPt is nullptr");
	return ret;
}

void WidgetPart::Create(QString name_, QString type_, QString caption, QString startValue)
{
	if(widgPt != nullptr) Error("Create: create in existing WidgPart (widgPt != nullptr) " + name +" "+ type);
	else
	{
		name = name_;
		type = type_;

		if(type_ == WidgetPartTypes::label)
		{
			widgPt = new QLabel(caption);
			widgPt->setMinimumWidth(25);
			((QLabel*)widgPt)->setAlignment(Qt::AlignCenter);
		}
		else if(type_ == WidgetPartTypes::edit)
		{
			widgPt = new QLineEdit(startValue);
			widgPt->resize(50,30);
		}
		else if(type_ == WidgetPartTypes::button)
		{
			widgPt = new QPushButton(caption);
			widgPt->resize(50,30);
		}
		else if(type_ == WidgetPartTypes::checkBox)
		{
			widgPt = new QCheckBox(caption);
		}
		else Error("Create: create an unrealesed WidgPart " + type);

		if(type_ == WidgetPartTypes::edit)
			if(caption != "") Error("Create: Warning " + type + " try set caption " + caption + ", but this type has no caption");

		if(type_ == WidgetPartTypes::button || type_ == WidgetPartTypes::checkBox || type_ == WidgetPartTypes::label)
			if(startValue != "") Error("Create: Warning " + type + " try set startValue " + startValue + ", but this type has no startValue");
	}
}

QWidget *WidgetPart::GetPtQWidget()
{
	return widgPt;
}

void Widget::AddWidgetPart(const WidgetPart &newPart)
{
	parts.push_back(newPart);
}

WidgetPart* Widget::FindWidgPartByName(const QString &name_)
{
	int size = parts.size();
	for(int i=0; i<size; i++) if(parts[i].GetName() == name_) return &parts[i];
	return nullptr;
}

QWidget *Widget::GetQWidgPtByPartName(const QString &name_)
{
	auto wPart = FindWidgPartByName(name_);
	if(wPart)
	{
		return wPart->GetPtQWidget();
	}
	else Error("GetQWidgPtByPartName can't find "+ name_);
	return nullptr;
}

void Widget::AddTransformsToParts()
{
	for(auto &trans:transforms)
		for(auto &part:parts)
		{
			if(part.GetName() == trans.name)
			{
				if(part.transforms.Add(trans) != Transforms::noError)
					Error("Add different name. part.transforms.name = " + part.transforms.name + " adding " + trans.name);
			}
		}
}

void Widget::SetName(const QString &name_)
{
	if(name != WidgetTypes::undefined)
		Error("SetName set name not undefined name " + name + " changed to " + name_);
	name = name_;
}

void Widget::SetType(const QString &type_)
{
	if(type != WidgetTypes::undefined)
	{
		if(type == WidgetTypes::customCombinated && type_ == WidgetTypes::customCombinated) {/* not error */}
		else Error("SetName set type not undefined type " + type + " changed to " + type_);
	}
	type = type_;
}

void Widget::CreateWidget(const QString &strWidgDescribtion)
{
	QStringList commands { Code::TextToCommands(strWidgDescribtion) };
	for(auto cmnd:commands)
	{
		auto words { Code::CommandToWords(cmnd) };
		int size = words.size();
		if(!size) Error("CreateWidget Wrong widget command. words.empty() " + cmnd);
		else if(words[0] == CommandsKeyWords::obrabotchik)
		{
			obrabotchiki.push_back(cmnd);
		}
		else if(words[0] == CommandsKeyWords::transform)
		{
			Transforms tr;
			tr.FromQStringList(words);
			if(tr.name != "" && tr.befors.size() && tr.afters.size())
				transforms.push_back(std::move(tr));
		}
		else if(Widget::isWidgType(words[0]))
		{
			if(words[0] == WidgetTypes::chekBoxesNButton)
			{
				if(size >= 4)
				{
					QString widgetName = words[1];
					SetType(WidgetTypes::chekBoxesNButton);
					SetName(widgetName);

					int btnIndex = words.size()-1;
					for(int i=2; i<words.size(); i++)
					{
						WidgetPart wpt;
						QString caption { TextConstant::GetTextConstVal(words[i]) };
						if(i != btnIndex) wpt.Create("", WidgetPartTypes::checkBox, caption, "");
						if(i == btnIndex) wpt.Create(widgetName, WidgetPartTypes::button, caption, "");
						AddWidgetPart(wpt);
					}
				}
				else Error("CreateWidget Wrong words size (" +QSn(size)+ ") in command " + cmnd);
			}
			else if(words[0] == WidgetTypes::buttons)
			{
				if(size >= 3)
				{
					QString widgetName = words[1];
					SetType(WidgetTypes::buttons);
					SetName(widgetName);

					for(int i=2; i<words.size(); i++)
					{
						WidgetPart wpt;
						QString caption { TextConstant::GetTextConstVal(words[i]) };
						wpt.Create(widgetName, WidgetPartTypes::button, caption, "");
						AddWidgetPart(wpt);
					}
				}
				else Error("CreateWidget Wrong word count (" +QSn(size)+ ") in command " + cmnd);
			}
			else if(words[0] == WidgetTypes::editButton)
			{
				if(size == 3)
				{
					QString widgetName = words[1];
					SetType(WidgetTypes::editButton);
					SetName(widgetName);

					QString caption { TextConstant::GetTextConstVal(words[2]) };

					WidgetPart wptEdit;
					wptEdit.Create("", WidgetPartTypes::edit, "", "");
					AddWidgetPart(wptEdit);

					WidgetPart wptBtn;
					wptBtn.Create(widgetName, WidgetPartTypes::button, caption, "");
					AddWidgetPart(wptBtn);
				}
				else Error("CreateWidget Wrong word count (" +QSn(size)+ ") in command " + cmnd);
			}
			else if(words[0] == WidgetTypes::twoFieldBtn)
			{
				CreateTwoFieldButton(words);
			}
			else Error("CreateWidget unrealesed Widget " + words[0]);
		}
		else if(WidgetPart::isWidgPartType(words[0]))
		{
			QString tip = words[0];

			QString name = "";
			if(size >= 2) name = words[1];

			QString caption = "";
			if(size >= 3) caption = TextConstant::GetTextConstVal(words[2]);

			QString startVal = "";
			if(size == 4) startVal = TextConstant::GetTextConstVal(words[3]);

			WidgetPart wpt;
			wpt.Create(name, tip, caption, startVal);
			AddWidgetPart(wpt);
			SetType(WidgetTypes::customCombinated);
		}
		else if(words[0] == WidgetPartTypes::commandNextRow)
		{
			AddWidgetPart(WidgetPart(nullptr, "", words[0]));
		}
		else Error("CreateWidget unnown command [" + cmnd + "] in strWidgDescribtion [" + strWidgDescribtion + "]");

		AddTransformsToParts();
	}
}

void Widget::CreateTwoFieldButton(const QStringList &words)
{
	int size = words.size();
	if(size == 7)
	{
		QString nameW = words[1];
		QString captionW = TextConstant::GetTextConstVal(words[2]);
		QString text1 = TextConstant::GetTextConstVal(words[3]);
		QString text2 = TextConstant::GetTextConstVal(words[4]);
		QString cl1 = words[5];
		QString cl2 = words[6];

		SetType(WidgetTypes::twoFieldBtn);
		SetName(nameW);

		QGroupBox *groupBox = new QGroupBox(captionW);
		groupBox->setMaximumWidth(100);
		groupBox->setMaximumHeight(110);
		QPushButton *pushButton = new QPushButton();
		pushButton->setMinimumHeight(80);
		pushButton->setMinimumWidth(80);
		QLineEdit *le1 = new QLineEdit();
		QLineEdit *le2 = new QLineEdit();
		QFont font = le1->font();
		font.setPointSize(12);
		font.setBold(true);
		le1->setFont(font);
		le2->setFont(font);
		le1->setMinimumHeight(20);
		le2->setMinimumHeight(20);
		le1->setAlignment(Qt::AlignCenter);
		le2->setAlignment(Qt::AlignCenter);
		le1->setEnabled(false);
		le2->setEnabled(false);

		QLabel *leText1 = new QLabel(text1);
		QLabel *leText2 = new QLabel(text2);
		QLabel *pressed = new QLabel("0");
		QLabel *on1 = new QLabel("0");
		QLabel *on2 = new QLabel("0");
		QLabel *blink1 = new QLabel("0");
		QLabel *blink2 = new QLabel("0");
		QLabel *leMorg = new QLabel();

		leText1->setVisible(false);
		leText2->setVisible(false);
		pressed->setVisible(false);
		on1->setVisible(false);
		on2->setVisible(false);
		blink1->setVisible(false);
		blink2->setVisible(false);
		leMorg->setVisible(false);

		QPalette palette;
		palette.setColor(QPalette::Base, Qt::black);
		palette.setColor(QPalette::Text, WidgetColors::GetColor(cl1));
		le1->setPalette(palette);
		palette.setColor(QPalette::Text, WidgetColors::GetColor(cl2));
		le2->setPalette(palette);

		QVBoxLayout *buttonLayout=new QVBoxLayout();
		buttonLayout->setMargin(5);
		buttonLayout->setSpacing(5);
		buttonLayout->addWidget(le1);
		buttonLayout->addWidget(le2);
		pushButton->setLayout(buttonLayout);

		QVBoxLayout *gbLO=new QVBoxLayout();
		gbLO->addWidget(pushButton);
		groupBox->setLayout(gbLO);

		WidgetPart wpt1(groupBox, TwoFieldBtnNamesParts::groupBox, WidgetPartTypes::groupBox);
		WidgetPart wpt2(pushButton, "pushButton", WidgetPartTypes::button);
		WidgetPart wpt3(le1, TwoFieldBtnNamesParts::label1, WidgetPartTypes::edit);
		WidgetPart wpt4(le2, TwoFieldBtnNamesParts::label2, WidgetPartTypes::edit);
		WidgetPart wpt5(leText1, TwoFieldBtnNamesParts::labelText1, WidgetPartTypes::label);
		WidgetPart wpt6(leText2, TwoFieldBtnNamesParts::labelText2, WidgetPartTypes::label);
		WidgetPart wpt7(pressed, TwoFieldBtnNamesParts::labelPressed, WidgetPartTypes::label);
		WidgetPart wpt8(on1, TwoFieldBtnNamesParts::label1On, WidgetPartTypes::label);
		WidgetPart wpt9(on2, TwoFieldBtnNamesParts::label2On, WidgetPartTypes::label);
		WidgetPart wpt10(blink1, TwoFieldBtnNamesParts::label1Blink, WidgetPartTypes::label);
		WidgetPart wpt11(blink2, TwoFieldBtnNamesParts::label2Blink, WidgetPartTypes::label);
		WidgetPart wpt12(leMorg, TwoFieldBtnNamesParts::labelCurrentBlink, WidgetPartTypes::label);

		AddWidgetPart(wpt1);
		AddWidgetPart(wpt2);
		AddWidgetPart(wpt3);
		AddWidgetPart(wpt4);
		AddWidgetPart(wpt5);
		AddWidgetPart(wpt6);
		AddWidgetPart(wpt7);
		AddWidgetPart(wpt8);
		AddWidgetPart(wpt9);
		AddWidgetPart(wpt10);
		AddWidgetPart(wpt11);
		AddWidgetPart(wpt12);
	}
	else Error("CreateViewWidg " + WidgetTypes::twoFieldBtn + " wrong word count");
}

void Widget::PlaceWidget(QGridLayout *layout, int row, int startCol, int &finishedCol)
{
	if(type == WidgetTypes::twoFieldBtn)
	{
		auto mainWpt = FindWidgPartByName(TwoFieldBtnNamesParts::groupBox);
		layout->addWidget(mainWpt->GetPtQWidget(),row,startCol);
		finishedCol = startCol;
	}
	else
	{
		int col = startCol;
		for(auto &wpt:parts)
		{
			if(wpt.GetType() == WidgetPartTypes::commandNextRow)
			{
				row++;
				col = startCol;
			}
			else
			{
				layout->addWidget(wpt.GetPtQWidget(),row,col);
				col++;
			}
		}
		finishedCol = col;
	}
}

void Widget::Blink()
{
	if(type == WidgetTypes::twoFieldBtn)
	{
		auto qwtext1 = GetQWidgPtByPartName(TwoFieldBtnNamesParts::labelText1);
		auto qwtext2 = GetQWidgPtByPartName(TwoFieldBtnNamesParts::labelText2);
		auto qwlabel1 = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label1);
		auto qwlabel2 = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label2);

		auto qwidgCurBlink = GetQWidgPtByPartName(TwoFieldBtnNamesParts::labelCurrentBlink);
		auto qwidg1on = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label1On);
		auto qwidg2on = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label2On);
		auto qwidg1bl = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label1Blink);
		auto qwidg2bl = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label2Blink);


		if(qwidgCurBlink && qwtext1 && qwtext2 && qwlabel1 && qwlabel2 && qwidg1on && qwidg2on && qwidg1bl && qwidg2bl)
		{
			if((QString)qwidgCurBlink->metaObject()->className() == "QLabel"
					&& (QString)qwtext1->metaObject()->className() == "QLabel"
					&& (QString)qwtext2->metaObject()->className() == "QLabel"
					&& (QString)qwidg1on->metaObject()->className() == "QLabel"
					&& (QString)qwidg2on->metaObject()->className() == "QLabel"
					&& (QString)qwidg1bl->metaObject()->className() == "QLabel"
					&& (QString)qwidg2bl->metaObject()->className() == "QLabel"
					&& (QString)qwlabel1->metaObject()->className() == "QLineEdit"
					&& (QString)qwlabel2->metaObject()->className() == "QLineEdit")
			{
				QString curStrBl = ((QLabel*)qwidgCurBlink)->text();
				QString text1 = ((QLabel*)qwtext1)->text();
				QString text2 = ((QLabel*)qwtext2)->text();
				QString on1 = ((QLabel*)qwidg1on)->text();
				QString on2 = ((QLabel*)qwidg2on)->text();
				QString bl1 = ((QLabel*)qwidg1bl)->text();
				QString bl2 = ((QLabel*)qwidg2bl)->text();

				if(on1 == "0") text1 = "";
				else if(on1 == "1")
				{
					if(bl1 == "1" && curStrBl != "blink")
						text1 = "";
				}
				else Error("wrong on1 text " + on1);

				if(on2 == "0") text2 = "";
				else if(on2 == "1")
				{
					if(bl2 == "1" && curStrBl != "blink")
						text2 = "";
				}
				else Error("wrong on2 text "+ on2);

				if(curStrBl == "")
				{
					curStrBl = "blink";
				}
				else if(curStrBl == "blink")
				{
					curStrBl = "";
				}
				else Error("Blink wrong qwidgCurBlink text " + curStrBl);

				((QLineEdit*)qwlabel1)->setText(text1);
				((QLineEdit*)qwlabel2)->setText(text2);
				((QLabel*)qwidgCurBlink)->setText(curStrBl);
			}
			else Error("Blink " + WidgetTypes::twoFieldBtn + " wrong typedef");
		}
		else Error("Blink " + WidgetTypes::twoFieldBtn + " can't find twoFieldBtn part");
	}
	else Error("Blink unrealesed blink widget type " + type);
}

void Widget::SetWidgetValue(const Value &value)
{
	if(type == WidgetTypes::twoFieldBtn)
	{
		auto getValuePair = value.IOGetValueAndType();
		auto vals = Value::GetStringListFromValueVector(getValuePair.first);
		if(getValuePair.second == ValuesTypes::vectorBool && vals.size() >=5)
		{
			auto qwidgprs = GetQWidgPtByPartName(TwoFieldBtnNamesParts::labelPressed);
			auto qwidg1on = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label1On);
			auto qwidg2on = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label2On);
			auto qwidg1bl = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label1Blink);
			auto qwidg2bl = GetQWidgPtByPartName(TwoFieldBtnNamesParts::label2Blink);
			((QLabel*)qwidgprs)->setText(vals[0]);
			((QLabel*)qwidg1on)->setText(vals[1]);
			((QLabel*)qwidg2on)->setText(vals[2]);
			((QLabel*)qwidg1bl)->setText(vals[3]);
			((QLabel*)qwidg2bl)->setText(vals[4]);
		}
		else Error("SetWidgetValue for widget " + type + " wrong value " + value.ToStrForLog());
	}
	else
	{
		for(auto &p:parts) p.SetWidgetPartText(value.ToStr());
	}
}

Value Widget::GetWidgetValue() const
{
	Value retVal;
	if(type == WidgetTypes::chekBoxesNButton)
	{
		QString values;
		for(auto &p:parts)
			if(p.GetType() == WidgetPartTypes::checkBox) values += p.GetWidgPartCurrentText() + ValuesTypes::vectorSplitter;
		retVal.Set(values, ValuesTypes::vectorBool);
	}
	else if(type == WidgetTypes::editButton)
	{
		if(parts.size())
			retVal.Set(parts[0].GetWidgPartCurrentText(),ValuesTypes::text);
	}
	else if(parts.size() == 1 && parts[0].GetType() == WidgetPartTypes::checkBox)
	{
		retVal.Set(parts[0].GetWidgPartCurrentText(),ValuesTypes::ubool);
	}
	else Error("GetWidgetValue unrealesed WidgType " + type);
	return retVal;
}

std::pair<int, int> Widget::GetWidgetWH()
{
	int w=0;
	int h=0;
	if(type == WidgetTypes::chekBoxesNButton || type == WidgetTypes::editButton || type == WidgetTypes::buttons || type == WidgetPartTypes::label
			|| type == WidgetTypes::customCombinated)
	{
		for(auto &wpt:parts)
		{
			w += wpt.GetPtQWidget()->width();
			if(h < wpt.GetPtQWidget()->height()) h = wpt.GetPtQWidget()->height();
		}
	}
	else if(type == WidgetTypes::twoFieldBtn)
	{
		auto grBox = GetQWidgPtByPartName(TwoFieldBtnNamesParts::groupBox);
		w = grBox->width();
		h = grBox->height();
	}
	else Error("GetWidgetWH unrealesed type " + type);
	return {w,h};
}

QColor WidgetColors::GetColor(QString colorStr)
{
	QColor ret = Qt::black;
	if(all().contains(colorStr))
	{

		if(colorStr == clRed()) ret = Qt::red;
		else if(colorStr == clGreen()) ret = Qt::green;
		else if(colorStr == clBlue()) ret = Qt::blue;
		else Logs::ErrorSt("GetColor wrong behavior");
	}
	else Logs::ErrorSt("GetColor unknown color "+colorStr);
	return ret;
}

int Transforms::FromQStringList(const QStringList &transformWords)
{
	int ret = noError;
	name.clear();
	befors.clear();
	afters.clear();
	if(Check(transformWords))
	{
		name = transformWords[1];
		for(int i=2; i<transformWords.size(); i+=2)
		{
			befors += TextConstant::GetTextConstVal(transformWords[i]);
			afters += TextConstant::GetTextConstVal(transformWords[i+1]);
		}
	}
	else ret = notCheckedTransform;
	return ret;
}

int Transforms::Add(Transforms transformsToAdd)
{
	int ret = noError;
	if(name == Undefined()) name = transformsToAdd.name;

	if(name == transformsToAdd.name)
	{
		befors += transformsToAdd.befors;
		afters += transformsToAdd.afters;
	}
	else ret = addDifferentName;
	return ret;
}

QString Transforms::ToStr() const
{
	QString ret;
	ret = name + ": befors: ";
	for(int i=0; i<befors.size(); i++)
		ret += "[" + befors[i] + "]";
	ret += " afters: ";
	for(int i=0; i<afters.size(); i++)
		ret += "[" + afters[i] + "]";
	if(ret == "") ret = "empty Transforms";
	ret = "<" + ret + ">";
	return ret;
}

bool Transforms::Check(const QStringList &transformWords)
{
	if(transformWords.size()>=4 && transformWords.size()%2 == 0) return true;

	return false;
}

QString Transforms::Transform(const QString &name, const Transforms &transforms, const QString &currText, int napravlenie, int &error)
{
	error = Errors::noError;
	QString ret = currText;
	if(transforms.befors.size())
	{
		if(transforms.name == name)
		{
			for(int i=0; i<transforms.befors.size(); i++)
			{
				if(napravlenie == in)
				{
					if(currText == transforms.befors[i])
						ret = transforms.afters[i];
				}
				else if(napravlenie == out)
				{
					if(currText == transforms.afters[i])
						ret = transforms.befors[i];
				}
			}
		}
		else error = Errors::widgetNameDifferNameInTransform;
	}
	return ret;
}

void Window::CreateTabs(QLayout *loPlace)
{
	if(!this->tabWidget)
	{
		this->tabWidget = new QTabWidget;

		for(uint widgI=0; widgI<this->widgets.size(); widgI++)
			if(this->widgets[widgI].strTabName != "" && !this->tabNames.contains(this->widgets[widgI].strTabName))
				this->tabNames += this->widgets[widgI].strTabName;

		for(auto &tname:this->tabNames)
		{
			QWidget* tab = new QWidget;
			tab->setObjectName(tname);
			tab->setAutoFillBackground(true);
			this->tabWidget->addTab(tab,tname);
			this->tabs.push_back(tab);

			QGridLayout *layOutAll  = new QGridLayout(tab);
			this->tabsLOs.push_back(layOutAll);
		}
		loPlace->addWidget(this->tabWidget);
	}
	else Error("CreateTabWidget: tabWidget is not nullptr");
}

QGridLayout *Window::FindTabLayOut(const QString &tabName)
{
	QGridLayout* retLO = nullptr;
	for(uint t=0; t<this->tabs.size(); t++)
	{
		if(this->tabs[t]->objectName() == tabName)
			retLO = this->tabsLOs[t];
	}
	if(!retLO) Error("FindTabLayOut: Can't find tab " + tabName);
	return retLO;
}

QTableWidget *Window::CreateTable(QString tableName, QGridLayout *grLayout)
{
	QTableWidget *table = FindTable(tableName, false);
	if(!table)
	{
		table = new QTableWidget;
		table->setColumnCount(1);
		table->setObjectName(tableName);
		this->tables.push_back(table);

		int row = grLayout->rowCount();
		grLayout->addWidget(table,row,0,1,-1);
	}
	else Error("CreateTable: found existing table with name " + tableName);
	return table;
}

QTableWidget *Window::FindTable(const QString &tableName, bool printLog)
{
	QTableWidget* retLO = nullptr;
	for(uint t=0; t<this->tables.size(); t++)
	{
		if(this->tables[t]->objectName() == tableName)
			retLO = this->tables[t];
	}
	if(!retLO && printLog) Log("FindTable: Can't find table " + tableName);
	return retLO;
}

QStringList WidgetPair::CellValuesToStringListForEditor() const
{
	QStringList ret;
	for(int i=0; i<Widget_ns::Fields::count; i++) ret.append("");

	ret[Widget_ns::Fields::idParamI		] = strIdToConnect	;
	ret[Widget_ns::Fields::idWidgetI	] = strId			;
	ret[Widget_ns::Fields::tabNameI		] = strTabName		;
	ret[Widget_ns::Fields::tableNameI	] = strTableName	;
	ret[Widget_ns::Fields::captionI		] = strCaption		;
	ret[Widget_ns::Fields::viewWidgI	] = strViewWidg		;
	ret[Widget_ns::Fields::ctrlWidgI	] = strCtrlWidg		;

	return ret;
}

void WidgetPair::SetCellValuesFromEditor(const QStringList & values)
{
	if(values.size() == Widget_ns::Fields::count)
	{
		strIdToConnect	= values[Widget_ns::Fields::idParamI	];
		strId			= values[Widget_ns::Fields::idWidgetI	];
		strTabName		= values[Widget_ns::Fields::tabNameI	];
		strTableName	= values[Widget_ns::Fields::tableNameI	];
		strCaption		= values[Widget_ns::Fields::captionI	];
		strViewWidg		= values[Widget_ns::Fields::viewWidgI	];
		strCtrlWidg		= values[Widget_ns::Fields::ctrlWidgI	];
	}
	else Logs::ErrorSt(GetClassName() + "::SetCellValuesFromEditor values.size() != Param_ns::Fields::count ("+QSn(values.size())+" != "+QSn(Param_ns::Fields::count)+")");
}

void WidgetPair::SetCellValue(QString fieldName, QString newValue)
{
	if(		fieldName ==	Widget_ns::Fields::idParam	)	strIdToConnect	= newValue;
	else if(fieldName ==	Widget_ns::Fields::idWidget	)	strId			= newValue;
	else if(fieldName ==	Widget_ns::Fields::tabName	)	strTabName		= newValue;
	else if(fieldName ==	Widget_ns::Fields::tableName)	strTableName	= newValue;
	else if(fieldName ==	Widget_ns::Fields::caption	)	strCaption		= newValue;
	else if(fieldName ==	Widget_ns::Fields::viewWidg	)	strViewWidg		= newValue;
	else if(fieldName ==	Widget_ns::Fields::ctrlWidg	)	strCtrlWidg		= newValue;
	else Logs::ErrorSt(GetClassName() + "::SetCellValue wrong attribute ["+fieldName+"]");
}

void WidgetPair::SetCellValuesFromDomElement(const QDomElement & paramElement)
{
	auto attrs = DomAdd::GetAttributes(paramElement);
	if((int)attrs.size() == Widget_ns::Fields::count)
	{
		for(auto &attr:attrs)
		{
			if(		attr.first ==	Widget_ns::Fields::idParam	)	strIdToConnect	= attr.second;
			else if(attr.first ==	Widget_ns::Fields::idWidget	)	strId			= attr.second;
			else if(attr.first ==	Widget_ns::Fields::tabName	)	strTabName		= attr.second;
			else if(attr.first ==	Widget_ns::Fields::tableName)	strTableName	= attr.second;
			else if(attr.first ==	Widget_ns::Fields::caption	)	strCaption		= attr.second;
			else if(attr.first ==	Widget_ns::Fields::viewWidg	)	strViewWidg		= attr.second;
			else if(attr.first ==	Widget_ns::Fields::ctrlWidg	)	strCtrlWidg		= attr.second;
			else Logs::ErrorSt(GetClassName() + "::SetCellValuesFromDomElement wrong param attribute ["+attr.first+"]");
		}
	}
	else Logs::ErrorSt(GetClassName() + "::SetCellValuesFromDomElement wrong attrs size ("+QSn(attrs.size())+")");
}

QStringPairVector WidgetPair::GetAttributes() const
{
	QStringPairVector atts;
	atts.push_back({Widget_ns::Fields::idParam		, strIdToConnect	});
	atts.push_back({Widget_ns::Fields::idWidget		, strId				});
	atts.push_back({Widget_ns::Fields::tabName		, strTabName		});
	atts.push_back({Widget_ns::Fields::tableName	, strTableName		});
	atts.push_back({Widget_ns::Fields::caption		, strCaption		});
	atts.push_back({Widget_ns::Fields::viewWidg		, strViewWidg		});
	atts.push_back({Widget_ns::Fields::ctrlWidg		, strCtrlWidg		});
	return atts;
}

void WidgetPair::PlaceWidgetPairInGrLayout(QGridLayout *grLayout, bool createCaption)
{
	int row = grLayout->rowCount();

	if(createCaption)
	{
		QLabel *lblName = new QLabel(strCaption);
		grLayout->addWidget(lblName,row,0);
	}

	int finishedCol = 0;

	if(view.parts.size()) view.PlaceWidget(grLayout,row,1,finishedCol);
	if(ctrl.parts.size()) ctrl.PlaceWidget(grLayout,row,finishedCol+1,finishedCol);
}

void WidgetPair::PlaceWidgetPairInTable(QTableWidget *table)
{
	if(!table) { Error("PlaceTableWidgetItem: table nullptr " + ToStr()); return; }
	if(strTableName == "") { Error("PlaceTableWidgetItem: strTableName empty " + ToStr()); return; }

	rowInTable = -1;
	bool findField = false;
	for(int r=0; r<table->rowCount(); r++)
	{
		if(strCaption == table->item(r,0)->text()) { rowInTable = r; findField = true; break; }
	}

	if(!findField)
	{
		table->setRowCount(table->rowCount()+1);
		rowInTable = table->rowCount()-1;
		table->setRowHeight(rowInTable,5);
		table->setItem(rowInTable,0,new QTableWidgetItem(strCaption));
	}

	if(rowInTable != -1)
	{
		if(rowInTable == 0)
			table->setColumnCount(table->columnCount()+1);

		colInTable = table->columnCount()-1;

		if(view.parts.size() == 1 && ctrl.parts.size() == 0)
			table->setCellWidget(rowInTable,colInTable,view.parts[0].GetPtQWidget());
		else
		{
			QWidget *w = new QWidget;
			QGridLayout *grLayout = new QGridLayout(w);
			grLayout->setContentsMargins(1,1,1,1);
			grLayout->setSpacing(1);
			PlaceWidgetPairInGrLayout(grLayout,false);

			table->setCellWidget(rowInTable,colInTable,w);
		}
	}
	else Error("Can't define row");
}
