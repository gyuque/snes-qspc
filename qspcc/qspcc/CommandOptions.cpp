#include "stdafx.h"
#include "CommandOptions.h"
static void copyStrArgs(StringArgList& dest, const StringArgList& src);
static void addSwitchOnly(bool& inoutPrevFlag, CommandOptionList& outList, const std::string& prevStr);

void parseCommandOptions(CommandOptionList& outList, int argc, _TCHAR* argv[]) {
	if (argc < 2) {
		return;
	}

	const int nOptions = argc - 1;
	for (int i = 0; i < nOptions; ++i) {
		std::string raw = argv[i+1];
		if (raw.at(0) == '-') {
			CommandOptionEntry oe;
			oe.typeString = raw.substr(1);
			oe.content = raw;
			oe.multiContent.push_back(raw);

			outList.push_back(oe);
		} else {
			// 種類指定を伴わなければ入力ファイル(-i)として処理
			CommandOptionEntry oe;
			oe.typeString = "i";
			oe.content = raw;
			oe.multiContent.push_back(raw);

			outList.push_back(oe);
		}
	}

}

void makeCommandOptionsSummary(CommandOptionsSummary& outSummary, const CommandOptionList& inList) {
	outSummary.verboseLevel = 0;
	outSummary.quickLoadEnabled = false;

	const size_t n = inList.size();
	for (size_t i = 0; i < n; ++i) {
		const std::string& tp = inList[i].typeString;

		// fprintf(stderr, "[%d] '%s' %s\n", i, tp.c_str(), inList[i].content.c_str());

		if (0 == tp.compare("i") || 0 == tp.compare("I")) {
			copyStrArgs(outSummary.inputFileList, inList[i].multiContent);
		} else if (0 == tp.compare("v")) {
			outSummary.verboseLevel = 1;
		} else if (0 == tp.compare("V")) {
			outSummary.verboseLevel = 2;
		} else if (0 == tp.compare("q")) {
			outSummary.quickLoadEnabled = true;
		}
	}
}

void copyStrArgs(StringArgList& dest, const StringArgList& src) {
	StringArgList::const_iterator it;

	for (it = src.begin(); it != src.end(); it++) {
		dest.push_back( *it );
	}
}

AppMode validateCommandOptions(const CommandOptionsSummary& inSummary) {
	if (inSummary.inputFileList.size() < 1) {
		return APPMODE_USAGE;
	}

	return APPMODE_COMPILE;
}