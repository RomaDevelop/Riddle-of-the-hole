#ifndef CONFIGITEMFABRIC_H
#define CONFIGITEMFABRIC_H

#include "IConfigItem.h"

class ConfigItemFabric
{
public:
	static std::shared_ptr<IConfigItem> Make(QString rowCaption);
};

#endif // CONFIGITEMFABRIC_H
