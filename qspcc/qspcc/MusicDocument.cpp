#include "MMLUtility.h"
#include "MusicDocument.h"
#include "win32/isdir.h"

MusicDocument::MusicDocument()
{
	mTempo = 120;
	mGeneratedTrackLength = 0;
	mpMusicHeaderSource = new BytesSourceProxy(mMusicHeaderBlob);
	mpSeqSource = new BytesSourceProxy(mGeneratedSequenceBlob);
	mpInstDirSource = new BytesSourceProxy(mInstDirBlob);
	mpBRRDirSource = new BytesSourceProxy(mBRRDirBlob);
	mpBRRBlockSource = new BytesSourceProxy(mBRRBlockBlob);
	mpFqTableSource = new BytesSourceProxy(mFqTableBlob);
}


MusicDocument::~MusicDocument()
{
	releaseAllTracks();

	if (mpMusicHeaderSource) {
		delete mpMusicHeaderSource;
		mpMusicHeaderSource = nullptr;
	}

	if (mpSeqSource) {
		delete mpSeqSource;
		mpSeqSource = nullptr;
	}

	if (mpInstDirSource) {
		delete mpInstDirSource;
		mpInstDirSource = nullptr;
	}

	if (mpBRRDirSource) {
		delete mpBRRDirSource;
		mpBRRDirSource = nullptr;
	}

	if (mpBRRBlockSource) {
		delete mpBRRBlockSource;
		mpBRRBlockSource = nullptr;
	}

	if (mpFqTableSource) {
		delete mpFqTableSource;
		mpFqTableSource = nullptr;
	}
}

void MusicDocument::releaseAllTracks() {
	const size_t n = mTrackPtrList.size();
	for (size_t i = 0; i < n; ++i) {
		if (mTrackPtrList[i]) {
			delete mTrackPtrList[i];
			mTrackPtrList[i] = nullptr;
		}
	}

	mTrackPtrList.clear();
}

size_t MusicDocument::countTracks() const {
	return mTrackPtrList.size();
}

MusicTrack* MusicDocument::appendTrack() {
	MusicTrack* t = new MusicTrack();
	mTrackPtrList.push_back(t);
	return t;
}

bool MusicDocument::isInstrumentSetNameSet() const {
	return mInstrumentSetName.size() > 0;
}

const std::string& MusicDocument::getInstrumentSetName() const {
	return mInstrumentSetName;
}

void MusicDocument::calcDataSize(int* poutTrackBufferLength, bool dumpDebugInfo) {
	int max_size = 0;
	const size_t nTracks = mTrackPtrList.size();
	for (size_t i = 0; i < nTracks; ++i) {
		MusicTrack* t = mTrackPtrList[i];

		const int sz = (int)t->size();
		if (dumpDebugInfo) {
			fprintf(stderr, "T%d: %d bytes\n", i, sz);
		}

		if (sz > max_size) {
			max_size = sz;
		}
	}

	const int aligned_size = ((max_size / 256) + 1) * 256;

	if (poutTrackBufferLength) {
		*poutTrackBufferLength = aligned_size;
	}

	if (dumpDebugInfo) {
		fprintf(stderr, "Max size per track: %d\n", max_size);
		fprintf(stderr, "Aligned track size: %d\n", aligned_size);
		fprintf(stderr, "Entire size: %d\n", aligned_size * nTracks);
	}
}

void MusicDocument::generateSequenceImage() {
	mGeneratedSequenceBlob.clear();

	int trackBufferLen;
	calcDataSize(&trackBufferLen, false);
	mGeneratedTrackLength = trackBufferLen;

	const int nTracks = (int)(countTracks());
	for (int i = 0; i < nTracks; ++i) {

		MusicTrack* tr = mTrackPtrList[i];

		for (int j = 0; j < trackBufferLen; ++j) {
			mGeneratedSequenceBlob.push_back( tr->at(j) );
		}

	}

	generateHeaderImage();
}

static uint8_t calcTimerIntervalForTempo(unsigned int tempo) {
	const double f = 60000.0 / (double)tempo; // Žl•ª‰¹•„‚Ì’·‚³‚ðŒvŽZ(ms)
	return (uint8_t)(f / (48.0 * 0.125) + 0.49);
}

