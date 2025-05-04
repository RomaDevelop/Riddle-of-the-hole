#ifndef CONFIGCONSTANTS_H
#define CONFIGCONSTANTS_H

#include <QStringList>

#include "icanchannel.h"

#include "logs.h"

namespace Import_ns {
	namespace Fields
	{
		const QString rowCaption = "Import";

		const QString type { "type" };
		const int typeIndex {0};
		const QString name { "name" };
		const int nameIndex {1};
		const QStringList all { type, name };
		const int count = all.size();
	}
}

namespace ObjectClass_ns
{
	const QString ObjectClass { "ObjectClass" };

	namespace Fields
	{
		const QString name { "name" };
		const int nameIndex {0};

		const QStringList all { name };
		const int count = all.size();
	}

	const QString Param {"Param"};
	const QStringList canBeAdded { Param, Import_ns::Fields::rowCaption };
}

namespace ProtocolClass_ns
{
	const QString ProtocolClass { "ProtocolClass" };

	namespace Fields
	{
		const QString name { "name" };
		const int nameIndex {0};
		const QStringList all { name };
		const int count = all.size();
	}

	const QString FrameWorker {"FrameWorker"};
	const QStringList canBeAdded { FrameWorker, Import_ns::Fields::rowCaption };
}

namespace Editor_ns
{
	const QString fileMain {"fileMain"};
	const QString fileXML {"fileXLS"};
	const QString FileInMain {"File"};
	const QString ProtocolClass {"ProtocolClass"};
	const QString Constants {"Constants"};
	const QString WindowsClass {"WindowClass"};
	const QString Definition {"Definition"};

	const QStringList allItemTypes { fileMain, fileXML, FileInMain,
				ObjectClass_ns::ObjectClass, ProtocolClass, Constants, WindowsClass, Definition };
	const QStringList canBeAddedAtFileXML { ObjectClass_ns::ObjectClass, ProtocolClass, Constants, WindowsClass, Definition };
	const QStringList canBeAddedAtFileMain { FileInMain };
	const QStringList headerMain { FileInMain };
	const QStringList headerXML { "Editor_ns::headerXML ERROR" };

	const QString xmlTagBody {"body"};

	const int mainFileFileIndex {0};

	const int colsCountInMainFile {1};
	const int colsCountInXMLFile {5};

	const int colCaption = 0;
	const int colPtrConfigItem = 1;
	const int colsCountInTree = 5;

	namespace Extentions
	{
		const QString main_ext {"main"};
		const QString xml {"xml"};

		const QStringList all { main_ext, xml };
	}
}

namespace SynchCols
{
	const int objectsToSynch {1};
}

namespace Constants_ns
{
	namespace Excel {
		const int name {1};
		const int category {2};
		const int describtion {3};
		const int bitSize {4};
		const int enc {5};
		const int value {6};
	}

	namespace Fields {
		const QString rowCaptionConstant = "Constant";
		const QString rowCaptionConstants = "Constants";

		const QString name {"name"};
		const QString category {"category"};
		const QString describtion {"describtion"};
		const QString bitSize {"bitSize"};
		const QString enc {"enc"};
		const QString value {"value"};

		const int nameI {0};
		const int categoryI {1};
		const int describtionI {2};
		const int bitSizeI {3};
		const int encI {4};
		const int valueI {5};

		const QStringList all { name, category, describtion, bitSize, enc, value };
		const int count = all.size();
	}
}

namespace ConstantsVector_ns
{
	namespace Fields
	{
		const QString name { "name" };
		const int nameIndex {0};
		const QStringList all { name };
		const int count = all.size();
	}
	const QStringList canBeAdded { Constants_ns::Fields::rowCaptionConstant, Import_ns::Fields::rowCaption };
}

namespace Object_ns
{
	const QString emulator {"Эмулятор"};
	const QString servis {"Сервис"};
	const int category {1};
	const int name {2};
	const int canChannel {3};
	const int regime {4};
	const int baud {5};
	const int startSettings {6};

	const QString reg11 { "SFF" };
	const QString reg29 { "EFF" };
	const QString reg1129 { "BFF" };

	const QString baud250 { "250" };
	const QString baud500 { "500" };
	const QString baud1000 { "1000" };

	struct Convert
	{
		static int GetICANRegime(QString regimeStr)
		{
			if(regimeStr == reg11) return ICANChannel::REG_CAN11;
			if(regimeStr == reg29) return ICANChannel::REG_CAN29;
			if(regimeStr == reg1129) return ICANChannel::REG_CAN11_29;
			Logs::ErrorSt("Convert::GetICANRegime wrong regimeStr ["+regimeStr+"]");
			return ICANChannel::REG_ERROR;
		}
		static int GetICANBaud(QString baudStr)
		{
			if(baudStr == baud250) return ICANChannel::BR_250K;
			if(baudStr == baud500) return ICANChannel::BR_500K;
			if(baudStr == baud1000) return ICANChannel::BR_1M;
			Logs::ErrorSt("Convert::GetICANBaud wrong baudStr ["+baudStr+"]");
			return ICANChannel::BR_ERROR;
		}
	};


}

