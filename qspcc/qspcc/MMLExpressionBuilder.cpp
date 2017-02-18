#include "MMLExpressionBuilder.h"
#include "MMLErrors.h"
static void groupTokens(MMLExprStruct& exprOut, const MMLTokenList& sourceTokenList, int startPos, int count);


MMLExpressionBuilder::MMLExpressionBuilder(class MMLTokenizer* pSourceTokenizer) : mpErrorRecv(NULL), mVerboseLevel(0)
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

bool MMLExpressionBuilder::buildExpressions() {
	mReadPos = 0;
	int i;
	for (i = 0; i < 65536; ++i) {
		if (!matchExpressionForm()) {
			raiseExpressionError();
			return false;
		}

		if (mReadPos >= (int)strlen(mpTokTypeString)) {
			if (mVerboseLevel) {
				fprintf(stderr, "Built expressions.\n");
			}
			break;
		}
	}

	return true;
}

bool MMLExpressionBuilder::matchExpressionForm() {
	const MMLExpFormList& ls = referExpressionFormList();
	MMLExpFormList::const_iterator it;

	for (it = ls.begin(); it != ls.end(); it++) {

		std::cmatch m;
		const bool found = std::regex_search(mpTokTypeString + mReadPos, m, it->referRegex());
		if (found && m.size() > 0) {
			const std::string& foundStr = m.str(0);
			const int numTokens = foundStr.size();

			if (mVerboseLevel > 0) {
				fprintf(stderr, "%s [%d]\n", foundStr.c_str(), numTokens);
			}
			MMLExprStruct expr;
			expr.exprType = it->getType();
			groupTokens(expr, mpSourceTokenizer->referTokenList(), mReadPos, numTokens);
			mExprList.push_back(expr);

			mReadPos += foundStr.size();
			return true;
		}

	}

	return false;
}

void MMLExpressionBuilder::raiseExpressionError() {
	if (mpSourceTokenizer && mpErrorRecv) {
		const MMLTokenStruct& tok = mpSourceTokenizer->referTokenList().at(mReadPos);

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

void groupTokens(MMLExprStruct& exprOut, const MMLTokenList& sourceTokenList, int startPos, int count) {
	for (int i = 0; i < count; ++i) {
		exprOut.tokenList.push_back( sourceTokenList.at( startPos+i ) );
	}
}

