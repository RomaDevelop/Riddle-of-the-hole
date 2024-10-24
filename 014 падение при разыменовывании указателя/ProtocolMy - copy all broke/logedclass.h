#ifndef LOGEDCLASS_H
#define LOGEDCLASS_H

#include <QDebug>
#include <QTextBrowser>

namespace Logs
{
	const QBrush clLog = Qt::darkGreen;
	const QBrush clWarning = Qt::darkYellow;;
	const QBrush clError = Qt::darkRed;

	typedef void (*MsgWorker)(QString log);
};

class LogsSt
{
protected:
	static Logs::MsgWorker cbfLogSt;
	static Logs::MsgWorker cbfWarningSt;
	static Logs::MsgWorker cbfErrorSt;

	static QTextBrowser *tBrowserSt;

	static QString msgPrefixSt_;

public:

	static void AppendInTextBrowser(QTextBrowser *tBrowser, const QString &str, const QBrush &brush)
	{
		tBrowser->append(str);
		auto curs = tBrowser->textCursor();
		curs.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
		curs.setPosition(curs.position() - (str.length()), QTextCursor::KeepAnchor);

		QTextCharFormat format;
		format.setForeground(brush);
		curs.setCharFormat(format);
	}

	void Clear()
	{
		cbfLogSt = nullptr;
		cbfWarningSt = nullptr;
		cbfErrorSt = nullptr;

		tBrowserSt = nullptr;

		msgPrefixSt_ = "";
	}

	static void SetStLogWorker(Logs::MsgWorker logWorker, const QString &msgPrefix) { cbfLogSt = logWorker; msgPrefixSt_ = msgPrefix;}
	static void SetStWarningWorker(Logs::MsgWorker warningWorker, const QString &msgPrefix) { cbfWarningSt = warningWorker; msgPrefixSt_ = msgPrefix; }
	static void SetStErrorWorker(Logs::MsgWorker errorWorker, const QString &msgPrefix) { cbfErrorSt = errorWorker; msgPrefixSt_ = msgPrefix; }
	static void SetAllToTextBrowser(QTextBrowser *tBrowser, const QString &msgPrefix) { tBrowserSt = tBrowser; msgPrefixSt_ = msgPrefix; }

	static void LogSt(QString str)
	{
		str = msgPrefixSt_ + str;
		qDebug() << "LogSt    " << str;
		if(cbfLogSt) cbfLogSt(str);
		else if(tBrowserSt)
		{
			str+="\n";
			AppendInTextBrowser(tBrowserSt, str, Logs::clLog);
		}
	}
	static void WarningSt(QString str)
	{
		str = msgPrefixSt_ + str;
		qDebug() << "WarningSt" << str;
		if(cbfWarningSt) cbfLogSt(str);
		else if(tBrowserSt)
		{
			str+="\n";
			AppendInTextBrowser(tBrowserSt, str, Logs::clWarning);
		}
	}
	static void ErrorSt(QString str)
	{
		str = msgPrefixSt_ + str;
		qDebug() << "ErrorSt  " << str;
		if(cbfErrorSt) cbfLogSt(str);
		else if(tBrowserSt)
		{
			str+="\n";
			AppendInTextBrowser(tBrowserSt, str, Logs::clError);
		}
	}
};

class LogedClass
{
protected:
	Logs::MsgWorker cbfLog = nullptr;
	Logs::MsgWorker cbfWarning = nullptr;
	Logs::MsgWorker cbfError = nullptr;

	QTextBrowser *tBrowser = nullptr;

	QString msgPrefix_;

public:
	virtual ~LogedClass() = default;

	void Clear()
	{
		cbfLog = nullptr;
		cbfWarning = nullptr;
		cbfError = nullptr;

		tBrowser = nullptr;

		msgPrefix_ = "";
	}

	virtual void SetLogWorker(Logs::MsgWorker logWorker, const QString &msgPrefix) { cbfLog = logWorker; msgPrefix_ = msgPrefix; }
	virtual void SetWarningWorker(Logs::MsgWorker warningWorker, const QString &msgPrefix) { cbfWarning = warningWorker; msgPrefix_ = msgPrefix; }
	virtual void SetErrorWorker(Logs::MsgWorker errorWorker, const QString &msgPrefix) { cbfError = errorWorker; msgPrefix_ = msgPrefix; }
	virtual void SetAllToTextBrowser(QTextBrowser *tBrowser_, const QString &msgPrefix) { tBrowser = tBrowser_; msgPrefix_ = msgPrefix; }

	virtual void SetFromLogedObject(LogedClass *object)
	{
		cbfLog = object->cbfLog;
		cbfWarning = object->cbfWarning;
		cbfError = object->cbfError;

		tBrowser = object->tBrowser;

		msgPrefix_ = object->msgPrefix_;
	}

	void Log(QString str) const
	{
		str = msgPrefix_ + str;
		if(cbfLog || tBrowser) qDebug() << "Log    " << str;
		if(cbfLog) cbfLog(str);
		else if(tBrowser)
		{
			str+="\n";
			LogsSt::AppendInTextBrowser(tBrowser, str, Logs::clLog);
		}
		else LogsSt::LogSt(str);
	}
	void Warning(QString str) const
	{
		str = msgPrefix_ + str;
		if(cbfWarning || tBrowser) qDebug() << "Warning" << str;
		if(cbfWarning) cbfLog(str);
		else if(tBrowser)
		{
			str+="\n";
			LogsSt::AppendInTextBrowser(tBrowser, str, Logs::clWarning);
		}
		else LogsSt::WarningSt(str);
	}
	void Error(QString str) const
	{
		str = msgPrefix_ + str;
		if(cbfError || tBrowser) qDebug() << "Error  " << str;
		if(cbfError) cbfLog(str);
		else if(tBrowser)
		{
			str+="\n";
			LogsSt::AppendInTextBrowser(tBrowser, str, Logs::clError);
		}
		else LogsSt::ErrorSt(str);
	}
};

#endif // LOGEDCLASS_H