namespace ObjectConfigItem_ns
{
	namespace Fields {
		const QString name { "name" };
		const int nameIndex {0};
		const QString type { "type" };
		const int typeIndex {1};
		const QString objectClass { ObjectClass_ns::ObjectClass };
		const int objectClassIndex {2};
		const QString protocolClass { ProtocolClass_ns::ProtocolClass };
		const int protocolClassIndex {3};
		const QString constants { Constants_ns::Fields::rowCaptionConstants };
		const int constantsIndex {4};
		const QString channel { "channel" };
		const int channelIndex {5};
		const QString regime { "regime" };
		const int regimeIndex {6};
		const QString baud { "baud" };
		const int baudIndex {7};
		const QString startSettings { "startSettings" };
		const int startSettingsIndex {8};

		const QStringList all { name, type, objectClass, protocolClass, constants, channel, regime, baud, startSettings };
		const int count = all.size();

		QString rowCaption = "Object";

		const QStringList canBeAdded {};
	}
}

namespace Param_ns
{
	const QString param { ObjectClass_ns::Param };
	namespace Fields
	{
		const QString name {"name"};
		const QString category {"category"};
		const QString id_ {"id_"};
		const QString type {"type"};
		const QString addProterties {"addProterties"};

		const int nameI = 0;
		const int categoryI = 1;
		const int id_I = 2;
		const int typeI = 3;
		const int addProtertiesI = 4;

		const QStringList all { name, category, id_, type, addProterties };
		const int count = all.size();

		const QString rowCaption = param;
	}

	namespace Excel {
		const int defenitionType {0};
		const int category {1};
		const int name {2};
		const int id_ {3}; // если без _ почему-то в другом месте подсвечивает синим
		const int type {4};
		const int addProterties {5};
		const int variables {6};
		const int frame {7};

		const int ColsCount {6};
	}
}

namespace ParamsColNames
{
	const QString moduleNumber {"Номер модуля"};
}

namespace FrameWorker_ns
{
	const QString FrameWorker { ProtocolClass_ns::FrameWorker };
	namespace Fields
	{
		const QString name {"name"};
		const QString category {"category"};
		const QString id_ {"id_"};
		const QString type {"type"};
		const QString addProterties {"addProterties"};
		const QString variables {"variables"};
		const QString frame {"frame"};

		const int nameI = 0;
		const int categoryI = 1;
		const int id_I = 2;
		const int typeI = 3;
		const int addProtertiesI = 4;
		const int variablesI = 5;
		const int frameI = 6;

		const QStringList all { name, category, id_, type, addProterties, variables, frame };
		const int count = all.size();
	}

	namespace Excel {
		const int defenitionType {0};
		const int category {1};
		const int name {2};
		const int id_ {3}; // если без _ почему-то в другом месте подсвечивает синим
		const int type {4};
		const int addProterties {5};
		const int variables {6};
		const int frame {7};

		const int ColsCount {8};
	}
}

namespace FrameTypes
{
	const QString inFrame{"Входящий_кадр"};
	const QString outFrame{"Исходящий_кадр"};

	const QStringList all {inFrame,outFrame};
};

namespace CopyParamsColIndexes
{
	const int defenitionType {Param_ns::Excel::defenitionType};
	const int category {Param_ns::Excel::category};
	const int name {Param_ns::Excel::name};
	const int id_ {Param_ns::Excel::id_};
	const int type {Param_ns::Excel::type};
	const int idPSrc {Param_ns::Excel::addProterties};
	const int idPDst {Param_ns::Excel::variables};
	const int modification {Param_ns::Excel::frame};
}

namespace WindowColIndexes
{
	const int name = 1;
	const int classWindow = 2;
	const int conObj = 3;
	const int w = 4;
	const int h = 5;
}

namespace Widget_ns
{
	namespace Excel {
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

	namespace Fields {
		const QString rowCaption = "Widget";

		const int idParamI		{0};
		const int idWidgetI		{1};
		const int tabNameI		{2};
		const int tableNameI	{3};
		const int captionI		{4};
		const int viewWidgI		{5};
		const int ctrlWidgI		{6};

		const QString idParam	{"idParam"};
		const QString idWidget	{"idWidget"};
		const QString tabName	{"tabName"};
		const QString tableName	{"tableName"};
		const QString caption	{"caption"};
		const QString viewWidg	{"viewWidg"};
		const QString ctrlWidg	{"ctrlWidg"};

		const QStringList all { idParam, idWidget, tabName, tableName, caption, viewWidg, ctrlWidg };
		const int count = all.size();
	}
}

namespace WindowClass_ns
{
	namespace Fields
	{
		const QString rowCaption = "WindowClass";

		const QString name { "name" };
		const int nameIndex {0};
		const QStringList all { name };
		const int count = all.size();
	}
	const QStringList canBeAdded { Widget_ns::Fields::rowCaption, Import_ns::Fields::rowCaption };
}

namespace Window_ns
{
	namespace Fields {
		const QString name { "name" };
		const int nameIndex {0};
		const QString windowClass { WindowClass_ns::Fields::rowCaption };
		const int windowClassIndex {1};
		const QString objectToConnect { "ObjectToConnect" };
		const int objectToConnectIndex {2};

		const QStringList all { name, windowClass, objectToConnect };
		const int count = all.size();

		QString rowCaption = "Window";
	}
}

namespace CopyWidgetsColIndexes
{
	const int classWindow {Widget_ns::Excel::classWindow};
	const int tabName {Widget_ns::Excel::tabName};
	const int tableName {Widget_ns::Excel::tableName};
	const int idWSrc {7};
	const int idWDst {8};
	const int idPConnect {9};
}

namespace MonitorColIndexes
{
	const int name = 1;
	const int conObj = 2;
}

#endif // CONFIGCONSTANTS_H
