#ifndef MUSICDOCUMENT_H_INCLUDED
#define MUSICDOCUMENT_H_INCLUDED

#include <vector>
#include "MMLCommand.h"
#include "Embedder.h"

typedef std::vector<class MusicTrack*> TrackPtrList;
typedef std::vector<uint8_t> ByteList;

class MusicDocument
{
public:
	MusicDocument();
	virtual ~MusicDocument();

	class MusicTrack* appendTrack();
	void calcDataSize(int* poutTrackBufferLength, bool dumpDebugInfo = false);

	void generateSequenceImage();
	size_t countTracks() const;

	void dumpSequenceBlob();

	int mGeneratedTrackLength;
	ByteList mGeneratedSequenceBlob;
protected:
	void releaseAllTracks();
	TrackPtrList mTrackPtrList;
};


class MusicTrack
{
public:
	MusicTrack();
	virtual ~MusicTrack();

	int size() const;
	void addByte(uint8_t b);
	uint8_t at(unsigned int position);

	void dump();
protected:
	ByteList mBytes;
};

#endif