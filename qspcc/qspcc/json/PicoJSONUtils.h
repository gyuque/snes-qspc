#ifndef MobmapSever_PicoJSONLoader_h
#define MobmapSever_PicoJSONLoader_h
#include "picojson.h"

picojson::object jsonObjectFromFile(const char* filename);
bool jHasProperty(const picojson::object& o, const char* name);

#endif
