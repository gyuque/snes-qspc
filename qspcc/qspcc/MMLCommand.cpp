#include "MMLCommand.h"
#include "MMLErrors.h"
#include "MMLUtility.h"
#include "MusicDocument.h"

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
	RepeatPointData r;
	r.pos = currentIndex;
	r.pExitCommand = nullptr;

	indexStack.push_back(r);
	return true;
}

uint8_t MMLCommand::getCode(int index) {
	return 0;
}

bool MMLCommand::closeSection(IndexStack& indexStack, ParamsContext& inoutContext, int* pBeginPosSave) {
	// スタックにリピート開始点が乗っていればポップ、なければエラー
	if (indexStack.size() > 0) {
		if (pBeginPosSave) {
			*pBeginPosSave = indexStack.at(indexStack.size() - 1).pos;
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
		outNL.dots += inContext.defaultLength.dots;
	}
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

// ■ 整数のパラメータを伴うコマンドの共通ベースクラス
MMLIntParamCommand::MMLIntParamCommand(const MMLExprStruct& sourceExpression, int defaultValue, const char* dumpName) : MMLCommand(sourceExpression) {
	mDumpName = dumpName;
	mSpecifiedValue = defaultValue;
	pickFromExpr(sourceExpression);
}

MMLIntParamCommand::~MMLIntParamCommand() {

}

void MMLIntParamCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	pickSimpleInt(mSpecifiedValue, sourceExpression);
}

void MMLIntParamCommand::dump() {
	fprintf(stderr, "(%s, %d)\n", mDumpName.c_str(), mSpecifiedValue);
}


/// factory -----------------------------------------------------------------------
MMLCommand* createMMLCommandFromExpr(const MMLExprStruct& sourceExpression) {
#define cmd_instance(klass) return new klass(sourceExpression); break;

	switch (sourceExpression.exprType) {
	case MX_TEMPO:
		cmd_instance(MMLTempoCommand);

	case MX_INSTCHG:
		cmd_instance(MMLInstChangeCommand);

	case MX_NSHIFT:
		cmd_instance(MMLNoteShiftCommand);

	case MX_NOTE:
		cmd_instance(MMLNoteCommand);

	case MX_ODEC:
	case MX_OINC:
		cmd_instance(MMLOctaveShiftCommand);

	case MX_LENSET:
		cmd_instance(MMLLengthSetCommand);

	case MX_OCTSET:
		cmd_instance(MMLOctaveSetCommand);

	case MX_QSET:
		cmd_instance(MMLQuantizeSetCommand);

	case MX_PANPOT:
		cmd_instance(MMLPanCommand);

	case MX_VELOSET:
		cmd_instance(MMLVelocityChangeCommand);

	case MX_LcREP_START:
		cmd_instance(MMLLocalRepeatBeginCommand);

	case MX_LcREP_END:
		cmd_instance(MMLLocalRepeatEndCommand);

	case MX_SLASH:
		cmd_instance(MMLSlashCommand);

	case MX_CMB_START:
		cmd_instance(MMLTupletBeginCommand);

	case MX_CMB_END:
		cmd_instance(MMLTupletEndCommand);

	case MX_TERM:
		cmd_instance(MMLTerminatorPseudoCommand);

	case MX_MACRODEF:
		cmd_instance(MMLMacroDefPseudoCommand);

	default:
		return NULL;
		break;
	}

	return NULL;
#undef cmd_instance
}

// ======= Set tempo t[n] =======
MMLTempoCommand::MMLTempoCommand(const MMLExprStruct& sourceExpression) : MMLIntParamCommand(sourceExpression, 120, "Tempo")
{
}

MMLTempoCommand::~MMLTempoCommand() {

}

void MMLTempoCommand::configureDocument(class MusicDocument* pDocument) {
	pDocument->setTempo(mSpecifiedValue);
}

// ======= Instrument change @[n] =======
MMLInstChangeCommand::MMLInstChangeCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression) {
	mSpecifiedValue = 0;
	pickFromExpr(sourceExpression);
	setCodeBytes(2);
}

MMLInstChangeCommand::~MMLInstChangeCommand() {

}

void MMLInstChangeCommand::dump() {
	fprintf(stderr, "(@ %d)\n", mSpecifiedValue);
}

void MMLInstChangeCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	pickSimpleInt(mSpecifiedValue, sourceExpression);
}

