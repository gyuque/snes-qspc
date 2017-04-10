#include <fstream>
#include "picojson.h"
#include "PicoJSONUtils.h"

picojson::object jsonObjectFromFile(const char* filename) {
    std::fstream inf(filename, std::ios::in);
    if (!inf) {
        picojson::object emp;
        return emp;
    }
    
    picojson::value j;
    inf >> j;

	if (inf.fail()) {
		std::cerr << "[picojson] " << picojson::get_last_error() << std::endl;
		picojson::object o;
		return o;
	}
    
    return j.get<picojson::object>();
}

bool jHasProperty(const picojson::object& o, const char* name) {
    picojson::object::const_iterator iEnd = o.end();
    return o.find(name) != iEnd;
}
