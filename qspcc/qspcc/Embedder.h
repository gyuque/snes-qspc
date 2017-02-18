#ifndef EMBEDDER_H_INCLUDED
#define EMBEDDER_H_INCLUDED

#include "MusicDocument.h"
#include "EmbedderConfig.h"
#include "BinFile.h"

class IEmbedderSource {
public:
	virtual size_t esGetSize() = 0;
	virtual uint8_t esGetAt(unsigned int index) = 0;
};

class Embedder
{
public:
	Embedder();
	virtual ~Embedder();

	void loadSourceBin(const char* filename);
	void loadLocationConfig(const char* filename);
	void dumpConfig();

	void embed(IEmbedderSource* pSeqSource);
protected:

	EmbedderConfig mConfig;
	BinFile* mpSourceBin;
};

#endif