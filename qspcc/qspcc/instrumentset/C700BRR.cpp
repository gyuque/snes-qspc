#include "C700BRR.h"

C700BRR::C700BRR(const std::string& name) : mName(name), mpBin(NULL) {
	mLoopPoint = 0;
	mBodySize = 0;
	mAttackPartBytesSize = 0;
	mLoopPartBytesSize = 0;
}

C700BRR::~C700BRR() {
	if (mpBin) {
		delete mpBin;
		mpBin = NULL;
	}
}

bool C700BRR::load(const std::string& path) {
	if (mpBin) { return false; }

	mpBin = new BinFile(path.c_str());
	fprintf(stderr, "SZ: %d\n", mpBin->size());

	mLoopPoint = readLoopPointData();
	fprintf(stderr, "LP: %d\n", mLoopPoint);

	mBodySize = mpBin->size() - 2;

	// sanity check
	if (mLoopPoint % 9) {
		fputs("Loop point must be 9n.\n", stderr);
		return false;
	}

	if (mBodySize % 9) {
		fputs("Body size must be 9n.\n", stderr);
		return false;
	}

	calcPartSizes();
	fixFlags();
	return true;
}

void C700BRR::fixFlags() {
	if (searchLoopBit()) {
		const int loopStartBlock = mLoopPoint / 9;
		if (loopStartBlock > 0) {
			addHeaderBit(0                 , BRRH_LOOP_BIT);
			addHeaderBit(loopStartBlock    , BRRH_LOOP_BIT);

			addHeaderBit(loopStartBlock - 1, BRRH_LOOP_BIT);
			addHeaderBit(loopStartBlock - 1, BRRH_END_BIT);
		}
	}
}

void C700BRR::calcPartSizes() {
	mLoopPartBytesSize = 0;
	if (searchLoopBit()) {
		mLoopPartBytesSize = mBodySize - mLoopPoint;
	}

	mAttackPartBytesSize = mBodySize - mLoopPartBytesSize;
}

unsigned int C700BRR::readLoopPointData() {
	// ループポイント読み取り（ファイル先頭16bit）
	// * ループポイントの値は自身を含まない（先頭+2から起算）
	return (mpBin->at(1) << 8) | mpBin->at(0);
}

unsigned int C700BRR::countBlocks() const {
	return mBodySize / 9;
}

bool C700BRR::searchLoopBit() const {
	const int n = countBlocks();
	for (int i = 0; i < n; ++i) {
		const uint8_t flg = blockHeaderAt(i);
		if (flg & BRRH_LOOP_BIT) {
			return true;
		}
	}

	return false;
}

uint8_t C700BRR::blockHeaderAt(unsigned int blockIndex) const {
	return mpBin->at( 2 + blockIndex * 9 );
}

void C700BRR::addHeaderBit(unsigned int blockIndex, uint8_t mask) {
	const uint8_t newVal = blockHeaderAt(blockIndex) | mask;
	mpBin->writeAt(2 + blockIndex * 9, newVal);
}

void C700BRR::exportAttackBlock(PackedBRRBlock& outBlock, unsigned int relBlockIndex) const {
	exportBlock(outBlock, relBlockIndex);
}

void C700BRR::exportLoopBlock(PackedBRRBlock& outBlock, unsigned int relBlockIndex) const {
	exportBlock(outBlock, (mLoopPoint / 9) + relBlockIndex);
}

void C700BRR::exportBlock(PackedBRRBlock& outBlock, unsigned int blockIndex) const {
	int pos = blockIndex * 9;

	outBlock.header = mpBin->at(pos++);

	for (int i = 0; i < BRR_BODY_LEN; ++i) {
		outBlock.body[i] = mpBin->at(pos++);
	}
}

void C700BRR::dump() {
	const int n = countBlocks();
	fprintf(stderr, "IDX |FLG| D0 D1 D2 D3 D4 D5 D6 D7\n");
	fprintf(stderr, "----+---+------------------------\n");
	for (int i = 0; i < n; ++i) {
		char lpmk = ' ';
		char eflg = '.';
		char Lflg = '.';

		const int blockOffset = 9 * i;
		if (blockOffset == mLoopPoint) {
			lpmk = '@';
		}

		const uint8_t flg = blockHeaderAt(i);
		if (flg & BRRH_END_BIT) {
			eflg = 'e';
		}
		if (flg & BRRH_LOOP_BIT) {
			Lflg = 'L';
		}

		fprintf(stderr, "%4d|%c%c%c| ", i , lpmk, eflg, Lflg);

		for (int j = 0; j < 8; ++j) {
			const int pos = 2 + blockOffset + 1 + j;
			if (j) {
				fputs(" ", stderr);
			}
			fprintf(stderr, "%02X", mpBin->at(pos));
		}
		fputs("\n", stderr);
	}

	fputs(searchLoopBit() ? "Loop: ENABLED\n" : "Loop: disabled\n", stderr);
	fprintf(stderr, "Attack part:%d bytes\n", getAttackPartBytesSize());
	fprintf(stderr, "Loop part:%d bytes\n", getLoopPartBytesSize());
}