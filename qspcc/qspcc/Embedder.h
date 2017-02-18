#ifndef EMBEDDER_H_INCLUDED
#define EMBEDDER_H_INCLUDED

#include "MusicDocument.h"
#include "EmbedderConfig.h"

class Embedder
{
public:
	Embedder();
	virtual ~Embedder();

	void loadLocationConfig(const char* filename);
protected:
	EmbedderConfig mConfig;
};

#endif