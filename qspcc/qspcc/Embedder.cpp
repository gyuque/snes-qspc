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

void Embedder::embed(bool bVerbose,
		IEmbedderSource* pMusicHeaderSource,
		IEmbedderSource* pFqTableSource,
		IEmbedderSource* pSeqSource,
		IEmbedderSource* pInstDirSource,
		IEmbedderSource* pBRRDirSource,
		IEmbedderSource* pBRRBodySource
	) {
	mResultList.clear();

	embedFromSource(pMusicHeaderSource, mConfig.getMusicHeaderOrigin(), mConfig.calcMusicHeaderCapacity(), "Header");
	embedFromSource(pFqTableSource, mConfig.getFqTableOrigin(), mConfig.calcFqTableCapacity(), "Fq Table");
	embedFromSource(pSeqSource, mConfig.getSequenceOrigin(), mConfig.calcSequenceCapacity(), "Sequence");
	embedFromSource(pInstDirSource, mConfig.getInstDirOrigin(), mConfig.calcInstDirCapacity(), "Inst Dir");
	embedFromSource(pBRRDirSource, mConfig.getBRRDirOrigin(), mConfig.calcBRRDirCapacity(), "BRR dir");
	embedFromSource(pBRRBodySource, mConfig.getBRRBodyOrigin(), mConfig.calcBRRBodyCapacity(), "BRR body");

	if (!bVerbose) {
		mResultList.erase(mResultList.begin());
		mResultList.erase(mResultList.begin()); // remove unnecessary information
	}

	showResults();
}

static void printNChars(int k, int num) {
	for (int i = 0; i < num; ++i) {
		fputc(k, stderr);
	}
}

void Embedder::showResults() {
	size_t i;
	size_t n = mResultList.size();

	unsigned int max_cols = 1;

	for (i = 0; i < n; ++i) {
		const EmbedResult& res = mResultList[i];
		const size_t name_len = res.sectionName.size();
		if (name_len > max_cols) {
			max_cols = name_len;
		}
	}

	// print ---------------------------------------------------------

	fprintf(stderr, "+----------++");
	for (i = 0; i < n; ++i) {
		printNChars('-', max_cols + 2);
		fputc('+', stderr);
	}

	fprintf(stderr, "\n| Section  ||");
	for (i = 0; i < n; ++i) {
		const EmbedResult& res = mResultList[i];
		const size_t name_len = res.sectionName.size();

		fputc(' ', stderr);
		std::cerr << res.sectionName;
		printNChars(' ', (max_cols - name_len) + 1);

		fprintf(stderr, "|");
	}

	fprintf(stderr, "\n+----------++");
	for (i = 0; i < n; ++i) {
		printNChars('-', max_cols+2);
		fputc('+', stderr);
	}

	fprintf(stderr, "\n| Capacity ||");
	for (i = 0; i < n; ++i) {
		const EmbedResult& res = mResultList[i];
		fprintf(stderr, " %6d B |", res.capacity);
	}

	fprintf(stderr, "\n| Used     ||");
	for (i = 0; i < n; ++i) {
		const EmbedResult& res = mResultList[i];
		const char mark = res.success ? ' ' : '!';
		fprintf(stderr, "%c    %3d%%%c|", mark, res.percentage, mark);
	}

	fprintf(stderr, "\n+----------++");
	for (i = 0; i < n; ++i) {
		printNChars('-', max_cols + 2);
		fputc('+', stderr);
	}

	fputc('\n', stderr);
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

	//fprintf(stderr, "Embedding: %s\n", chunkName.c_str());
	//fprintf(stderr, " Address: %04X - %04X\n", origin, lastAddr);
	//fprintf(stderr, " Size: %d/%d bytes(%d%%)\n", pSource->esGetSize(), capacity, percentage);

	EmbedResult res;
	res.firstAddress = origin;
	res.lastAddress = lastAddr;
	res.percentage = percentage;
	res.sectionName = chunkName;
	res.capacity = capacity;
	res.success = false;
	mResultList.push_back(res);

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

	mResultList[mResultList.size() - 1].success = true; // Change success flag
}

bool Embedder::checkAllSuccess() const {
	EmbedResultList::const_iterator it;

	for (it = mResultList.begin(); it != mResultList.end(); it++) {
		if (!(it->success)) {
			return false;
		}
	}

	return true;
}