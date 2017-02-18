#include "MMLCommand.h"
#include "MMLErrors.h"
#include "MMLUtility.h"

MMLCommand::MMLCommand(const MMLExprStruct& sourceExpression) : mCodeBytes(0), mVerbose(false)
{
	mpErrorRecv = NULL;
	mAssignedTrack = 0;
	mByteCodePosition = -1;

	if (sourceExpression.tokenList.size() > 0) {
		mFirstSourceToken = sourceExpression.tokenList.at(0);
	}
}

MMLCommand::~MMLCommand()
{
}

void MMLCommand::setVerbose(bool v) {
	mVerbose = v;
}

void MMLCommand::raiseError(const char* relStr, int messageId) {
	if (mpErrorRecv) {
		const int lineno = mFirstSourceToken.lineNo;
		const int charno = mFirstSourceToken.colNo;
		mpErrorRecv->raiseError(lineno, charno, relStr, MMLErrors::getErrorString( messageId ) );
	}
}

void MMLCommand::setByteCodePosition(int pos) {
	mByteCodePosition = pos;
}

void MMLCommand::assignTrack(int t) {
	mAssignedTrack = t;
}

void MMLCommand::dump() {
	fprintf(stderr, "<abstract command>\n");
}

void MMLCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	// nothing
}

void MMLCommand::applyContext(const ParamsContext& inContext) {
	// nothing
}

void MMLCommand::processGrouping(const struct _ParamsContext& inContext, MMLCommandPtrList& commandPtrList, int currentCommandIndex) {
	// nothing
}

bool MMLCommand::beginSection(IndexStack& indexStack, int currentIndex) {
	indexStack.push_back(currentIndex);
	return true;
}

uint8_t MMLCommand::getCode(int index) {
	return 0;
}

bool MMLCommand::closeSection(IndexStack& indexStack, ParamsContext& inoutContext, int* pBeginPosSave) {
	// スタックにリピート開始点が乗っていればポップ、なければエラー
	if (indexStack.size() > 0) {
		if (pBeginPosSave) {
			*pBeginPosSave = indexStack.at(indexStack.size() - 1);
		}

		indexStack.pop_back();
		inoutContext.groupingNeeded = true; // グルーピング実行を要求
		return true;
	}

	return false;
}

void MMLCommand::applyContextDependentLength(NoteLength& outNL, const ParamsContext& inContext) {
	// Length
	if (0 == outNL.N) {
		outNL.N = inContext.defaultLength.N;
	}

	outNL.dots += inContext.defaultLength.dots;
}

bool MMLCommand::makeGroupedCommandList(MMLCommandPtrList& outList, const MMLCommandPtrList& sourceList, int beginCommandPosition, int endCommandPosition, bool verbose) {
	const int nInner = (endCommandPosition - 1) - beginCommandPosition;
	if (nInner < 1) {
		return false;
	}

	if (verbose) {
		fprintf(stderr, " ** grouping repeated commands\n");
		fprintf(stderr, " ** num of inner commands=%d\n", nInner);
	}

	for (int i = 0; i < nInner; ++i) {
		const int cmdIndex = beginCommandPosition + 1 + i;
		MMLCommand* p = sourceList[cmdIndex];
		if (p) {
			outList.push_back(p);
			if (verbose) {
				fprintf(stderr, "    ");
				p->dump();
			}
		}
	}


	return true;
}

/// local utility

char firstTokChar(const MMLExprStruct& sourceExpression) {
	const MMLTokenList& tl = sourceExpression.tokenList;
	if (tl.size() > 0) {
		const std::string& s = tl[0].rawStr;
		if (s.size() > 0) {
			return s.at(0);
		}
	}

	return 0;
}

int tokIntAt(const MMLExprStruct& sourceExpression, int pos) {
	const MMLTokenList& tl = sourceExpression.tokenList;
	if ( (int)(tl.size()) > pos ) {
		return tl[pos].intVal;
	}

	return 0;
}

void MMLCommand::pickSimpleInt(int& outVal, const MMLExprStruct& sourceExpression) {
	// サブクラス用ユーティリティメソッド
	// コマンド名+整数 という書式の場合に（コマンド名の次に）指定された値を取得する
	outVal = tokIntAt(sourceExpression, 1);
}

