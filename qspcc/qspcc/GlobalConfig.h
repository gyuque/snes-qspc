#ifndef GLOBALCONFIG_H_INCLUDED
#define GLOBALCONFIG_H_INCLUDED

#include "json/PicoJSONUtils.h"

class GlobalConfig
{
public:
	GlobalConfig();
	virtual ~GlobalConfig();

	void setBaseDir(const std::string& baseDir);
	bool load();

	std::string getDriverConfigFileName() const {
		return mDriverConfigFileName;
	}

	std::string getDriverImageFileName() const {
		return mDriverImageFileName;
	}

	std::string getRomImageFileName() const {
		return mRomImageFileName;
	}

	std::string getRomMapFileName() const {
		return mRomMapFileName;
	}

	size_t getMaxSongTracks() const {
		return mMaxTracks;
	}

protected:
	std::string mBaseDir;
	std::string mDriverConfigFileName;
	std::string mDriverImageFileName;

	std::string mRomImageFileName;
	std::string mRomMapFileName;

	size_t mMaxTracks;
};

#endif
