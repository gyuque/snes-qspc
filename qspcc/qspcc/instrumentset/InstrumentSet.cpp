#include "InstrumentSet.h"
#include <iostream>

InstrumentSet::InstrumentSet()
{
}


InstrumentSet::~InstrumentSet()
{
}

bool InstrumentSet::load(const char* manifestPath) {

	picojson::object jObj = jsonObjectFromFile(manifestPath);

	if (!jHasProperty(jObj, "base-frequency")) {
		return false;
	}

	mBaseFq = (float)jObj["base-frequency"].get<double>();
	mOriginOctave = (int)jObj["origin-octave"].get<double>();

	readInstDefs(jObj);

	return true;
}

void InstrumentSet::readInstDefs(picojson::object& parentObj) {
	char buf[8];
	for (int i = 0; i < 128; ++i) {
		sprintf_s(buf, 8, "%d", i);

		if (jHasProperty(parentObj, buf)) {
			picojson::object& def = parentObj[buf].get<picojson::object>();

			if (isInstDefValid(def)) {
				const std::string& brrFilename = def["brr"].get<std::string>();
				addInst(def);
			}
		}
	}
}

bool InstrumentSet::isInstDefValid(const picojson::object& def) {
	return jHasProperty(def, "brr") && jHasProperty(def, "adsr");
}

void InstrumentSet::addInst(const picojson::object& src) {
	InstDef def_s;

	def_s.filename = src.at("brr").get<std::string>();
	fprintf(stderr, "Inst filename: %s\n", def_s.filename.c_str());
}
