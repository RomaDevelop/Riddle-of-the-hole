#ifndef LOGS_H
#define LOGS_H

#include <QDebug>
#include <QTextBrowser>
#include <QDateTime>
#include <QObject>
#include <QTimer>

class Logs final : public QObject
{
	Q_OBJECT
public:
	static QBrush clLog;
	static QBrush clWarning;
	static QBrush clError;

	using MsgWorker = void(*)(QString log);
	using cbMsgGet_t = std::function<void()>;

private:
	inline static QTextEdit *tBrowserSt;

	inline static std::vector<std::pair<cbMsgGet_t,int>> cbsMsgGet;  // int - id cbf
	inline static int idCbsMsgGetCounter = 0;

	inline static qint64 logsCount = 0;
	inline static qint64 warningsCount = 0;
	inline static qint64 errorsCount = 0;

public:
	explicit Logs(QObject *parent = nullptr);

	static qint64 LogsCount() { return logsCount; }
	static qint64 WarningsCount() { return warningsCount; }
	static qint64 ErrorsCount() { return errorsCount; }
	static qint64 AllCount() { return logsCount + warningsCount + errorsCount; }

	static void AppendInTextBrowser(QTextEdit *tBrowser, const QString &str, const QBrush &brush);

	static void AddCbMsgGet(cbMsgGet_t cbMsgGet, int &id);
	static void RemoveCbfMsgGet(int id);

	static void SetTextBrowser(QTextEdit *tBrowser) { tBrowserSt = tBrowser; }

	static void Clear();

private:
	static void LogCommon(QString str, qint64 &counter, int code, std::list<QString*> &currentList,
						  const char *debugPrefix, const QBrush& brush);
public:
	static void LogSt(const QString & str);
	static void WarningSt(const QString &str);
	static void ErrorSt(const QString &str);

	inline static std::list<std::pair<QString,int>> allMsgs;
	inline static std::list<QString*> logs;
	inline static std::list<QString*> warnings;
	inline static std::list<QString*> errors;

	struct LogData
	{
		QString text;
		const char *prefix;
		int code;
		const QBrush &brush;
		qint64 &counter;
		std::list<QString*> &currentList;

		LogData(const QString &log,
				const char *prefix,
				int code,
				const QBrush &brush,
				qint64 &counter,
				std::list<QString*> &currentList):
			text {log}, prefix {prefix}, code {code},
			brush{brush}, counter {counter}, currentList {currentList}
		{

		}
	};


	inline static std::list<LogData> logsDataList;

	inline static QString logsStartDateTime = QDateTime::currentDateTime().toString("yyyy.MM.dd HH.mm.ss");

public: signals:
	void SingalLog(QString log,
				   const char *prefix,
				   int code,
				   const QBrush *brush,
				   qint64 *counter,
				   std::list<QString*> *currentList);
public slots:
	void SlotLog(QString log,
				 const char *prefix,
				 int code,
				 const QBrush *brush,
				 qint64 *counter,
				 std::list<QString*> *currentList);

	static void RaceTest();
};

class LogedClass
{
private:
	QString msgPrefix;

public:
	virtual ~LogedClass() = default;

	virtual void SetLogPrefix(QString msgPrefix) final { this->msgPrefix = msgPrefix; };

	virtual void Log(QString str) const final;
	virtual void Warning(QString str) const final;
	virtual void Error(QString str) const final;
};

#endif // LOGS_H
