#ifndef MMLPARSER_H_INCLUDED
#define MMLPARSER_H_INCLUDED
#include "IMMLError.h"

#include <string>
#include <vector>
#include <regex>
class MMLTokenTypeInfo;
class MMLExpressionForm;
typedef std::vector<MMLTokenTypeInfo> MMLTokTypeList;
typedef std::vector<MMLExpressionForm> MMLExpFormList;

typedef short DriverTick;
#define FULL_TICK 192
// 1  = 192 -> 127   (special)  
// 2. = 144 -> 126   (special)  
// 2  = 96
// 4  = 48

#define MML_QMAX 16
#define MML_VMAX 16

typedef enum _MMLTokenizeResult {
	TRES_OK = 0,
	TRES_EOF = 1,
	TRES_ERROR = -99
} MMLTokenizeResult;

typedef enum _MMLExprBuildResult {
	XRES_OK = 0,
	XRES_EOF = 1,
	XRES_ERROR = -99
} MMLExprBuildResult;



typedef enum _MMLTokenType {
	TT_BLANK,
	TT_TEMPO_CMD,
	TT_OCT_CMD,
	TT_LEN_CMD,
	TT_QSET_CMD,
	TT_OINC_CMD,
	TT_ODEC_CMD,
	TT_NOTE,
	TT_INTEGER,
	TT_DOTS,
	TT_TERM, // ;

	TT_CMB_START, // {
	TT_CMB_END,   // }

	TT_LcREP_START, // /:
	TT_LcREP_END    // :/
} MMLTokenType;

typedef enum _MMLExpressionType {
	MX_TEMPO,
	MX_OCTSET,
	MX_LENSET,
	MX_QSET,
	MX_OINC,
	MX_ODEC,
	MX_NOTE,
	MX_TERM,

	MX_CMB_START, // {
	MX_CMB_END,   // }

	MX_LcREP_START, // /:n
	MX_LcREP_END    // :/
} MMLExpressionType;



typedef struct _MMLTokenStruct {
	std::string rawStr;
	MMLTokenType type;

	int intVal;

	// Source information
	int colNo;
	int lineNo;
} MMLTokenStruct;

typedef std::vector<MMLTokenStruct> MMLTokenList;

typedef struct _MMLExprStruct {
	MMLExpressionType exprType;
	MMLTokenList tokenList;
} MMLExprStruct;

typedef struct _NoteLength {
	int N;
	int dots;
	int tuplet;
} NoteLength;

typedef std::vector<MMLExprStruct> MMLExprList;



class MMLTokenizer
{
public:
	MMLTokenizer();
	virtual ~MMLTokenizer();
	void setVerboseLevel(int lv) { mVerboseLevel = lv; }

	void resetReadPosition();
	bool loadFile(const char* filename);

	void setErrorReceiver(IMMLError* er) {
		mpErrorRecv = er;
	}

	const MMLTokenList& referTokenList() const {
		return mParsedTokenList;
	}

	const std::string& referTokenTypeLetterList() const {
		return mTokenTypeLetterList;
	}
protected:
	IMMLError* mpErrorRecv;

	MMLTokenList mParsedTokenList;
	std::string mTokenTypeLetterList;

	int mVerboseLevel;
	int mCurrentColNum;
	int mCurrentLineNum;

	int mReadPosition;
	int mContentLength;
	char* mpSourceBuffer;
	bool tokenizeAll();

	MMLTokenizeResult tokenizeNext();

	bool addTokenInstance(const std::string& rawStr, const MMLTokenTypeInfo& tokTypeInfo);
	void putConvertedValue(MMLTokenStruct& tk);

	void resetColAndLine();
	int countNewLine(const std::string& tokenStr);
	void updateColAndLine(const std::string& tokenStr);

	void raiseTokenizeError();
};

class MMLTokenTypeInfo
{
public:
	MMLTokenTypeInfo(const char letter, const char* name, MMLTokenType type, const char* re_pattern);
	virtual ~MMLTokenTypeInfo();

	const std::regex& referRegex() const {
		return mRe;
	}

	char getLetter() const {
		return mLetter;
	}

	const std::string& getName() const {
		return mName;
	}

	MMLTokenType getType() const {
		return mType;
	}
protected:
	char mLetter;
	MMLTokenType mType;
	std::regex mRe;
	std::string mName;
};

class MMLExpressionForm {
public:
	MMLExpressionForm(const char* name, MMLExpressionType type, const char* re_pattern);
	virtual ~MMLExpressionForm();

	const std::regex& referRegex() const { return mRe; }
	MMLExpressionType getType() const { return mType; }
protected:
	MMLExpressionType mType;
	std::regex mRe;
	std::string mName;
};

void registerMMlTokenTypeList();
const MMLTokTypeList& referTokenTypeList();
void registerMMLExpressionForm();
const MMLExpFormList& referExpressionFormList();

#endif