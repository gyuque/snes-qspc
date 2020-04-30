#ifndef EMBEDDER_H_INCLUDED
#define EMBEDDER_H_INCLUDED
#include "EmbedderConfig.h"
#include "BinFile.h"
#include <vector>

class IEmbedderSource {
public:
	virtual size_t esGetSize() = 0;
	virtual uint8_t esGetAt(unsigned int index) = 0;
};

#include "MusicDocument.h"

typedef struct _EmbedResult {
	std::string sectionName;
	unsigned int firstAddress;
	unsigned int lastAddress;
	unsigned int percentage;
	size_t capacity;
	bool success;
} EmbedResult;

typedef std::vector<EmbedResult> EmbedResultList;

class Embedder
{
public:
	Embedder();
	virtual ~Embedder();

	void setBaseDir(const std::string& baseDir);
	bool loadSourceBin(const char* filename);
	bool loadLocationConfig(const char* filename);
	void dumpConfig();

	bool exportToFile(const char* filename, const char* hi_filename);
	bool exportToFileHalf(const char* filename);
	int getQuickLoadSize() const;

	void embed(bool bVerbose, bool debugOut,
		IEmbedderSource* pMusicHeaderSource,
		IEmbedderSource* pFqTableSource,
		IEmbedderSource* pSeqSource,
		IEmbedderSource* pInstDirSource,
		IEmbedderSource* pBRRDirSource,
		IEmbedderSource* pBRRBodySource,
		IEmbedderSource* pSeqDirSource
	);

	void embedFromSource(IEmbedderSource* pSource, unsigned int origin, unsigned int capacity, const std::string& chunkName, bool debugOut);
	const EmbedderConfig& referConfig() const { return mConfig; }
	const BinFile* referBin() const { return mpSourceBin; }

	bool checkAllSuccess() const;

	static BinFile* loadBinFileWithBase(const std::string& baseDir, const char* filename);
protected:
	EmbedResultList mResultList;

	std::string mBaseDir;
	EmbedderConfig mConfig;
	BinFile* mpSourceBin;

	void showResults();
};

#endif