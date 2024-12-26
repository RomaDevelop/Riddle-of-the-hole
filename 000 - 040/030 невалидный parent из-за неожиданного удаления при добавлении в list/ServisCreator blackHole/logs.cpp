#include "logs.h"

#include <mutex>
#include <thread>

#include <QFile>

#include "thread_box.h"
#include "MyCppDifferent.h"
#include "MyQDifferent.h"
#include "MyQFileDir.h"

QBrush Logs::clLog = Qt::darkGreen;
QBrush Logs::clWarning = QColor(180, 110, 0);
QBrush Logs::clError = Qt::darkRed;

namespace forLogs {
	const QString log =		"log";
	const QString warning = "warning";
	const QString error =	"error";
	const QString badCode = "badCode";
	const int longestWordSize = warning.size();

	const char *prefixLog = "Log    ";
	const char *prefixWrg = "Warning";
	const char *prefixErr = "Error  ";

	Logs logsEmiter;

	bool previousLogWasMuted = false;

	std::mutex mtx;
}

class LogsWriterErrors
{
	QString logsWriterErrors;
	int previousLenth = 0;
	const QString endingAdd = "\r\n";
public:
	const QString& Content() { return logsWriterErrors; }
	QString LastError();
	void AddLogsWriterError(QString newError);
	void operator += (const QString &newError) { AddLogsWriterError(newError); }
};

class LogsWriter
{
	thread_box treadWriter;
	thread_box treadMuter;
	int countWrited = 0;
	QString currentLogsFile;
	inline static bool logsWriterCreated = false;

	LogsWriterErrors logsWriterErrors;

public:

	inline static volatile bool temporaryMuteOutputLogs = false;
	inline static volatile bool msgMutePrinted = false;

	LogsWriter();
	~LogsWriter();
	void WriteLogsLogsWriter();
	bool WriteLogs(int count, bool toEnd);

	enum logCodes { log, warning, error };
	static QString LogCodeToStr(int code)
	{
		if(0) {}
		else if(code == log)		return forLogs::log;
		else if(code == warning)	return forLogs::warning;
		else if(code == error)		return forLogs::error;
		return forLogs::badCode;
	}
	static QString LogCodeStrOneLenth(int code)
	{
		auto str = LogCodeToStr(code);
		while(str.length() < forLogs::longestWordSize)
			str += " ";
		if(str.length() > forLogs::longestWordSize)
			 Logs::ErrorSt("str.length() > forLogsWriter::longestWordSize");
		return str;
	}
} basicLogsWriter;

Logs::Logs(QObject * parent): QObject(parent)
{
	connect(this,&Logs::SingalLog,this,&Logs::SlotLog);
}

void Logs::AppendInTextBrowser(QTextEdit *tBrowser, const QString &str, const QBrush &brush)
{
	tBrowser->append(str);
	auto curs = tBrowser->textCursor();
	curs.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	curs.setPosition(curs.position() - (str.length()), QTextCursor::KeepAnchor);

	QTextCharFormat format;
	format.setForeground(brush);
	curs.setCharFormat(format);
	tBrowser->repaint();
}

void Logs::AddCbMsgGet(cbMsgGet_t cbMsgGet, int &id) // return id cbf
{
	idCbsMsgGetCounter++;
	cbsMsgGet.push_back(std::pair<cbMsgGet_t,int>(cbMsgGet,idCbsMsgGetCounter));
	id = idCbsMsgGetCounter;
}

void Logs::RemoveCbfMsgGet(int id)
{
	for(uint i=0; i<cbsMsgGet.size(); i++)
		if(cbsMsgGet[i].second == id)
		{
			cbsMsgGet.erase(cbsMsgGet.begin() + i);
			return;
		}
	ErrorSt("RemoveCbfMsgGet can't find cdf with id " + QString::number(id));
}

void Logs::Clear()
{
	tBrowserSt = nullptr;

	warningsCount = 0;
	errorsCount = 0;
}

void Logs::LogCommon(QString str, qint64 &counter, int code, std::list<QString*> &currentList,
					 const char *debugPrefix, const QBrush& brush)
{
	forLogs::mtx.lock();
	emit forLogs::logsEmiter.SingalLog(str, debugPrefix, code, &brush, &counter, &currentList);
	forLogs::mtx.unlock();
}

void Logs::LogSt(const QString &str)
{
	LogCommon(str, Logs::logsCount, LogsWriter::log, logs, forLogs::prefixLog, Logs::clLog);
}

void Logs::WarningSt(const QString &str)
{
	LogCommon(str, Logs::warningsCount, LogsWriter::warning, warnings, forLogs::prefixWrg, Logs::clWarning);
}

void Logs::ErrorSt(const QString &str)
{
	LogCommon(str, Logs::errorsCount, LogsWriter::error, errors, forLogs::prefixErr, Logs::clError);
}

void Logs::SlotLog(QString log,
				   const char *prefix,
				   int code,
				   const QBrush *brush,
				   qint64 *counter,
				   std::list<QString*> *currentList)
{

	qint64 &counterRef = *counter;
	counterRef++;
	allMsgs.emplace_back(log, code);
	currentList->push_back(&allMsgs.back().first);

	if(LogsWriter::temporaryMuteOutputLogs)
	{
		if(!LogsWriter::msgMutePrinted)
		{
			LogsWriter::msgMutePrinted = true;
			Logs::AppendInTextBrowser(tBrowserSt,"Слишком частый вывод логов, часть логов будет пропущено. Пропуск с лога "+QSn(Logs::AllCount())+"\n",Logs::clWarning);
			qDebug() << forLogs::prefixWrg << "Слишком частый вывод логов, часть логов будет пропущено. Пропуск с лога "+QSn(Logs::AllCount());
		}
		return;
	}

	qDebug() << prefix << log;
	log += "\n";
	if(tBrowserSt) Logs::AppendInTextBrowser(tBrowserSt,log,*brush);
}

