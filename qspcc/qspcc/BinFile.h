#ifndef BINFILE_H_INCLUDED
#define BINFILE_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

class BinFile
{
public:
	BinFile(const char* filename, bool verbose_mode = false);
	virtual ~BinFile();

	uint8_t at(unsigned int pos) const;
	void writeAt(unsigned int pos, uint8_t val);
	size_t size() const { return mFileSize; }
	void writeByte(unsigned int position, uint8_t val);

	bool exportToFile(const char* filename);
protected:
	uint8_t* mContent;
	void load(const char* filename);

	size_t mFileSize;
};

#endif