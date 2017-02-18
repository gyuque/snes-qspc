#include "MMLCommand.h"
#include "MMLUtility.h"


// ======= Note [a-g] =======
MMLNoteCommand::MMLNoteCommand(const MMLExprStruct& sourceExpression) : MMLCommand(sourceExpression)
{
	mQ = MML_QMAX;
	mVelo = MML_VMAX;
	mAbsPI = mRelPI = 0;
	mTicks = 0;
	pickFromExpr(sourceExpression);

	// バイトコード長をここで決定（休符か音符かで変わるので注意）
	setCodeBytes(calcNoteBytes());
}

MMLNoteCommand::~MMLNoteCommand()
{

}

void MMLNoteCommand::pickFromExpr(const MMLExprStruct& sourceExpression) {
	const MMLTokenList& tl = sourceExpression.tokenList;
	if (tl.size() > 0) {
		mNLen = calcNoteLengthFromTokens(tl, 1);

		mRelPI = fromKeyNameToIndex(tl[0].rawStr);
		// fprintf(stderr, "[%s %d]", tl[0].rawStr.c_str(), mRelPI);
	}
}

void MMLNoteCommand::dump() {
	fprintf(stderr, "[NOTE I=%d,%d L=%d+%d t=%d", mRelPI, mAbsPI, mNLen.N, mNLen.dots, mTicks);
	if (mNLen.tuplet > 1) {
		fprintf(stderr, " (%d)", mNLen.tuplet);
	}
	fprintf(stderr, "]\n");
}

void MMLNoteCommand::applyContext(const ParamsContext& inContext) {
	applyContextDependentLength(mNLen, inContext);
	mTicks = calcTickCount(mNLen);

	// Octave
	if (mRelPI == KEY_NAME_REST) {
		mAbsPI = mRelPI;
	} else {
		mAbsPI = mRelPI + 12 * inContext.currentOctave;
	}
	// Q(gate time)
	mQ = inContext.q;
}

int MMLNoteCommand::calcNoteBytes() {
	if (mRelPI == KEY_NAME_REST) {
		return 2; //休符はベロシティ、ゲートタイム不要
	}

	return 3;
}

// ◆◆◆ バイトコード生成 ◆◆◆
// ::    発声(Note)コマンド    ::
// 第1バイト: 80h-C7h（音程）
// 第2バイト: 01h-7Fh（音長 ticks）
// 第3バイト: 00h-7Fh（休符なら省略 Q 3bits | Velo 4bits）

uint8_t MMLNoteCommand::getCode(int index) {
	int k;

	switch (index)
	{
	case 1: //2nd
		return generateCompressedTicks(mTicks);
		break;

	case 2: //3rd (a-g only)
		return generateQVbits(mQ, mVelo);
		break;

	default: // 1st

		// 80h -> O1-C
		// :
		// :
		// C7h -> O6-B
		// - - - - - -
		// C9h -> Rest

		if (mRelPI == KEY_NAME_REST) {
			return 0xC9;
		}

		return 0x80 + mAbsPI;
		break;
	}

	return 0;
}
