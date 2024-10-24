#ifndef PROGRAMM_H
#define PROGRAMM_H

#include <QMessageBox>

#include "object.h"

namespace GeneralDefines
{
	const QString version {"Версия файла"};
	const QString emulator {"Эмулятор"};
	const QString servis {"Сервис"};
	const QString window {"Окно"};
	const QString synchron {"Синхронизаиция"};

	const QStringList all {version, emulator, servis, synchron};
}

class Programm
{

public:
	QString version;
	std::vector<QStringList> generalStrs;
	std::vector<QStringList> windowsStrs;
	std::vector<QStringList> contantsStrs;
	std::vector<QStringList> paramsStrs;
	std::vector<QStringList> widgetsStrs;

	std::vector<Constants*> constants;
	std::vector<Object*> objects;

	Programm(IExcelWorkbook *wb);
	~Programm();

	void WorkGeneralStrs();

	void Log(const QString &logStr) {		qdbg << "Log:   " + logStr; }
	void Error(const QString &errorStr) {	qdbg << "Error: " + errorStr; }
};

#endif // PROGRAMM_H
