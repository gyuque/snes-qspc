#ifndef C700BRR_H_INCLUDED
#define C700BRR_H_INCLUDED

#include <string>
#include "../BinFile.h"

#define BRRH_END_BIT  (0x01)
#define BRRH_LOOP_BIT (0x02)

#define BRR_BODY_LEN 8
typedef struct _PackedBRRBlock {
	uint8_t header;
	uint8_t body[BRR_BODY_LEN];
} PackedBRRBlock;


class C700BRR {
public:
	C700BRR(const std::string& name);
	virtual ~C700BRR();

	std::string getName() const { return mName; }
	bool load(const std::string& path);

	void dump();

	// accessor methods
	unsigned int getAttackPartBytesSize() const { return mAttackPartBytesSize; }
	unsigned int getLoopPartBytesSize() const { return mLoopPartBytesSize; }

	unsigned int countAttackBlocks() const { return mAttackPartBytesSize / 9; }
	unsigned int countLoopBlocks() const { return mLoopPartBytesSize / 9; }

	void exportAttackBlock(PackedBRRBlock& outBlock, unsigned int relBlockIndex) const;
	void exportLoopBlock(PackedBRRBlock& outBlock, unsigned int relBlockIndex) const;
protected:
	std::string mName;
	BinFile* mpBin;
	unsigned int mLoopPoint;
	unsigned int mBodySize;

	unsigned int mAttackPartBytesSize;
	unsigned int mLoopPartBytesSize;

	void fixFlags();
	void calcPartSizes();
	unsigned int readLoopPointData();
	bool searchLoopBit() const;

	// accessor methods
	unsigned int countBlocks() const;
	uint8_t blockHeaderAt(unsigned int blockIndex) const;
	void addHeaderBit(unsigned int blockIndex, uint8_t mask);

	void exportBlock(PackedBRRBlock& outBlock, unsigned int blockIndex) const;
};

#endif