/// factory -----------------------------------------------------------------------
MMLCommand* createMMLCommandFromExpr(const MMLExprStruct& sourceExpression) {
	switch (sourceExpression.exprType) {
	case MX_TEMPO:
		return new MMLTempoCommand(sourceExpression);
		break;

	case MX_NOTE:
		return new MMLNoteCommand(sourceExpression);
		break;

	case MX_ODEC:
	case MX_OINC:
		return new MMLOctaveShiftCommand(sourceExpression);
		break;

	case MX_LENSET:
		return new MMLLengthSetCommand(sourceExpression);
		break;

	case MX_OCTSET:
		return new MMLOctaveSetCommand(sourceExpression);
		break;

	case MX_QSET:
		return new MMLQuantizeSetCommand(sourceExpression);
		break;

	case MX_LcREP_START:
		return new MMLLocalRepeatBeginCommand(sourceExpression);
		break;

	case MX_LcREP_END:
		return new MMLLocalRepeatEndCommand(sourceExpression);
		break;

	case MX_CMB_START:
		return new MMLTupletBeginCommand(sourceExpression);
		break;

	case MX_CMB_END:
		return new MMLTupletEndCommand(sourceExpression);
		break;

	case MX_TERM:
		return new MMLTerminatorPseudoCommand(sourceExpression);
		break;

	default:
		return NULL;
		break;
	}

	return NULL;
}

// ======= Set tempo t[n] =======
MMLTempoCommand::MMLTempoCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mSpecifiedValue = 120;
	pickFromExpr(sourceExpression);
}

MMLTempoCommand::~MMLTempoCommand() {

}

void MMLTempoCommand::dump() {
	fprintf(stderr, "(Tempo=%d)\n", mSpecifiedValue);
}

void MMLTempoCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	pickSimpleInt(mSpecifiedValue, sourceExpression);
}

// ======= Q command(gate time) q[n] =======
MMLQuantizeSetCommand::MMLQuantizeSetCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mSpecifiedValue = MML_QMAX;
	pickFromExpr(sourceExpression);
}

MMLQuantizeSetCommand::~MMLQuantizeSetCommand() {

}

void MMLQuantizeSetCommand::dump() {
	fprintf(stderr, "(q %d)\n", mSpecifiedValue);
}

void MMLQuantizeSetCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	pickSimpleInt(mSpecifiedValue, sourceExpression);
}

void MMLQuantizeSetCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	inoutContext.q = mSpecifiedValue;
}



// ======= Default length set l[n] =======
MMLLengthSetCommand::MMLLengthSetCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	pickFromExpr(sourceExpression);
}

MMLLengthSetCommand::~MMLLengthSetCommand() {

}

void MMLLengthSetCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	mNLen = calcNoteLengthFromTokens(sourceExpression.tokenList, 1);
}

void MMLLengthSetCommand::dump() {
	fprintf(stderr, "(L %d+%d)\n", mNLen.N, mNLen.dots);
}

void MMLLengthSetCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	inoutContext.defaultLength = mNLen;
}

// ======= Octave set o[n] =======
MMLOctaveSetCommand::MMLOctaveSetCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mSpecifiedValue = 4;
	pickFromExpr(sourceExpression);
}

MMLOctaveSetCommand::~MMLOctaveSetCommand() {

}

void MMLOctaveSetCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	pickSimpleInt(mSpecifiedValue, sourceExpression);
}

void MMLOctaveSetCommand::dump() {
	fprintf(stderr, "(o %d)\n", mSpecifiedValue);
}

void MMLOctaveSetCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	inoutContext.currentOctave = mSpecifiedValue;
}


// ======= Octave shift < > =======
MMLOctaveShiftCommand::MMLOctaveShiftCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mDelta = 1;
	pickFromExpr(sourceExpression);
}

MMLOctaveShiftCommand::~MMLOctaveShiftCommand()
{

}

void MMLOctaveShiftCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	char k = firstTokChar(sourceExpression);
	mDelta = (k == '<') ? 1 : -1;
}

void MMLOctaveShiftCommand::dump() {
	fprintf(stderr, "<OCT %c>\n", (mDelta > 0) ? '+' : '-');
}

void MMLOctaveShiftCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	inoutContext.currentOctave += mDelta;
}

// ======= Local repeat start and end =======
// 部分リピート /:n～:/

// start
MMLLocalRepeatBeginCommand::MMLLocalRepeatBeginCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mNumToPlay = 2;
	pickFromExpr(sourceExpression);
}

MMLLocalRepeatBeginCommand::~MMLLocalRepeatBeginCommand() {
}

void MMLLocalRepeatBeginCommand::dump() {
	fprintf(stderr, "[vvv begin repeat (n=%d) vvv]\n", mNumToPlay);
}

void MMLLocalRepeatBeginCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	pickSimpleInt(mNumToPlay, sourceExpression);
}

void MMLLocalRepeatBeginCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	if (inoutContext.localRepeatPointStack.size() > 0) {
		// 既にリピート区間の中にある（不正）
		raiseError(NULL, MMLErrors::E_NESTED_REPEAT);
		return;
	}
	beginSection(inoutContext.localRepeatPointStack, currentCommandIndex);
}

