#include "MMLExpressionBuilder.h"
#include "MMLErrors.h"
static void groupMacroDefTokens(MMLExprStruct& exprOut, MMLTokenizer* pSourceTokenizer, int startPos, int count, bool mark_macrodef);
static void groupTokens(MMLExprStruct& exprOut, const MMLTokenList& srcTokenList, int startPos, int count);


MMLExpressionBuilder::MMLExpressionBuilder(MMLTokenizer* pSourceTokenizer) : mpErrorRecv(NULL), mVerboseLevel(0)
{
	mpSourceTokenizer = pSourceTokenizer;
	mpTokTypeString = _strdup( pSourceTokenizer->referTokenTypeLetterList().c_str() );
}

MMLExpressionBuilder::~MMLExpressionBuilder()
{
	if (mpTokTypeString) {
		free(mpTokTypeString);
		mpTokTypeString = NULL;
	}
}

int MMLExpressionBuilder::count() const {
	return mExprList.size();
}

MMLExprStruct* MMLExpressionBuilder::referAt(int index) {
	if (index < 0 || index >= (int)mExprList.size()) {
		return NULL;
	}

	return &( mExprList[index] );
}

bool MMLExpressionBuilder::buildExpressions(MacroDictionary& macroDic) {
	int i;
	const int slen = (int)strlen(mpTokTypeString);

	// Phase 1
	mReadPos = 0;
	for (i = 0; i < 65536; ++i) {
		if (!matchMacroDefinitionExpressionForm(macroDic)) {
			break;
		}

		if (mReadPos >= slen) {
			break;
		}
	}

	// Phase 2
	generateMacroExpandedTokenList(macroDic);

	// Phase 3
	int xReadPos = 0;
	const int xlen = (int)( mExpandedTokenLetters.size() );
	mExprList.clear();
	for (i = 0; i < 65536; ++i) {
		if ( !matchExpressionForm(mExprList, xReadPos, mExpandedTokenList, mExpandedTokenLetters.c_str(), mVerboseLevel) ) {
			raiseExpressionError(xReadPos);
			return false;
		}

		if (xReadPos >= xlen) {
			if (mVerboseLevel) {
				fprintf(stderr, "Built expressions.\n");
			}
			break;
		}
	}

	return true;
}

bool MMLExpressionBuilder::matchExpressionForm(
		MMLExprList& outXList,
		int& inoutReadPos,
		const MMLTokenList& inTokenList,
		const char* letterString,
		int verboseLevel
	) {

	const MMLExpFormList& ls = referExpressionFormList();
	MMLExpFormList::const_iterator it;

	for (it = ls.begin(); it != ls.end(); it++) {

		std::cmatch m;
		const bool found = std::regex_search(letterString + inoutReadPos, m, it->referRegex(), std::regex_constants::match_continuous);
		if (found && m.size() > 0) {
			const std::string& foundStr = m.str(0);
			const int numTokens = foundStr.size();

			if (verboseLevel > 0) {
				fprintf(stderr, "%s [%d]\n", foundStr.c_str(), numTokens);
			}
			MMLExprStruct expr;
			expr.exprType = it->getType();
			groupTokens(expr, inTokenList, inoutReadPos, numTokens);
			outXList.push_back(expr);

			inoutReadPos += foundStr.size();
			return true;
		}

	}

	return false;
}

// マクロ宣言のみ抽出しマクロ内のトークン列を保存
bool MMLExpressionBuilder::matchMacroDefinitionExpressionForm(MacroDictionary& macroDic) {
	const MMLExpFormList& ls = referExpressionFormList();
	MMLExpFormList::const_iterator it;

	for (it = ls.begin(); it != ls.end(); it++) {

		std::cmatch m;
		const bool found = std::regex_search(mpTokTypeString + mReadPos, m, it->referRegex(), std::regex_constants::match_continuous);
		if (found && m.size() > 0) {
			const std::string& foundStr = m.str(0);
			const int numTokens = foundStr.size();

			if (it->getType() == MX_MACRODEF) {
				MMLExprStruct expr;
				expr.exprType = it->getType();
				groupMacroDefTokens(expr, mpSourceTokenizer, mReadPos, numTokens, true);

				macroDic.registerFromExpr(expr);

				if (mVerboseLevel > 0) {
					fprintf(stderr, "Macro definition. Name=%s\n", mpSourceTokenizer->referTokenList().at(mReadPos).rawStr.c_str());
				}
			}

			mReadPos += foundStr.size();
			return true;
		}
	}

	return false;
}

