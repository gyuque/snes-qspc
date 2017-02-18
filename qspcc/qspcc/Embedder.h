#ifndef EMBEDDER_H_INCLUDED
#define EMBEDDER_H_INCLUDED

#include "MusicDocument.h"
#include "EmbedderConfig.h"
#include "BinFile.h"

class Embedder
{
public:
	Embedder();
	virtual ~Embedder();

	void loadLocationConfig(const char* filename);
	void dumpConfig();

	void embed();
protected:
	EmbedderConfig mConfig;
	BinFile* mpSourceBin;
};

#endif