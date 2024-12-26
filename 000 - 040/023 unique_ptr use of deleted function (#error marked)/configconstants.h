#ifndef COLINDEXES_H
#define COLINDEXES_H

#include <QStringList>

namespace Editor_ns
{
	const QString fileMain {"fileMain"};
	const QString fileXML {"fileXLS"};
	const QString FileInMain {"File"};
	const QString ObjectClass {"ObjectClass"};
	const QString ProtocolClass {"ProtocolClass"};
	const QString ConstantsClass {"ConstantsClass"};
	const QString WindowsClass {"WindowsClass"};
	const QString Definition {"Definition"};

	const QStringList allItemTypes { fileMain, fileXML, FileInMain, ObjectClass, ProtocolClass, ConstantsClass, WindowsClass, Definition };
	const QStringList canBeAddedAtFileXML { ObjectClass, ProtocolClass, ConstantsClass, WindowsClass, Definition };
	const QStringList canBeAddedAtFileMain { FileInMain };

	const int colsCountInXMLFile {5};

	const int colCaption = 0;
	const int colPtrConfigItem = 1;
	const int colType = 2;
	const int colFileName = 3;
	const int colFilePath = 4;
	const int colsCountInTree = 5;

	namespace Extentions
	{
		const QString main_ext {"main"};
		const QString xml {"xml"};

		const QStringList all { main_ext, xml };
	}
}

namespace ObjectsCols
{
	const QString ObjectClass { "ObjectClass" };
	const QString emulator {"Эмулятор"};
	const QString servis {"Сервис"};
	const int category {1};
	const int name {2};
	const int canChannel {3};
	const int startSettings {4};
}

namespace SynchCols
{
	const int objectsToSynch {1};
}

namespace ConstantsColIndexes
{
	const int name {1};
	const int category {2};
	const int describtion {3};
	const int bitSize {4};
	const int enc {5};
	const int value {6};
}

namespace ParamsColIndexes
{
	const int defenitionType {0};
	const int category {1};
	const int name {2};
	const int id_ {3}; // если без _ почему-то в другом месте подсвечивает синим
	const int type {4};
	const int addProterties {5};
	const int strPeremennie {6};
	const int frame {7};
}

namespace ParamsColNames
{
	const QString moduleNumber {"Номер модуля"};
}

namespace FrameTypes
{
	const QString inFrame{"Входящий_кадр"};
	const QString outFrame{"Исходящий_кадр"};

	const QStringList all {inFrame,outFrame};
};

namespace CopyParamsColIndexes
{
	const int defenitionType {ParamsColIndexes::defenitionType};
	const int category {ParamsColIndexes::category};
	const int name {ParamsColIndexes::name};
	const int id_ {ParamsColIndexes::id_};
	const int type {ParamsColIndexes::type};
	const int idPSrc {ParamsColIndexes::addProterties};
	const int idPDst {ParamsColIndexes::strPeremennie};
	const int modification {ParamsColIndexes::frame};
}

namespace WindowColIndexes
{
	const int name = 1;
	const int classWindow = 2;
	const int conObj = 3;
	const int w = 4;
	const int h = 5;
}

namespace WidgetColIndexes
{
	const int idParam {1};
	const int idWidget {2};
	const int paramDescribtion {3};
	const int classWindow {4};
	const int tabName {5};
	const int tableName {6};
	const int caption {7};
	const int viewWidg {8};
	const int ctrlWidg {9};
}

namespace CopyWidgetsColIndexes
{
	const int classWindow {WidgetColIndexes::classWindow};
	const int tabName {WidgetColIndexes::tabName};
	const int tableName {WidgetColIndexes::tableName};
	const int idWSrc {7};
	const int idWDst {8};
	const int idPConnect {9};
}

namespace MonitorColIndexes
{
	const int name = 1;
	const int conObj = 2;
}

#endif // COLINDEXES_H
