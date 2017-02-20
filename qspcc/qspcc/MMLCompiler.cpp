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
	mpLastDocument->loadInstrumentSet(); // �v���v���Z�X��inst set���w�肳��Ă��锤�i����Ă��Ȃ���΃G���[�j

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
			// ���݂̃g���b�N�ԍ���ݒ�
			pCmd->assignTrack(ctx.track);

			// �R���e�L�X�g��ύX����R�}���h�ł���΂����ŏ��������i����ȊO�͋�����j
			pCmd->changeContext(ctx, i);

			// �R���e�L�X�g�ɉe�����󂯂�R�}���h�ł���΂����Ŕ��f�i����ȊO�͋�����j
			pCmd->applyContext(ctx);

			// �O���[�s���O�i�R�}���h������v������Ă���Ύ��s�j
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

					// �ŏ��̃o�C�g�R�[�h�̈ʒu���R�}���h�I�u�W�F�N�g���ɋL�^
					if (k == 0) {
						pCmd->setByteCodePosition(newPosition);
					}
				}
			} else {
				// �o�C�g�R�[�h�𐶐����Ȃ��R�}���h�̏ꍇ�����݈ʒu���L�^�i���s�[�g�Ȃǂ̂��߁j
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
