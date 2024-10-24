#ifndef AllConstants
#define AllConstants

#include <QStringList>

#include "MyQDifferent.h"

//#include "icanchannel.h"

#include "logs.h"

namespace Common {
	const QString name = "name";
	const QString category = "category";
	const QString addProps = "addProterties";
	const QString variables = "variables";
	const QString caption = "caption";
	const QString Other = "Other";
}

namespace Import_ns {
	const QString rowCaption = "Import";

	const QString type { "type" };
	const int typeIndex {0};
	const QString name { Common::name };
	const int nameIndex {1};
	const QString policy { "policy" };
	const int policyIndex {2};
	const QString modification { "modification" };
	const int modificationIndex {3};
	const QString postModification { "postModification" };
	const int postModificationIndex {4};
	const QStringList all { type, name, policy, modification, postModification };
	const int count = all.size();

	namespace Policies {
		const QString empty { "" };
		const QString deepImport { "deep import" };
		const QString noDeepImport { "no deep import" };
		const QString removeAfterCopying { "removeAfterCopying" };
	}
}

namespace ObjectClass_ns
{
	const QString ObjectClass { "ObjectClass" };

	const QString name { Common::name };
	const int nameIndex {0};

	const QStringList all { name };
	const int count = all.size();

	const QString Param {"Param"};
	const QStringList canBeAdded { Param, Import_ns::rowCaption };
}

namespace ProtocolClass_ns
{
	const QString ProtocolClass { "ProtocolClass" };

	const QString name { Common::name };
	const int nameIndex {0};
	const QStringList all { name };
	const int count = all.size();

	const QString Worker {"Worker"};
	const QStringList canBeAdded { Worker, Import_ns::rowCaption };
}

namespace Editor_ns
{
	inline QString SettingsFile() { return MyQDifferent::PathToExe() + "/files/EditorSettings.ini"; }
	const QString fileMain {"fileMain"};
	const QString fileXML {"fileXLS"};
	const QString rowCaptionFileInMainFile {"File"};
	const QString ProtocolClass {"ProtocolClass"};
	const QString Constants {"Constants"};
	const QString WindowsClass {"WindowClass"};
	const QString Object {"Object"};
	const QString Window {"Window"};
	const QString Sync {"Sync"};
	const QString Channel {"Channel"};
	const QString Other { Common::Other };

	const QStringList canBeAddedAtFileXML { ObjectClass_ns::ObjectClass, ProtocolClass, Constants, WindowsClass, Object, Window, Sync, Channel, Other };
	const QStringList canBeAddedAtFileMain { rowCaptionFileInMainFile };
	const QStringList headerMain { rowCaptionFileInMainFile };
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

	const int colsCount {2};
}

namespace Constant_ns
{
	const QString rowCaptionConstant = "Constant";
	const QString rowCaptionConstants = Editor_ns::Constants;

	const QString name {Common::name};
	const QString category {Common::category};
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

namespace ConstantsVector_ns
{
	const QString name { Common::name };
	const int nameIndex {0};
	const QStringList all { name };
	const int count = all.size();

	const QStringList canBeAdded { Constant_ns::rowCaptionConstant, Import_ns::rowCaption };
}

namespace Object_ns
{
	const QString Object {Editor_ns::Object};

	const QString name {Common::name};
	const QString type { "type" };
	const QStringList all { name, type };
	const int count = all.size();
}

namespace ObjectConfigItem_ns
{
	const QString name { Common::name };
	const int nameIndex {0};
	const QString type { "type" };
	const int typeIndex {1};
	const QString objectClass { ObjectClass_ns::ObjectClass };
	const int objectClassIndex {2};
	const QString protocolClass { ProtocolClass_ns::ProtocolClass };
	const int protocolClassIndex {3};
	const QString constants { Constant_ns::rowCaptionConstants };
	const int constantsIndex {4};
	const QString channel { "channel" };
	const int channelIndex {5};
	const QString startSettings { "startSettings" };
	const int startSettingsIndex {6};

	const QStringList all { name, type, objectClass, protocolClass, constants, channel, startSettings };
	const int count = all.size();

	const QString rowCaption = Editor_ns::Object;

	const QStringList canBeAdded {};
}

namespace Param_ns
{
	const QString Param { ObjectClass_ns::Param };

	const QString name {Common::name};
	const QString category {Common::category};
	const QString id_ {"id"};
	const QString type {"type"};
	const QString addProterties {Common::addProps};

	const int nameI = 0;
	const int categoryI = 1;
	const int id_I = 2;
	const int typeI = 3;
	const int addProtertiesI = 4;

	const QStringList all { name, category, id_, type, addProterties };
	const int count = all.size();

	const QString rowCaption = Param;
}

namespace Protocol_ns
{
	const QString Protocol { "Protocol" };

