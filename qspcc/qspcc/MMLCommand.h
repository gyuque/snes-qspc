#ifndef MMLCOMMAND_H_INCLUDED
#define MMLCOMMAND_H_INCLUDED

#include <stdint.h>
#include "MMLTokenizer.h"
#include "IMMLError.h"
#include<vector>

#define MC_PUBLIC_DECL virtual void dump();
#define MC_PROTECTED_DECL virtual void pickFromExpr(const MMLExprStruct& sourceExpression);
#define MC_CTXCHG_DECL virtual void changeContext(struct _ParamsContext& inoutContext, int currentCommandIndex);
#define MC_CTXAPL_DECL virtual void applyContext(const struct _ParamsContext& inContext);
#define MC_GROUPING_DECL virtual void processGrouping(const struct _ParamsContext& inContext, MMLCommandPtrList& commandPtrList, int currentCommandIndex);

#define MC_GETCODE_DECL virtual uint8_t getCode(int index);
#define MC_REWRITETICKS_IMPL virtual void rewriteTicks(DriverTick t) { mTicks = t; }

class MMLCommand* createMMLCommandFromExpr(const MMLExprStruct& sourceExpression);

typedef std::vector<class MMLCommand*> MMLCommandPtrList;

// �R�}���h�C���f�b�N�X�p�X�^�b�N
typedef std::vector<int> IndexStack;


// �� MMLCommand: ���ۃN���X�Ŏ��ۂ̃R�}���h�ɂ͊��蓖�ĂȂ�
class MMLCommand
{
public:
	MMLCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLCommand();
	void assignTrack(int t);
	int getAssignedTrack() const { return mAssignedTrack; }

	void setByteCodePosition(int pos);
	int getByteCodePosition() const { return mByteCodePosition; }

	virtual int countCodeBytes() const { return mCodeBytes;  }
	virtual void rewriteTicks(DriverTick t) {}

	void raiseError(const char* relStr, int messageId);
	void setErrorReceiver(IMMLError* er) {
		mpErrorRecv = er;
	}

	void setVerbose(bool v);

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
	MC_CTXAPL_DECL
	MC_GROUPING_DECL
	MC_GETCODE_DECL
protected:
	IMMLError* mpErrorRecv;
	MMLTokenStruct mFirstSourceToken;
	bool mVerbose;

	int mByteCodePosition;
	int mCodeBytes;
	int mAssignedTrack;
	virtual void pickFromExpr(const MMLExprStruct& sourceExpression) = 0;
	static void pickSimpleInt(int& outVal, const MMLExprStruct& sourceExpression);
	static bool beginSection(IndexStack& indexStack, int currentIndex);
	static bool closeSection(IndexStack& indexStack, struct _ParamsContext& inoutContext, int* pBeginPosSave);
	static void applyContextDependentLength(NoteLength& outNL, const struct _ParamsContext& inContext);
	static bool makeGroupedCommandList(MMLCommandPtrList& outList, const MMLCommandPtrList& sourceList, int beginCommandPosition, int endCommandPosition, bool verbose = false);


	void setCodeBytes(int i) { mCodeBytes = i; }
};


// �� ParamsContext: �R���p�C�����ɃR���e�L�X�g�ˑ��̃p�����[�^���i�[����
// �Ⴆ�΃m�[�g�ɉ������w�肳��Ă��Ȃ��ꍇ�́A�R���e�L�X�g�ɐݒ肳�ꂽ�f�t�H���g�������K�p�����
typedef struct _ParamsContext {
	NoteLength defaultLength;
	int currentOctave;
	int q;
	int track;
	IndexStack localRepeatPointStack;
	IndexStack tupletPointStack;
	bool groupingNeeded;

	_ParamsContext() {
		// �����l: �t�_����4�������A�I�N�^�[�u3�A�Q�[�g�^�C���ő�
		defaultLength.N    = 4;
		defaultLength.dots = 0;
		currentOctave = 3;
		track = 0;
		groupingNeeded = false;
		q = MML_QMAX;
	}
} ParamsContext;

// �� ��������̃T�u�N���X�����ۂ̃R�}���h�Ɋ��蓖��

// ======= Set tempo t[n] =======
class MMLTempoCommand : public MMLCommand {
public:
	MMLTempoCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLTempoCommand();

	MC_PUBLIC_DECL
protected:
	int mSpecifiedValue;

	MC_PROTECTED_DECL
};

// ======= Q command(gate time) q[n] =======
class MMLQuantizeSetCommand : public MMLCommand {
public:
	MMLQuantizeSetCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLQuantizeSetCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	int mSpecifiedValue;

	MC_PROTECTED_DECL
};

// ======= Note [a-g] =======
class MMLNoteCommand : public MMLCommand {
public:
	MMLNoteCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLNoteCommand();

	MC_PUBLIC_DECL
	MC_CTXAPL_DECL
	MC_GETCODE_DECL
	MC_REWRITETICKS_IMPL
protected:
	int calcNoteBytes();

	int mRelPI; //���΁i�I�N�^�[�u���j�����C���f�b�N�X
	int mAbsPI; //��΁i�I�N�^�[�u�܂ށj�����C���f�b�N�X

	int mQ;
	int mVelo;
	NoteLength mNLen;
	DriverTick mTicks;

	MC_PROTECTED_DECL
};


// ======= Default length set l[n] =======
class MMLLengthSetCommand : public MMLCommand {
public:
	MMLLengthSetCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLLengthSetCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	NoteLength mNLen;

	MC_PROTECTED_DECL
};

// ======= Octave set o[n] =======
class MMLOctaveSetCommand : public MMLCommand {
public:
	MMLOctaveSetCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLOctaveSetCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	int mSpecifiedValue;

	MC_PROTECTED_DECL
};

// ======= Octave shift < > =======
class MMLOctaveShiftCommand : public MMLCommand {
public:
	MMLOctaveShiftCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLOctaveShiftCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	int mDelta;

	MC_PROTECTED_DECL
};


// ======= Local repeat start and end =======
// �������s�[�g /:n�`:/

class MMLLocalRepeatBeginCommand : public MMLCommand {
public:
	MMLLocalRepeatBeginCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLLocalRepeatBeginCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	int mNumToPlay;

	MC_PROTECTED_DECL
};

class MMLLocalRepeatEndCommand : public MMLCommand {
public:
	MMLLocalRepeatEndCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLLocalRepeatEndCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
	MC_GETCODE_DECL
	MC_GROUPING_DECL
protected:
	MMLCommandPtrList mInnerList;
	int mBeginCommandPosition;

	MC_PROTECTED_DECL
};

// ======= tuplet notes start and end =======
// �A�� {�`} �J�n
class MMLTupletBeginCommand : public MMLCommand {
public:
	MMLTupletBeginCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLTupletBeginCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	MC_PROTECTED_DECL
};

// �A�� {�`} �I��
class MMLTupletEndCommand : public MMLCommand {
public:
	MMLTupletEndCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLTupletEndCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
	MC_CTXAPL_DECL
	MC_GROUPING_DECL
protected:
	NoteLength mNLen;
	DriverTick mTicks;
	int mBeginCommandPosition;
	MMLCommandPtrList mInnerList;

	MC_PROTECTED_DECL
};

// ======= Terminator pseudo command =======
// �g���b�N�I�[�i�Z�~�R�����j�ɑΉ�����^���R�}���h

class MMLTerminatorPseudoCommand : public MMLCommand {
public:
	MMLTerminatorPseudoCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLTerminatorPseudoCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	MC_PROTECTED_DECL
};

#undef MC_PUBLIC_DECL
#undef MC_PROTECTED_DECL
#endif