void Logs::RaceTest()
{
	std::thread tr1([](){
		for(int i=0; i<1000; i++)
			Logs::LogSt("tr1 " + QSn(i));
	});
	tr1.detach();

	std::thread tr2([](){
		for(int i=0; i<1000; i++)
			Logs::LogSt("tr2 " + QSn(i));
	});
	tr2.detach();
}

void LogedClass::Log(QString str) const
{
	Logs::LogSt(msgPrefix + str);
}

void LogedClass::Warning(QString str) const
{
	Logs::WarningSt(msgPrefix + str);
}

void LogedClass::Error(QString str) const
{
	Logs::ErrorSt(msgPrefix + str);
}





QString LogsWriterErrors::LastError()
{
	QString lastError = logsWriterErrors.mid(previousLenth);
	lastError.chop(endingAdd.length());
	return lastError;
}

void LogsWriterErrors::AddLogsWriterError(QString newError)
{
	if(logsWriterErrors.size() < 2000)
	{
		qdbg << newError;
		previousLenth = logsWriterErrors.length();
		logsWriterErrors += newError + endingAdd;
	}
	else
	{
		QString lastError = "to much logsWriterErrors, writing stopped";
		if(!logsWriterErrors.endsWith(lastError))
			logsWriterErrors += std::move(lastError);
	}
}

LogsWriter::LogsWriter():
	treadWriter("LogsWriter.treadWriter"),
	treadMuter("LogsWriter.treadMuter")
{
	if(logsWriterCreated)
	{
		logsWriterErrors += "LogsWriter created multiple times";
		qdbg << logsWriterErrors.LastError();
		return;
	}
	logsWriterCreated = true;

	treadWriter.start([this](stopper_t stopper){
		std::this_thread::sleep_for(std::chrono::seconds(1));

		QString pathToCurrentLogsFile = MyQDifferent::PathToExe() + "/files/logs";
		currentLogsFile = pathToCurrentLogsFile +"/log " + Logs::logsStartDateTime + ".txt";

		if(MyQFileDir::CreatePath(pathToCurrentLogsFile))
		{
			while(1)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				int countNow = Logs::AllCount();
				int countNotWrited = countNow - countWrited;
				if(countNotWrited > 130) //11000
				{
					if(!WriteLogs(30, false)) // 1000
						break;
				}
				if(stopper) break;
			}
		}
		else logsWriterErrors += "LogsWriter can't create path " + currentLogsFile;
	});

	treadMuter.start([](stopper_t stopper){

		auto countFor = [](int milliseconds)
		{
			int countNow1 = Logs::AllCount();
			std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
			return Logs::AllCount() - countNow1;
		};

		while(1)
		{
			while(countFor(100) > 80)
			{
				temporaryMuteOutputLogs = true;
				msgMutePrinted = false;
			}

			temporaryMuteOutputLogs = false;

			if(stopper) break;
		}
	});
}

LogsWriter::~LogsWriter()
{
	treadWriter.finish(3000);
	treadMuter.finish(3000);
	WriteLogs(Logs::allMsgs.size(),true);
	WriteLogsLogsWriter();
}

void LogsWriter::WriteLogsLogsWriter()
{
	QFile file(currentLogsFile);
	if(file.open(QIODevice::Append))
	{
		file.write(logsWriterErrors.Content().toUtf8());
		file.close();
	}
	else qdbg <<  "WriteLogsLogsWriter can't open file " + currentLogsFile;
}

bool LogsWriter::WriteLogs(int count, bool toEnd)
{
	int countOriginal = count;
	QFile file(currentLogsFile);
	if(file.open(QIODevice::Append))
	{
		for(int i=0; i<count; i++)
		{
			if(!Logs::allMsgs.empty())
			{
				std::pair<QString,int> log = Logs::allMsgs.front();
				std::list<QString*> *currentLog = nullptr;
				QString currentLogName = LogCodeStrOneLenth(log.second);
				QString strToWrite = currentLogName + " " + log.first + "\r\n";
				file.write(strToWrite.toUtf8());
				countWrited++;

				if(log.second == logCodes::log) currentLog = &Logs::logs;
				else if(log.second == logCodes::warning) currentLog = &Logs::warnings;
				else if(log.second == logCodes::error) currentLog = &Logs::errors;

				if(currentLog)
				{
					if(!currentLog->empty())
					{
						QString *currentLogFront = currentLog->front();
						QString *ptrFrontLogInAllLogs = &Logs::allMsgs.front().first;
						if(currentLogFront != ptrFrontLogInAllLogs)
							logsWriterErrors += "LogsWriter log strings in allMsgs and currentLogName are different";

						currentLog->pop_front(); // очистить надо, даже если была ошибка выше
					}
					else logsWriterErrors += "LogsWriter thread Logs currentLogName empty";
				}

				Logs::allMsgs.pop_front();

				if(toEnd && i+1 == count && !Logs::allMsgs.empty())
				{
					count++;
					if(countOriginal + 100 >= count)
					{
						logsWriterErrors += "LogsWriter to much logs get while finishing work";
						break;
					}
				}
			}
			else
			{
				if(!toEnd) logsWriterErrors += "LogsWriter thread Logs::allMsgs.empty()";
				break;
			}
		}
		file.close();
	}
	else
	{
		logsWriterErrors += "LogsWriter thread can't open file " + file.fileName();
		return false;
	}

	return true;
}
