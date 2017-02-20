#ifndef MUSICDOCUMENT_H_INCLUDED
#define MUSICDOCUMENT_H_INCLUDED

#include <vector>
#include "MMLCommand.h"
#include "Embedder.h"
#include "instrumentset/InstrumentSet.h"

typedef std::vector<class MusicTrack*> TrackPtrList;
typedef std::vector<uint8_t> ByteList;

class BytesSourceProxy;

class MusicDocument
{
public:
	MusicDocument();
	virtual ~MusicDocument();
	void setInstrumentSetName(const std::string& s) { mInstrumentSetName = s; }
	bool isInstrumentSetNameSet() const;
	bool loadInstrumentSet();

	class MusicTrack* appendTrack();
	void calcDataSize(int* poutTrackBufferLength, bool dumpDebugInfo = false);

	void generateSequenceImage();
	size_t countTracks() const;

	void dumpSequenceBlob();

	int mGeneratedTrackLength;
	ByteList mGeneratedSequenceBlob;

	class BytesSourceProxy* referSequenceBytesSource() { return mpSeqSource; }
protected:
	class BytesSourceProxy* mpSeqSource;
	
	std::string mInstrumentSetName;
	InstrumentSet mInsts;


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

class BytesSourceProxy : public IEmbedderSource {
public:
	BytesSourceProxy(const ByteList& sourceBytes);
	virtual ~BytesSourceProxy();

	virtual size_t esGetSize();
	virtual uint8_t esGetAt(unsigned int index);
protected:
	const ByteList& mBytes;
};

#endif