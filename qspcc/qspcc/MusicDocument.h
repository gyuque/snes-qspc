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
	void setRecommendedDuration(unsigned int d) { mRecommendedDuration = d; }
	void setComment(const std::string& s) { mComment = s; }
	void setCoderName(const std::string& s) { mCoderName = s; }
	void setGameTitle(const std::string& s) { mGameTitle = s; }

	void validateMetadata();

	const std::string& getTitle() const { return mTitle; }
	const std::string& getArtistName() const { return mArtistName; }
	const std::string& getComment() const { return mComment; }
	const std::string& getCoderName() const { return mCoderName; }
	const std::string& getGameTitle() const { return mGameTitle; }
	const std::string& getInstrumentSetName() const;
	const unsigned int getRecommendedDuration() const { return mRecommendedDuration; }

	bool isInstrumentSetNameSet() const;
	InstLoadResult loadInstrumentSet(int verboseLevel);
	void generateInstrumentDataBinaries(unsigned int baseAddress);
	const InstrumentSet& referInstrumentSet() const { return mInsts; };
	unsigned int getBrrFixPoint() const;
	void writeQuickLoadSizeHeader(unsigned int val);

	class MusicTrack* appendTrack();
	class MusicTrack* referTrack(unsigned int i);
	void calcDataSize(int* poutTrackBufferLength, bool dumpDebugInfo = false);

	void generateSequenceImage(bool bVerbose, bool pausedMode);
	size_t countTracks() const;

	void dumpSequenceBlob();

	// int mGeneratedTrackLength;

	void setOctaveReverseEnabled(bool b) { mOctaveReverseEnabled = b; }
	bool getOctaveReverseEnabled() const { return mOctaveReverseEnabled; }

	class BytesSourceProxy* referMusicHeaderSource() { return mpMusicHeaderSource; }
	class BytesSourceProxy* referFqTableBytesSource() { return mpFqTableSource; }
	class BytesSourceProxy* referSequenceBytesSource() { return mpSeqSource; }
	class BytesSourceProxy* referSeqDirBytesSource() { return mpSeqDirSource; }
	class BytesSourceProxy* referInstDirBytesSource() { return mpInstDirSource; }
	class BytesSourceProxy* referBRRDirBytesSource() { return mpBRRDirSource; }
	class BytesSourceProxy* referBRRBodyBytesSource() { return mpBRRBlockSource; }

	void addSETrackReference(class MusicDocument* pRefDoc);
protected:
	void generateSESequenceImages(int beginOffset, bool bVerbose);

	bool mOctaveReverseEnabled;
	unsigned int mTempo;
	unsigned int mRecommendedDuration;

	// FqTable
	FqRegisterValueList mFqRegTable;
	void generateFqRegTable(double baseFq, int originOctave);
	void generateFqRegTableFromInsts();

	class BytesSourceProxy* mpMusicHeaderSource;
	class BytesSourceProxy* mpSeqSource;
	class BytesSourceProxy* mpSeqDirSource;
	class BytesSourceProxy* mpInstDirSource;
	class BytesSourceProxy* mpBRRDirSource;
	class BytesSourceProxy* mpBRRBlockSource;

	class BytesSourceProxy* mpFqTableSource;

	ByteList mMusicHeaderBlob;
	ByteList mGeneratedSequenceBlob;
	ByteList mSeqDirBlob;
	ByteList mInstDirBlob;
	ByteList mBRRDirBlob;
	ByteList mBRRBlockBlob;
	ByteList mFqTableBlob;

	std::string mTitle;
	std::string mArtistName;
	std::string mInstrumentSetName;
	std::string mComment;
	std::string mCoderName;
	std::string mGameTitle;
	InstrumentSet mInsts;

	void generateHeaderImage(bool bVerbose, bool pausedMode);
	void releaseAllTracks();
	TrackPtrList mTrackPtrList;

	TrackPtrList mSETracksRefList;
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