#ifndef INSTRUMENTSET_H_INCLUDED
#define INSTRUMENTSET_H_INCLUDED

#include "../json/PicoJSONUtils.h"
#include <vector>

typedef struct _InstDef {
	std::string filename;
} InstDef;

typedef std::vector<InstDef> InstDefList;

class InstrumentSet
{
public:
	InstrumentSet();
	virtual ~InstrumentSet();

	bool load(const char* manifestPath);

protected:
	float mBaseFq;
	int mOriginOctave;
	InstDefList mInstList;

	void readInstDefs(picojson::object& parentObj);
	static bool isInstDefValid(const picojson::object& def);

	void addInst(const picojson::object& src);
};

#endif