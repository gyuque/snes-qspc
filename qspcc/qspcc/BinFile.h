#ifndef BINFILE_H_INCLUDED
#define BINFILE_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

class BinFile
{
public:
	BinFile(const char* filename, bool verbose_mode = false);
	virtual ~BinFile();

protected:
	uint8_t* mContent;
	void load(const char* filename);

	size_t mFileSize;
};

#endif