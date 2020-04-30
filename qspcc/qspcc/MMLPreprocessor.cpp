#include "MMLPreprocessor.h"
#include "MMLUtility.h"

MMLPreprocessor::MMLPreprocessor(MusicDocument* pDoc) : mpDocument(pDoc)
{
}


MMLPreprocessor::~MMLPreprocessor()
{
}

void MMLPreprocessor::processExpression(const MMLExprStruct& expr) {
	switch (expr.exprType) {
	case MX_USINGDECL:
		process_Using(expr);
		break;

	case MX_TITLEDECL:
		process_Title(expr);
		break;

	case MX_ARTISTDECL:
		process_Artist(expr);
		break;

	case MX_OCTREVDECL:
		process_OctaveReverse(expr);
		break;

	case MX_DURATIONDECL:
		process_Duration(expr);
		break;

	case MX_COMMENT:
		process_Comment(expr);
		break;

	case MX_CODERDECL:
		process_Coder(expr);
		break;

	case MX_GAMETITLEDECL:
		process_GameTitle(expr);
		break;

	default:
		// ignore
		break;
	}
}

void MMLPreprocessor::process_Using(const MMLExprStruct& expr) {
	const std::string& instNameQuoted = expr.tokenList[1].rawStr;
	const std::string& instName = cleanStringLiteral(instNameQuoted);

	//fprintf(stderr, "U:: %s\n", instName.c_str());
	if (mpDocument) {
		mpDocument->setInstrumentSetName(instName);
	}
}

void MMLPreprocessor::process_Title(const MMLExprStruct& expr) {
	const std::string& sQuoted = expr.tokenList[1].rawStr;
	const std::string& s = cleanStringLiteral(sQuoted);

	// fprintf(stderr, "T:: %s\n", s.c_str());
	if (mpDocument) {
		mpDocument->setTitle(s);
	}
}

void MMLPreprocessor::process_Artist(const MMLExprStruct& expr) {
	const std::string& sQuoted = expr.tokenList[1].rawStr;
	const std::string& s = cleanStringLiteral(sQuoted);

	// fprintf(stderr, "A:: %s\n", s.c_str());
	if (mpDocument) {
		mpDocument->setArtistName(s);
	}
}

void MMLPreprocessor::process_OctaveReverse(const MMLExprStruct& expr) {
	if (mpDocument) {
		mpDocument->setOctaveReverseEnabled(true);
	}
}

void MMLPreprocessor::process_Duration(const MMLExprStruct& expr) {
	if (mpDocument) {
		mpDocument->setRecommendedDuration( expr.tokenList[1].intVal );
	}
}

void MMLPreprocessor::process_Comment(const MMLExprStruct& expr) {
	const std::string& sQuoted = expr.tokenList[1].rawStr;
	const std::string& s = cleanStringLiteral(sQuoted);

	if (mpDocument) {
		mpDocument->setComment(s);
	}
}

void MMLPreprocessor::process_Coder(const MMLExprStruct& expr) {
	const std::string& sQuoted = expr.tokenList[1].rawStr;
	const std::string& s = cleanStringLiteral(sQuoted);

	if (mpDocument) {
		mpDocument->setCoderName(s);
	}
}

void MMLPreprocessor::process_GameTitle(const MMLExprStruct& expr) {
	const std::string& sQuoted = expr.tokenList[1].rawStr;
	const std::string& s = cleanStringLiteral(sQuoted);

	if (mpDocument) {
		mpDocument->setGameTitle(s);
	}
}