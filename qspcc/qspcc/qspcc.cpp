#include "stdafx.h"
#include <iostream>
#include "MMLCompiler.h"
#include "GlobalConfig.h"
#include "Embedder.h"
#include "CommandOptions.h"
#include "tester/testers.h"
#include "win32/pwd.h"

static GlobalConfig sGlobalConfig;
static bool globalSetup(Embedder& embd);
static void showUsage();

int _tmain(int argc, _TCHAR* argv[])
{
	Embedder embd;
	if (!globalSetup(embd)) {
		fputs("Failed to launch.\n", stderr);
		return -1;
	}

//doDocumentTest(&embd);
//return 0;//

	// Read command options
	CommandOptionList opt_list;
	CommandOptionsSummary opt_summary;
	parseCommandOptions(opt_list, argc, argv);
	makeCommandOptionsSummary(opt_summary, opt_list);
	const AppMode mode = validateCommandOptions(opt_summary);

	switch (mode) {
	case APPMODE_USAGE:
		showUsage();
		return 0;
		break;
	default:
		break;
	}

	if (opt_summary.verboseLevel) {
		fprintf(stderr, "Verbose mode enabled. (Level=%d)\n", opt_summary.verboseLevel);
	}

	MMLCompiler compiler;
	compiler.setVerboseLevel( opt_summary.verboseLevel );
	compiler.compile( opt_summary.inputFileList[0].c_str() );

	if (opt_summary.verboseLevel > 0) {
		fprintf(stderr, "Embedding sequence data...\n");
		embd.dumpConfig();
	}

	embd.embed(NULL);

	return 0;
}

bool globalSetup(Embedder& embd) {
	const std::string& selfDir = getSelfDir();
	sGlobalConfig.setBaseDir(selfDir);
	if (!sGlobalConfig.load()) { return false; }

	// ドライバのソースから配置情報を拾って来る
	// Pick locating information from driver source code
	embd.setBaseDir(selfDir);
	if (!embd.loadLocationConfig(sGlobalConfig.getDriverConfigFileName().c_str())) { return false; }
	if (!embd.loadSourceBin(sGlobalConfig.getDriverImageFileName().c_str())) { return false; }

	// MML文法に関するデータを登録
	// Register grammer information
	registerMMlTokenTypeList();
	registerMMLExpressionForm();
	return true;
}

void showUsage() {
	fprintf(stderr, "qSPC version 1.9.0\n");
	fprintf(stderr, "--------------------------------------------------------\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  qspc -i <input mml 1> <input mml 2> <input mml 3>...\n\n");
	fprintf(stderr, "All options:\n");
	fprintf(stderr, "  -i : Input mml file(s)\n");
	fprintf(stderr, "  -v : Verbose mode\n");
	fprintf(stderr, "  -V : Very verbose mode\n");
}