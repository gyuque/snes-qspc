#include "../MMLUtility.h"
#include "InstrumentSet.h"
#include "../win32/isdir.h"
#include <iostream>
static void array_if(uint8_t& outU8, const picojson::array& a, unsigned int pos);

InstrumentSet::InstrumentSet() : mMaxInstIndex(0), mFirstFixPoint(0)
{
}


InstrumentSet::~InstrumentSet()
{
	releaseAllBRRs();
}

void InstrumentSet::releaseAllBRRs() {
	BRRPtrList::iterator it;

	for (it = mBRRPtrs.begin(); it != mBRRPtrs.end(); it++) {
		C700BRR* p = *it;
		delete p;
	}

	mBRRPtrs.clear();
}

void InstrumentSet::setVerboseLevel(int lv) {
	mVerboseLevel = lv;
}

InstLoadResult InstrumentSet::load(const char* manifestPath, const char* baseDir) {

	picojson::object jObj = jsonObjectFromFile(manifestPath);
	if (!jHasProperty(jObj, "base-frequency")) {
		return INSTLD_BAD_MANIFEST;
	}

	mBaseFq = (float)jObj["base-frequency"].get<double>();
	mOriginOctave = (int)jObj["origin-octave"].get<double>();

	if (readInstDefs(jObj) != INSTS_OK) {
		return INSTLD_BAD_MANIFEST;
	}

	if (!loadBRRBodies(baseDir)) {
		return INSTLD_BRR_NOTFOUND;
	}
	mBRRPacker.addDummyBlock();

	buildSrcTable();
	writeInstSrcNumbers();

	if (mVerboseLevel > 0) {
		dumpSrcTable();
		dumpInstTable();
		if (mVerboseLevel > 1) {
			dumpPackedBRR();
		}
	}
	return INSTLD_OK;
}

unsigned int InstrumentSet::findMaxInst(const picojson::object& parentObj) {
	char buf[8];
	int m = 0;

	for (int i = 0; i < 128; ++i) {
		sprintf_s(buf, 8, "%d", i);

		if (jHasProperty(parentObj, buf)) {
			m = i;
		}
	}

	return m;
}

InstsResult InstrumentSet::readInstDefs(picojson::object& parentObj) {
	mMaxInstIndex = findMaxInst(parentObj);

	char buf[8];
	for (unsigned int i = 0; i <= mMaxInstIndex; ++i) {
		sprintf_s(buf, 8, "%d", i);

		if (jHasProperty(parentObj, buf)) {
			picojson::object& def = parentObj[buf].get<picojson::object>();

			if (isInstDefValid(def)) {
				const std::string& brrFilename = def["brr"].get<std::string>();
				const auto ares = addInst(def);
				if (ares != INSTS_OK) {
					return ares;
				}
			} else {
				return INSTS_ERR_BADDEF;
			}
		} else {
			addDummyInst();
		}
	}

	return INSTS_OK;
}

bool InstrumentSet::isInstDefValid(const picojson::object& def) {
	return jHasProperty(def, "brr") && jHasProperty(def, "adsr");
}

void InstrumentSet::setDefaultADSR(InstADSR& outADSR) {
	outADSR.A = 15;
	outADSR.D = 7;
	outADSR.Slv = 7;
	outADSR.R = 18;
}

InstsResult InstrumentSet::addInst(const picojson::object& src) {
	InstDef def_s;

	def_s.fixPoint = 0;
	def_s.priority = 0;
	def_s.srcn = -1;
	def_s.filename = src.at("brr").get<std::string>();

	if (jHasProperty(src, "adsr")) {
		const picojson::array& j_adsr = src.at("adsr").get<picojson::array>();

		// 初期値（指定が無くてもそれらしい音が鳴るように）
		setDefaultADSR(def_s.adsr);

		// 指定値読み込み
		array_if(def_s.adsr.A  , j_adsr, 0);
		array_if(def_s.adsr.D  , j_adsr, 1);
		array_if(def_s.adsr.Slv, j_adsr, 2);
		array_if(def_s.adsr.R  , j_adsr, 3);
	}

	// - アドレス固定 -
	// ドライバの部分入れ替えをする場合は、固定BRRの先頭アドレスを指定する(ここより後は複数のinstsで同じになるように)
	if (jHasProperty(src, "fix")) {
		def_s.fixPoint = (unsigned int)src.at("fix").get<double>();
	}

	if (jHasProperty(src, "priority")) {
		def_s.priority = 1;
	}

	if (mVerboseLevel > 0) {
		fprintf(stderr, "Inst filename: %s\n", def_s.filename.c_str());
	}

	if (def_s.fixPoint) {
		if (mFirstFixPoint) {
			// 既に指定がある場合: 複数指定不可エラー
			std::cerr << "Can't set fix point again.";
			return INSTS_ERR_DUPFIX;
		} else {
			mFirstFixPoint = def_s.fixPoint;
		}
	}

	mInstList.push_back(def_s);
	return INSTS_OK;
}

