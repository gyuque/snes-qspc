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

	default:
		// ignore
		break;
	}
}

void MMLPreprocessor::process_Using(const MMLExprStruct& expr) {
	const std::string& instNameQuoted = expr.tokenList[1].rawStr;
	const std::string& instName = cleanStringLiteral(instNameQuoted);

	fprintf(stderr, "U:: %s\n", instName.c_str());
	if (mpDocument) {
		mpDocument->setInstrumentSetName(instName);
	}
}