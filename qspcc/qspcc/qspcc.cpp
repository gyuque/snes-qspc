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
#include "MMLUtility.h"

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
		const std::string kStrExporting = MMLErrors::getErrorString(MMLErrors::MSG_EXPORTING);

		if (opt_summary.verboseLevel > 0) {
			fprintf(stderr, "Embedding sequence data...\n");
			embd.dumpConfig();
		}

		// ==== コンパイル済みデータを埋め込み ====
		//           Embed generated data
		ROMEmbedder r_embd;
		r_embd.setVerboseLevel(opt_summary.verboseLevel);
		r_embd.setConfig(&embd.referConfig());
		r_embd.setBaseDir(getSelfDir());
		r_embd.loadTemplate(sGlobalConfig.getRomImageFileName(), sGlobalConfig.getRomMapFileName(), opt_summary.verboseLevel);
		r_embd.clearMetadataArea(sGlobalConfig.getMaxSongTracks());
		r_embd.writeMetadataHeader(opt_summary.quickLoadEnabled);

		std::cerr << std::endl << "==================== Embedder Result ====================" << std::endl;
		std::cerr << MMLErrors::getErrorString(MMLErrors::MSG_EMBEDEXP) << std::endl;

		const size_t nCompiledFiles = sCompilerList.size();
		for (i = 0; i < nCompiledFiles; ++i) {
			MMLCompiler* pCompiler = sCompilerList[i];
			MusicDocument* pDoc = pCompiler->referLastDocument();
			std::cerr << std::endl << pCompiler->getSourceFileName() << "  " << pDoc->getTitle() << std::endl;

			// ドライバイメージに書き込み
			// Write data to driver image
			embd.embed( opt_summary.verboseLevel > 0,
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

			// 個別ファイル出力
			// Export single music file
			const std::string nameOnly = getFilenameOnly(pCompiler->getSourceFileName());
			const std::string spcFN = nameOnly + ".spc";
			std::cerr << kStrExporting << " SPC file -> " << spcFN << std::endl;
		}

		if (embd.checkAllSuccess()) {
			std::cerr << std::endl;
			std::string romFileName = "qSPCPlay.smc";
			std::string binLoName = "drvimg-lo.bin";
			std::string binHiName = "drvimg-hi.bin";

			// EX: BIN
			std::cerr << kStrExporting << " Raw driver -> " << binLoName << ", " << binHiName << std::endl;
			embd.exportToFile(binLoName.c_str(), binHiName.c_str());

			// EX: ROM
			std::cerr << kStrExporting << " Player ROM -> " << romFileName << std::endl;
			r_embd.updateChecksum();
			r_embd.exportToFile(romFileName.c_str());

			std::cerr << MMLErrors::getErrorString(MMLErrors::MSG_SUCCESSS) << std::endl;
		} else {
			std::cerr << "**** " << MMLErrors::getErrorString(MMLErrors::E_EMB_CAPACITY_EX) << " ****" << std::endl;
		}
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
	// Generate binary codes
	doc->generateSequenceImage(opt_summary.verboseLevel > 0);
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
	fprintf(stderr, "  qspc [options] <input mml 1> <input mml 2> <input mml 3>...\n\n");
	fprintf(stderr, "All options:\n");
	fprintf(stderr, "  -bin: Generate driver image (raw) binary\n");
	fprintf(stderr, "  -rom: Generate SNES ROM file only\n");
	fprintf(stderr, "  -spc: Generate SPC file only\n");
	fprintf(stderr, "  -q  : Quick load mode (TRACKS MUST SHARE INSTRUMENTS)\n");
	fprintf(stderr, "  -v  : Verbose mode\n");
	fprintf(stderr, "  -V  : Very verbose mode\n");
	fprintf(stderr, "  -o=filename to specify name of output ROM file\n");
}