void InstrumentSet::addDummyInst() {
	InstDef def_s;
	def_s.fixPoint = 0;
	def_s.srcn = 0;
	def_s.filename = DUMMY_INST_WAV_NAME;
	setDefaultADSR(def_s.adsr);

	mInstList.push_back(def_s);
}

bool InstrumentSet::loadBRRBodies(const std::string& baseDir) {
	InstDefList::const_iterator it;
	for (auto phase = 0; phase < 2; ++phase) {
		// 1周目 -> priority=1 をロード
		// 2周目 -> priority=0 をロード
		const auto prio_to_load = 1 - phase;

		for (it = mInstList.begin(); it != mInstList.end(); it++) {
			if (it->priority != prio_to_load) { continue; }

			const std::string& fn = it->filename;

			if (0 == fn.compare(DUMMY_INST_WAV_NAME)) {
				// ダミーなのでBRRファイルは存在しない
				continue;
			}

			std::string path = baseDir + '/' + fn;
			if (!loadBRRIf(fn, path, it->fixPoint)) {
				return false;
			}
		}
	}

	return true;
}

bool InstrumentSet::loadBRRIf(const std::string& name, const std::string& path, unsigned int fixPoint) {
	if (findBRR(name)) {
		return true;
	}

	fprintf(stderr, "Loading BRR \"%s\" from %s\n", name.c_str(), path.c_str());
	if (!checkFileExistsRel(path.c_str())) {
		fprintf(stderr, " ^^ NOT FOUND!! ^^\n");
		return false;
	}

	C700BRR* pBRR = new C700BRR(name);
	pBRR->load(path, mVerboseLevel > 0);
	//pBRR->dump();

	mBRRPtrs.push_back(pBRR);
	mBRRPacker.addBRR(pBRR, fixPoint, mVerboseLevel > 0);
	return true;
}

C700BRR* InstrumentSet::findBRR(const std::string& name) const {
	BRRPtrList::const_iterator it;
	for (it = mBRRPtrs.begin(); it != mBRRPtrs.end(); it++) {
		if (0 == (*it)->getName().compare(name)) {
			return *it;
		}
	}

	return NULL;
}

const WavSrc* InstrumentSet::findSrcWithName(const std::string& name, int* pOutIndex) const {
	int index = 0;
	SrcList::const_iterator it;
	for (it = mSrcList.begin(); it != mSrcList.end(); it++) {
		const WavSrc& s = *it;
		if (0 == s.brrName.compare(name)) {
			if (pOutIndex) {
				*pOutIndex = index;
			}

			return &s;
		}

		++index;
	}

	return NULL;
}

void InstrumentSet::buildSrcTable() {
	const int n = mInstList.size();
	for (int i = 0; i < n; ++i) {
		InstDef& def = mInstList[i];
		
		const WavSrc* existingSrc = findSrcWithName(def.filename);
		if (!existingSrc) {
			int a_ofs = mBRRPacker.getAttackOffsetByName(def.filename);
			int l_ofs = mBRRPacker.getLoopOffsetByName(def.filename, true);
			if (mVerboseLevel > 0) {
				fprintf(stderr, " <<<<< %d  %d \n", a_ofs, l_ofs);
			}

			if (a_ofs < 0 && l_ofs >= 0) {
				// loop only
				WavSrc src;
				src.brrName = def.filename;
				src.attackOffset = l_ofs;
				src.loopOffset = l_ofs;

				mSrcList.push_back(src);
			} else if (a_ofs >= 0 && l_ofs >= 0) {
				WavSrc src;
				src.brrName = def.filename;
				src.attackOffset = a_ofs;
				src.loopOffset = l_ofs;

				mSrcList.push_back(src);
			}
		}
	}
}

