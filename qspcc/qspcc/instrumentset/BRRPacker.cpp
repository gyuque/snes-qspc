#include "BRRPacker.h"

BRRPacker::BRRPacker() {
	mCurrentBytesSize = 0;
}

BRRPacker::~BRRPacker() {

}

BPResult BRRPacker::addBRR(const C700BRR* pBRR, unsigned int fixPoint, bool verbose) {
	const unsigned int a_size = pBRR->getAttackPartBytesSize();
	const unsigned int l_size = pBRR->getLoopPartBytesSize();

	if (fixPoint) {
		// 固定ポイントまで強制的にアドレスを進める
		// 現時点でオーバーしていればエラー

		const int bsize = BRR_BODY_LEN + 1;
		fixPoint = (fixPoint / bsize) * bsize; // BLOCKSIZEの倍数にする

		if (mCurrentBytesSize < fixPoint) {
			const int blank_size = fixPoint - mCurrentBytesSize;
			const auto blocks_to_pad = blank_size / bsize;
			if (verbose) {
				fprintf(stderr, "\n\n[BLANKTOFIX] %d (%d)\n\n", blank_size, blocks_to_pad);
			}
			for (auto i = 0; i < blocks_to_pad; ++i) {
				addPaddingBlock();
			}
		}
	}

	if (a_size) {
		const unsigned int a_offset = mCurrentBytesSize;
		addBlocks(pBRR, false);
		mAttackNameMap[pBRR->getName()] = a_offset;

		if (verbose) {
			fprintf(stderr, " + BRR \"%s\" attack offset: %04X(%d)\n", pBRR->getName().c_str(), a_offset, a_offset);
		}
	}

	if (l_size) {
		const unsigned int l_offset = mCurrentBytesSize;
		addBlocks(pBRR, true);
		mLoopNameMap[pBRR->getName()] = l_offset;

		if (verbose) {
			fprintf(stderr, " + BRR \"%s\" loop offset: %04X(%d)\n", pBRR->getName().c_str(), l_offset, l_offset);
		}
	}

	return BRRPACK_OK;
}

unsigned int BRRPacker::addBlocks(const C700BRR* pBRR, bool is_loop) {
	int wroteBytes = 0;

	const int n = is_loop ? pBRR->countLoopBlocks() : pBRR->countAttackBlocks();
	for (int i = 0; i < n; ++i) {
		PackedBRRBlock blk;

		if (is_loop) {
			pBRR->exportLoopBlock(blk, i);
		} else {
			pBRR->exportAttackBlock(blk, i);
		}

		push_block(blk);
		wroteBytes += (BRR_BODY_LEN + 1);
	}

	return wroteBytes;
}

void BRRPacker::push_block(const PackedBRRBlock& block) {
	mBlockList.push_back(block);
	mCurrentBytesSize += (BRR_BODY_LEN + 1);
}

int BRRPacker::getAttackOffsetByName(const std::string& name) const {
	PackedBRRNameMap::const_iterator it = mAttackNameMap.find(name);
	if (it == mAttackNameMap.end()) { return -1; }

	return it->second;
}

int BRRPacker::getLoopOffsetByName(const std::string& name, bool fallbackToDummy) const {
	PackedBRRNameMap::const_iterator it = mLoopNameMap.find(name);
	if (it == mLoopNameMap.end() && fallbackToDummy) {
		// ループ部が見つからない場合はダミーデータにフォールバックする
		it = mLoopNameMap.find(DUMMY_BLOCK_NAME);
	}

	if (it == mLoopNameMap.end()) { return -1;  }

	return it->second;
}

void BRRPacker::addDummyBlock() {
	if (getLoopOffsetByName(DUMMY_BLOCK_NAME) >= 0) {
		return;
	}

	PackedBRRBlock b;
	writeZeroBlock(b);

	const unsigned int l_offset = mCurrentBytesSize;
	push_block(b);
	mLoopNameMap[DUMMY_BLOCK_NAME] = l_offset;
}

void BRRPacker::addPaddingBlock() {
	// fix位置調整用の埋め草ブロックを生成・追加

	PackedBRRBlock b;
	writeZeroBlock(b);
	push_block(b);
}

void BRRPacker::writeZeroBlock(PackedBRRBlock& b) {
	b.header = BRRH_END_BIT | BRRH_LOOP_BIT;
	for (int i = 0; i < BRR_BODY_LEN; ++i) {
		b.body[i] = 0;
	}
}

void BRRPacker::exportAll(ByteList& outBytes) const {
	const size_t n = mBlockList.size();

	for (size_t i = 0; i < n; ++i) {
		exportBlock(outBytes, mBlockList[i]);
	}
}

void BRRPacker::exportBlock(ByteList& outBytes, const PackedBRRBlock& block) const {
	outBytes.push_back( block.header );
	for (int i = 0; i < BRR_BODY_LEN; ++i) {
		outBytes.push_back(block.body[i]);
	}
}

/*
void BRRPacker::exportSrcEntry(ByteList& outBytes, const std::string& name) {
	const uint16_t a = (uint16_t)( getAttackOffsetByName(name) );
	const uint16_t l = (uint16_t)( getLoopOffsetByName(name, true) );

	outBytes.push_back(a & 0xFF);
	outBytes.push_back((a >> 8) & 0xFF);
	outBytes.push_back(l & 0xFF);
	outBytes.push_back((l >> 8) & 0xFF);
}*/
