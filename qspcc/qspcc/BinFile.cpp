#include "BinFile.h"
#include <vector>


BinFile::BinFile(const char* filename, bool verbose_mode)
{
	mFileSize = 0;
	mContent = NULL;
	load(filename);
}


BinFile::~BinFile()
{
	if (mContent) {
		delete[] mContent;
		mContent = NULL;
	}
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

bool BinFile::exportToFile(const char* filename) {
	FILE* fpOut = fopen(filename, "wb");
	if (!fpOut) {
		return false;
	}

	const size_t n = mFileSize;
	fwrite(mContent, 1, n, fpOut);

	fclose(fpOut);
	return true;
}

uint8_t BinFile::at(unsigned int pos) const {
	if (pos >= mFileSize) { return 0; }
	return mContent[pos];
}

void BinFile::writeAt(unsigned int pos, uint8_t val) {
	if (pos >= mFileSize) { return; }
	mContent[pos] = val;
}