#ifndef INSTRUMENTSET_H_INCLUDED
#define INSTRUMENTSET_H_INCLUDED

#include "../json/PicoJSONUtils.h"
#include "C700BRR.h"
#include "BRRPacker.h"
#include <vector>

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
} InstDef;

typedef struct _WavSrc {
	std::string brrName;
	unsigned int attackOffset;
	unsigned int loopOffset;
} WavSrc;

typedef std::vector<C700BRR*> BRRPtrList;
typedef std::vector<InstDef> InstDefList;
typedef std::vector<WavSrc> SrcList;

class InstrumentSet
{
public:
	InstrumentSet();
	virtual ~InstrumentSet();

	bool load(const char* manifestPath, const char* baseDir);

	C700BRR* findBRR(const std::string& name) const;
protected:
	float mBaseFq;
	int mOriginOctave;
	InstDefList mInstList;

	void readInstDefs(picojson::object& parentObj);
	static bool isInstDefValid(const picojson::object& def);

	void addInst(const picojson::object& src);
	bool loadBRRBodies(const std::string& baseDir);
	bool loadBRRIf(const std::string& name, const std::string& path);
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