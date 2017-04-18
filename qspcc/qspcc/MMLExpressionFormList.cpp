#include "MMLTokenizer.h"

static MMLExpFormList sExpFormList;
static void rx_(const char* name, MMLExpressionType type, const char* reLit);

// ■ MMLExpressionForm
// コマンドの書式を正規表現で定義する。入力MMLがここで指定したパターンに沿っているか自動的にチェックされる。
// トークンの型ごとに文字(letter)が割り当てられているので、letterの並びを正規表現で記述する。
// 例えばデフォルト音長指定コマンドは o → 整数 → 付点(optional) 
// パターンに合致したトークンの集合はExpressionとして纏められる。

// ↓ 新しいコマンドを追加する場合はここに定義

void registerMMLExpressionForm() {
	if (sExpFormList.size() > 0) { return; }

	rx_("QuantSet", MX_QSET   , "^QI");
	rx_("Tempo"   , MX_TEMPO  , "^TI");
	rx_("NShift"  , MX_NSHIFT , "^s[Ii]");
	rx_("OctSet"  , MX_OCTSET , "^OI");
	rx_("LenSet"  , MX_LENSET , "^LId?");
	rx_("Note"    , MX_NOTE   , "^NI?d?&?");
	rx_("OctInc"  , MX_OINC   , "^<");
	rx_("OctDec"  , MX_ODEC   , "^>");
	rx_("Term"    , MX_TERM   , "^;");
	rx_("VeloSet" , MX_VELOSET, "^vI");
	rx_("InstChg" , MX_INSTCHG, "^@I");
	rx_("Panpot"  , MX_PANPOT , "^pI");

	// combined commands(tuplet notes)
	rx_("CmbStart", MX_CMB_START, "^\\{");
	rx_("CmbEnd"  , MX_CMB_END  , "^\\}I?d?");

	// local repeat
	rx_("LRpStart", MX_LcREP_START, "^\\[I?");
	rx_("LRpEnd"  , MX_LcREP_END  , "^\\]");

	// global repeat or repeat out
	rx_("Slash"   , MX_SLASH, "^/");

	// #using
	rx_("Using"   , MX_USINGDECL, "^U\"");
	// #title
	rx_("Title"   , MX_TITLEDECL, "^`\"");
	// #artist
	rx_("Artist"  , MX_ARTISTDECL,"^A\"");
	// #octrev
	rx_("Octrev"  , MX_OCTREVDECL, "^8");

	// Macro
	rx_("MacroDef", MX_MACRODEF, "^M=[^;]*");
	rx_("MacroUse", MX_MACROUSE, "^M");
}

const MMLExpFormList& referExpressionFormList() {
	return sExpFormList;
}


void rx_(const char* name, MMLExpressionType type, const char* reLit) {
	MMLExpressionForm f(name, type, reLit);
	sExpFormList.push_back(f);
}
