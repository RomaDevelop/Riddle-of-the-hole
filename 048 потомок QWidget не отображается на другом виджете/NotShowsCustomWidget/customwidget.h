#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <QPainter>
#include <QWidget>

class CustomWidget : public QWidget
{
	Q_OBJECT
public:
	explicit CustomWidget(QWidget *parent = nullptr) : QWidget(parent)
	{
		setParent(parent); // не помогло
	}

	void paintEvent(QPaintEvent*) override
	{
		//QPainter p(this);
		//p.fillRect(rect(), Qt::red); // закрашивает область указанного размера
	}

};

#endif // CUSTOMWIDGET_H
