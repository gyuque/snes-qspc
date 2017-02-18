#include "MusicDocument.h"


MusicDocument::MusicDocument()
{
}


MusicDocument::~MusicDocument()
{
	releaseAllTracks();
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

MusicTrack* MusicDocument::appendTrack() {
	MusicTrack* t = new MusicTrack();
	mTrackPtrList.push_back(t);
	return t;
}

void MusicDocument::calcDataSize(bool dumpDebugInfo) {
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

	if (dumpDebugInfo) {
		const int aligned_size = ((max_size / 256) + 1) * 256;
		fprintf(stderr, "Max size per track: %d\n", max_size);
		fprintf(stderr, "Aligned track size: %d\n", aligned_size);
		fprintf(stderr, "Entire size: %d\n", aligned_size * nTracks);
	}
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