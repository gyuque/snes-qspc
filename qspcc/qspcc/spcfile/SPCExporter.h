#ifndef SPCEXPORTER_H_INCLUDED
#define SPCEXPORTER_H_INCLUDED

#include <string>
#include "../BinFile.h"


typedef struct _ID666 {
	char headerString[33];
	char unused1[2];

	unsigned char tagType;
	unsigned char tagVer;

	unsigned short regPC;
	unsigned char  regA;
	unsigned char  regX;
	unsigned char  regY;
	unsigned char  regP;
	unsigned char  regS;

	char unused2[2];

	char songTitle[32];
	char gameTitle[32];
	char spcDumper[16];
	char comment[32];

	char date[11];

	char duration[3];
	char fadeTime[4];

	char composer[32];

	char unused6;
	char emulatorType;

	char unused7[46];
} ID666;


class SPCExporter {
public:
	SPCExporter();
	virtual ~SPCExporter();

	bool exportToFile(const char* filename);
	void setProgramCounter(unsigned short val);
	void setComposer(const std::string& composer);
	void setTitle(const std::string& title);
	void setDuration(unsigned int duration);
	void setComment(const std::string& comment);
	void setDumperName(const std::string& name);
	void setGameTitle(const std::string& gameTitle);
	void writeDriverImage(const class BinFile* pSource, unsigned short destOrigin);
protected:
	BinFile mFileImage;
	ID666 mID666;

	void initID666();
	void writeHeader();
};

#endif