void MusicDocument::generateHeaderImage() {
	mMusicHeaderBlob.clear();
	mMusicHeaderBlob.push_back( countTracks() );
	mMusicHeaderBlob.push_back( mGeneratedTrackLength >> 8 );
	mMusicHeaderBlob.push_back( calcTimerIntervalForTempo(mTempo) );

	dumpHex(mMusicHeaderBlob);
}

void MusicDocument::dumpSequenceBlob() {
	fprintf(stderr, "Num of tracks: %d\n", countTracks());
	fprintf(stderr, "Length of track: %d\n", mGeneratedTrackLength);
	dumpHex(mGeneratedSequenceBlob);
}

bool MusicDocument::loadInstrumentSet() {
	if (!isInstrumentSetNameSet()) { return false; }

	const char* inst_dirname = mInstrumentSetName.c_str();
	if (!isValidDirectory(inst_dirname)) {
		fprintf(stderr, "'%s' not found\n", inst_dirname);
		return false;
	}

	std::string manifestPath = mInstrumentSetName + "/manifest.json";
	if (!checkFileExists(manifestPath.c_str())) {
		fprintf(stderr, "'%s' not found\n", manifestPath.c_str() );
		return false;
	}

	fprintf(stderr, "Loading instrument set: %s\n", manifestPath.c_str());
	mInsts.load( manifestPath.c_str(), inst_dirname );

	generateFqRegTableFromInsts();
	return true;
}

void MusicDocument::generateInstrumentDataBinaries(unsigned int baseAddress) {
	mInsts.exportPackedSrcTable(mBRRDirBlob, baseAddress);
	mInsts.exportBRR(mBRRBlockBlob);

	mInsts.exportPackedInstTable(mInstDirBlob);
}

void MusicDocument::generateFqRegTable(double baseFq, int originOctave) {
	RawFqList fq_ls = generateNotesFqTable(originOctave, 6);
	FqFactorList r_ls = generateFqFactorTable(fq_ls, baseFq);
	mFqRegTable = generateFqRegisterValueTable(r_ls);

	// write
	mFqTableBlob.clear();
	const size_t n = mFqRegTable.size();
	for (size_t i = 0; i < n; ++i) {
		const short& v = mFqRegTable[i];
		mFqTableBlob.push_back( v & 0xFF );
		mFqTableBlob.push_back( (v >> 8) & 0xFF);
	}
}

void MusicDocument::generateFqRegTableFromInsts() {
	generateFqRegTable( mInsts.getBaseEq(), mInsts.getOriginOctave() );
}




MusicTrack::MusicTrack() {

}

MusicTrack::~MusicTrack() {

}


int MusicTrack::size() const {
	return mBytes.size();
}

void MusicTrack::addByte(uint8_t b) {
	mBytes.push_back(b);
}

uint8_t MusicTrack::at(unsigned int position) {
	if (position >= mBytes.size()) { return 0; }

	return mBytes.at(position);
}


void MusicTrack::dump() {
	int n = size();
	int i;

	for (i = 0; i < n; ++i) {
		fprintf(stderr, "%02d", i);
		if (i < (n - 1)){
			fprintf(stderr, "|");
		}
	}
	fprintf(stderr, "\n");

	for (i = 0; i < n; ++i) {
		fprintf(stderr, "--", i);
		if (i < (n - 1)){
			fprintf(stderr, "+");
		}
	}
	fprintf(stderr, "\n");

	for (i = 0; i < n; ++i) {
		fprintf(stderr, "%02X", mBytes[i]);
		if (i < (n - 1)){
			fprintf(stderr, " ");
		}
	}
	fprintf(stderr, "\n");

}


// interface proxy
BytesSourceProxy::BytesSourceProxy(const ByteList& sourceBytes) : mBytes(sourceBytes)
{
}

BytesSourceProxy::~BytesSourceProxy() {

}

size_t BytesSourceProxy::esGetSize() {
	return mBytes.size();
}

uint8_t BytesSourceProxy::esGetAt(unsigned int index) {
	return mBytes.at(index);
}