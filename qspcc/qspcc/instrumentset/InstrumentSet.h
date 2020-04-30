#ifndef INSTRUMENTSET_H_INCLUDED
#define INSTRUMENTSET_H_INCLUDED

#include "../json/PicoJSONUtils.h"
#include "C700BRR.h"
#include "BRRPacker.h"
#include <vector>

#define DUMMY_INST_WAV_NAME "*dmy*"

typedef struct _InstADSR {
	uint8_t A;
	uint8_t D;
	uint8_t Slv;
	uint8_t R;
} InstADSR;

typedef struct _InstDef {
	std::string filename;
	int srcn;
	InstADSR adsr;
	unsigned int fixPoint;
	int priority;
} InstDef;

typedef struct _WavSrc {
	std::string brrName;
	unsigned int attackOffset;
	unsigned int loopOffset;
} WavSrc;

typedef std::vector<C700BRR*> BRRPtrList;
typedef std::vector<InstDef> InstDefList;
typedef std::vector<WavSrc> SrcList;

typedef enum {
	INSTS_OK = 0,
	INSTS_ERR_BADDEF = -8,
	INSTS_ERR_DUPFIX = -9
} InstsResult;

class InstrumentSet
{
public:
	InstrumentSet();
	virtual ~InstrumentSet();
	void setVerboseLevel(int lv);

	InstLoadResult load(const char* manifestPath, const char* baseDir);

	C700BRR* findBRR(const std::string& name) const;
	static void setDefaultADSR(InstADSR& outADSR);
	static uint8_t makeADregister(const InstADSR& inADSR);
	static uint8_t makeSRregister(const InstADSR& inADSR);

	void dumpPackedBRR();
	void exportBRR(ByteList& outBytes) const;
	void exportPackedSrcTable(ByteList& outBytes, unsigned int baseAddr);
	void exportPackedInstTable(ByteList& outBytes);

	float getBaseEq() const { return mBaseFq; }
	int getOriginOctave() const { return mOriginOctave; }

	unsigned int getFixPoint() const { return mFirstFixPoint; }
protected:
	int mVerboseLevel;

	float mBaseFq;
	int mOriginOctave;
	InstDefList mInstList;
	unsigned int mMaxInstIndex;
	unsigned int mFirstFixPoint;

	unsigned int findMaxInst(const picojson::object& parentObj);
	InstsResult readInstDefs(picojson::object& parentObj);
	static bool isInstDefValid(const picojson::object& def);

	InstsResult addInst(const picojson::object& src);
	void addDummyInst();
	bool loadBRRBodies(const std::string& baseDir);
	bool loadBRRIf(const std::string& name, const std::string& path, unsigned int fixPoint);
	void releaseAllBRRs();

	const WavSrc* findSrcWithName(const std::string& name, int* pOutIndex = NULL) const;
	void buildSrcTable();
	void dumpSrcTable();

	void writeInstSrcNumbers();
	void dumpInstTable();

	BRRPtrList mBRRPtrs;
	BRRPacker mBRRPacker;
	SrcList mSrcList;
};

#endif