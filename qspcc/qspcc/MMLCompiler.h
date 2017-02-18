#ifndef MMLCOMPILER_H_INCLUDED
#define MMLCOMPILER_H_INCLUDED

#include <string>
#include <vector>
#include "MMLTokenizer.h"
#include "MMLExpressionBuilder.h"
#include "MMLCommand.h"
#include "MusicDocument.h"
#include "IMMLError.h"

typedef std::vector<struct _MMLCompileError> CompileErrorList;

class MMLCompiler : public IMMLError
{
public:
	MMLCompiler();
	virtual ~MMLCompiler();

	bool compile(std::string filename);
	void dumpAllErrors();
	void setVerboseLevel(int lv);
protected:
	bool shouldAbort() const;

	void clearCommands();
	void generateCommands();
	void applyContextDependentParams();
	void generateByteCodeTracks();
	void generateATrack(int trackIndex, MusicTrack* pTrack);

	void dumpAllCommands();
	virtual void raiseError(int lineno, int charno, const char* relStr, const std::string& message);

	int mVerboseLevel;

	MMLTokenizer mTokenizer;
	MMLExpressionBuilder* mpExprBuilder;
	MMLCommandPtrList mCommandPtrList;

	int countTracks();

	CompileErrorList mErrorList;
};

typedef struct _MMLCompileError {
	int lineno;
	int charno;
	std::string message;
	std::string relatedString;
} MMLCompileError;

#endif