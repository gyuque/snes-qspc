#include "MMLTokenizer.h"
#include "MMLErrors.h"
#include <fstream>
#include <iostream>
#include <string.h>

MMLTokenizer::MMLTokenizer() : mpErrorRecv(NULL), mVerboseLevel(0)
{
	mContentLength = 0;
	mpSourceBuffer = NULL;
	resetReadPosition();
}


MMLTokenizer::~MMLTokenizer()
{
	if (mpSourceBuffer) {
		free(mpSourceBuffer);
		mpSourceBuffer = NULL;
	}
}

void MMLTokenizer::resetReadPosition() {
	mReadPosition = 0;
}

bool MMLTokenizer::loadFile(const char* filename) {
	if (mpSourceBuffer) {
		return false;
	}

	std::ifstream ifs(filename);
	if (ifs.fail()) {
		return false;
	}

	std::string str((std::istreambuf_iterator<char>(ifs)),
	                 std::istreambuf_iterator<char>());

	mContentLength = str.size();
	mpSourceBuffer = _strdup(str.c_str());

	return tokenizeAll();
}

bool MMLTokenizer::tokenizeAll() {
	resetColAndLine();

	int i;
	for (i = 0; i < 65536; ++i) {
		const MMLTokenizeResult res = tokenizeNext();
		if (res == TRES_ERROR) {
			// no match
			raiseTokenizeError();

			return false;
		} else if (res == TRES_EOF) {
			break;
		}
	}

	if (mVerboseLevel > 0) {
		fprintf(stderr, "TL: %s\n", mTokenTypeLetterList.c_str());
	}

	return true;
}

void MMLTokenizer::raiseTokenizeError() {
	char buf[2];
	buf[0] = *(mpSourceBuffer + mReadPosition);
	buf[1] = '\0';

	if (mpErrorRecv) {
		mpErrorRecv->raiseError(mCurrentLineNum, mCurrentColNum, buf, MMLErrors::getErrorString( MMLErrors::E_UNKNOWN_CHAR ));
	}
}

MMLTokenizeResult MMLTokenizer::tokenizeNext() {
	const MMLTokTypeList& ls = referTokenTypeList();
	MMLTokTypeList::const_iterator it;

	if (mReadPosition >= mContentLength) {
		return TRES_EOF;
	}

	for (it = ls.begin(); it != ls.end(); it++) {
		const std::regex& re = it->referRegex();

		std::cmatch m;
		const bool found = std::regex_search(mpSourceBuffer + mReadPosition, m, re, std::regex_constants::match_continuous);
		if (found && m.size() > 0) {
			const std::string& foundStr = m.str(0);
			if (0 != m.position(0)) {
				continue; // 位置が0以外の場合、次行にマッチしているので無視
			}

			//fprintf(stderr, "** %s **\n", mpSourceBuffer + mReadPosition);
			if (mVerboseLevel > 0) {
				fprintf(stderr, "'%s' (%s)\n", foundStr.c_str(), it->getName().c_str());
			}

			if (it->getType() != TT_BLANK) {
				addTokenInstance(foundStr, *it);
			}

			// 行番号、桁を更新（改行があれば桁を0に戻す）
			updateColAndLine(foundStr);

			// 探索位置を更新
			mReadPosition += foundStr.size();

			return TRES_OK;
		}
	}

	return TRES_ERROR;
}

bool MMLTokenizer::addTokenInstance(const std::string& rawStr, const MMLTokenTypeInfo& tokTypeInfo) {
	MMLTokenStruct tk;
	tk.is_macrodef = false;

	tk.lineNo = mCurrentLineNum;
	tk.colNo = mCurrentColNum;

	tk.rawStr = rawStr;
	tk.type = tokTypeInfo.getType();
	putConvertedValue(tk);

	const char typeLetter = tokTypeInfo.getLetter();

	mParsedTokenList.push_back(tk);
	mTokenTypeLetterList.push_back(typeLetter);

	return true;
}

void MMLTokenizer::putConvertedValue(MMLTokenStruct& tk) {
	if (TT_INTEGER == tk.type || TT_NEG_INT == tk.type) {
		tk.intVal = atoi( tk.rawStr.c_str() );
	} else if (TT_DOTS == tk.type) {
		tk.intVal = tk.rawStr.size(); // Count dots
	} else {
		tk.intVal = 0;
	}
}


void MMLTokenizer::resetColAndLine() {
	mCurrentColNum = 0;
	mCurrentLineNum = 0;
}

int MMLTokenizer::countNewLine(const std::string& tokenStr) {
	int sum = 0;
	const int n = tokenStr.size();
	for (int i = 0; i < n; ++i) {
		if (tokenStr.at(i) == '\n') {
			++sum;
		}
	}

	return sum;
}

void MMLTokenizer::updateColAndLine(const std::string& tokenStr) {
	const int newlines = countNewLine(tokenStr);
	if (newlines) {
		mCurrentLineNum += newlines;
		mCurrentColNum = 0;
	} else {
		mCurrentColNum += tokenStr.size();
	}
}

void MMLTokenizer::markAsMacroDefinition(unsigned int position) {
	MMLTokenStruct& tok = mParsedTokenList.at(position);
	tok.is_macrodef = true;
}

MMLTokenTypeInfo::MMLTokenTypeInfo(const char letter, const char* name, MMLTokenType type, const char* re_pattern) :
	mLetter(letter), mType(type), mRe(re_pattern), mName(name)
{
}

MMLTokenTypeInfo::~MMLTokenTypeInfo() {

}

MMLExpressionForm::MMLExpressionForm(const char* name, MMLExpressionType type, const char* re_pattern) :
	mType(type), mName(name), mRe(re_pattern)
{

}

MMLExpressionForm::~MMLExpressionForm()
{

}
