#include <memory.h>
#include <string.h>
#include "SPCExporter.h"

#define kSPCFileHeaderSize 256
#define kSPCFileBodySize   65536
#define kSPCFileFooterSize 256

// footer = DSP + unused + Extra RAM
// DSP registers will be initialized by the driver. Thus this program does not set any values.

SPCExporter::SPCExporter() : mFileImage(NULL, false) {
	initID666();
	mFileImage.blank(kSPCFileHeaderSize + kSPCFileBodySize + kSPCFileFooterSize);
}

SPCExporter::~SPCExporter() {

}

void SPCExporter::initID666() {
	memset(&mID666, 0, sizeof(mID666));

	// : HEADER STRING
	const char* header_s = "SNES-SPC700 Sound File Data v0.30";
	memcpy_s(mID666.headerString, sizeof(mID666.headerString), header_s, strlen(header_s));

	// : (unused)
	mID666.unused1[0] = 0x1A;
	mID666.unused1[1] = 0x1A;

	// : Tag type/ver
	mID666.tagType = 0x1A;
	mID666.tagVer = 30;

	// : Controller Register
	mID666.regPC = 0x400;
	mID666.regA = 0;
	mID666.regX = 0;
	mID666.regY = 0;
	mID666.regP = 0;
	mID666.regS = 0xEF; // Stack top = 01EFh

	// : Texts

	static const char* kDefaultSongTitle = "Untitled";
	static const char* kDefaultGameTitle = "Unknown";
	static const char* kDumperName = "qSPC2";
	static const char* kDefaultComment = "Exported from qSPC2";
	static const char* kDefaultComposer = " Unknown";

	memcpy_s(mID666.songTitle, sizeof(mID666.songTitle), kDefaultSongTitle, strlen(kDefaultSongTitle));
	memcpy_s(mID666.gameTitle, sizeof(mID666.gameTitle), kDefaultGameTitle, strlen(kDefaultGameTitle));
	memcpy_s(mID666.spcDumper, sizeof(mID666.spcDumper), kDumperName      , strlen(kDumperName));
	memcpy_s(mID666.comment  , sizeof(mID666.comment)  , kDefaultComment  , strlen(kDefaultComment));
	memcpy_s(mID666.composer , sizeof(mID666.composer) , kDefaultComposer , strlen(kDefaultComposer));

	// : Date
	mID666.day = 21;
	mID666.month = 11;
	mID666.year = 1990;

	mID666.duration = 15;
	mID666.fadeTime[2] = 0x0A; // 00 00 0A 00 -> (2560ms)


	mID666.emulatorType = 2; // Snes9x(fake)
}

void SPCExporter::setProgramCounter(unsigned short val) {
	mID666.regPC = val;
}

void SPCExporter::writeHeader() {
#define write_field(pos, field) mFileImage.writeByteArray((pos), &(field), sizeof(field))

	write_field(0x00, mID666.headerString);
	write_field(0x21, mID666.unused1);
	write_field(0x23, mID666.tagType);
	write_field(0x24, mID666.tagVer);

	// registers
	write_field(0x25, mID666.regPC);
	write_field(0x27, mID666.regA);
	write_field(0x28, mID666.regX);
	write_field(0x29, mID666.regY);
	write_field(0x2A, mID666.regP);
	write_field(0x2B, mID666.regS);

	// Song data
	write_field(0x2E, mID666.songTitle);
	write_field(0x4E, mID666.gameTitle);
	write_field(0x6E, mID666.spcDumper);
	write_field(0x7E, mID666.comment);

	write_field(0x9E, mID666.day);
	write_field(0x9F, mID666.month);
	write_field(0xA0, mID666.year);

	write_field(0xA9, mID666.duration);
	write_field(0xAC, mID666.fadeTime);

	write_field(0xB0, mID666.composer);

	
	write_field(0xD1, mID666.emulatorType);
}

void SPCExporter::writeDriverImage(const BinFile* pSource, unsigned short destOrigin) {
	mFileImage.writeBinFile(kSPCFileHeaderSize + destOrigin, pSource);
}

bool SPCExporter::exportToFile(const char* filename) {
	writeHeader();
	mFileImage.exportToFile(filename);
	return true;
}

static void applyStringLengthLimit(std::string& s, size_t limit) {
	if (s.length() > limit) {
		s.resize(limit);
	}
}

void SPCExporter::setTitle(const std::string& title) {
	std::string cutStr = title;
	applyStringLengthLimit(cutStr, sizeof(mID666.songTitle) - 1);

	memset(&(mID666.songTitle), 0, sizeof(mID666.songTitle));
	memcpy_s(mID666.songTitle, sizeof(mID666.songTitle), cutStr.c_str(), cutStr.length());
}

void SPCExporter::setComposer(const std::string& composer) {
	std::string cutStr = " " + composer;
	applyStringLengthLimit(cutStr, sizeof(mID666.composer) - 1);

	memset(&(mID666.composer), 0, sizeof(mID666.composer));
	memcpy_s(mID666.composer, sizeof(mID666.composer), cutStr.c_str(), cutStr.length());
}