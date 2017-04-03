#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include "ROMEmbedder.h"

#define kSoundMetaBlockSize (32)
#define kMaxTitleLength (31)

ROMEmbedder::ROMEmbedder()
{
	mpUsingConfig = nullptr;
	mpSourceBin = nullptr;
}


ROMEmbedder::~ROMEmbedder()
{
	if (mpSourceBin) {
		delete mpSourceBin;
		mpSourceBin = nullptr;
	}
}

void ROMEmbedder::setConfig(const EmbedderConfig* pConfig) {
	mpUsingConfig = pConfig;
}

void ROMEmbedder::loadTemplate(const std::string& filename, const std::string& mapFilename) {
	if (mpSourceBin || !(mpUsingConfig) ) { return ; }
	
	mpSourceBin = Embedder::loadBinFileWithBase(mBaseDir, filename.c_str());

	std::string mapFullPath = mBaseDir + "/" + mapFilename;
	mRomMap.load(mapFullPath.c_str());

}

void ROMEmbedder::setBaseDir(const std::string& baseDir) {
	mBaseDir = baseDir;
}

void ROMEmbedder::writeMetadata(unsigned int index, const std::string& title, const std::string& authorName) {
	RomSectionEntry* sec = mRomMap.findSoundMetadataSection();
	if (!sec) { return; }

	const int base_ofs = kSoundMetaBlockSize * 2 * index;
	const int author_ofs = base_ofs + kSoundMetaBlockSize;

	writeString(base_ofs, title, kMaxTitleLength, kSoundMetaBlockSize);
	writeString(author_ofs, authorName, kMaxTitleLength, kSoundMetaBlockSize);
}

bool ROMEmbedder::writeSoundDriverImage(unsigned int index, const BinFile* pBin) {
	RomSectionEntry* sec = mRomMap.findDriverSection(index, false);
	RomSectionEntry* hiSec = mRomMap.findDriverSection(index, true);


	if (sec && hiSec) {
		fprintf(stderr, "L:%04X H:%04X\n", sec->offset, hiSec->offset);
		writeBin(sec->offset, pBin, 0);
		writeBin(hiSec->offset, pBin, BinFile::k32KB);
	}

	return true;
}

void ROMEmbedder::writeString(unsigned int startAddress, const std::string& strData, size_t validMaxLength, size_t bufferLength) {
	fprintf(stderr, "%X <- %s\n", startAddress, strData.c_str());

	size_t i;
	const size_t n = strData.size();

	for (i = 0; i < n; ++i) {

	}
}

bool ROMEmbedder::writeBin(unsigned int destStart, const BinFile* pSourceBin, unsigned int srcStart) {
	if (!mpSourceBin) {
		return false;
	}

	size_t i;
	const size_t n = BinFile::k32KB;

	for (i = 0; i < n; ++i) {
		mpSourceBin->writeByte(destStart + i, pSourceBin->at(srcStart + i));
	}

	return true;
}

bool ROMEmbedder::exportToFile(const char* filename) {
	if (!mpSourceBin) {
		return false;
	}

	mpSourceBin->exportToFile(filename);

	return true;
}

// Mapping file loader

ROMMapLoader::ROMMapLoader() : mReRomDef("(ROM[0-9a-zA-Z]+)\\s*:.+size\\s*=\\s*\\$([0-9a-fA-F]+).+#(.+)") {
}

ROMMapLoader::~ROMMapLoader() {
}

bool ROMMapLoader::load(const char* path) {
	std::ifstream ifs(path);
	if (ifs.fail()) {
		return false;
	}

	std::string ln;
	while (std::getline(ifs, ln)) {
		parseLine(ln);
	}

	calcOffset();
	dump();
	return true;
}

static bool is_snddrv(const std::string& s) {
	return NULL != strstr(s.c_str(), "snd");
}

static bool is_snd_meta(const std::string& s) {
	return NULL != strstr(s.c_str(), "smeta");
}

void ROMMapLoader::parseLine(const std::string& ln) {
	std::cmatch m;

	const bool found = std::regex_search(ln.c_str(), m, mReRomDef);
	//fprintf(stderr,"%d ",m.size());
	if (found && m.size() > 3) {
		const std::string& hex_size = m[2].str();
		const std::string& comment = m[3].str();
		size_t h;
		sscanf(hex_size.c_str(), "%x", &h);

		fprintf(stderr, "%s | %s(%d) | %s\n", m[1].str().c_str(), hex_size.c_str(), h, comment.c_str());

		RomSectionEntry ent;
		ent.size = h;
		ent.type = ROMSEC_OTHER;
		if (is_snddrv(comment)) {
			fprintf(stderr, " ^\n");
			ent.type = ROMSEC_SOUND_DRIVER;
		} else if (is_snd_meta(comment)) {
			fprintf(stderr, " ~\n");
			ent.type = ROMSEC_SOUND_META;
		}

		mSectionList.push_back(ent);
	}
}

void ROMMapLoader::calcOffset() {
	int di = 0;
	unsigned int sum = 0;
	size_t n = mSectionList.size();
	for (size_t i = 0; i < n; ++i) {
		RomSectionEntry& ent = mSectionList[i];
		ent.offset = sum;

		if (ent.type == ROMSEC_SOUND_DRIVER) {
			ent.driverIndex = di++;
		} else {
			ent.driverIndex = -1;
		}

		sum += ent.size;
	}
}

RomSectionEntry* ROMMapLoader::findDriverSection(unsigned int index, bool hi_part) {
	int n = (int)mSectionList.size();
	int foundIndex = findSectionWithType(ROMSEC_SOUND_DRIVER, index);
	if (foundIndex < 0) { return nullptr; }

	if (!hi_part) {
		return &(mSectionList[foundIndex]);
	} else {
		++foundIndex;
		if (foundIndex >= n) { return nullptr; }
		return &(mSectionList[foundIndex]);
	}
}

RomSectionEntry* ROMMapLoader::findSoundMetadataSection() {
	int foundIndex = findSectionWithType(ROMSEC_SOUND_META, 0);
	if (foundIndex < 0) { return nullptr; }

	return &(mSectionList[foundIndex]);
}

int ROMMapLoader::findSectionWithType(RomSectionType type, unsigned int driverIndex) {
	size_t n = mSectionList.size();

	for (size_t i = 0; i < n; ++i) {
		const RomSectionEntry& ent = mSectionList[i];
		if (ent.type == type) {
			if (ROMSEC_SOUND_DRIVER == type && ent.driverIndex != driverIndex) {
				continue;
			}

			return i;
		}
	}

	return -1;
}

void ROMMapLoader::dump() {
	size_t n = mSectionList.size();

	for (size_t i = 0; i < n; ++i) {
		char mk = ' ';
		const RomSectionEntry& ent = mSectionList[i];

		if (ent.type == ROMSEC_SOUND_DRIVER) {
			mk = '-';
		} else if (ent.type == ROMSEC_SOUND_META) {
			mk = '+';
		}

		fprintf(stderr, "%c %04X (%d) %d\n", mk, ent.offset, ent.offset, ent.driverIndex);
	}
}

