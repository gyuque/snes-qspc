#include "BinFile.h"
#include <vector>


BinFile::BinFile(const char* filename, bool verbose_mode)
{
	mFileSize = 0;
	mContent = NULL;
	if (filename) {
		load(filename);
	}
}


BinFile::~BinFile()
{
	if (mContent) {
		delete[] mContent;
		mContent = NULL;
	}
}

void BinFile::blank(size_t size) {
	if (mContent) {
		return;
	}

	mFileSize = size;
	mContent = new uint8_t[mFileSize];
	memset(mContent, 0, size);
}

void BinFile::load(const char* filename) {
	std::vector<uint8_t> tempBuf;

	FILE* fp = fopen(filename, "rb");
	if (!fp) {
		return;
	}

	int k;
	while ((k = fgetc(fp)) != EOF) {
		tempBuf.push_back(k);
	}

	fclose(fp);


	mFileSize = tempBuf.size();
	mContent = new uint8_t[mFileSize];
	for (size_t i = 0; i < mFileSize; ++i) {
		mContent[i] = tempBuf[i];
	}

	//fprintf(stderr, "Driver image: %s (%d Bytes)\n", filename, mFileSize);
}

void BinFile::writeByte(unsigned int position, uint8_t val) {
	if (position < mFileSize) {
		mContent[position] = val;
	}
}

void BinFile::writeByteArray(unsigned int position, const void* pValues, size_t count) {
	for (size_t i = 0; i < count; ++i) {
		writeByte(position+i, ((const uint8_t*)pValues)[i]);
	}
}

void BinFile::writeBinFile(unsigned int position, const class BinFile* pSource) {
	const size_t len = pSource->size();
	for (size_t i = 0; i < len; ++i) {
		writeByte(position + i, pSource->at(i));
	}
}

bool BinFile::exportToFile(const char* filename, unsigned int startPos, int exportSize) {
	FILE* fpOut = fopen(filename, "wb");
	if (!fpOut) {
		return false;
	}

	int max = (int)(mFileSize - startPos);
	int n = (exportSize < 0) ? max : exportSize;
	if (n > max) { n = max; }

	fwrite(mContent + startPos, 1, n, fpOut);

	fclose(fpOut);
	return true;
}

uint8_t BinFile::at(unsigned int pos) const {
	if (pos >= mFileSize) { return 0; }
	return mContent[pos];
}

uint16_t BinFile::at_LE16(unsigned int pos) const {
	return at(pos) | ( at(pos) << 8 );
}

void BinFile::writeAt(unsigned int pos, uint8_t val) {
	if (pos >= mFileSize) { return; }
	mContent[pos] = val;
}

void BinFile::writeUint16LE(unsigned int pos, uint16_t val) {
	writeAt(pos  , val & 0xFF);
	writeAt(pos+1, (val >> 8) & 0xFF);
}

uint16_t BinFile::calcChecksum() const {
	if (!mContent) { return 0; }

	uint16_t sum = 0;
	for (size_t i = 0; i < mFileSize; ++i) {
		sum += (int)mContent[i];
	}

	return sum;
}