	const QString name			{Common::name};
	const QString type			{"type"};
	const QString constants		{"constants"};
	const QString channel		{"channel"};
	const QString startSettings {"startSettings"};

	const QStringList all { name, type, constants, channel, startSettings };
	const int count = all.size();
}

namespace Worker_ns
{
	const QString Worker { ProtocolClass_ns::Worker };

	const QString name {Common::name};
	const QString category {Common::category};
	const QString id_ {"id"};
	const QString type {"type"};
	const QString addProps {Common::addProps};
	const QString variables { "variables" };
	const QString frame		{ "frame" };

	const int nameI = 0;
	const int categoryI = 1;
	const int id_I = 2;
	const int typeI = 3;
	const int addPropsI = 4;
	const int variablesI = 5;
	const int frameI = 6;

	const QStringList all { name, category, id_, type, addProps, variables, frame };
	const int count = all.size();
}

namespace FrameTypes
{
	const QString inFrame {"Входящий_кадр"};
	const QString outFrame {"Исходящий_кадр"};

	const QStringList allFrames {inFrame,outFrame};

	const QString upravlenie {"Управление"};
	const QString timer {"Таймер"};

	const QStringList all {inFrame,outFrame,upravlenie,timer};

};

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
	const QString rowCaption = "Widget";

	const int paramI		{0};
	const int frameI		{1};
	const int idWidgetI		{2};
	const int tabNameI		{3};
	const int tableNameI	{4};
	const int captionI		{5};
	const int viewWidgI		{6};
	const int ctrlWidgI		{7};

	const QString param		{"param"};
	const QString worker	{"worker"};
	const QString idWidget	{"idWidget"};
	const QString tabName	{"tabName"};
	const QString tableName	{"tableName"};
	const QString caption	{Common::caption};
	const QString viewWidg	{"viewWidg"};
	const QString ctrlWidg	{"ctrlWidg"};

	const QStringList all { param, worker, idWidget, tabName, tableName, caption, viewWidg, ctrlWidg };
	const int count = all.size();
}

namespace WindowClass_ns
{
	namespace Row
	{
		const QString rowCaption = "Row";

		const QString caption { Common::caption };
		const int captionI {0};
		const QStringList all { caption };
		const int count = all.size();

		const QStringList canBeAdded { Widget_ns::rowCaption, Import_ns::rowCaption };
	}

	const QString rowCaption = "WindowClass";

	const QString name { Common::name };
	const int nameIndex {0};
	const QStringList all { name };
	const int count = all.size();

	const QStringList canBeAdded { Widget_ns::rowCaption, Row::rowCaption, Import_ns::rowCaption };
}

namespace WindowConfigItem_ns
{
	namespace Fields {
		const QString name { Common::name };
		const int nameIndex {0};
		const QString windowClass { WindowClass_ns::rowCaption };
		const int windowClassIndex {1};
		const QString objectToConnect { "ObjectToConnect" };
		const int objectToConnectIndex {2};
		const QString Other { Common::Other };
		const int OtherIndex {3};

		const QStringList all { name, windowClass, objectToConnect, Other };
		const int count = all.size();

		const QString rowCaption = Editor_ns::Window;

		const QString external = "external ";
	}
}

namespace SyncConfigItem_ns
{
	namespace Fields
	{
		const QString rowCaption = Editor_ns::Sync;

		const QString name { Common::name };
		const int nameIndex {0};
		const QString command { "command" };
		const int commandIndex {1};
		const QStringList all { name, command };
		const int count = all.size();
	}
	const QStringList canBeAdded {  };
}

namespace ChannelConfigItem_ns
{
	namespace Fields
	{
		const QString rowCaption = Editor_ns::Channel;

		const QString name { Common::name };
		const int nameIndex {0};
		const QString type { "type" };
		const int typeIndex {1};
		const QString params { "params" };
		const int paramsIndex {2};
		const QStringList all { name, type, params };
		const int count = all.size();
	}
	const QStringList canBeAdded {  };

	namespace Settings {
		const QString reg11 { "SFF" };
		const QString reg29 { "EFF" };
		const QString reg1129 { "BFF" };

		const QString baud250 { "250" };
		const QString baud500 { "500" };
		const QString baud1000 { "1000" };
	}
}

namespace OtherConfigItem_ns
{
	const QString rowCaption = Editor_ns::Other;

	const QString content { "content" };
	const int contentIndex {0};
	const QStringList all { content };
	const int count = all.size();

	namespace Content
	{
		const QString ctrlPanel {"ControlPanel"};
		const QString monitor {"Monitor"};
	}

	namespace ControlPanel {
		const QString all {"all"};
		const QString connectSettings {"connectSettings"};
		const QString close {"close"};
		const QString closeNotSaveSettings {"closeNotSaveSettings"};
	}

	const QStringList canBeAdded {  };
}

namespace MonitorColIndexes
{
	const int name = 1;
	const int conObj = 2;
}

#endif // AllConstants
