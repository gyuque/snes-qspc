#include "stdafx.h"
#include "CommandOptions.h"
#include <regex>
#include <iostream>
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
	outSummary.outTypeFlags = OUTTYPE_DEFAULT;
	outSummary.compilationName = "qSPCPlay";
	outSummary.debugExportEnabled = false;
	outSummary.effectSeqEnabled = false;
	outSummary.initialPauseEnabled = false;
	outSummary.bin_is_half = false;

	static const std::regex reOName("[oO]=([-_0-9a-zA-Z\\(\\)\\[\\]=]+)");

	const size_t n = inList.size();
	for (size_t i = 0; i < n; ++i) {
		const std::string& tp = inList[i].typeString;

		// fprintf(stderr, "[%d] '%s' %s\n", i, tp.c_str(), inList[i].content.c_str());
		int k1 = 0;
		if (tp.length() > 0) {
			k1 = tp[0];
		}

		if (0 == tp.compare("i") || 0 == tp.compare("I")) {
			copyStrArgs(outSummary.inputFileList, inList[i].multiContent);
		}
		else if (0 == tp.compare("v")) {
			outSummary.verboseLevel = 1;
		} else if (k1 == 'e' || k1 == 'E') {
			outSummary.effectSeqEnabled = true;
		} else if (0 == tp.compare("V")) {
			outSummary.verboseLevel = 2;
		} else if (k1 == 'p' || k1 == 'P') {
			outSummary.initialPauseEnabled = true;
		} else if (0 == tp.compare("q")) {
			outSummary.quickLoadEnabled = true;
		} else if (k1 == 'r' || k1 == 'R') {
			outSummary.outTypeFlags = OUTTYPE_ROM;
		} else if (k1 == 's' || k1 == 'S') {
			outSummary.outTypeFlags = OUTTYPE_SPC;

		} else if (k1 == 'b' || k1 == 'B') {
			outSummary.outTypeFlags |= OUTTYPE_BIN;
		} else if (k1 == 'h' || k1 == 'H') {
			outSummary.outTypeFlags |= OUTTYPE_BIN;
			outSummary.bin_is_half = true;

		} else if (k1 == 'd' || k1 == 'D') {
			outSummary.debugExportEnabled = true;
		} else {
			std::cmatch m;
			const bool found = std::regex_search(tp.c_str(), m, reOName);
			if (found && m.size() > 1) {
				outSummary.compilationName = m[1].str();
			}
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