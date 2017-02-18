#include "GlobalConfig.h"

#define kConfigurationFileName "qspcc-conf.json"

#define kDriverConfigEntryName "driver-config"

GlobalConfig::GlobalConfig()
{
}

GlobalConfig::~GlobalConfig()
{
}

void GlobalConfig::load() {
	picojson::object j = jsonObjectFromFile(kConfigurationFileName);
	if (jHasProperty(j, kDriverConfigEntryName)) {
		mDriverConfigFileName = j[kDriverConfigEntryName].get<std::string>();
	}
}