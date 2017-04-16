#ifndef BRRPACKER_H_INCLUDED
#define BRRPACKER_H_INCLUDED

#include "C700BRR.h"
#include "../mml_types.h"
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

	void addBRR(const C700BRR* pBRR, bool verbose);
	void addDummyBlock();

	int getAttackOffsetByName(const std::string& name) const;
	int getLoopOffsetByName(const std::string& name, bool fallbackToDummy = false) const;

	void exportAll(ByteList& outBytes);
	// void exportSrcEntry(ByteList& outBytes, const std::string& name);
protected:
	void push_block(const PackedBRRBlock& block);
	unsigned int addBlocks(const C700BRR* pBRR, bool is_loop);
	void exportBlock(ByteList& outBytes, const PackedBRRBlock& block);

	unsigned int mCurrentBytesSize;
	PackedBlockList mBlockList;
	PackedBRRNameMap mAttackNameMap;
	PackedBRRNameMap mLoopNameMap;
};

#endif