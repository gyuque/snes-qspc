#include "Embedder.h"


Embedder::Embedder()
{
	mpSourceBin = NULL;
}


Embedder::~Embedder()
{
	if (mpSourceBin) {
		delete mpSourceBin;
		mpSourceBin = NULL;
	}
}

void Embedder::loadLocationConfig(const char* filename) {
	mConfig.load(filename);
}

void Embedder::dumpConfig() {
	mConfig.dump();
}

void Embedder::embed() {

}
