#ifndef MUSICDOCUMENT_H_INCLUDED
#define MUSICDOCUMENT_H_INCLUDED

#include <vector>
#include "MMLCommand.h"

typedef std::vector<class MusicTrack*> TrackPtrList;
typedef std::vector<uint8_t> ByteList;

class MusicDocument
{
public:
	MusicDocument();
	virtual ~MusicDocument();

	class MusicTrack* appendTrack();
	void calcDataSize(bool dumpDebugInfo = false);
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

	void dump();
protected:
	ByteList mBytes;
};

#endif