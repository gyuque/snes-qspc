#ifndef MMLEXPRESSIONBUILDER_H_INCLUDED
#define MMLEXPRESSIONBUILDER_H_INCLUDED

#include "MMLTokenizer.h"
#include "IMMLError.h"
#include "MacroDictionary.h"

class MMLExpressionBuilder
{
public:
	MMLExpressionBuilder(class MMLTokenizer* pSourceTokenizer);
	virtual ~MMLExpressionBuilder();
	void setVerboseLevel(int lv) { mVerboseLevel = lv; }

	void setErrorReceiver(IMMLError* er) {
		mpErrorRecv = er;
	}

	bool buildExpressions(MacroDictionary& macroDic);
	void dump();

	int count() const;
	MMLExprStruct* referAt(int index);
protected:
	MMLTokenList mExpandedTokenList;
	std::string mExpandedTokenLetters;

	IMMLError* mpErrorRecv;

	int mReadPos;
	MMLExprList mExprList;
	char* mpTokTypeString;
	class MMLTokenizer* mpSourceTokenizer;
	int mVerboseLevel;

	bool matchMacroDefinitionExpressionForm(MacroDictionary& macroDic);
	static bool matchExpressionForm(MMLExprList& outXList, int& inoutReadPos, const MMLTokenList& inTokenList, const char* letterString, int verboseLevel);
	void raiseExpressionError(int tokenPos);
	void raiseExpressionErrorWithToken(const MMLTokenStruct& tok, int errorId = -1);

	void generateMacroExpandedTokenList(MacroDictionary& macroDic);
	void expandMacrosInTokenList(MMLTokenList& outList, const MMLTokenList& inList, MacroDictionary& macroDic, int nestLevel);
	static void generateTokenLetterList(std::string& outStr, const MMLTokenList& inList);
};

#endif