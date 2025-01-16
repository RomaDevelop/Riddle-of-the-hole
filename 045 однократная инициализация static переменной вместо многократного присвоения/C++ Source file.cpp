// неправильно
void SetCurrentLogsFile(const QString &name)
{
	InterruptLogsWriting();
	static QString curFile = MyQDifferent::PathToExe() + "/files/logs/" + CurrentDateTime() + " " + name + ".txt";
	ContinueLogsWriting(curFile);
}

// правильно
void SetCurrentLogsFile(const QString &name)
{
	InterruptLogsWriting();
	static QString curFile;
	curFile = MyQDifferent::PathToExe() + "/files/logs/" + CurrentDateTime() + " " + name + ".txt";
	ContinueLogsWriting(curFile);
}