// マクロ展開済みのトークン列を生成
void MMLExpressionBuilder::generateMacroExpandedTokenList(MacroDictionary& macroDic) {
	const MMLTokenList& srcList = mpSourceTokenizer->referTokenList();

	mExpandedTokenList.clear();
	expandMacrosInTokenList(mExpandedTokenList, srcList, macroDic, 0);

	// 展開済みのレターリストを生成
	mExpandedTokenLetters.clear();
	generateTokenLetterList(mExpandedTokenLetters, mExpandedTokenList);

	fprintf(stderr, "%s\n", mExpandedTokenLetters.c_str());
}

void MMLExpressionBuilder::generateTokenLetterList(std::string& outStr, const MMLTokenList& inList) {
	MMLTokenList::const_iterator it;
	for (it = inList.begin(); it != inList.end(); it++) {
		char ltr = getTokenTypeLetterFromTypeId(it->type);
		if (ltr) {
			outStr.push_back(ltr);
		}
	}
}

void MMLExpressionBuilder::expandMacrosInTokenList(MMLTokenList& outList, const MMLTokenList& inList, MacroDictionary& macroDic, int nestLevel) {
	const size_t n = inList.size();
	for (size_t i = 0; i < n; ++i) {
		const MMLTokenStruct& tok = inList.at(i);

		if (tok.type == TT_MAC_IDENT && !tok.is_macrodef) {
			const bool found = macroDic.exists(tok.rawStr);
			if (found) {
				const MacroDefinition& mdef = macroDic.referMacroDefinition(tok.rawStr);
				expandMacrosInTokenList(outList, mdef.tokenList, macroDic, nestLevel + 1);
			}

		} else {
			if (!tok.is_macrodef) {
				outList.push_back(tok);
			}
		}
	}
}


void MMLExpressionBuilder::raiseExpressionError(int tokenPos) {
	if (mpSourceTokenizer && mpErrorRecv) {
		const MMLTokenStruct& tok = mExpandedTokenList.at(tokenPos);

		mpErrorRecv->raiseError(tok.lineNo, tok.colNo, tok.rawStr.c_str(), MMLErrors::getErrorString( MMLErrors::E_UNKNOWN_EXPR ));
	}
}

void MMLExpressionBuilder::dump() {
	const int n = mExprList.size();
	for (int i = 0; i < n; ++i) {
		MMLExprStruct& expr = mExprList[i];

		fprintf(stderr, "%3d |", i);
		const int numTokens = expr.tokenList.size();
		for (int j = 0; j < numTokens; ++j) {
			fprintf(stderr, "| %s ", expr.tokenList[j].rawStr.c_str());
		}
		fprintf(stderr, "\n");
	}
}

void groupMacroDefTokens(MMLExprStruct& exprOut, MMLTokenizer* pSourceTokenizer, int startPos, int count, bool mark_macrodef) {
	const MMLTokenList& sourceTokenList = pSourceTokenizer->referTokenList();

	for (int i = 0; i < count; ++i) {
		exprOut.tokenList.push_back(sourceTokenList.at(startPos + i));

		if (mark_macrodef) {
			pSourceTokenizer->markAsMacroDefinition(startPos + i);
		}
	}

	if (mark_macrodef) {
		// マクロの終端もマークしておく
		const unsigned int term_pos = startPos + count;
		if (term_pos < sourceTokenList.size()) {
			if (sourceTokenList[term_pos].type == TT_TERM) {
				pSourceTokenizer->markAsMacroDefinition(term_pos);
			}
		}
	}
}

void groupTokens(MMLExprStruct& exprOut, const MMLTokenList& srcTokenList, int startPos, int count) {
	for (int i = 0; i < count; ++i) {
		exprOut.tokenList.push_back(srcTokenList.at(startPos + i));
	}
}

