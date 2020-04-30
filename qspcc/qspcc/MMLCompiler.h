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

	bool compile(std::string filename, MMLCompiler* prevCompiler);
	void dumpAllErrors();
	void setVerboseLevel(int lv);

	MusicDocument* referLastDocument() { return mpLastDocument; }
	const MusicDocument* constLastDocument() const { return mpLastDocument; }
	const std::string& getSourceFileName() const { return mCurrentFilename; }

	bool checkCanShareDriver(const class MMLCompiler& theOther, size_t mutableZoneSize, const EmbedderConfig& eConfig);
protected:
	void releaseDocumentIf();
	bool shouldAbort() const;

	void clearCommands();
	void preprocess();
	void usePrevInstsIf(class MMLCompiler* pPrevCompiler);
	void generateCommands();
	void appendTrackFooter();
	void applyContextDependentParams();
	void generateByteCodeTracks();
	void determineCommandAddress(int trackIndex, MusicTrack* pTrack);
	void generateATrack(int trackIndex, MusicTrack* pTrack);

	void dumpAllCommands();
	void checkGlobalErrors(InstLoadResult instLdResult);
	virtual void raiseError(int lineno, int charno, const char* relStr, const std::string& message);

	int mVerboseLevel;

	std::string mCurrentFilename;
	MusicDocument* mpLastDocument;
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