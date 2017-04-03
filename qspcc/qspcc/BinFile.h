#ifndef BINFILE_H_INCLUDED
#define BINFILE_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

class BinFile
{
public:
	BinFile(const char* filename, bool verbose_mode = false);
	virtual ~BinFile();

	static const size_t k32KB = 32 * 1024;

	uint8_t at(unsigned int pos) const;
	void writeAt(unsigned int pos, uint8_t val);
	size_t size() const { return mFileSize; }
	void writeByte(unsigned int position, uint8_t val);

	bool exportToFile(const char* filename, unsigned int startPos = 0, int exportSize = -1);
protected:
	uint8_t* mContent;
	void load(const char* filename);

	size_t mFileSize;
};

#endif