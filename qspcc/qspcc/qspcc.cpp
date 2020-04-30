#include "stdafx.h"
#include <iostream>
#include "MMLCompiler.h"
#include <vector>
#include "GlobalConfig.h"
#include "Embedder.h"
#include "ROMEmbedder.h"
#include "spcfile/SPCExporter.h"
#include "CommandOptions.h"
#include "tester/testers.h"
#include "win32/pwd.h"
#include "MMLErrors.h"
#include "MMLUtility.h"

#define TEST_NEW_QL true

typedef std::vector<MMLCompiler*> PCompilerList;

static PCompilerList sCompilerList;
static GlobalConfig sGlobalConfig;
static bool globalSetup(Embedder& embd);
static void showUsage();
static void releaseAllCompilers(PCompilerList& inoutCompilerList);
static void addEffectTracks(PCompilerList& inCompilerList);
static unsigned int calcDriverMutableSize(PCompilerList& inCompilerList, const EmbedderConfig& eConfig, QuickLoadType qltype);
static unsigned int alignDriverQLSize(unsigned int originalVal);
static QuickLoadType checkSharableInstruments(PCompilerList& inCompilerList, const EmbedderConfig& eConfig);
static bool addCompiler(PCompilerList& outCompilerList, const std::string inputFile, const CommandOptionsSummary& opt_summary, const EmbedderConfig& embd_config);
static void generateAllImageBlobs(PCompilerList& compilerList, const CommandOptionsSummary& opt_summary, const EmbedderConfig& embd_config);
static void configureSPC(SPCExporter& exporter, MusicDocument* pDoc);

int _tmain(int argc, _TCHAR* argv[])
{
	Embedder embd;
	if (!globalSetup(embd)) {
		fputs("Failed to launch.\n", stderr);
		return -1;
	}

//doDocumentTest(&embd);
//doFrequencyTableTest();
//	doSPCExporterTest(); return 0;
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

	if (nFiles > sGlobalConfig.getMaxSongTracks()) {
		std::cerr << MMLErrors::getErrorString(MMLErrors::E_TOO_MANY_FILES) << std::endl;
		any_failed = true;
	} else {
		for (i = 0; i < nFiles; ++i) {
			if (!addCompiler(sCompilerList, infileList[i], opt_summary, embd.referConfig())) {
				any_failed = true;
			}
		}
	}
	/**
	MMLCompiler compiler;
	compiler.setVerboseLevel( opt_summary.verboseLevel );
	compiler.compile( opt_summary.inputFileList[0].c_str() );
	*/

	QuickLoadType ql_type = QL_NONE;
	const size_t nCompiledFiles = sCompilerList.size();
	if (any_failed) {
		embed_ready = false;
	} else {
		if (nCompiledFiles > 1 && opt_summary.effectSeqEnabled) {
			addEffectTracks(sCompilerList);
		}

		generateAllImageBlobs(sCompilerList, opt_summary, embd.referConfig());

		if (opt_summary.quickLoadEnabled || opt_summary.effectSeqEnabled) {
			ql_type = checkSharableInstruments(sCompilerList, embd.referConfig());
			if (QL_NONE == ql_type) {
				fprintf(stderr, "FATAL: Tracks must share instrument set in quick load/SE mode.\n");
				embed_ready = false;
			} else {
				fprintf(stderr, "Shared instruments confirmed.\n");
			}
		}
	}

	if (embed_ready) {
		const std::string kStrExporting = MMLErrors::getErrorString(MMLErrors::MSG_EXPORTING);

		auto driverMutableSize = 0;
		if (opt_summary.quickLoadEnabled) {
			driverMutableSize = alignDriverQLSize( calcDriverMutableSize(sCompilerList, embd.referConfig(), ql_type) );
			fprintf(stderr, "Quickload size=%d(%04xh) bytes\n", driverMutableSize, driverMutableSize);
		}

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
		r_embd.writeMetadataHeader(opt_summary.quickLoadEnabled, driverMutableSize);

		std::cerr << std::endl << "==================== Embedder Result ====================" << std::endl;
		std::cerr << MMLErrors::getErrorString(MMLErrors::MSG_EMBEDEXP) << std::endl;

		bool all_drivers_ok = true;

		for (i = 0; i < nCompiledFiles; ++i) {
			MMLCompiler* pCompiler = sCompilerList[i];
			MusicDocument* pDoc = pCompiler->referLastDocument();
			pDoc->writeQuickLoadSizeHeader(driverMutableSize);
			std::cerr << std::endl << pCompiler->getSourceFileName() << "  " << pDoc->getTitle() << std::endl;

			// ドライバイメージに書き込み
			// Write data to driver image
			embd.embed( opt_summary.verboseLevel > 0, opt_summary.debugExportEnabled,
				pDoc->referMusicHeaderSource(),
				pDoc->referFqTableBytesSource(),
				pDoc->referSequenceBytesSource(),
				pDoc->referInstDirBytesSource(),
				pDoc->referBRRDirBytesSource(),
				pDoc->referBRRBodyBytesSource(),
				pDoc->referSeqDirBytesSource()
				);

			const BinFile* driverBin = embd.referBin();
			r_embd.writeMetadata(i, pDoc->getTitle(), pDoc->getArtistName());
			r_embd.writeSoundDriverImage(i, driverBin);

			if (embd.checkAllSuccess()) {
				// 個別ファイル出力
				// Export single music file
				const std::string nameOnly = getFilenameOnly(pCompiler->getSourceFileName());
				if (opt_summary.outTypeFlags & OUTTYPE_SPC) {
					const std::string spcFN = nameOnly + ".spc";
					std::cerr << kStrExporting << " SPC file -> " << spcFN << std::endl;

					SPCExporter spcx;
					spcx.setProgramCounter(embd.referConfig().getProgramOrigin());
					configureSPC(spcx, pDoc);
					spcx.writeDriverImage(embd.referBin(), embd.referConfig().getProgramOrigin());
					spcx.exportToFile(spcFN.c_str());
				}

				// EX: BIN
				if (opt_summary.outTypeFlags & OUTTYPE_BIN) {
					if ( !(opt_summary.bin_is_half) ) {
						// 通常(フル)
						std::string binLoName = nameOnly + "-lo.bin";
						std::string binHiName = nameOnly + "-hi.bin";
						std::cerr << kStrExporting << " Raw driver -> " << binLoName << ", " << binHiName << std::endl;
						embd.exportToFile(binLoName.c_str(), binHiName.c_str());
					} else {
						// クイックロード用bin
						std::string binName = nameOnly + "-q.bin";
						std::cerr << kStrExporting << " Raw driver(quick) -> " << binName << std::endl;
						embd.exportToFileHalf(binName.c_str());
					}
				}
			} else {
				all_drivers_ok = false;
			}

			if (opt_summary.effectSeqEnabled) {
				// SEは最初のDocumentにまとめるのでループ脱出
				break;
			}
		}

		if (all_drivers_ok) {
			// EX: ROM
			if (opt_summary.outTypeFlags & OUTTYPE_ROM) {
				std::cerr << std::endl;
				std::string romFileName = opt_summary.compilationName + ".smc";

				std::cerr << kStrExporting << " Player ROM -> " << romFileName << std::endl;
				r_embd.updateChecksum();
				r_embd.exportToFile(romFileName.c_str());

				std::cerr << MMLErrors::getErrorString(MMLErrors::MSG_SUCCESSS) << std::endl;
			}
		} else {
			std::cerr << "**** ERROR " << MMLErrors::getErrorString(MMLErrors::E_EMB_CAPACITY_EX) << " ****" << std::endl;
		}
	}

	releaseAllCompilers(sCompilerList);
	return 0;
}