// ◆◆◆ バイトコード生成 ◆◆◆
// ::    @コマンド    ::
// 第1バイト: E0h
// 第2バイト: Inst index
uint8_t MMLInstChangeCommand::getCode(int index) {
	return (0 == index) ? 0xE0 : mSpecifiedValue;
}

// ======= ns command(Note Shift) ns[n] =======
MMLNoteShiftCommand::MMLNoteShiftCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression) {
	mSpecifiedValue = 0;
	pickFromExpr(sourceExpression);
}

MMLNoteShiftCommand::~MMLNoteShiftCommand() {

}

void MMLNoteShiftCommand::dump() {
	fprintf(stderr, "(ns %d)\n", mSpecifiedValue);
}

void MMLNoteShiftCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	pickSimpleInt(mSpecifiedValue, sourceExpression);
}

void MMLNoteShiftCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	inoutContext.nShift = mSpecifiedValue;
}

// ============= @p command (panpot) =============
MMLPanCommand::MMLPanCommand(const MMLExprStruct& sourceExpression) : MMLIntParamCommand(sourceExpression, PANPOT_MID, "Pan") {
	setCodeBytes(2);
}

MMLPanCommand::~MMLPanCommand() {
}

// ◆◆◆ バイトコード生成 ◆◆◆
// ::    @コマンド    ::
// 第1バイト: E1h
// 第2バイト: Position(0-8-16)
uint8_t MMLPanCommand::getCode(int index) {
	if (index == 0) {
		return 0xE1;
	} else {
		if (mSpecifiedValue < 0) { return 0; }
		if (mSpecifiedValue > PANPOT_MAX) { return PANPOT_MAX; }
		return mSpecifiedValue;
	}
}

// ======= v command(velocity change) v[n] =======
MMLVelocityChangeCommand::MMLVelocityChangeCommand(const MMLExprStruct& sourceExpression) : MMLIntParamCommand(sourceExpression, MML_VMAX, "v") {
	mSpecifiedValue = MML_VMAX;
	pickFromExpr(sourceExpression);
}

MMLVelocityChangeCommand::~MMLVelocityChangeCommand() {

}

void MMLVelocityChangeCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	inoutContext.v = mSpecifiedValue;
}


// ======= Q command(gate time) q[n] =======
MMLQuantizeSetCommand::MMLQuantizeSetCommand(const MMLExprStruct& sourceExpression) : MMLIntParamCommand(sourceExpression, MML_QMAX, "q")
{
}

MMLQuantizeSetCommand::~MMLQuantizeSetCommand() {
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
MMLOctaveSetCommand::MMLOctaveSetCommand(const MMLExprStruct& sourceExpression) : MMLIntParamCommand(sourceExpression, 4, "Oct")
{
}

MMLOctaveSetCommand::~MMLOctaveSetCommand() {
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

// ====== Global repeat or Repeat out ======
MMLSlashCommand::MMLSlashCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression) {
	mExitAddress = 0;
	pickFromExpr(sourceExpression);
	setCodeBytes(0);
}

MMLSlashCommand::~MMLSlashCommand() { }

void MMLSlashCommand::setExitAddress(unsigned int a) {
	mExitAddress = a;
}

void MMLSlashCommand::dump() {
	fprintf(stderr, "/\n");
}

void MMLSlashCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {}

void MMLSlashCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	// リピート区間の内か外かでコマンドの内容が変わる

	const size_t nest = inoutContext.localRepeatPointStack.size();
	if (nest > 0) {
		// リピート区間の中=ループ抜け
		inoutContext.localRepeatPointStack[nest - 1].pExitCommand = this;
		setCodeBytes(3); // この場合は3バイトのコマンドを生成
	} else {
		// リピート区間外=曲全体リピートの開始点を指示
		inoutContext.pGloalRepeatCmd = this;
	}
}

// DEh に続きジャンプ先アドレス(Lo, Hi)
uint8_t MMLSlashCommand::getCode(int index) {
	if (index == 0) {
		return 0xDE;
	} else if (index == 1) {
		return (mExitAddress & 0x00FF);
	}

	return ((mExitAddress >> 8) & 0x00FF);
}


// 曲全体のループを指示するために付加するコマンド（自動生成専用、MMLには現れない）
MMLFooterCommand::MMLFooterCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression) {
	mpJumpTargetCmd = nullptr;
	pickFromExpr(sourceExpression);
	setCodeBytes(0); // <-- 初期状態ではコードを生成しない
}

MMLFooterCommand::~MMLFooterCommand() {

}

