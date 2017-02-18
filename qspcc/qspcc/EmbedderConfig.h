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
	void dump();

	int getSequenceOrigin() const { return mSequenceEmbedPosition; }
	int calcSequenceCapacity() const;
protected:
	EmbedAddressList mAllAddresses;
	std::regex mReAddressConstant;
	int mSequenceEmbedPosition;

	int findNextPosition(int prevOrigin) const;
	int countSizeToNext(int prevOrigin) const;

	void parseLine(const std::string& ln);
};

#endif