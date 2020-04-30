#include <memory.h>
#include <string.h>
#include <time.h>
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

static void writeCurrentDate(ID666& id666) {
	const time_t nowt = time(NULL);

	struct tm lt;
	localtime_s(&lt, &nowt);

	sprintf_s(id666.date, "%02d/%02d/%04d", lt.tm_mon+1, lt.tm_mday, lt.tm_year + 1900);
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
	static const char* kDefaultComposer = "Unknown";
	static const char* kDefaultDate = "11/21/1990";

	memcpy_s(mID666.songTitle, sizeof(mID666.songTitle), kDefaultSongTitle, strlen(kDefaultSongTitle));
	memcpy_s(mID666.gameTitle, sizeof(mID666.gameTitle), kDefaultGameTitle, strlen(kDefaultGameTitle));
	memcpy_s(mID666.spcDumper, sizeof(mID666.spcDumper), kDumperName      , strlen(kDumperName));
	memcpy_s(mID666.comment  , sizeof(mID666.comment)  , kDefaultComment  , strlen(kDefaultComment));
	memcpy_s(mID666.composer , sizeof(mID666.composer) , kDefaultComposer , strlen(kDefaultComposer));

	// : Date
	memcpy_s(mID666.date, sizeof(mID666.date), kDefaultDate, strlen(kDefaultDate));

	mID666.duration[0] = '1';
	mID666.duration[1] = '5';

	mID666.fadeTime[0] = '2';
	mID666.fadeTime[1] = '0';
	mID666.fadeTime[2] = '0';
	mID666.fadeTime[3] = '0';


	mID666.emulatorType = '2'; // Snes9x(fake)
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

	write_field(0x9E, mID666.date);

	write_field(0xA9, mID666.duration);
	write_field(0xAC, mID666.fadeTime);

	write_field(0xB1, mID666.composer);

	
	write_field(0xD2, mID666.emulatorType);
}

void SPCExporter::writeDriverImage(const BinFile* pSource, unsigned short destOrigin) {
	mFileImage.writeBinFile(kSPCFileHeaderSize + destOrigin, pSource);
}

bool SPCExporter::exportToFile(const char* filename) {
	writeCurrentDate(mID666);
	writeHeader();
	mFileImage.exportToFile(filename);
	return true;
}

static void applyStringLengthLimit(std::string& s, size_t limit) {
	if (s.length() > limit) {
		s.resize(limit);
	}
}

static void copyFieldString(void* dest, size_t size, const std::string& str) {
	memset(dest, 0, size);
	memcpy_s(dest, size, str.c_str(), str.length());
}

#define set_id666_field(field)  copyFieldString(mID666. field, sizeof(mID666. field), cutStr);

void SPCExporter::setTitle(const std::string& title) {
	std::string cutStr = title;
	applyStringLengthLimit(cutStr, sizeof(mID666.songTitle) - 1);
	set_id666_field(songTitle);
}

void SPCExporter::setComposer(const std::string& composer) {
	std::string cutStr = composer;
	applyStringLengthLimit(cutStr, sizeof(mID666.composer) - 1);
	set_id666_field(composer);
}

void SPCExporter::setComment(const std::string& comment) {
	if (comment.length() > 0) {
		std::string cutStr = comment;
		applyStringLengthLimit(cutStr, sizeof(mID666.comment) - 1);
		set_id666_field(comment);
	}
}

void SPCExporter::setDumperName(const std::string& name) {
	if (name.length() > 0) {
		std::string cutStr = name;
		applyStringLengthLimit(cutStr, sizeof(mID666.spcDumper));
		set_id666_field(spcDumper);
	}
}

void SPCExporter::setGameTitle(const std::string& gameTitle) {
	if (gameTitle.length() > 0) {
		std::string cutStr = gameTitle;
		applyStringLengthLimit(cutStr, sizeof(mID666.gameTitle));
		set_id666_field(gameTitle);
	}
}

void SPCExporter::setDuration(unsigned int duration) {
	if (duration > 999) { duration = 999; }
	else if (duration < 1) { duration = 1; }

	char buf[4];
	sprintf_s(buf, "%d", duration);

	for (unsigned int i = strlen(buf); i < 4; ++i) {
		buf[i] = 0;
	}

	mID666.duration[0] = buf[0];
	mID666.duration[1] = buf[1];
	mID666.duration[2] = buf[2];
}