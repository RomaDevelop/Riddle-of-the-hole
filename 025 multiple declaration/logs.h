#ifndef LOGS_H
#define LOGS_H

#include <QDebug>
#include <QTextBrowser>

namespace NsLogs
{
	const QBrush clLog = Qt::darkGreen;
	const QBrush clWarning = Qt::darkYellow;;
	const QBrush clError = Qt::darkRed;

	typedef void (*MsgWorker)(QString log);
};

class Logs final
{
private:
	static NsLogs::MsgWorker cbfLogSt;
	static NsLogs::MsgWorker cbfWarningSt;
	static NsLogs::MsgWorker cbfErrorSt;

	static QTextBrowser *tBrowserSt;

	static QString msgPrefixSt_;

	static qint64 warningsCount;
	static qint64 errorsCount;

public:

	static qint64 WarningsCount() { return warningsCount; }
	static qint64 ErrorsCount() { return errorsCount; }
	static inline void WarningsPP() { warningsCount++; }
	static inline void ErrorsPP() { errorsCount++; }

	static void AppendInTextBrowser(QTextBrowser *tBrowser, const QString &str, const QBrush &brush);

	static void SetStLogWorker(NsLogs::MsgWorker logWorker, const QString &msgPrefix) { cbfLogSt = logWorker; msgPrefixSt_ = msgPrefix;}
	static void SetStWarningWorker(NsLogs::MsgWorker warningWorker, const QString &msgPrefix) { cbfWarningSt = warningWorker; msgPrefixSt_ = msgPrefix; }
	static void SetStErrorWorker(NsLogs::MsgWorker errorWorker, const QString &msgPrefix) { cbfErrorSt = errorWorker; msgPrefixSt_ = msgPrefix; }
	static void SetAllToTextBrowser(QTextBrowser *tBrowser, const QString &msgPrefix) { tBrowserSt = tBrowser; msgPrefixSt_ = msgPrefix; }

	static void Clear();

	static void LogSt(QString str);
	static void WarningSt(QString str);
	static void ErrorSt(QString str);
};

class LogedClass
{
protected:
	NsLogs::MsgWorker cbfLog = nullptr;
	NsLogs::MsgWorker cbfWarning = nullptr;
	NsLogs::MsgWorker cbfError = nullptr;

	QTextBrowser *tBrowser = nullptr;

	QString msgPrefix_;

public:
	virtual ~LogedClass() = default;

	virtual void SetLogWorker(NsLogs::MsgWorker logWorker, const QString &msgPrefix) { cbfLog = logWorker; msgPrefix_ = msgPrefix; }
	virtual void SetWarningWorker(NsLogs::MsgWorker warningWorker, const QString &msgPrefix) { cbfWarning = warningWorker; msgPrefix_ = msgPrefix; }
	virtual void SetErrorWorker(NsLogs::MsgWorker errorWorker, const QString &msgPrefix) { cbfError = errorWorker; msgPrefix_ = msgPrefix; }
	virtual void SetAllLogsToTextBrowser(QTextBrowser *tBrowser_, const QString &msgPrefix) { tBrowser = tBrowser_; msgPrefix_ = msgPrefix; }
	virtual void Clear();

	virtual void Log(QString str) const final;
	virtual void Warning(QString str) const final;
	virtual void Error(QString str) const final;
};

#endif // LOGS_H
