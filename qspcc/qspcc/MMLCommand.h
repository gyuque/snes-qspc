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

// コマンドインデックス用スタック
typedef struct _RepeatPointData { int pos; class MMLSlashCommand* pExitCommand; } RepeatPointData;
typedef std::vector<RepeatPointData> IndexStack;


// ■ MMLCommand: 抽象クラスで実際のコマンドには割り当てない
class MMLCommand
{
public:
	MMLCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLCommand();
	void assignTrack(int t);
	int getAssignedTrack() const { return mAssignedTrack; }

	virtual void setByteCodePosition(int pos);
	int getByteCodePosition() const { return mByteCodePosition; }

	virtual int countCodeBytes() const { return mCodeBytes;  }
	virtual void rewriteTicks(DriverTick t) {}
	virtual void applyDocumentGlobalConfiguration(const class MusicDocument* pDocument) {}
	virtual void configureDocument(class MusicDocument* pDocument) {}

	void raiseError(const char* relStr, int messageId);
	void setErrorReceiver(IMMLError* er) {
		mpErrorRecv = er;
	}

	void setVerbose(bool v);
	virtual bool isNote() const { return false; }

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

// ■ 整数のパラメータを伴うコマンドの共通ベースクラス
class MMLIntParamCommand : public MMLCommand
{
public:
	MMLIntParamCommand(const MMLExprStruct& sourceExpression, int defaultValue, const char* dumpName);
	virtual ~MMLIntParamCommand();
	virtual void dump();
protected:
	virtual void pickFromExpr(const MMLExprStruct& sourceExpression);
	int mSpecifiedValue;
	std::string mDumpName;
};

// ■ ParamsContext: コンパイル時にコンテキスト依存のパラメータを格納する
// 例えばノートに音長が指定されていない場合は、コンテキストに設定されたデフォルト音長が適用される
typedef struct _ParamsContext {
	NoteLength defaultLength;
	int currentOctave;
	int q, v;
	int track;
	int nShift;
	int vShift;
	IndexStack localRepeatPointStack;
	IndexStack tupletPointStack;
	bool groupingNeeded;

	class MMLSlashCommand* pGloalRepeatCmd;

	_ParamsContext() {
		// 初期値: 付点無し4分音符、オクターブ3、ゲートタイム最大
		defaultLength.N    = 4;
		defaultLength.dots = 0;
		currentOctave = 3;
		nShift = 0;
		vShift = 0;
		track = 0;
		groupingNeeded = false;
		q = MML_QMAX;
		v = MML_VMAX;

		pGloalRepeatCmd = nullptr;
	}
} ParamsContext;

// ■ ここからのサブクラスを実際のコマンドに割り当て

// ======= Set tempo t[n] =======
class MMLTempoCommand : public MMLIntParamCommand {
public:
	MMLTempoCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLTempoCommand();
	virtual void configureDocument(class MusicDocument* pDocument);
};

// ======= Instrument change @[n] =======
class MMLInstChangeCommand : public MMLCommand {
public:
	MMLInstChangeCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLInstChangeCommand();

	MC_PUBLIC_DECL
	MC_GETCODE_DECL
protected:
	int mSpecifiedValue;
	MC_PROTECTED_DECL
};

// ======= ns command(Note Shift) ns[n] =======
class MMLNoteShiftCommand : public MMLCommand {
public:
	MMLNoteShiftCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLNoteShiftCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	int mSpecifiedValue;

	MC_PROTECTED_DECL
};

// ======= vs command(Velocity Shift) vs[n] =======
class MMLVelocityShiftCommand : public MMLCommand {
public:
	MMLVelocityShiftCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLVelocityShiftCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	int mSpecifiedValue;

	MC_PROTECTED_DECL
};

// ============= @p command (panpot) =============
class MMLPanCommand : public MMLIntParamCommand {
public:
	MMLPanCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLPanCommand();
	MC_GETCODE_DECL
};

// ======= v command(velocity change) v[n] =======
class MMLVelocityChangeCommand : public MMLIntParamCommand {
public:
	MMLVelocityChangeCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLVelocityChangeCommand();
	MC_CTXCHG_DECL
};

// ======= Q command(gate time) q[n] =======
class MMLQuantizeSetCommand : public MMLIntParamCommand {
public:
	MMLQuantizeSetCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLQuantizeSetCommand();
	MC_CTXCHG_DECL
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

