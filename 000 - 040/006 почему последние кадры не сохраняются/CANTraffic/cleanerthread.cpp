#include "cleanerthread.h"

CleanerThread::CleanerThread(QTextEdit  *objToClean_, unsigned int linesThreshold, QObject *parent): QObject(parent)
{
	StartCleanObject(objToClean_, linesThreshold);
}
CleanerThread::~CleanerThread() { StopClean(); };

void CleanerThread::SuspendClean()
{
	doClean = false;
}
void CleanerThread::ContinueClean()
{
	doClean = true;
}

void CleanerThread::StopClean() // если поток существует, то он останавливается и удаляется
{
	if(threadCleaner)
	{
		threadCleanerStop = true;
		threadCleaner->join();
		delete threadCleaner;
		threadCleaner = nullptr;
	}
}

void CleanerThread::StartCleanObject(QTextEdit  *objToClean_, unsigned int linesThreshold_)
{
	objToClean = objToClean_;
	threadCleanerStop = false;
	linesThreshold = linesThreshold_;

	connect(this, &CleanerThread::SignalNeedClean, this, &CleanerThread::SlotClean);

	StopClean(); // если поток существует - останавливается и удаляется

	threadCleaner = new std::thread([this]
	{
		unsigned int count;
		while(!threadCleanerStop)
		{
			count = objToClean->document()->lineCount();
			if(doClean && count > linesThreshold)
			{
				count = count - linesThreshold;
				emit SignalNeedClean(count);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});
}

void CleanerThread::SetLinesThreshold(unsigned int linesThreshold_)
{
	linesThreshold = linesThreshold_;
}

unsigned int  CleanerThread::GetLinesThreshold()
{
	return linesThreshold;
}

void CleanerThread::SlotClean(int count)
{
	QTextCursor cursor = objToClean->textCursor();
	cursor.setPosition(0);
	cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, count);
	cursor.removeSelectedText();
}
