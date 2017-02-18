#ifndef EMBEDDERCONFIG_H_INCLUDED
#define EMBEDDERCONFIG_H_INCLUDED
#include <string>
#include <regex>
#include <vector>

typedef std::vector<int> EmbedAddressList;

class EmbedderConfig
{
public:
	EmbedderConfig();
	virtual ~EmbedderConfig();

	bool load(const char* filename);

protected:
	EmbedAddressList mAllAddresses;
	std::regex mReAddressConstant;
	int mSequenceEmbedPosition;

	void parseLine(const std::string& ln);
};

#endif