	virtual bool isNote() const { return true; }
	static bool findTieToken(const MMLExprStruct& expr);
protected:
	int calcNoteBytes();

	bool mTie;
	int mRelPI; //相対（オクターブ内）音名インデックス
	int mAbsPI; //絶対（オクターブ含む）音名インデックス

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
class MMLOctaveSetCommand : public MMLIntParamCommand {
public:
	MMLOctaveSetCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLOctaveSetCommand();
	MC_CTXCHG_DECL
};

// ======= Octave shift < > =======
class MMLOctaveShiftCommand : public MMLCommand {
public:
	MMLOctaveShiftCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLOctaveShiftCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL

	virtual void applyDocumentGlobalConfiguration(const class MusicDocument* pDocument);
protected:
	int mReverseFactor;
	int mDelta;

	MC_PROTECTED_DECL
};

// ====== Global repeat or Repeat out ======
// 全体リピート または リピート抜けポイント
class MMLSlashCommand : public MMLCommand {
public:
	MMLSlashCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLSlashCommand();

	void setExitAddress(unsigned int a);

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
	MC_GETCODE_DECL
protected:
	MC_PROTECTED_DECL

	unsigned int mExitAddress;
};

// 曲全体のループを指示するために付加するコマンド（自動生成専用、MMLには現れない）
class MMLFooterCommand : public MMLCommand {
public:
	MMLFooterCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLFooterCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
	MC_GETCODE_DECL
protected:
	MC_PROTECTED_DECL

	MMLCommand* mpJumpTargetCmd;
};

// ======= Local repeat start and end =======
// 部分リピート /:n～:/

class MMLLocalRepeatBeginCommand : public MMLCommand {
public:
	MMLLocalRepeatBeginCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLLocalRepeatBeginCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
	MC_GETCODE_DECL
protected:
	int mNumToPlay;

	MC_PROTECTED_DECL
};

class MMLLocalRepeatEndCommand : public MMLCommand {
public:
	MMLLocalRepeatEndCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLLocalRepeatEndCommand();
	virtual void setByteCodePosition(int pos);

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
	MC_GETCODE_DECL
	MC_GROUPING_DECL
protected:
	MMLSlashCommand* mpInnerSlashCommand;
	MMLCommandPtrList mInnerList;
	int mBeginCommandPosition;

	MC_PROTECTED_DECL
};

// ======= tuplet notes start and end =======
// 連符 {～} 開始
class MMLTupletBeginCommand : public MMLCommand {
public:
	MMLTupletBeginCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLTupletBeginCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	MC_PROTECTED_DECL
};

// 連符 {～} 終了
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

	int countValidInnerCommands() const;

	MC_PROTECTED_DECL
};

// ======= Macro definition pseudo command =======
// マクロ宣言に対応する疑似コマンド

class MMLMacroDefPseudoCommand : public MMLCommand {
public:
	MMLMacroDefPseudoCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLMacroDefPseudoCommand();
	MC_PUBLIC_DECL
protected:
	MC_PROTECTED_DECL
};

// ======= Terminator pseudo command =======
// トラック終端（セミコロン）に対応する疑似コマンド

class MMLTerminatorPseudoCommand : public MMLCommand {
public:
	MMLTerminatorPseudoCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLTerminatorPseudoCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
protected:
	MC_PROTECTED_DECL
};

// ======= Stop BGM command =======
// BGMを止める(特殊SE用)

class MMLStopBGMCommand : public MMLCommand {
public:
	MMLStopBGMCommand(const MMLExprStruct& sourceExpression);
	virtual ~MMLStopBGMCommand();

	MC_PUBLIC_DECL
	MC_CTXCHG_DECL
	MC_GETCODE_DECL
protected:
	MC_PROTECTED_DECL
};


#undef MC_PUBLIC_DECL
#undef MC_PROTECTED_DECL
#endif