bool addCompiler(PCompilerList& outCompilerList, const std::string inputFile, const CommandOptionsSummary& opt_summary, const EmbedderConfig& embd_config) {
	const auto oldSize = outCompilerList.size();
	MMLCompiler* prev = nullptr;
	if (oldSize > 0) {
		prev = outCompilerList.at(oldSize - 1);
	}

	MMLCompiler* compiler = new MMLCompiler();
	compiler->setVerboseLevel(opt_summary.verboseLevel);
	if (!compiler->compile(inputFile, prev)) {
		return false;
	}

	outCompilerList.push_back(compiler);
	return true;
}

void generateAllImageBlobs(PCompilerList& compilerList, const CommandOptionsSummary& opt_summary, const EmbedderConfig& embd_config) {
	const size_t n = compilerList.size();
	for (size_t i = 0; i < n; ++i) {
		MMLCompiler* compiler = compilerList.at(i);

		MusicDocument* doc = compiler->referLastDocument();
		// バイナリ生成
		// Generate binary codes
		doc->generateSequenceImage(opt_summary.verboseLevel > 0, opt_summary.initialPauseEnabled);
		doc->generateInstrumentDataBinaries(
			embd_config.getProgramOrigin() + embd_config.getBRRBodyOrigin()
		);

		if (opt_summary.verboseLevel > 0) {
			doc->dumpSequenceBlob();
		}

	}
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

void addEffectTracks(PCompilerList& inCompilerList) {
	size_t n = inCompilerList.size();
	if (n < 2) {
		return;
	}

	MMLCompiler* pDestCmp = inCompilerList.at(0);
	MusicDocument* pDestDoc = pDestCmp->referLastDocument();

	for (size_t i = 1; i < n; ++i) {
		MMLCompiler* pSrc = inCompilerList.at(i);
		pDestDoc->addSETrackReference(pSrc->referLastDocument());
	}
}

unsigned int calcDriverMutableSize(PCompilerList& inCompilerList, const EmbedderConfig& eConfig, QuickLoadType qltype) {
	const size_t n = inCompilerList.size();
	if (n < 1) { return 0; }

	if (QL_FULL == qltype) {
		// Full shared の場合はBRRより前だけ再ロードで可
		return eConfig.getBRRBodyOrigin();
	}

	const MusicDocument* doc = inCompilerList[0]->referLastDocument();
	auto fp = doc->getBrrFixPoint();
	if (!fp) {
		return 0;
	}

	auto msize = (unsigned int)eConfig.getBRRBodyOrigin() + fp;
	return msize;
}

unsigned int alignDriverQLSize(unsigned int originalVal) {
	auto i = (originalVal + 255) / 256;
	return i * 256;
}

QuickLoadType checkSharableInstruments(PCompilerList& inCompilerList, const EmbedderConfig& eConfig) {
	const size_t n = inCompilerList.size();

	std::string firstName;
	auto all_same = true;
	for (size_t i = 0; i < n; ++i) {
		MMLCompiler* compiler = inCompilerList[i];

		if (0 == i) {
			firstName = compiler->referLastDocument()->getInstrumentSetName();
		}
		else {
			if (0 != compiler->referLastDocument()->getInstrumentSetName().compare(firstName)) {
				all_same = false;
				break;
			}
		}
	}

	if (all_same) {
		std::cerr << "[QuickLoad OK] Full shared BRR" << std::endl;
		// BRR入れ替えなしでクイックロード可
		return QL_FULL;
	}

	// NEW -----------------------------------
	// 新実装テスト
	if (n > 1) {
		auto fixpoint = inCompilerList[0]->referLastDocument()->getBrrFixPoint();
		if (fixpoint > 0) {
			std::cerr << MMLErrors::getErrorString(MMLErrors::MSG_FIXENABLED) << std::endl;
			fprintf(stderr, "Offset=%04xh\n", fixpoint);
		}

		for (size_t i = 1; i < n; ++i) {
			// 前半のロード時に書き換え=曲により変化可能なサイズを指定 ↓
			if (!inCompilerList[0]->checkCanShareDriver(*inCompilerList[i], fixpoint, eConfig)) {
				std::cerr << MMLErrors::getErrorString(MMLErrors::E_INST_BAD_FIXBRR) << ": " << inCompilerList[i]->getSourceFileName() << std::endl;
				return QL_NONE; // ◆全失敗 クイックロード不可
			}
		}

		std::cerr << "[QuickLoad OK] Partial shared BRR" << std::endl;
	}

	// BRR部分入れ替えでクイックロード可
	return QL_PART;
}

void configureSPC(SPCExporter& exporter, MusicDocument* pDoc) {
	exporter.setTitle(pDoc->getTitle());
	exporter.setComposer(pDoc->getArtistName());
	exporter.setDuration(pDoc->getRecommendedDuration());
	exporter.setComment(pDoc->getComment());
	exporter.setGameTitle(pDoc->getGameTitle());
	exporter.setDumperName(pDoc->getCoderName());
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
	fprintf(stderr, "qSPC MML compiler version 2.3.2\n");
	if (MMLErrors::getCurrentLanguage() == MSGLNG_JAPANESE) {
		fprintf(stderr, "この環境ではエラーメッセージが日本語で表示されます.\n");
	}
	fprintf(stderr, "------------------------------------------------------------------------\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  qspc [options] <input mml 1> <input mml 2> <input mml 3> <input mml 4>\n\n");
	fprintf(stderr, "All options:\n");
	fprintf(stderr, "  -bin: Generate driver image (raw) binary\n");
	fprintf(stderr, "  -rom: Generate SNES ROM file only\n");
	fprintf(stderr, "  -spc: Generate SPC file only\n");
	fprintf(stderr, "  -half-bin: Generate driver image (raw) binary for quick load\n");
	fprintf(stderr, "  -q  : Quick load mode (TRACKS MUST SHARE INSTRUMENTS)\n");
	fprintf(stderr, "  -e  : Enable sound effect sequences (TRACKS MUST SHARE INSTRUMENTS)\n");
	fprintf(stderr, "  -p  : Pause at boot\n");
	fprintf(stderr, "  -v  : Verbose mode\n");
	fprintf(stderr, "  -V  : Very verbose mode\n");
	fprintf(stderr, "  -o=filename to specify name of output ROM file\n");

}