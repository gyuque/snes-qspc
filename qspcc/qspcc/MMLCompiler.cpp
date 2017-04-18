#include "MMLCompiler.h"
#include "MMLPreprocessor.h"
#include "MMLErrors.h"
#include <iostream>

MMLCompiler::MMLCompiler() :mpExprBuilder(NULL), mVerboseLevel(0), mpLastDocument(NULL)
{

}


MMLCompiler::~MMLCompiler()
{
	clearCommands();
	releaseDocumentIf();

	if (mpExprBuilder) {
		delete mpExprBuilder;
		mpExprBuilder = NULL;
	}
}

void MMLCompiler::releaseDocumentIf() {
	if (mpLastDocument) {
		delete mpLastDocument;
		mpLastDocument = NULL;
	}
}

void MMLCompiler::setVerboseLevel(int lv) {
	mVerboseLevel = lv;
	mTokenizer.setVerboseLevel(lv);
}

bool MMLCompiler::shouldAbort() const {
	return mErrorList.size() > 0;
}

void MMLCompiler::clearCommands() {
	MMLCommandPtrList::iterator it;

	for (it = mCommandPtrList.begin(); it != mCommandPtrList.end(); it++) {
		MMLCommand* p = *it;
		delete p;

		*it = NULL;
	}

	mCommandPtrList.clear();
}

bool MMLCompiler::compile(std::string filename) {
	mCurrentFilename = filename;
	if (mpExprBuilder) { return false; }

	std::cerr << MMLErrors::getErrorString(MMLErrors::MSG_COMPILING) << filename << "..";


	mTokenizer.setErrorReceiver(this);
	if (!mTokenizer.loadFile(filename.c_str())) {
		//fprintf(stderr, "Failed to load: %s\n", filename.c_str());
		dumpAllErrors();
		return false;
	}

	if (0 == mVerboseLevel) {
		std::cerr << "." << std::endl;
	}

	MacroDictionary macro_dic;
	macro_dic.setVerboseLevel(mVerboseLevel);

	mpExprBuilder = new MMLExpressionBuilder(&mTokenizer);
	mpExprBuilder->setErrorReceiver(this);
	mpExprBuilder->setVerboseLevel(mVerboseLevel);
	if (!mpExprBuilder->buildExpressions(macro_dic)) {
		dumpAllErrors();
		return false;
	}

	if (mVerboseLevel > 0) {
		mpExprBuilder->dump();
	}

	// New document here.
	releaseDocumentIf();
	mpLastDocument = new MusicDocument();

	preprocess();
	const InstLoadResult ld_res = mpLastDocument->loadInstrumentSet(mVerboseLevel); // プリプロセスでinst setが指定されている筈（されていなければエラー）
	checkGlobalErrors(ld_res);
	if (shouldAbort()) { // ERROR CHECK -------------
		dumpAllErrors();
		return false;
	} // --------------------------------------------
	mpLastDocument->validateMetadata();


	generateCommands();
	if (mVerboseLevel > 0) {
		dumpAllCommands();
	}

	applyContextDependentParams();
	if (shouldAbort()) { // ERROR CHECK -------------
		dumpAllErrors();
		return false;
	} // --------------------------------------------

	if (mVerboseLevel > 0) {
		dumpAllCommands();
	}

	generateByteCodeTracks();

	return true;
}

void MMLCompiler::preprocess() {
	if (!mpExprBuilder || !mpLastDocument) {
		return;
	}

	const int n = mpExprBuilder->count();
	if (mVerboseLevel > 0) {
		fprintf(stderr, "Preprocessing expressions(len=%d)\n", n);
	}

	MMLPreprocessor pp(mpLastDocument);

	for (int i = 0; i < n; ++i) {
		MMLExprStruct* pX = mpExprBuilder->referAt(i);
		if (pX) {
			pp.processExpression(*pX);
		}
	}
}

void MMLCompiler::generateCommands() {
	if (!mpExprBuilder) {
		return;
	}

	const int n = mpExprBuilder->count();
	if (mVerboseLevel > 0) {
		fprintf(stderr, "Generating commands from expressions(len=%d)\n", n);
	}

	bool last_is_footer = false;
	for (int i = 0; i < n; ++i) {
		last_is_footer = false;
		MMLExprStruct* pX = mpExprBuilder->referAt(i);
		if (pX) {
			MMLCommand* pCmd = createMMLCommandFromExpr(*pX);
			if (pCmd) {
				pCmd->setVerbose(mVerboseLevel > 0);
				pCmd->setErrorReceiver(this);
				pCmd->applyDocumentGlobalConfiguration(mpLastDocument);
				mCommandPtrList.push_back(pCmd);
			}

			if (pX->exprType == MX_TERM) {
				// トラック終端の後に全体リピート用のコマンドを追加
				appendTrackFooter();
				last_is_footer = true;
			}
		}
	}

	if (!last_is_footer) {
		// 最後のトラックに終端がなかった場合はここでフッタ追加
		appendTrackFooter();
	}
}

void MMLCompiler::appendTrackFooter() {
	MMLExprStruct dmyExpr;
	dmyExpr.exprType = MX_TERM;
	MMLFooterCommand* cmd = new MMLFooterCommand(dmyExpr);
	mCommandPtrList.push_back(cmd);
}

