#ifndef CONFIGITEMFABRIC_H
#define CONFIGITEMFABRIC_H

#include "iconfigitem.h"

class ConfigItemFabric
{
public:
	static std::shared_ptr<IConfigItem> Make(QString className)
	{
		//if(className == ProtocolClass::classProtocolClass())
		{
			//return std::make_shared<ProtocolClass>();
		}
		//	else if(className == ObjectClass::classObjectClass())
		//	{
		//		return std::make_shared<ObjectClass>();
		//	}
		//	else if(className == Param_ns::Fields::rowCaption)
		//	{
		//		return std::make_shared<PraramConfigItem>();
		//	}
		//	else if(className == Editor_ns::Constants)
		//	{
		//		return std::make_shared<ConstantsConfigItem>();
		//	}
		//	else if(className == Editor_ns::WindowsClass)
		//	{
		//		return std::make_shared<WindowClass>();
		//	}
		//	else if(className == Import_ns::Fields::rowCaption)
		//	{
		//		return std::make_shared<ImportConfigItem>();
		//	}
		//else
		{
			Logs::ErrorSt("ConfigItemFabric::Make wrong className ["+className+"]");
			return nullptr;
		}
	}
};

#endif // CONFIGITEMFABRIC_H
