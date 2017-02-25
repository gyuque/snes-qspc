#ifndef BRRPACKER_H_INCLUDED
#define BRRPACKER_H_INCLUDED

#include "C700BRR.h"
#include <vector>
#include <map>
#include <string>

typedef std::vector<PackedBRRBlock> PackedBlockList;
typedef std::map<std::string, unsigned int> PackedBRRNameMap;

#define DUMMY_BLOCK_NAME "/dmy/"

class BRRPacker
{
public:
	BRRPacker();
	virtual ~BRRPacker();

	void addBRR(const C700BRR* pBRR);
	void addDummyBlock();

	int getAttackOffsetByName(const std::string& name) const;
	int getLoopOffsetByName(const std::string& name, bool fallbackToDummy = false) const;
protected:
	void push_block(const PackedBRRBlock& block);
	unsigned int addBlocks(const C700BRR* pBRR, bool is_loop);

	unsigned int mCurrentBytesSize;
	PackedBlockList mBlockList;
	PackedBRRNameMap mAttackNameMap;
	PackedBRRNameMap mLoopNameMap;
};

#endif