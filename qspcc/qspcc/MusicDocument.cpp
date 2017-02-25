#include "MusicDocument.h"
#include "win32/isdir.h"

MusicDocument::MusicDocument()
{
	mGeneratedTrackLength = 0;
	mpSeqSource = new BytesSourceProxy(mGeneratedSequenceBlob);
}


MusicDocument::~MusicDocument()
{
	releaseAllTracks();

	if (mpSeqSource) {
		delete mpSeqSource;
		mpSeqSource = NULL;
	}
}

void MusicDocument::releaseAllTracks() {
	const size_t n = mTrackPtrList.size();
	for (size_t i = 0; i < n; ++i) {
		if (mTrackPtrList[i]) {
			delete mTrackPtrList[i];
			mTrackPtrList[i] = NULL;
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
}

void MusicDocument::dumpSequenceBlob() {
	fprintf(stderr, "Num of tracks: %d\n", countTracks());
	fprintf(stderr, "Length of track: %d\n", mGeneratedTrackLength);
	size_t n = mGeneratedSequenceBlob.size();
	for (size_t i = 0; i < n; ++i) {
		const int col = i % 16;

		if (col == 0) {
			fprintf(stderr, "%04X | ", i);
		}

		fprintf(stderr, "%02X", mGeneratedSequenceBlob.at(i));

		if (col == 15) {
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, " ");
		}
	}
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

	return true;
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