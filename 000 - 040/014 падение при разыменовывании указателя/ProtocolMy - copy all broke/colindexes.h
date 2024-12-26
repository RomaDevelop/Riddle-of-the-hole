#ifndef COLINDEXES_H
#define COLINDEXES_H

#include <QString>

namespace ObjectsCols
{
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
	const int system {1};
	const int category {2};
	const int module {3};
	const int moduleNumber {4};
	const int name {5};
	const int id_ {6}; // если без _ почему-то в другом месте подсвечивает синим
	const int type {7};
	const int strPeremennie {8};
	const int frame {9};
}

namespace ParamsColNames
{
	const QString moduleNumber {"Номер модуля"};
}

namespace CopyParamsColIndexes
{
	const int system {ParamsColIndexes::system};
	const int category {ParamsColIndexes::category};
	const int module {ParamsColIndexes::module};
	const int moduleNumber {ParamsColIndexes::moduleNumber};
	const int name {ParamsColIndexes::name};
	const int id_ {ParamsColIndexes::id_};
	const int idPSrc {ParamsColIndexes::type};
	const int idPDst {ParamsColIndexes::strPeremennie};
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
