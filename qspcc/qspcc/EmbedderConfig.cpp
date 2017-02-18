#include "EmbedderConfig.h"
#include <fstream>
#include <iostream>
#include <string>

EmbedderConfig::EmbedderConfig() :
	mSequenceEmbedPosition(0),
	mReAddressConstant("constant +([_a-zA-Z]+Origin)\\(\\$ *([0-9a-fA-F]+) *\\)")
{
}

EmbedderConfig::~EmbedderConfig()
{
}

bool EmbedderConfig::load(const char* filename) {
	std::ifstream ifs(filename);
	if (ifs.fail()) {
		return false;
	}

	std::string ln;
	while (std::getline(ifs, ln)) {
		parseLine(ln);
	}

	return true;
}

void EmbedderConfig::parseLine(const std::string& ln) {
	std::cmatch m;
	const bool found = std::regex_search(ln.c_str(), m, mReAddressConstant);
	if (found && m.size() > 2) {
		
		// fetch values
		const std::string& decl_name = m[1].str();

		int addr;
		sscanf_s(m[2].str().c_str(), "%x", &addr);

		if (0 == decl_name.compare("MusicSeqOrigin")) {
			mSequenceEmbedPosition = addr;
		}

		//std::cerr << decl_name << "  " << addr;
		mAllAddresses.push_back(addr);
	}
}