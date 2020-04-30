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
	void process_OctaveReverse(const MMLExprStruct& expr);
	void process_Using(const MMLExprStruct& expr);
	void process_Title(const MMLExprStruct& expr);
	void process_Artist(const MMLExprStruct& expr);
	void process_Duration(const MMLExprStruct& expr);
	void process_Comment(const MMLExprStruct& expr);
	void process_Coder(const MMLExprStruct& expr);
	void process_GameTitle(const MMLExprStruct& expr);
};


#endif