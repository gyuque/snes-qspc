#include "Embedder.h"


Embedder::Embedder()
{
	mBaseDir = ".";
	mpSourceBin = nullptr;
}


Embedder::~Embedder()
{
	if (mpSourceBin) {
		delete mpSourceBin;
		mpSourceBin = nullptr;
	}
}

void Embedder::setBaseDir(const std::string& baseDir) {
	mBaseDir = baseDir;
}

bool Embedder::loadLocationConfig(const char* filename) {
	std::string fullPath = mBaseDir + "/" + filename;

	return mConfig.load( fullPath.c_str() );
}

void Embedder::dumpConfig() {
	mConfig.dump();
}

bool Embedder::exportToFile(const char* filename, const char* hi_filename) {
	if (!mpSourceBin) { return false; }

	const bool suc_1 = mpSourceBin->exportToFile(filename, 0, BinFile::k32KB);
	const bool suc_2 = mpSourceBin->exportToFile(hi_filename, BinFile::k32KB);
	return suc_1 && suc_2;
}

void Embedder::embed(
		IEmbedderSource* pMusicHeaderSource,
		IEmbedderSource* pFqTableSource,
		IEmbedderSource* pSeqSource,
		IEmbedderSource* pInstDirSource,
		IEmbedderSource* pBRRDirSource,
		IEmbedderSource* pBRRBodySource
	) {
	embedFromSource(pMusicHeaderSource, mConfig.getMusicHeaderOrigin(), mConfig.calcMusicHeaderCapacity(), "music_header");
	embedFromSource(pFqTableSource, mConfig.getFqTableOrigin(), mConfig.calcFqTableCapacity(), "fqtable");
	embedFromSource(pSeqSource, mConfig.getSequenceOrigin(), mConfig.calcSequenceCapacity(), "sequence");
	embedFromSource(pInstDirSource, mConfig.getInstDirOrigin(), mConfig.calcInstDirCapacity(), "instdir");
	embedFromSource(pBRRDirSource, mConfig.getBRRDirOrigin(), mConfig.calcBRRDirCapacity(), "brrdir");
	embedFromSource(pBRRBodySource, mConfig.getBRRBodyOrigin(), mConfig.calcBRRBodyCapacity(), "brrbody");
}

BinFile* Embedder::loadBinFileWithBase(const std::string& baseDir, const char* filename) {
	std::string fullPath = baseDir + "\\" + filename;
	BinFile* b = new BinFile(fullPath.c_str());
	if (b->size() == 0) {
		delete b;
		b = nullptr;
	}

	return b;
}

bool Embedder::loadSourceBin(const char* filename) {
	if (mpSourceBin) { return true; }
	mpSourceBin = loadBinFileWithBase(mBaseDir, filename);
	return !!(mpSourceBin);
}

void Embedder::embedFromSource(IEmbedderSource* pSource, unsigned int origin, unsigned int capacity, const std::string& chunkName) {
	if (!mpSourceBin || !pSource) { return; }

	const unsigned int lastAddr = origin + pSource->esGetSize() - 1;
	const int percentage = (capacity == 0) ? 0 : (pSource->esGetSize() * 100) / capacity;

	fprintf(stderr, "Embedding: %s\n", chunkName.c_str());
	fprintf(stderr, " Address: %04X - %04X\n", origin, lastAddr);
	fprintf(stderr, " Size: %d/%d bytes(%d%%)\n", pSource->esGetSize(), capacity, percentage);

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