void MMLFooterCommand::dump() {
	fprintf(stderr, "//\n");
}

void MMLFooterCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {}

void MMLFooterCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	if (inoutContext.pGloalRepeatCmd) {
		setCodeBytes(3);

		mpJumpTargetCmd = inoutContext.pGloalRepeatCmd;
		inoutContext.pGloalRepeatCmd = nullptr; // 次のトラックに移る前に無効化しておく
	}

	inoutContext.track += 1;
}

// DDh に続きジャンプ先アドレス(Lo, Hi)
uint8_t MMLFooterCommand::getCode(int index) {
	unsigned int jumpAddress = 0;
	if (mpJumpTargetCmd) {
		jumpAddress = mpJumpTargetCmd->getByteCodePosition();
	}

	if (index == 0) {
		return 0xDD;
	} else if (index == 1) {
		return (jumpAddress & 0x00FF);
	}

	return ((jumpAddress >> 8) & 0x00FF);
}

// ======= Local repeat start and end =======
// 部分リピート /:n～:/

// start
MMLLocalRepeatBeginCommand::MMLLocalRepeatBeginCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mNumToPlay = 2;
	pickFromExpr(sourceExpression);
	setCodeBytes(2);
}

MMLLocalRepeatBeginCommand::~MMLLocalRepeatBeginCommand() {
}

void MMLLocalRepeatBeginCommand::dump() {
	fprintf(stderr, "[vvv begin repeat (n=%d) vvv]\n", mNumToPlay);
}

void MMLLocalRepeatBeginCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	if (sourceExpression.tokenList.size() > 1) {
		pickSimpleInt(mNumToPlay, sourceExpression);
	}
}

void MMLLocalRepeatBeginCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
	if (inoutContext.localRepeatPointStack.size() > 0) {
		// 既にリピート区間の中にある（不正）
		raiseError(NULL, MMLErrors::E_NESTED_REPEAT);
		return;
	}
	beginSection(inoutContext.localRepeatPointStack, currentCommandIndex);
}

// リピートコマンド(始点): DFh に続き折り返し回数（演奏回数-1）

uint8_t MMLLocalRepeatBeginCommand::getCode(int index) {
	if (index == 0) {
		return 0xDF;
	}

	return mNumToPlay - 1;
}

// end
MMLLocalRepeatEndCommand::MMLLocalRepeatEndCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mpInnerSlashCommand = nullptr;
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
	// ループ抜けポイント '/' がある場合は対応するコマンドを保存しておく
	const size_t n = inoutContext.localRepeatPointStack.size();
	if (n > 0) {
		mpInnerSlashCommand = inoutContext.localRepeatPointStack[n - 1].pExitCommand;
	}


	closeSection(inoutContext.localRepeatPointStack, inoutContext, &mBeginCommandPosition);
}

void MMLLocalRepeatEndCommand::processGrouping(const struct _ParamsContext& inContext, MMLCommandPtrList& commandPtrList, int currentCommandIndex) {
	makeGroupedCommandList(mInnerList, commandPtrList, mBeginCommandPosition, currentCommandIndex, mVerbose);
}

void MMLLocalRepeatEndCommand::setByteCodePosition(int pos) {
	MMLCommand::setByteCodePosition(pos);

	if (mpInnerSlashCommand) {
		// 自身の次の位置を設定
		mpInnerSlashCommand->setExitAddress( pos + countCodeBytes() );
	}
}

// リピートコマンド(終点): EFh に続き戻り位置を Lo, Hi の順

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

// ======= Macro definition pseudo command =======

MMLMacroDefPseudoCommand::MMLMacroDefPseudoCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
}

MMLMacroDefPseudoCommand::~MMLMacroDefPseudoCommand() {

}

void MMLMacroDefPseudoCommand::dump() {
	fprintf(stderr, "(macro def)\n");
}

void MMLMacroDefPseudoCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {}

// ======= Terminator pseudo command =======
// トラック終端（セミコロン）に対応する疑似コマンド

MMLTerminatorPseudoCommand::MMLTerminatorPseudoCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
}

MMLTerminatorPseudoCommand::~MMLTerminatorPseudoCommand() {

}

void MMLTerminatorPseudoCommand::changeContext(ParamsContext& inoutContext, int currentCommandIndex) {
}

void MMLTerminatorPseudoCommand::dump() {
	fprintf(stderr, "(term)\n");
}

void MMLTerminatorPseudoCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {}