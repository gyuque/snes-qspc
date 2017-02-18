#include "GlobalConfig.h"

#define kConfigurationFileName "qspcc-conf.json"

#define kDriverConfigEntryName "driver-config"
#define kDriverImageEntryName  "driver-image"

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

	if (jHasProperty(j, kDriverImageEntryName)) {
		mDriverImageFileName = j[kDriverImageEntryName].get<std::string>();
	}
}