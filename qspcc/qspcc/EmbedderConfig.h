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

	int getProgramOrigin() const { return mProgramOrigin; }
	int getMusicHeaderOrigin() const { return mMusicHeaderEmbedPosition; }
	int getSequenceOrigin() const { return mSequenceEmbedPosition; }
	int getInstDirOrigin() const { return mInstDirEmbedPosition; }
	int getBRRDirOrigin() const { return mBRRDirEmbedPosition; }
	int getFqTableOrigin() const { return mFqTableEmbedPosition; }
	int getBRRBodyOrigin() const { return mBRRBodyEmbedPosition; }
	int calcMusicHeaderCapacity() const;
	int calcSequenceCapacity() const;
	int calcInstDirCapacity() const;
	int calcBRRDirCapacity() const;
	int calcFqTableCapacity() const;
	int calcBRRBodyCapacity() const;
protected:
	EmbedAddressList mAllAddresses;
	std::regex mReAddressConstant;

	int mProgramOrigin;
	int mMusicHeaderEmbedPosition;
	int mSequenceEmbedPosition;
	int mInstDirEmbedPosition;
	int mBRRDirEmbedPosition;
	int mFqTableEmbedPosition;
	int mBRRBodyEmbedPosition;

	int findNextPosition(int prevOrigin) const;
	int countSizeToNext(int prevOrigin) const;

	void parseLine(const std::string& ln);
};

#endif