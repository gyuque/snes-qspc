#include "Embedder.h"


Embedder::Embedder()
{
}


Embedder::~Embedder()
{
}

void Embedder::loadLocationConfig(const char* filename) {
	mConfig.load(filename);
}