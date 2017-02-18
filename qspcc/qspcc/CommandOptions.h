#ifndef COMMANDOPTIONS_H_INCLUDED
#define COMMANDOPTIONS_H_INCLUDED

#include <vector>
#include <string>

typedef int AppMode;
#define APPMODE_USAGE   0x1
#define APPMODE_COMPILE 0x2

typedef std::vector<std::string> StringArgList;

typedef struct _CommandOptionEntry {
	std::string typeString;
	std::string content;
	StringArgList multiContent;
} CommandOptionEntry;

typedef std::vector<CommandOptionEntry> CommandOptionList;

typedef struct _CommandOptionsSummary {
	StringArgList inputFileList;
	int verboseLevel;
} CommandOptionsSummary;


void parseCommandOptions(CommandOptionList& outList, int argc, _TCHAR* argv[]);
void makeCommandOptionsSummary(CommandOptionsSummary& outSummary, const CommandOptionList& inList);
AppMode validateCommandOptions(const CommandOptionsSummary& inSummary);

#endif