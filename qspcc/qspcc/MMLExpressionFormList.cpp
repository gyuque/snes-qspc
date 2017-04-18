#include "MMLTokenizer.h"

static MMLExpFormList sExpFormList;
static void rx_(const char* name, MMLExpressionType type, const char* reLit);

// �� MMLExpressionForm
// �R�}���h�̏����𐳋K�\���Œ�`����B����MML�������Ŏw�肵���p�^�[���ɉ����Ă��邩�����I�Ƀ`�F�b�N�����B
// �g�[�N���̌^���Ƃɕ���(letter)�����蓖�Ă��Ă���̂ŁAletter�̕��т𐳋K�\���ŋL�q����B
// �Ⴆ�΃f�t�H���g�����w��R�}���h�� o �� ���� �� �t�_(optional) 
// �p�^�[���ɍ��v�����g�[�N���̏W����Expression�Ƃ��ēZ�߂���B

// �� �V�����R�}���h��ǉ�����ꍇ�͂����ɒ�`

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
