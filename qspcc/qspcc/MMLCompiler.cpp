#include "MMLCompiler.h"
#include "MMLPreprocessor.h"
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
	if (mpExprBuilder) { return false; }

	mTokenizer.setErrorReceiver(this);
	if (!mTokenizer.loadFile(filename.c_str())) {
		//fprintf(stderr, "Failed to load: %s\n", filename.c_str());
		dumpAllErrors();
		return false;
	}

	mpExprBuilder = new MMLExpressionBuilder(&mTokenizer);
	mpExprBuilder->setErrorReceiver(this);
	mpExprBuilder->setVerboseLevel(mVerboseLevel);
	if (!mpExprBuilder->buildExpressions()) {
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
	mpLastDocument->loadInstrumentSet(); // プリプロセスでinst setが指定されている筈（されていなければエラー）

	generateCommands();
	if (mVerboseLevel > 0) {
		dumpAllCommands();
	}

	applyContextDependentParams();
	if (shouldAbort()) {
		dumpAllErrors();
		return false;
	}

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
	for (int i = 0; i < n; ++i) {
		MMLExprStruct* pX = mpExprBuilder->referAt(i);
		if (pX) {
			MMLCommand* pCmd = createMMLCommandFromExpr(*pX);
			if (pCmd) {
				pCmd->setVerbose(mVerboseLevel > 0);
				pCmd->setErrorReceiver(this);
				mCommandPtrList.push_back(pCmd);
			}
		}
	}
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

			ctx.groupingNeeded = false;
		}

		if (shouldAbort()) {
			break;
		}
	}
}

void MMLCompiler::generateByteCodeTracks() {
	const int numOfTracks = countTracks();
	fprintf(stderr, "[  %d Track(s)  ]\n", numOfTracks);

	for (int i = 0; i < numOfTracks; ++i) {
		MusicTrack* track = mpLastDocument->appendTrack();
		generateATrack(i, track);
	}

	mpLastDocument->calcDataSize(NULL, true);
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

					// 最初のバイトコードの位置をコマンドオブジェクト内に記録
					if (k == 0) {
						pCmd->setByteCodePosition(newPosition);
					}
				}
			} else {
				// バイトコードを生成しないコマンドの場合も現在位置を記録（リピートなどのため）
				pCmd->setByteCodePosition( pTrack->size() );
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
		std::cerr << "At line " << (err.lineno + 1) << ", column " << (err.charno + 1);
		if (err.relatedString.size() > 0) {
			std::cerr << "  '" << err.relatedString << "'";
		}

		std::cerr << std::endl;
		std::cerr << err.message << std::endl;
	}
}
