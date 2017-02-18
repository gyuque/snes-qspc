#ifndef MMLEXPRESSIONBUILDER_H_INCLUDED
#define MMLEXPRESSIONBUILDER_H_INCLUDED

#include "MMLTokenizer.h"
#include "IMMLError.h"

class MMLExpressionBuilder
{
public:
	MMLExpressionBuilder(class MMLTokenizer* pSourceTokenizer);
	virtual ~MMLExpressionBuilder();
	void setVerboseLevel(int lv) { mVerboseLevel = lv; }

	void setErrorReceiver(IMMLError* er) {
		mpErrorRecv = er;
	}

	bool buildExpressions();
	void dump();

	int count() const;
	MMLExprStruct* referAt(int index);
protected:
	IMMLError* mpErrorRecv;

	int mReadPos;
	MMLExprList mExprList;
	char* mpTokTypeString;
	class MMLTokenizer* mpSourceTokenizer;
	int mVerboseLevel;

	bool matchExpressionForm();
	void raiseExpressionError();
};

#endif