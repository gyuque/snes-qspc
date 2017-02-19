#ifndef EMBEDDER_H_INCLUDED
#define EMBEDDER_H_INCLUDED
#include "EmbedderConfig.h"
#include "BinFile.h"

class IEmbedderSource {
public:
	virtual size_t esGetSize() = 0;
	virtual uint8_t esGetAt(unsigned int index) = 0;
};

#include "MusicDocument.h"

class Embedder
{
public:
	Embedder();
	virtual ~Embedder();

	void setBaseDir(const std::string& baseDir);
	bool loadSourceBin(const char* filename);
	bool loadLocationConfig(const char* filename);
	void dumpConfig();
	bool exportToFile(const char* filename);

	void embed(IEmbedderSource* pSeqSource);

	void embedFromSource(IEmbedderSource* pSource, unsigned int origin, unsigned int capacity, const std::string& chunkName);
protected:

	std::string mBaseDir;
	EmbedderConfig mConfig;
	BinFile* mpSourceBin;
};

#endif