void MMLCompiler::applyContextDependentParams() {
	const int n = (int)( mCommandPtrList.size() );
	int i;

	ParamsContext ctx;

	for (i = 0; i < n; ++i) {
		MMLCommand* pCmd = mCommandPtrList[i];
		if (pCmd) {
			// 現在のトラック番号を設定
			pCmd->assignTrack(ctx.track);

			// コンテキストを変更するコマンドであればここで書き換え（それ以外は空実装）
			pCmd->changeContext(ctx, i);

			// コンテキストに影響を受けるコマンドであればここで反映（それ以外は空実装）
			pCmd->applyContext(ctx);

			// グルーピング（コマンド側から要求されていれば実行）
			if (ctx.groupingNeeded) {
				pCmd->processGrouping(ctx, mCommandPtrList, i);
			}

			// ドキュメント全体に影響するコマンドはここで反映
			pCmd->configureDocument(mpLastDocument);

			ctx.groupingNeeded = false;
		}

		if (shouldAbort()) {
			break;
		}
	}
}

void MMLCompiler::generateByteCodeTracks() {
	const int numOfTracks = countTracks();
	if (mVerboseLevel > 0) {
		fprintf(stderr, "[  %d Track(s)  ]\n", numOfTracks);
	}

	for (int i = 0; i < numOfTracks; ++i) {
		MusicTrack* track = mpLastDocument->appendTrack();
		determineCommandAddress(i, track);
		generateATrack(i, track);
	}

	mpLastDocument->calcDataSize(NULL, mVerboseLevel > 0);
}

void MMLCompiler::determineCommandAddress(int trackIndex, MusicTrack* pTrack) {
	int address = 0;

	const int n = (int)(mCommandPtrList.size());
	for (int i = 0; i < n; ++i) {
		MMLCommand* pCmd = mCommandPtrList[i];
		if (pCmd->getAssignedTrack() == trackIndex) {
			// バイトコードの位置をコマンドオブジェクト内に記録
			pCmd->setByteCodePosition(address);

			const int blen = pCmd->countCodeBytes();
			address += blen;
		}
	}
}

void MMLCompiler::generateATrack(int trackIndex, MusicTrack* pTrack) {
	int bpos = 0;

	int maxt = -1;
	const int n = (int)(mCommandPtrList.size());
	for (int i = 0; i < n; ++i) {
		MMLCommand* pCmd = mCommandPtrList[i];
		if (pCmd->getAssignedTrack() == trackIndex) {

			const int blen = pCmd->countCodeBytes();

			if (blen > 0) {
				for (int k = 0; k < blen; ++k) {
					uint8_t bc = pCmd->getCode(k);
					const int newPosition = pTrack->size();

					pTrack->addByte(bc);
				}
			}
		}
	}
}

void MMLCompiler::dumpAllCommands() {
	const int n = (int)(mCommandPtrList.size());
	int i;

	fprintf(stderr, "||||||||||| %d commands |||||||||||\n", n);
	fprintf(stderr, " IDX |TRCK|L| COMMAND\n");
	fprintf(stderr, "-----+----+-+----------------------------------------\n");
	for (i = 0; i < n; ++i) {
		MMLCommand* pCmd = mCommandPtrList[i];
		if (pCmd) {
			fprintf(stderr, "%04d | T%d |%d| ", i, pCmd->getAssignedTrack(), pCmd->countCodeBytes());
			pCmd->dump();
		} else {
			fprintf(stderr, "(null)\n");
		}
	}
}

int MMLCompiler::countTracks() {
	int maxt = -1;
	const int n = (int)(mCommandPtrList.size());
	for (int i = 0; i < n; ++i) {
		const MMLCommand* const pCmd = mCommandPtrList[i];
		const int t = pCmd->getAssignedTrack();
		if (t > maxt) {
			maxt = t;
		}
	}

	return maxt + 1;
}

void MMLCompiler::checkGlobalErrors(InstLoadResult instLdResult) {
	switch (instLdResult) {
	case INSTLD_NOT_SET:
		raiseError(-1, -1, "", MMLErrors::getErrorString(MMLErrors::E_INST_NOTSET));
		break;

	case INSTLD_DIR_NOTFOUND:
		raiseError(-1, -1, "", MMLErrors::getErrorString(MMLErrors::E_INST_NOTFOUND));
		break;

	case INSTLD_MANIFEST_NOTFOUND:
		raiseError(-1, -1, "", MMLErrors::getErrorString(MMLErrors::E_INST_M_NOTFOUND));
		break;

	case INSTLD_BAD_MANIFEST:
		raiseError(-1, -1, "", MMLErrors::getErrorString(MMLErrors::E_INST_M_BAD));
		break;

	case INSTLD_BRR_NOTFOUND:
		raiseError(-1, -1, "", MMLErrors::getErrorString(MMLErrors::E_INST_B_NOTFOUND));
		break;

	default:
		break;
	}
}

void MMLCompiler::raiseError(int lineno, int charno, const char* relStr, const std::string& message) {
	MMLCompileError err;

	err.lineno = lineno;
	err.charno = charno;
	err.message = message;

	if (relStr) {
		err.relatedString = relStr;
	}

	mErrorList.push_back(err);
}

void MMLCompiler::dumpAllErrors() {
	const size_t n = mErrorList.size();
	for (size_t i = 0; i < n; ++i) {
		const MMLCompileError& err = mErrorList[i];
		std::cerr << mCurrentFilename << ": ";

		if (err.lineno >= 0) {
			std::cerr << "Line " << (err.lineno + 1) << ", column " << (err.charno + 1);
		}

		if (err.relatedString.size() > 0) {
			std::cerr << "  '" << err.relatedString << "'";
		}

		std::cerr << std::endl;
		std::cerr << err.message << std::endl;
	}
}
