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
	_er(MMLErrors::E_UNKNOWN_CHAR   , "認識できない文字があります（MMLで利用できない文字がないか、または利用できる文字であっても不適切な位置にないか確認してください）");
	_er(MMLErrors::E_UNKNOWN_EXPR   , "認識できないトークン列があります（コマンドを伴わずに整数があるなど、正しくないトークンの並び方になっていないか確認してください）");
	_er(MMLErrors::E_NESTED_TUP     , "連符を多重にすることはできません");
	_er(MMLErrors::E_NESTED_REPEAT  , "区間リピートを多重にすることはできません");
	_er(MMLErrors::E_RECURSIVE_MACRO, "マクロが再帰しています");
	_er(MMLErrors::E_NOTFOUND_MACRO , "定義されていないマクロを使おうとしています");
	_er(MMLErrors::E_INST_NOTSET    , "音色セットが指定されていません");
	_er(MMLErrors::E_INST_NOTFOUND  , "音色セットをロードできません");
	_er(MMLErrors::E_INST_M_NOTFOUND, "音色セットのマニフェストをロードできません");
	_er(MMLErrors::E_INST_M_BAD     , "音色セットのマニフェストにエラーがあります");
	_er(MMLErrors::E_INST_B_NOTFOUND, "マニフェスト内で指定されているBRRファイルが見つかりません");
	_er(MMLErrors::E_INST_MULTI_FIX , "BRRの固定ポイントは1回のみ指定できます");
	_er(MMLErrors::E_INST_EXCEED_FIX, "可変部のBRRデータが固定ポイントを超過しました");
	_er(MMLErrors::E_INST_BAD_FIXBRR, "固定部のBRRデータが一致していません");
	_er(MMLErrors::E_EMB_CAPACITY_EX, "データセクションの容量が足りません");
	_er(MMLErrors::E_TOO_MANY_FILES , "入力ファイルが多すぎます");


	_er(MMLErrors::MSG_COMPILING    , "●コンパイル中: ");
	_er(MMLErrors::MSG_EMBEDEXP     , "容量を超過すると ! マークが表示されます");
	_er(MMLErrors::MSG_EXPORTING    , "出力しています...");
	_er(MMLErrors::MSG_FIXENABLED   , "データ固定ポイントが設定されました");
	_er(MMLErrors::MSG_SUCCESSS     , "<正常に終了しました>");
}

bool ensureMessageRegistration() {
	if (sMsgMap.size() > 0) {
		return false;
	}

	registerErrorMessages_JP();

	return true;
}

