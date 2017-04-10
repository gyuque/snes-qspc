#include "stdafx.h"
#include <iostream>
#include "MMLCompiler.h"
#include <vector>
#include "GlobalConfig.h"
#include "Embedder.h"
#include "ROMEmbedder.h"
#include "CommandOptions.h"
#include "tester/testers.h"
#include "win32/pwd.h"
#include "MMLErrors.h"

typedef std::vector<MMLCompiler*> PCompilerList;

static PCompilerList sCompilerList;
static GlobalConfig sGlobalConfig;
static bool globalSetup(Embedder& embd);
static void showUsage();
static void releaseAllCompilers(PCompilerList& inoutCompilerList);
static bool checkSingleInstruments(PCompilerList& inCompilerList);
static bool addCompiler(PCompilerList& outCompilerList, const std::string inputFile, const CommandOptionsSummary& opt_summary, const EmbedderConfig& embd_config);

int _tmain(int argc, _TCHAR* argv[])
{
	Embedder embd;
	if (!globalSetup(embd)) {
		fputs("Failed to launch.\n", stderr);
		return -1;
	}

//doDocumentTest(&embd);
//doFrequencyTableTest();
//return 0; //

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

	bool any_failed = false;
	bool embed_ready = true;
	size_t i;
	// ==== 各MMLファイルをコンパイル ====
	const StringArgList& infileList = opt_summary.inputFileList;
	const size_t nFiles = infileList.size();
	for (i = 0; i < nFiles; ++i) {
		if (!addCompiler(sCompilerList, infileList[i], opt_summary, embd.referConfig())) {
			any_failed = true;
		}
	}
	/**
	MMLCompiler compiler;
	compiler.setVerboseLevel( opt_summary.verboseLevel );
	compiler.compile( opt_summary.inputFileList[0].c_str() );
	*/

	if (any_failed) {
		embed_ready = false;
	} else {
		if (opt_summary.quickLoadEnabled) {
			if (!(checkSingleInstruments(sCompilerList))) {
				fprintf(stderr, "FATAL: Tracks must share instrument set in quick load mode.\n");
				embed_ready = false;
			} else {
				fprintf(stderr, "Quick load enabled.\n");
			}
		}
	}

	if (embed_ready) {
		if (opt_summary.verboseLevel > 0) {
			fprintf(stderr, "Embedding sequence data...\n");
			embd.dumpConfig();
		}

		// ==== コンパイル済みデータを埋め込み ====
		ROMEmbedder r_embd;
		r_embd.setConfig(&embd.referConfig());
		r_embd.setBaseDir(getSelfDir());
		r_embd.loadTemplate(sGlobalConfig.getRomImageFileName(), sGlobalConfig.getRomMapFileName());
		r_embd.clearMetadataArea(sGlobalConfig.getMaxSongTracks());
		r_embd.writeMetadataHeader(opt_summary.quickLoadEnabled);

		const size_t nCompiledFiles = sCompilerList.size();
		for (i = 0; i < nCompiledFiles; ++i) {
			MMLCompiler* pCompiler = sCompilerList[i];
			MusicDocument* pDoc = pCompiler->referLastDocument();

			// ドライバイメージに書き込み
			embd.embed(
				pDoc->referMusicHeaderSource(),
				pDoc->referFqTableBytesSource(),
				pDoc->referSequenceBytesSource(),
				pDoc->referInstDirBytesSource(),
				pDoc->referBRRDirBytesSource(),
				pDoc->referBRRBodyBytesSource()
				);

			const BinFile* driverBin = embd.referBin();
			r_embd.writeMetadata(i, pDoc->getTitle(), pDoc->getArtistName());
			r_embd.writeSoundDriverImage(i, driverBin);
		}

		embd.exportToFile("drvimg-lo.bin", "drvimg-hi.bin");
		r_embd.exportToFile("output-test.smc");
	}

	releaseAllCompilers(sCompilerList);
	return 0;
}

bool addCompiler(PCompilerList& outCompilerList, const std::string inputFile, const CommandOptionsSummary& opt_summary, const EmbedderConfig& embd_config) {
	MMLCompiler* compiler = new MMLCompiler();
	compiler->setVerboseLevel(opt_summary.verboseLevel);
	if (!compiler->compile(inputFile)) {
		return false;
	}

	MusicDocument* doc = compiler->referLastDocument();
	// バイナリ生成
	doc->generateSequenceImage();
	doc->generateInstrumentDataBinaries(
		embd_config.getProgramOrigin() + embd_config.getBRRBodyOrigin()
		);

	if (opt_summary.verboseLevel > 0) {
		doc->dumpSequenceBlob();
	}

	outCompilerList.push_back(compiler);
	return true;
}

void releaseAllCompilers(PCompilerList& inoutCompilerList) {
	const size_t n = inoutCompilerList.size();
	for (size_t i = 0; i < n; ++i) {
		if (inoutCompilerList[i]) {
			delete inoutCompilerList[i];
		}
	}

	inoutCompilerList.clear();
}

bool checkSingleInstruments(PCompilerList& inCompilerList) {
	bool same = true;
	std::string firstName;

	const size_t n = inCompilerList.size();
	for (size_t i = 0; i < n; ++i) {
		MMLCompiler* compiler = inCompilerList[i];

		if (0 == i) {
			firstName = compiler->referLastDocument()->getInstrumentSetName();
		} else {
			if (0 != compiler->referLastDocument()->getInstrumentSetName().compare(firstName)) {
				return false;
			}
		}
	}

	return true;
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
	if (MMLErrors::getCurrentLanguage() == MSGLNG_JAPANESE) {
		fprintf(stderr, "この環境ではエラーメッセージが日本語で表示されます.\n");
	}
	fprintf(stderr, "------------------------------------------------------------------------\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  qspc [options] -i <input mml 1> -i <input mml 2> -i <input mml 3>...\n");
	fprintf(stderr, "    or simply\n");
	fprintf(stderr, "  qspc <input mml 1> <input mml 2> <input mml 3>...\n\n");
	fprintf(stderr, "All options:\n");
	fprintf(stderr, "  -i  : Input mml file\n");
	fprintf(stderr, "  -rom: Generate SNES ROM file only\n");
	fprintf(stderr, "  -spc: Generate SPC file only\n");
	fprintf(stderr, "  -q  : Quick load mode (TRACKS MUST SHARE INSTRUMENTS)\n");
	fprintf(stderr, "  -v  : Verbose mode\n");
	fprintf(stderr, "  -V  : Very verbose mode\n");
}