void InstrumentSet::dumpSrcTable() {
	const int n = mSrcList.size();

	fprintf(stderr, "============== SRC TABLE ==============\n");
	fprintf(stderr, "Idx | Atck | Loop | Name\n");
	fprintf(stderr, "----+------+------+--------------------\n");
	for (int i = 0; i < n; ++i) {
		const WavSrc& s = mSrcList[i];
		fprintf(stderr, "%3d | %04X | %04X | %s\n", i, s.attackOffset, s.loopOffset, s.brrName.c_str());
	}
	fprintf(stderr, "----+------+------+---------------------\nBin:\n");

	ByteList stbytes;
	exportPackedSrcTable(stbytes, 0);
	dumpHex(stbytes);
	fprintf(stderr, "----------------------------------------\n\n");
}

void InstrumentSet::writeInstSrcNumbers() {
	const int n = mInstList.size();
	for (int i = 0; i < n; ++i) {
		InstDef& def = mInstList[i];
		findSrcWithName(def.filename, &(def.srcn));
	}
}

void InstrumentSet::dumpInstTable() {
	fprintf(stderr, "============== INST TABLE ==============\n");
	fprintf(stderr, "Idx | Srcn | ADSR\n");
	fprintf(stderr, "----+------+----------------------------\n");

	const int n = mInstList.size();
	for (int i = 0; i < n; ++i) {
		const InstDef& def = mInstList[i];

		char dmy_mark = ' ';
		if (0 == def.filename.compare(DUMMY_INST_WAV_NAME)) { dmy_mark = '*'; }
		fprintf(stderr, "%3d%c| %4d | %d, %d, %d, %d\n", i, dmy_mark, def.srcn, def.adsr.A, def.adsr.D, def.adsr.Slv, def.adsr.R);
	}

	fprintf(stderr, "----+------+----------------------------\nBin:\n");
	ByteList bs;
	exportPackedInstTable(bs);
	dumpHex(bs);
	fprintf(stderr, "----------------------------------------\n\n");
}

void InstrumentSet::dumpPackedBRR() {
	fprintf(stderr, "BRR body:\n");

	ByteList bytes;
	mBRRPacker.exportAll(bytes);
	dumpHex(bytes);
}

void InstrumentSet::exportPackedSrcTable(ByteList& outBytes, unsigned int baseAddr) {
	const int n = mSrcList.size();

	for (int i = 0; i < n; ++i) {
		const WavSrc& s = mSrcList[i];
		const unsigned int a = baseAddr + s.attackOffset;
		const unsigned int l = baseAddr + s.loopOffset;

		outBytes.push_back(a & 0xFF);
		outBytes.push_back((a >> 8) & 0xFF);
		outBytes.push_back(l & 0xFF);
		outBytes.push_back((l >> 8) & 0xFF);
	}
}

void InstrumentSet::exportPackedInstTable(ByteList& outBytes) {
	const int n = mInstList.size();
	for (int i = 0; i < n; ++i) {
		const InstDef& def = mInstList[i];
		
		outBytes.push_back(def.srcn);
		outBytes.push_back(makeADregister(def.adsr));
		outBytes.push_back(makeSRregister(def.adsr));
		outBytes.push_back(0);
	}
}

void InstrumentSet::exportBRR(ByteList& outBytes) const {
	mBRRPacker.exportAll(outBytes);
}

uint8_t InstrumentSet::makeADregister(const InstADSR& inADSR) {
	//     v- ENABLE bit
	return 0x80         | (inADSR.A & 0x0F) | ((inADSR.D & 0x07) << 4);
}

uint8_t InstrumentSet::makeSRregister(const InstADSR& inADSR) {
	return ((inADSR.Slv & 0x07) << 5) | (inADSR.R & 0x1F);
}

void array_if(uint8_t& outU8, const picojson::array& a, unsigned int pos) {
	if (pos < a.size()) {
		outU8 = (uint8_t) a.at(pos).get<double>();
	}
}
