#include "EmbedderConfig.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#define kProgramOriginName "ProgramOrigin"
#define kMusicHeaderName   "MusicHeaderOrigin"
#define kSeqOriginName     "MusicSeqOrigin"
#define kSeqDirOriginName "SeqDirOrigin"
#define kInstDirOriginName "InstDirOrigin"
#define kBRRDirOriginName  "BRRDirOrigin"
#define kFqTableOriginName "FqTableOrigin"
#define kBRRBodyOriginName "BRRBodyOrigin"

EmbedderConfig::EmbedderConfig() :
	mProgramOrigin(0),
	mMusicHeaderEmbedPosition(0),
	mSequenceEmbedPosition(0),
	mSeqDirEmbedPosition(0),
	mInstDirEmbedPosition(0),
	mBRRDirEmbedPosition(0),
	mFqTableEmbedPosition(0),
	mBRRBodyEmbedPosition(0),
	mReAddressConstant("constant +([_a-zA-Z]+Origin|TailPadding)\\( *\\$ *([0-9a-fA-F]+) *\\)")
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

	std::sort(mAllAddresses.begin(), mAllAddresses.end());

	return true;
}

static void pickAddressWithName(int& outAddr, int inAddr, const std::string& decl, const char* name) {
	if (0 == decl.compare(name)) {
		outAddr = inAddr;
	}
}

void EmbedderConfig::parseLine(const std::string& ln) {
	std::cmatch m;
	const bool found = std::regex_search(ln.c_str(), m, mReAddressConstant);
	if (found && m.size() > 2) {
		
		// fetch values
		const std::string& decl_name = m[1].str();

		int addr;
		sscanf_s(m[2].str().c_str(), "%x", &addr);

		pickAddressWithName(mProgramOrigin, addr, decl_name, kProgramOriginName);
		pickAddressWithName(mMusicHeaderEmbedPosition, addr, decl_name, kMusicHeaderName);
		pickAddressWithName(mSequenceEmbedPosition, addr, decl_name, kSeqOriginName);
		pickAddressWithName(mSeqDirEmbedPosition, addr, decl_name, kSeqDirOriginName);
		pickAddressWithName(mInstDirEmbedPosition, addr, decl_name, kInstDirOriginName);
		pickAddressWithName(mBRRDirEmbedPosition, addr, decl_name, kBRRDirOriginName);
		pickAddressWithName(mFqTableEmbedPosition, addr, decl_name, kFqTableOriginName);
		pickAddressWithName(mBRRBodyEmbedPosition, addr, decl_name, kBRRBodyOriginName);

		//std::cerr << decl_name << "  " << addr;
		mAllAddresses.push_back(addr);
	}
}

int EmbedderConfig::countSizeToNext(int prevOrigin) const {
	const int nxt = findNextPosition(prevOrigin);
	if (nxt > prevOrigin) {
		return nxt - prevOrigin;
	}

	return -1;
}

int EmbedderConfig::findNextPosition(int prevOrigin) const {
	EmbedAddressList::const_iterator it;
	for (it = mAllAddresses.begin(); it != mAllAddresses.end(); it++) {
		if (*it > prevOrigin) {
			return *it;
		}
	}

	return -1;
}

int EmbedderConfig::calcMusicHeaderCapacity() const {
	return countSizeToNext(mMusicHeaderEmbedPosition);
}

int EmbedderConfig::calcSequenceCapacity() const {
	return countSizeToNext(mSequenceEmbedPosition);
}

int EmbedderConfig::calcSeqDirCapacity() const {
	return countSizeToNext(mSeqDirEmbedPosition);
}

int EmbedderConfig::calcInstDirCapacity() const {
	return countSizeToNext(mInstDirEmbedPosition);
}

int EmbedderConfig::calcBRRDirCapacity() const {
	return countSizeToNext(mBRRDirEmbedPosition);
}

int EmbedderConfig::calcFqTableCapacity() const {
	return countSizeToNext(mFqTableEmbedPosition);
}

int EmbedderConfig::calcBRRBodyCapacity() const {
	return countSizeToNext(mBRRBodyEmbedPosition);
}


void EmbedderConfig::dump() {
	fprintf(stderr, "SPC embedder configuration\n");
	fprintf(stderr, "--------------------------------------------------\n");

	fprintf(stderr, "* Sequence data origin: %04Xh\n", mSequenceEmbedPosition);
	const int sqsize = calcSequenceCapacity();
	fprintf(stderr, "* Sequence data capacity: %d(%04Xh) bytes\n", sqsize, sqsize);

//	fprintf(stderr, "* BRR dir origin: %04Xh\n", getBRRDirOrigin());

	fprintf(stderr, "* BRR dir origin: %04Xh\n", getBRRDirOrigin());
	const int dirsize = calcBRRDirCapacity();
	fprintf(stderr, "* BRR dir capacity: %d(%04Xh) bytes , %d entries\n", dirsize, dirsize, dirsize / 4);

	fprintf(stderr, "* FqTable origin: %04Xh\n", getFqTableOrigin());
	const int fqsize = calcFqTableCapacity();

	fprintf(stderr, "* BRR body origin: %04Xh\n", getBRRBodyOrigin());
	const int bbsize = calcBRRBodyCapacity();
	fprintf(stderr, "* BRR body capacity: %d(%04Xh) bytes\n", bbsize, bbsize);
}