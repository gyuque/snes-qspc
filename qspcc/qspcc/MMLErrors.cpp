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

	_er(MMLErrors::E_UNKNOWN_CHAR , "認識できない文字があります（MMLで利用できない文字がないか、または利用できる文字であっても不適切な位置にないか確認してください）");
	_er(MMLErrors::E_UNKNOWN_EXPR , "認識できないトークン列があります（コマンドを伴わずに整数があるなど、正しくないトークンの並び方になっていないか確認してください）");
	_er(MMLErrors::E_NESTED_TUP   , "連符を多重にすることはできません");
	_er(MMLErrors::E_NESTED_REPEAT, "区間リピートを多重にすることはできません");

	return true;
}

