#ifndef GLOBALCONFIG_H_INCLUDED
#define GLOBALCONFIG_H_INCLUDED

#include "json/PicoJSONUtils.h"

class GlobalConfig
{
public:
	GlobalConfig();
	virtual ~GlobalConfig();

	void load();

	std::string getDriverConfigFileName() const {
		return mDriverConfigFileName;
	}

protected:
	std::string mDriverConfigFileName;
};

#endif
