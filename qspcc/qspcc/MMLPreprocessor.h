#ifndef MMLPREPROCESSOR_H_INCLUDED
#define MMLPREPROCESSOR_H_INCLUDED

#include "MMLTokenizer.h"
#include "MusicDocument.h"

class MMLPreprocessor
{
public:
	MMLPreprocessor(MusicDocument* pDoc);
	virtual ~MMLPreprocessor();

	void processExpression(const MMLExprStruct& expr);
protected:
	MusicDocument* mpDocument;
	void process_Using(const MMLExprStruct& expr);
};


#endif