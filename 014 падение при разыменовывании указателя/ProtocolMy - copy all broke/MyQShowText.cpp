#include <QDialog>
#include <QHBoxLayout>
#include <QTextBrowser>

#include "MyQShowText.h"

void ShowText::Show(QString text, uint w, uint h)
{
	QDialog *dialog = new QDialog;
	QHBoxLayout *all  = new QHBoxLayout(dialog);
	QTextBrowser *tb = new QTextBrowser;
	tb->setPlainText(text);
	all->addWidget(tb);

	if(h && w) dialog->resize(w, h);
	dialog->exec();
	delete tb;
	delete all;
	delete dialog;
}
