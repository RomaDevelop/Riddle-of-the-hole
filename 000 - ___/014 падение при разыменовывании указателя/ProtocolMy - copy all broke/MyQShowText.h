#ifndef MYQSHOWTEXT_H
#define MYQSHOWTEXT_H

#include <QString>

class ShowText
{
public:
	static void Show(QString text, uint w = 0, uint h = 0);
};

#define ShT ShowText::Show

#endif // MYQSHOWTEXT_H
