#include "configItemFabric.h"

#include "config.h"
#include "protocolConfig.h"
#include "objectConfig.h"
#include "constantsConfig.h"
#include "windowConfig.h"
#include "import.h"
#include "sync.h"
#include "channelConfig.h"
#include "otherConfig.h"

std::shared_ptr<IConfigItem> ConfigItemFabric::Make(QString rowCaption)
{
	if(0) {}
	else if(rowCaption == FileRowInMainFile	    ::RowCaptionSt())	return std::make_shared<FileRowInMainFile>();
	//else if(rowCaption == ConfigFile		    ::RowCaptionSt())	return std::make_shared<ConfigFile>();
	// ConfigFile тут не нужен, он не создаётся через Make и у него нет конструктора по умолчанию
	else if(rowCaption == ConstantConfigItem	::RowCaptionSt())	return std::make_shared<ConstantConfigItem>();
	else if(rowCaption == ConstantsConfigItem	::RowCaptionSt())	return std::make_shared<ConstantsConfigItem>();
	else if(rowCaption == PraramConfigItem	    ::RowCaptionSt())	return std::make_shared<PraramConfigItem>();
	else if(rowCaption == ObjectClass			::RowCaptionSt())	return std::make_shared<ObjectClass>();
	else if(rowCaption == WorkerConfigItem	    ::RowCaptionSt())	return std::make_shared<WorkerConfigItem>();
	else if(rowCaption == ProtocolClass		    ::RowCaptionSt())	return std::make_shared<ProtocolClass>();
	else if(rowCaption == WidgetConfigItem	    ::RowCaptionSt())	return std::make_shared<WidgetConfigItem>();
	else if(rowCaption == RowConfigItem   	    ::RowCaptionSt())	return std::make_shared<RowConfigItem>();
	else if(rowCaption == WindowClass    		::RowCaptionSt())	return std::make_shared<WindowClass>();
	else if(rowCaption == ImportConfigItem	    ::RowCaptionSt())	return std::make_shared<ImportConfigItem>();
	else if(rowCaption == ObjectConfigItem	    ::RowCaptionSt())	return std::make_shared<ObjectConfigItem>();
	else if(rowCaption == WindowConfigItem	    ::RowCaptionSt())	return std::make_shared<WindowConfigItem>();
	else if(rowCaption == SyncConfigItem		::RowCaptionSt())	return std::make_shared<SyncConfigItem>();
	else if(rowCaption == ChannelConfigItem	    ::RowCaptionSt())	return std::make_shared<ChannelConfigItem>();
	else if(rowCaption == OtherConfigItem 	    ::RowCaptionSt())	return std::make_shared<OtherConfigItem>();
	else
	{
		Logs::ErrorSt("ConfigItemFabric::Make wrong className ["+rowCaption+"]");
		return nullptr;
	}
}
