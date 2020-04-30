#ifndef COMMANDOPTIONS_H_INCLUDED
#define COMMANDOPTIONS_H_INCLUDED

#include <vector>
#include <string>

typedef int AppMode;
#define APPMODE_USAGE   0x1
#define APPMODE_COMPILE 0x2

#define OUTTYPE_SPC     0x01
#define OUTTYPE_ROM     0x02
#define OUTTYPE_BIN     0x04
#define OUTTYPE_DEFAULT (OUTTYPE_SPC | OUTTYPE_ROM)

typedef std::vector<std::string> StringArgList;

typedef struct _CommandOptionEntry {
	std::string typeString;
	std::string content;
	StringArgList multiContent;
} CommandOptionEntry;

typedef std::vector<CommandOptionEntry> CommandOptionList;

typedef struct _CommandOptionsSummary {
	std::string compilationName;
	StringArgList inputFileList;
	int verboseLevel;
	unsigned char outTypeFlags;
	bool quickLoadEnabled;
	bool effectSeqEnabled;
	bool initialPauseEnabled;
	bool debugExportEnabled;

	bool bin_is_half;
} CommandOptionsSummary;


void parseCommandOptions(CommandOptionList& outList, int argc, _TCHAR* argv[]);
void makeCommandOptionsSummary(CommandOptionsSummary& outSummary, const CommandOptionList& inList);
AppMode validateCommandOptions(const CommandOptionsSummary& inSummary);

#endif