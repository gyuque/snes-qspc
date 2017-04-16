#ifndef ROMEMBEDDER_H_INCLUDED
#define ROMEMBEDDER_H_INCLUDED

#include <string>
#include <regex>
#include <vector>
#include "Embedder.h"

typedef enum {
	ROMSEC_OTHER,
	ROMSEC_SOUND_META,
	ROMSEC_SOUND_DRIVER
} RomSectionType;
typedef struct _RomSectionEntry {
	RomSectionType type;
	size_t offset;
	size_t size;
	int driverIndex;
} RomSectionEntry;

typedef std::vector<RomSectionEntry> RomSectionList;

class ROMMapLoader {
public:
	ROMMapLoader();
	virtual ~ROMMapLoader();
	bool load(const char* path, int verboseLevel);

	RomSectionEntry* findDriverSection(unsigned int index, bool hi_part);
	RomSectionEntry* findSoundMetadataSection();
	void dump();
protected:
	void parseLine(const std::string& ln, int verboseLevel);
	void calcOffset();

	int findSectionWithType(RomSectionType type, unsigned int driverIndex);

	std::regex mReRomDef;
	RomSectionList mSectionList;
};


class ROMEmbedder
{
public:
	ROMEmbedder();
	virtual ~ROMEmbedder();
	void setVerboseLevel(int lv) { mVerboseLevel = lv; }

	void setBaseDir(const std::string& baseDir);
	void setConfig(const EmbedderConfig* pConfig);
	void loadTemplate(const std::string& filename, const std::string& mapFilename, int verboseLevel);

	void clearMetadataArea(size_t nTracks);
	void writeMetadataHeader(bool quickLoad);
	void writeMetadata(unsigned int index, const std::string& title, const std::string& authorName);
	void writeString(unsigned int startAddress, const std::string& strData, size_t validMaxLength, size_t bufferLength);

	bool writeSoundDriverImage(unsigned int index, const BinFile* pBin);

	bool exportToFile(const char* filename);

	bool updateChecksum();
	bool validateChecksumPair() const;
protected:
	int mVerboseLevel;
	std::string mBaseDir;
	BinFile* mpSourceBin;
	const EmbedderConfig* mpUsingConfig;
	bool writeBin(unsigned int destStart, const BinFile* pSourceBin, unsigned int srcStart);

	ROMMapLoader mRomMap;
};

#endif