#include "MMLTokenizer.h"

static MMLTokTypeList sTokenTypeList;

static void rt_(char letter, const char* name, MMLTokenType type, const char* reLit) {
	MMLTokenTypeInfo t(letter, name, type, reLit);
	sTokenTypeList.push_back(t);
}

// ■ TokenType
// 各トークンを正規表現で指定する
// 順番には意味があり、先に書かれているものが優先的にマッチするので注意

void registerMMlTokenTypeList() {
	if (sTokenTypeList.size() > 0) { return; }

	// ^ High priority
	// | (matches earlier)

	//  Ltr | DispName | Type        | Pattern
	rt_('Q' , "QSet"    , TT_QSET_CMD , "^q");
	rt_('O' , "OctSet"  , TT_OCT_CMD  , "^o");
	rt_('L' , "LenSet"  , TT_LEN_CMD  , "^l");
	rt_('T' , "Tempo"   , TT_TEMPO_CMD, "^t");
	rt_('I' , "intnum"  , TT_INTEGER  , "^[1-9][0-9]*");
	rt_('<' , "OctInc"  , TT_OINC_CMD , "^<");
	rt_('>' , "OctDec"  , TT_ODEC_CMD , "^>");
	rt_('N' , "Note"    , TT_NOTE     , "^[a-gr][-+]?");
	rt_('d' , "Dots"    , TT_DOTS     , "^\\.+");
	rt_(';' , "Term."   , TT_TERM     , "^;");
	rt_('{' , "CmbStart", TT_CMB_START, "^\\{");
	rt_('}' , "CmbEnd"  , TT_CMB_END  , "^\\}");
	rt_('[' , "LRpStart", TT_LcREP_START, "^/:");
	rt_(']' , "LRpEnd"  , TT_LcREP_END, "^:/");
	rt_('_' , "(blank)" , TT_BLANK,     "^[\\t\\r\\n ]+");
	rt_('"' , "StrLit"  , TT_STRLIT,    "^\"[^\"]*\"");
	rt_('U' , "#using"  , TT_USING,     "^#[uU][sS][iI][nN][gG]");
}

const MMLTokTypeList& referTokenTypeList() {
	return sTokenTypeList;
}