#include "MMLTokenizer.h"

static MMLTokTypeList sTokenTypeList;
static LetterMap sTokenTypeLetterMap;

static void rt_(char letter, const char* name, MMLTokenType type, const char* reLit) {
	if (sTokenTypeLetterMap.find(letter) != sTokenTypeLetterMap.end()) {
		fprintf(stderr, "**BUG** Letter duplicated! '%c'\n", letter);
		throw 1;
	}

	MMLTokenTypeInfo t(letter, name, type, reLit);
	sTokenTypeList.push_back(t);
	sTokenTypeLetterMap[letter] = true;
}

// ■ TokenType
// 各トークンを正規表現で指定する
// 順番には意味があり、先に書かれているものが優先的にマッチするので注意

void registerMMlTokenTypeList() {
	if (sTokenTypeList.size() > 0) { return; }

	// ^ High priority
	// | (matches earlier)

	//  Ltr | DispName | Type          | Pattern
	rt_('s' , "NShift"  , TT_NSHIFT_CMD, "^ns");
	rt_('S' , "VShift"  , TT_VSHIFT_CMD, "^vs");
	rt_('N' , "Note"    , TT_NOTE      , "^[a-gr][-+]?");
	rt_('I' , "intnum"  , TT_INTEGER   , "^([1-9][0-9]*|0)");
	rt_('Q' , "QSet"    , TT_QSET_CMD  , "^q");
	rt_('O' , "OctSet"  , TT_OCT_CMD   , "^o");
	rt_('L' , "LenSet"  , TT_LEN_CMD   , "^l");
	rt_('T' , "Tempo"   , TT_TEMPO_CMD , "^t");
	rt_('i' , "sign_int", TT_NEG_INT   , "^[-+][0-9]+");
	rt_('<' , "OctInc"  , TT_OINC_CMD  , "^<");
	rt_('>' , "OctDec"  , TT_ODEC_CMD  , "^>");
	rt_('d' , "Dots"    , TT_DOTS      , "^\\.+");
	rt_(';' , "Term."   , TT_TERM      , "^;");
	rt_('{' , "CmbStart", TT_CMB_START , "^\\{");
	rt_('}' , "CmbEnd"  , TT_CMB_END   , "^\\}");
	rt_('[' , "LRpStart", TT_LcREP_START, "^/:");
	rt_(']' , "LRpEnd"  , TT_LcREP_END , "^:/");
	rt_('*' , "Remark"  , TT_REMARK    , "^/\\*.*?\\*/");
	rt_('/' , "Slash"   , TT_SLASH     , "^/");
	rt_('U' , "#using"  , TT_USING     , "^#[uU][sS][iI][nN][gG]");
	rt_('`' , "#title"  , TT_TITLE     , "^#[tT][iI][tT][lL][eE]");
	rt_('A' , "#artist" , TT_ARTIST    , "^#[aA][rR][tT][iI][sS][tT]");
	rt_('8' , "#octrev" , TT_OCTREV    , "^#[oO][cC][ a-zA-Z]+");
	rt_('D' , "#duration",TT_DURATION  , "^#[dD][uU][rR][aA][tT][iI][oO][nN]");
	rt_('C' , "#comment", TT_COMMENT   , "^#[cC][oO][mM][mM][eE][nN][tT]");
	rt_('c' , "#coding" , TT_CODER     , "^#[cC][oO][dD][iI][nN][gG]");
	rt_('G' , "#game"   , TT_GAMETITLE , "^#[gG][aA][mM][eE]");
	rt_('_' , "(blank)" , TT_BLANK     , "^[\\t\\r\\n ]+");
	rt_('"' , "StrLit"  , TT_STRLIT    , "^\"[^\"]*\"");
	rt_('v' , "Velocity", TT_VELO_CMD  , "^v");
	rt_('M' , "MacIdent", TT_MAC_IDENT , "^\\$[_a-zA-Z][_a-zA-Z0-9]*");
	rt_('=' , "Equal"   , TT_EQUAL     , "^=");
	rt_('p' , "Panpot"  , TT_PANPOT    , "^@p");
	rt_('x' , "StopBGM" , TT_STOPBGM   , "^@x");
	rt_('@' , "Atmark"  , TT_ATMARK    , "^@");
	rt_('&' , "Amp"     , TT_AMP       , "^&");
}

const MMLTokTypeList& referTokenTypeList() {
	return sTokenTypeList;
}

char getTokenTypeLetterFromTypeId(MMLTokenType tid) {
	MMLTokTypeList::const_iterator it;
	for (it = sTokenTypeList.begin(); it != sTokenTypeList.end(); it++) {
		if (it->getType() == tid) {
			return it->getLetter();
		}
	}

	return 0;
}