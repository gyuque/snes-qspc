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
}