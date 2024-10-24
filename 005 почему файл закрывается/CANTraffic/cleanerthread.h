#ifndef CLEANERTHREAD_HPP
#define CLEANERTHREAD_HPP

#include <memory>
#include <thread>

#include <QtDebug>
#include <QObject>
#include <QTextEdit>

#include "MyQShortings.h"

class CleanerThread : public QObject
{
	Q_OBJECT

	std::thread *threadCleaner = nullptr;
	QTextEdit  *objToClean = nullptr;
	unsigned int linesThreshold = 1000;
	bool threadCleanerStop = false;
	bool doClean = true;

public:
	explicit CleanerThread(QTextEdit  *objToClean_, unsigned int linesThreshold, QObject *parent = nullptr);
	~CleanerThread();

	// приостановка очистки
	void SuspendClean();
	// продолжение очистки
	void ContinueClean();

	void StartCleanObject(QTextEdit  *objToClean_, unsigned int linesThreshold_);
	void StopClean(); // если поток существует, то он останавливается и удаляется

	void SetLinesThreshold(unsigned int linesThreshold_);
	unsigned int GetLinesThreshold();

private: signals:
	void SignalNeedClean(int count);

private slots:

	void SlotClean(int count);
};

#endif // CLEANERTHREAD_HPP
