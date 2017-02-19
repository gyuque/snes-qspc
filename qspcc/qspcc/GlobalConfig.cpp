#include "GlobalConfig.h"

#define kConfigurationFileName "qspcc-conf.json"

#define kDriverConfigEntryName "driver-config"
#define kDriverImageEntryName  "driver-image"

GlobalConfig::GlobalConfig()
{
	mBaseDir = ".";
}

GlobalConfig::~GlobalConfig()
{
}

void GlobalConfig::setBaseDir(const std::string& baseDir) {
	mBaseDir = baseDir;
}

bool GlobalConfig::load() {
	std::string path = mBaseDir;
	path += "/";
	path += kConfigurationFileName;

	picojson::object j = jsonObjectFromFile( path.c_str() );

	if (jHasProperty(j, kDriverConfigEntryName)) {
		mDriverConfigFileName = j[kDriverConfigEntryName].get<std::string>();
	} else {
		fprintf(stderr, "*** Configuration file \"%s\" is broken or removed!! ***\n", path.c_str());
		return false;
	}

	if (jHasProperty(j, kDriverImageEntryName)) {
		mDriverImageFileName = j[kDriverImageEntryName].get<std::string>();
	}

	return true;
}