// end
MMLLocalRepeatEndCommand::MMLLocalRepeatEndCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mBeginCommandPosition = -1;
	pickFromExpr(sourceExpression);
	setCodeBytes(3);
}

MMLLocalRepeatEndCommand::~MMLLocalRepeatEndCommand() {

}

void MMLLocalRepeatEndCommand::dump() {
	fprintf(stderr, "[^^^ end repeat (begin pos=%d) ^^^]\n", mBeginCommandPosition);
}

void MMLLocalRepeatEndCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
}

void MMLLocalRepeatEndCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	closeSection(inoutContext.localRepeatPointStack, inoutContext, &mBeginCommandPosition);
}

void MMLLocalRepeatEndCommand::processGrouping(const struct _ParamsContext& inContext, MMLCommandPtrList& commandPtrList, int currentCommandIndex) {
	makeGroupedCommandList(mInnerList, commandPtrList, mBeginCommandPosition, currentCommandIndex, mVerbose);
}

// リピートコマンド: EFh に続き戻り位置を Lo, Hi の順

uint8_t MMLLocalRepeatEndCommand::getCode(int index) {
	int bpos = 0;
	if (mInnerList.size() > 0) {
		bpos = mInnerList[0]->getByteCodePosition();
		if (bpos < 0) {
			return 0;
		}
	}

	switch (index)
	{
	case 1:  // 2nd(PosLo)
		return bpos & 0xFF;
		break;

	case 2:  // 2nd(PosHi)
		return (bpos >> 8) & 0xFF;
		break;

	default: // 1st(Command, EFh)
		return 0xEF;
		break;
	}

	return 0;
}


// ======= tuplet notes start and end =======
// 連符 {～} 開始
MMLTupletBeginCommand::MMLTupletBeginCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	pickFromExpr(sourceExpression);
}

MMLTupletBeginCommand::~MMLTupletBeginCommand() {

}

void MMLTupletBeginCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {}

void MMLTupletBeginCommand::dump() {
	fprintf(stderr, "[vv begin tuplet vv]\n");
}

void MMLTupletBeginCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	if (inoutContext.tupletPointStack.size() > 0) {
		//すでに連符の中にある（不正）
		raiseError(NULL, MMLErrors::E_NESTED_TUP);
		return;
	}

	beginSection(inoutContext.tupletPointStack, currentCommandIndex);
}

// 連符 {～} 終了
MMLTupletEndCommand::MMLTupletEndCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mBeginCommandPosition = -1;
	pickFromExpr(sourceExpression);
}

MMLTupletEndCommand::~MMLTupletEndCommand() {

}

void MMLTupletEndCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	mNLen = calcNoteLengthFromTokens(sourceExpression.tokenList, 1);
}

void MMLTupletEndCommand::dump() {
	fprintf(stderr, "[^^ end tuplet (begin pos=%d) L=%d+%d t=%d ^^]\n", mBeginCommandPosition, mNLen.N, mNLen.dots, mTicks);
}

void MMLTupletEndCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	closeSection(inoutContext.tupletPointStack, inoutContext, &mBeginCommandPosition);
}

void MMLTupletEndCommand::applyContext(const ParamsContext& inContext) {
	applyContextDependentLength(mNLen, inContext);
	mTicks = calcTickCount(mNLen);
}

void MMLTupletEndCommand::processGrouping(const struct _ParamsContext& inContext, MMLCommandPtrList& commandPtrList, int currentCommandIndex) {
	const bool success = makeGroupedCommandList(mInnerList, commandPtrList, mBeginCommandPosition, currentCommandIndex, mVerbose);

	if (success) {
		const int n = (int)mInnerList.size();
		DriverTick tupTick = mTicks / n;
		if (mVerbose) {
			fprintf(stderr, "  tuplet ticks=%d\n", tupTick);
		}

		// rewrite
		for (int i = 0; i < n; ++i) {
			MMLCommand* p = mInnerList[i];
			p->rewriteTicks(tupTick);
		}
	}
}

// ======= Terminator pseudo command =======
// トラック終端（セミコロン）に対応する疑似コマンド

MMLTerminatorPseudoCommand::MMLTerminatorPseudoCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
}

MMLTerminatorPseudoCommand::~MMLTerminatorPseudoCommand() {

}

void MMLTerminatorPseudoCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	inoutContext.track += 1;
}

void MMLTerminatorPseudoCommand::dump() {
	fprintf(stderr, "(term)\n");
}

void MMLTerminatorPseudoCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {}