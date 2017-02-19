#include "Embedder.h"


Embedder::Embedder()
{
	mBaseDir = ".";
	mpSourceBin = NULL;
}


Embedder::~Embedder()
{
	if (mpSourceBin) {
		delete mpSourceBin;
		mpSourceBin = NULL;
	}
}

void Embedder::setBaseDir(const std::string& baseDir) {
	mBaseDir = baseDir;
}

bool Embedder::loadLocationConfig(const char* filename) {
	std::string fullPath = mBaseDir + "\\" + filename;

	return mConfig.load( fullPath.c_str() );
}

void Embedder::dumpConfig() {
	mConfig.dump();
}

bool Embedder::exportToFile(const char* filename) {
	if (!mpSourceBin) { return false; }

	return mpSourceBin->exportToFile(filename);
}

void Embedder::embed(IEmbedderSource* pSeqSource) {
	embedFromSource(pSeqSource, mConfig.getSequenceOrigin(), mConfig.calcSequenceCapacity(), "sequence");
}

bool Embedder::loadSourceBin(const char* filename) {
	if (mpSourceBin) { return true; }

	std::string fullPath = mBaseDir + "\\" + filename;
	mpSourceBin = new BinFile( fullPath.c_str() );
	return mpSourceBin->size() > 0;
}

void Embedder::embedFromSource(IEmbedderSource* pSource, unsigned int origin, unsigned int capacity, const std::string& chunkName) {
	if (!mpSourceBin || !pSource) { return; }

	const unsigned int lastAddr = origin + pSource->esGetSize() - 1;

	fprintf(stderr, "Embedding: %s\n", chunkName.c_str());
	fprintf(stderr, "Address: %04X - %04X\n", origin, lastAddr);

	const size_t imgSize = mpSourceBin->size();
	if (lastAddr >= imgSize) {
		return;
	}

	if (pSource->esGetSize() > capacity) {
		return;
	}

	const size_t n = pSource->esGetSize();
	for (size_t i = 0; i < n; ++i) {
		const uint8_t k = pSource->esGetAt(i);
		mpSourceBin->writeByte(origin + i, k);
	}
}