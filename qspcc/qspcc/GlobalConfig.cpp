#include "GlobalConfig.h"

#define kConfigurationFileName "qspcc-conf.json"

#define kDriverConfigEntryName "driver-config"
#define kDriverImageEntryName  "driver-image"
#define kRomImageEntryName  "rom-image"
#define kRomMapEntryName    "rom-map"
#define kMaxTracksEntryName "max-tracks"
#define kQuickLoadBlobKBName "qload-blob-kb"

GlobalConfig::GlobalConfig() : mMaxTracks(1), mQuickLoadBlobSize(16*1024)
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

	if (jHasProperty(j, kRomImageEntryName)) {
		mRomImageFileName = j[kRomImageEntryName].get<std::string>();
	}

	if (jHasProperty(j, kRomMapEntryName)) {
		mRomMapFileName = j[kRomMapEntryName].get<std::string>();
	}

	if (jHasProperty(j, kMaxTracksEntryName)) {
		mMaxTracks = (int)( j[kMaxTracksEntryName].get<double>() );
	}

	if (jHasProperty(j, kQuickLoadBlobKBName)) {
		mQuickLoadBlobSize = 1024 * (size_t)(j[kQuickLoadBlobKBName].get<double>());
	}

	return true;
}