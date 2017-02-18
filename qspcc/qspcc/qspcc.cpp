#include "stdafx.h"
#include <iostream>
#include "MMLCompiler.h"
#include "GlobalConfig.h"
#include "Embedder.h"
#include "CommandOptions.h"

static GlobalConfig sGlobalConfig;
static void globalSetup(Embedder& embd);
static void showUsage();

int _tmain(int argc, _TCHAR* argv[])
{
	Embedder embd;
	globalSetup(embd);

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
	}

	return 0;
}

void globalSetup(Embedder& embd) {
	sGlobalConfig.load();

	// ドライバのソースから配置情報を拾って来る
	// Pick locating information from driver source code 
	embd.loadLocationConfig(sGlobalConfig.getDriverConfigFileName().c_str());

	// MML文法に関するデータを登録
	// Register grammer information
	registerMMlTokenTypeList();
	registerMMLExpressionForm();
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