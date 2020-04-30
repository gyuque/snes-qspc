#include "MMLUtility.h"
#include "MusicDocument.h"
#include "win32/isdir.h"

MusicDocument::MusicDocument()
{
	mOctaveReverseEnabled = false;
	mTempo = 120;
//	mGeneratedTrackLength = 0;
	mRecommendedDuration = 15;
	mpMusicHeaderSource = new BytesSourceProxy(mMusicHeaderBlob);
	mpSeqSource = new BytesSourceProxy(mGeneratedSequenceBlob);
	mpSeqDirSource = new BytesSourceProxy(mSeqDirBlob);
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

	if (mpSeqDirSource) {
		delete mpSeqDirSource;
		mpSeqDirSource = nullptr;
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

MusicTrack* MusicDocument::referTrack(unsigned int i) {
	if (i >= mTrackPtrList.size()) { return nullptr; }
	return mTrackPtrList.at(i);
}

void MusicDocument::validateMetadata() {
	const std::string& t = getTitle();
	const std::string& a = getArtistName();

	if (t.size() == 0) {
		fprintf(stderr, "Warning: Title is empty.\n");
		setTitle("Untitled");
	}

	if (a.size() == 0) {
		fprintf(stderr, "Warning: Artist name is empty.\n");
		setArtistName("Unknown");
	}
}

bool MusicDocument::isInstrumentSetNameSet() const {
	return mInstrumentSetName.size() > 0;
}

const std::string& MusicDocument::getInstrumentSetName() const {
	return mInstrumentSetName;
}

void MusicDocument::calcDataSize(int* poutTrackBufferLength, bool dumpDebugInfo) {
	int totalSize = 0;
	const size_t nTracks = mTrackPtrList.size();
	for (size_t i = 0; i < nTracks; ++i) {
		MusicTrack* t = mTrackPtrList[i];

		const int sz = (int)t->size();
		if (dumpDebugInfo) {
			fprintf(stderr, "T%d: %d bytes\n", i, sz);
		}

		totalSize += sz;
	}


	if (poutTrackBufferLength) {
		*poutTrackBufferLength = totalSize;
	}

	if (dumpDebugInfo) {
		fprintf(stderr, "Entire size: %d\n", totalSize);
	}
}

void MusicDocument::generateSequenceImage(bool bVerbose, bool pausedMode) {
	mGeneratedSequenceBlob.clear();

	//int trackBufferLen;
	//calcDataSize(&trackBufferLen, false);
	//mGeneratedTrackLength = trackBufferLen;

	int ofs = 0;
	const int nTracks = (int)(countTracks());
	for (int i = 0; i < nTracks; ++i) {

		MusicTrack* tr = mTrackPtrList[i];

		int t_size = tr->size();
		for (int j = 0; j < t_size; ++j) {
			mGeneratedSequenceBlob.push_back( tr->at(j) );
		}


		mSeqDirBlob.push_back( (uint8_t)(ofs & 0xFF) );
		mSeqDirBlob.push_back((uint8_t)((ofs >> 8) & 0xFF));

		ofs += t_size;
	}

	generateSESequenceImages(ofs, bVerbose);

	generateHeaderImage(bVerbose, pausedMode);
}

void MusicDocument::generateSESequenceImages(int beginOffset, bool bVerbose) {
	int ofs = beginOffset;

	size_t n = mSETracksRefList.size();
	if (n) {
		fprintf(stderr, "SE tracks enabled: %d\n", n);

		// 8トラック(16B)に満たない分を埋める
		for (; mSeqDirBlob.size() < 16;) {
			mSeqDirBlob.push_back(0xff);
		}
	}

	for (size_t i = 0; i < n; ++i) {
		MusicTrack* pT = mSETracksRefList.at(i);
		int t_size = pT->size();
		for (int j = 0; j < t_size; ++j) {
			mGeneratedSequenceBlob.push_back(pT->at(j));
		}

		mSeqDirBlob.push_back((uint8_t)(ofs & 0xFF));
		mSeqDirBlob.push_back((uint8_t)((ofs >> 8) & 0xFF));

		ofs += t_size;
	}

	if (n) {
		for (; mSeqDirBlob.size() < 256;) {
			mSeqDirBlob.push_back(0xff);
		}
	}
}

static uint8_t calcTimerIntervalForTempo(unsigned int tempo) {
	const double f = 60000.0 / (double)tempo; // 四分音符の長さを計算(ms)
	return (uint8_t)(f / (48.0 * 0.125) + 0.49);
}

void MusicDocument::generateHeaderImage(bool bVerbose, bool pausedMode) {
	mMusicHeaderBlob.clear();
	mMusicHeaderBlob.push_back( countTracks() );
	mMusicHeaderBlob.push_back( mSETracksRefList.size() );
	mMusicHeaderBlob.push_back( calcTimerIntervalForTempo(mTempo) );

	uint8_t pause_flag = pausedMode ? 1 : 0;
	mMusicHeaderBlob.push_back(pause_flag);

	if (bVerbose) {
		fputs("SeqHeader= ", stderr);
		dumpHex(mMusicHeaderBlob);
	}
}

void MusicDocument::writeQuickLoadSizeHeader(unsigned int val) {
	static const size_t pos = 4;
	if (mMusicHeaderBlob.size() != pos) {
		return;
	}

	mMusicHeaderBlob.push_back( val & 0x00FF ); // LO
	mMusicHeaderBlob.push_back( (val >> 8) & 0x00FF); // HI
}

void MusicDocument::dumpSequenceBlob() {
	fprintf(stderr, "Num of tracks: %d\n", countTracks());
//	fprintf(stderr, "Length of track: %d\n", mGeneratedTrackLength);
	dumpHex(mGeneratedSequenceBlob);
	fprintf(stderr, "----------------------------\nOffsets:\n");
	dumpHex(mSeqDirBlob);
}

InstLoadResult MusicDocument::loadInstrumentSet(int verboseLevel) {
	if (!isInstrumentSetNameSet()) { return INSTLD_NOT_SET; }
	mInsts.setVerboseLevel(verboseLevel);

	const char* inst_dirname = mInstrumentSetName.c_str();
	if (!isValidDirectory(inst_dirname)) {
		fprintf(stderr, "'%s' not found\n", inst_dirname);
		return INSTLD_DIR_NOTFOUND;
	}

	std::string manifestPath = mInstrumentSetName + "/manifest.json";
	if (!checkFileExists(manifestPath.c_str())) {
		fprintf(stderr, "'%s' not found\n", manifestPath.c_str() );
		return INSTLD_MANIFEST_NOTFOUND;
	}

	fprintf(stderr, "Loading instrument set: %s\n", manifestPath.c_str());
	const InstLoadResult rv = mInsts.load(manifestPath.c_str(), inst_dirname);
	if (rv != INSTLD_OK) {
		return rv;
	}

	generateFqRegTableFromInsts();
	return INSTLD_OK;
}

void MusicDocument::generateInstrumentDataBinaries(unsigned int baseAddress) {
	mInsts.exportPackedSrcTable(mBRRDirBlob, baseAddress);
	mInsts.exportBRR(mBRRBlockBlob);

	mInsts.exportPackedInstTable(mInstDirBlob);
}

void MusicDocument::generateFqRegTable(double baseFq, int originOctave) {
	RawFqList fq_ls = generateNotesFqTable(originOctave, 6, true);
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

void MusicDocument::addSETrackReference(class MusicDocument* pRefDoc) {
	size_t n = pRefDoc->countTracks();
	for (size_t i = 0; i < n; ++i) {
		mSETracksRefList.push_back( pRefDoc->referTrack(i) );
	}

}

unsigned int MusicDocument::getBrrFixPoint() const {
	return mInsts.getFixPoint();
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