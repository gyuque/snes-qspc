#include "MMLErrors.h"
#include <map>

typedef std::map<int, std::string> ErrorMessageMap;
static ErrorMessageMap sMsgMap;
static bool ensureMessageRegistration();

MMLErrors::MMLErrors()
{
}

MMLErrors::~MMLErrors()
{
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

bool ensureMessageRegistration() {
	if (sMsgMap.size() > 0) {
		return false;
	}

	_er(MMLErrors::E_UNKNOWN_CHAR , "�F���ł��Ȃ�����������܂��iMML�ŗ��p�ł��Ȃ��������Ȃ����A�܂��͗��p�ł��镶���ł����Ă��s�K�؂Ȉʒu�ɂȂ����m�F���Ă��������j");
	_er(MMLErrors::E_UNKNOWN_EXPR , "�F���ł��Ȃ��g�[�N���񂪂���܂��i�R�}���h�𔺂킸�ɐ���������ȂǁA�������Ȃ��g�[�N���̕��ѕ��ɂȂ��Ă��Ȃ����m�F���Ă��������j");
	_er(MMLErrors::E_NESTED_TUP   , "�A���𑽏d�ɂ��邱�Ƃ͂ł��܂���");
	_er(MMLErrors::E_NESTED_REPEAT, "��ԃ��s�[�g�𑽏d�ɂ��邱�Ƃ͂ł��܂���");

	return true;
}

