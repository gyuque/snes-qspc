#include "MMLErrors.h"
#include <map>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


typedef std::map<int, std::string> ErrorMessageMap;
static ErrorMessageMap sMsgMap;
static bool ensureMessageRegistration();

static int sLang = MSGLNG_UNKNOWN;

MMLErrors::MMLErrors()
{
}

MMLErrors::~MMLErrors()
{
}

int MMLErrors::getCurrentLanguage() {
	if (sLang == MSGLNG_UNKNOWN) {
		static const WORD ja_jp = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
		const LANGID lid = ::GetThreadUILanguage();

		if (lid == ja_jp) {
			sLang = MSGLNG_JAPANESE;
		} else {
			sLang = MSGLNG_ENGLISH;
		}
	}

	return sLang;
}

std::string MMLErrors::getErrorString(int errorId) {
	ensureMessageRegistration();
	if (sMsgMap.find(errorId) != sMsgMap.end()) {
		return sMsgMap[errorId];
	}

	return "Unknown error";
}

// REGISTRATION ++++++++++++++++++++++++++++++++++++++++++++++

static void _er(int eid, const char* s) {
	sMsgMap[eid] = s;
}

static void registerErrorMessages_JP() {
	_er(MMLErrors::E_UNKNOWN_CHAR   , "�F���ł��Ȃ�����������܂��iMML�ŗ��p�ł��Ȃ��������Ȃ����A�܂��͗��p�ł��镶���ł����Ă��s�K�؂Ȉʒu�ɂȂ����m�F���Ă��������j");
	_er(MMLErrors::E_UNKNOWN_EXPR   , "�F���ł��Ȃ��g�[�N���񂪂���܂��i�R�}���h�𔺂킸�ɐ���������ȂǁA�������Ȃ��g�[�N���̕��ѕ��ɂȂ��Ă��Ȃ����m�F���Ă��������j");
	_er(MMLErrors::E_NESTED_TUP     , "�A���𑽏d�ɂ��邱�Ƃ͂ł��܂���");
	_er(MMLErrors::E_NESTED_REPEAT  , "��ԃ��s�[�g�𑽏d�ɂ��邱�Ƃ͂ł��܂���");
	_er(MMLErrors::E_RECURSIVE_MACRO, "�}�N�����ċA���Ă��܂�");
	_er(MMLErrors::E_NOTFOUND_MACRO , "��`����Ă��Ȃ��}�N�����g�����Ƃ��Ă��܂�");
	_er(MMLErrors::E_INST_NOTSET    , "���F�Z�b�g���w�肳��Ă��܂���");
	_er(MMLErrors::E_INST_NOTFOUND  , "���F�Z�b�g�����[�h�ł��܂���");
	_er(MMLErrors::E_INST_M_NOTFOUND, "���F�Z�b�g�̃}�j�t�F�X�g�����[�h�ł��܂���");
	_er(MMLErrors::E_INST_M_BAD     , "���F�Z�b�g�̃}�j�t�F�X�g�ɃG���[������܂�");
	_er(MMLErrors::E_INST_B_NOTFOUND, "�}�j�t�F�X�g���Ŏw�肳��Ă���BRR�t�@�C����������܂���");
	_er(MMLErrors::E_INST_MULTI_FIX , "BRR�̌Œ�|�C���g��1��̂ݎw��ł��܂�");
	_er(MMLErrors::E_INST_EXCEED_FIX, "�ϕ���BRR�f�[�^���Œ�|�C���g�𒴉߂��܂���");
	_er(MMLErrors::E_INST_BAD_FIXBRR, "�Œ蕔��BRR�f�[�^����v���Ă��܂���");
	_er(MMLErrors::E_EMB_CAPACITY_EX, "�f�[�^�Z�N�V�����̗e�ʂ�����܂���");
	_er(MMLErrors::E_TOO_MANY_FILES , "���̓t�@�C�����������܂�");


	_er(MMLErrors::MSG_COMPILING    , "���R���p�C����: ");
	_er(MMLErrors::MSG_EMBEDEXP     , "�e�ʂ𒴉߂���� ! �}�[�N���\������܂�");
	_er(MMLErrors::MSG_EXPORTING    , "�o�͂��Ă��܂�...");
	_er(MMLErrors::MSG_FIXENABLED   , "�f�[�^�Œ�|�C���g���ݒ肳��܂���");
	_er(MMLErrors::MSG_SUCCESSS     , "<����ɏI�����܂���>");
}

bool ensureMessageRegistration() {
	if (sMsgMap.size() > 0) {
		return false;
	}

	registerErrorMessages_JP();

	return true;
}

