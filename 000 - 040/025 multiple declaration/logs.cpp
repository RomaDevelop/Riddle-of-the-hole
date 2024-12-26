#include "logs.h"

qint64 Logs::warningsCount = 0;
qint64 Logs::errorsCount = 0;
NsLogs::MsgWorker Logs::cbfLogSt = nullptr;
NsLogs::MsgWorker Logs::cbfWarningSt = nullptr;
NsLogs::MsgWorker Logs::cbfErrorSt = nullptr;
QTextBrowser * Logs::tBrowserSt = nullptr;
QString Logs::msgPrefixSt_ = "";

void Logs::AppendInTextBrowser(QTextBrowser *tBrowser, const QString &str, const QBrush &brush)
{
	tBrowser->append(str);
	auto curs = tBrowser->textCursor();
	curs.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	curs.setPosition(curs.position() - (str.length()), QTextCursor::KeepAnchor);

	QTextCharFormat format;
	format.setForeground(brush);
	curs.setCharFormat(format);
}

void Logs::Clear()
{
	cbfLogSt = nullptr;
	cbfWarningSt = nullptr;
	cbfErrorSt = nullptr;

	tBrowserSt = nullptr;

	msgPrefixSt_ = "";

	warningsCount = 0;
	errorsCount = 0;
}

void Logs::LogSt(QString str)
{
	str = msgPrefixSt_ + str;
	qDebug() << "LogSt    " << str;
	if(cbfLogSt) cbfLogSt(str);
	else if(tBrowserSt)
	{
		str+="\n";
		AppendInTextBrowser(tBrowserSt, str, NsLogs::clLog);
	}
}

void Logs::WarningSt(QString str)
{
	str = msgPrefixSt_ + str;
	qDebug() << "WarningSt" << str;
	if(cbfWarningSt) cbfLogSt(str);
	else if(tBrowserSt)
	{
		str+="\n";
		AppendInTextBrowser(tBrowserSt, str, NsLogs::clWarning);
	}
	Logs::warningsCount++;
}

void Logs::ErrorSt(QString str)
{
	str = msgPrefixSt_ + str;
	qDebug() << "ErrorSt  " << str;
	if(cbfErrorSt) cbfLogSt(str);
	else if(tBrowserSt)
	{
		str+="\n";
		AppendInTextBrowser(tBrowserSt, str, NsLogs::clError);
	}
	Logs::errorsCount++;
}

void LogedClass::Clear()
{
	cbfLog = nullptr;
	cbfWarning = nullptr;
	cbfError = nullptr;

	tBrowser = nullptr;

	msgPrefix_ = "";
}

void LogedClass::Log(QString str) const
{
	str = msgPrefix_ + str;
	if(cbfLog || tBrowser) qDebug() << "Log    " << str;
	if(cbfLog) cbfLog(str);
	else if(tBrowser)
	{
		str+="\n";
		Logs::AppendInTextBrowser(tBrowser, str, NsLogs::clLog);
	}
	else Logs::LogSt(str);
}

void LogedClass::Warning(QString str) const
{
	str = msgPrefix_ + str;
	if(cbfWarning || tBrowser) qDebug() << "Warning" << str;
	if(cbfWarning) cbfLog(str);
	else if(tBrowser)
	{
		str+="\n";
		Logs::AppendInTextBrowser(tBrowser, str, NsLogs::clWarning);
	}
	else Logs::WarningSt(str);
	Logs::WarningsPP();
}

void LogedClass::Error(QString str) const
{
	str = msgPrefix_ + str;
	if(cbfError || tBrowser) qDebug() << "Error  " << str;
	if(cbfError) cbfLog(str);
	else if(tBrowser)
	{
		str+="\n";
		Logs::AppendInTextBrowser(tBrowser, str, NsLogs::clError);
	}
	else Logs::ErrorSt(str);
	Logs::ErrorsPP();
}
