#ifndef MUSICDOCUMENT_H_INCLUDED
#define MUSICDOCUMENT_H_INCLUDED

#include <vector>
#include "mml_types.h"
#include "MMLCommand.h"
#include "Embedder.h"
#include "instrumentset/InstrumentSet.h"
#include "FrequencyTable.h"

typedef std::vector<class MusicTrack*> TrackPtrList;

class BytesSourceProxy;

class MusicDocument
{
public:
	MusicDocument();
	virtual ~MusicDocument();
	void setTempo(unsigned int t) { mTempo = t; }
	void setInstrumentSetName(const std::string& s) { mInstrumentSetName = s; }
	void setTitle(const std::string& s) { mTitle = s; }
	void setArtistName(const std::string& s) { mArtistName = s; }

	void validateMetadata();

	const std::string& getTitle() const { return mTitle; }
	const std::string& getArtistName() const { return mArtistName; }
	const std::string& getInstrumentSetName() const;

	bool isInstrumentSetNameSet() const;
	InstLoadResult loadInstrumentSet(int verboseLevel);
	void generateInstrumentDataBinaries(unsigned int baseAddress);

	class MusicTrack* appendTrack();
	void calcDataSize(int* poutTrackBufferLength, bool dumpDebugInfo = false);

	void generateSequenceImage(bool bVerbose);
	size_t countTracks() const;

	void dumpSequenceBlob();

	int mGeneratedTrackLength;

	class BytesSourceProxy* referMusicHeaderSource() { return mpMusicHeaderSource; }
	class BytesSourceProxy* referFqTableBytesSource() { return mpFqTableSource; }
	class BytesSourceProxy* referSequenceBytesSource() { return mpSeqSource; }
	class BytesSourceProxy* referInstDirBytesSource() { return mpInstDirSource; }
	class BytesSourceProxy* referBRRDirBytesSource() { return mpBRRDirSource; }
	class BytesSourceProxy* referBRRBodyBytesSource() { return mpBRRBlockSource; }
protected:
	unsigned int mTempo;

	// FqTable
	FqRegisterValueList mFqRegTable;
	void generateFqRegTable(double baseFq, int originOctave);
	void generateFqRegTableFromInsts();

	class BytesSourceProxy* mpMusicHeaderSource;
	class BytesSourceProxy* mpSeqSource;
	class BytesSourceProxy* mpInstDirSource;
	class BytesSourceProxy* mpBRRDirSource;
	class BytesSourceProxy* mpBRRBlockSource;

	class BytesSourceProxy* mpFqTableSource;

	ByteList mMusicHeaderBlob;
	ByteList mGeneratedSequenceBlob;
	ByteList mInstDirBlob;
	ByteList mBRRDirBlob;
	ByteList mBRRBlockBlob;
	ByteList mFqTableBlob;

	std::string mTitle;
	std::string mArtistName;
	std::string mInstrumentSetName;
	InstrumentSet mInsts;

	void generateHeaderImage(bool bVerbose);
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