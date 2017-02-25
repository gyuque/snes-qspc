#include "InstrumentSet.h"
#include "../win32/isdir.h"
#include <iostream>
static void array_if(uint8_t& outU8, const picojson::array& a, unsigned int pos);

InstrumentSet::InstrumentSet()
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

bool InstrumentSet::load(const char* manifestPath, const char* baseDir) {

	picojson::object jObj = jsonObjectFromFile(manifestPath);

	if (!jHasProperty(jObj, "base-frequency")) {
		return false;
	}

	mBaseFq = (float)jObj["base-frequency"].get<double>();
	mOriginOctave = (int)jObj["origin-octave"].get<double>();

	readInstDefs(jObj);
	loadBRRBodies(baseDir);
	mBRRPacker.addDummyBlock();

	buildSrcTable();
	writeInstSrcNumbers();

	dumpSrcTable();
	dumpInstTable();
	return true;
}

void InstrumentSet::readInstDefs(picojson::object& parentObj) {
	char buf[8];
	for (int i = 0; i < 128; ++i) {
		sprintf_s(buf, 8, "%d", i);

		if (jHasProperty(parentObj, buf)) {
			picojson::object& def = parentObj[buf].get<picojson::object>();

			if (isInstDefValid(def)) {
				const std::string& brrFilename = def["brr"].get<std::string>();
				addInst(def);
			}
		}
	}
}

bool InstrumentSet::isInstDefValid(const picojson::object& def) {
	return jHasProperty(def, "brr") && jHasProperty(def, "adsr");
}

void InstrumentSet::addInst(const picojson::object& src) {
	InstDef def_s;

	def_s.srcn = -1;
	def_s.filename = src.at("brr").get<std::string>();

	if (jHasProperty(src, "adsr")) {
		const picojson::array& j_adsr = src.at("adsr").get<picojson::array>();

		// 初期値（指定が無くてもそれらしい音が鳴るように）
		def_s.adsr.A   = 15;
		def_s.adsr.D   = 7;
		def_s.adsr.Slv = 7;
		def_s.adsr.R   = 18;

		// 指定値読み込み
		array_if(def_s.adsr.A  , j_adsr, 0);
		array_if(def_s.adsr.D  , j_adsr, 1);
		array_if(def_s.adsr.Slv, j_adsr, 2);
		array_if(def_s.adsr.R  , j_adsr, 3);
	}

	fprintf(stderr, "Inst filename: %s\n", def_s.filename.c_str());

	mInstList.push_back(def_s);
}

bool InstrumentSet::loadBRRBodies(const std::string& baseDir) {
	InstDefList::const_iterator it;
	for (it = mInstList.begin(); it != mInstList.end(); it++) {
		const std::string& fn = it->filename;
		std::string path = baseDir + '/' + fn;
		loadBRRIf(fn, path);
	}

	return true;
}

bool InstrumentSet::loadBRRIf(const std::string& name, const std::string& path) {
	if (findBRR(name)) {
		return true;
	}

	fprintf(stderr, "Loading BRR \"%s\" from %s\n", name.c_str(), path.c_str());
	if (!checkFileExistsRel(path.c_str())) {
		fprintf(stderr, " ^^ NOT FOUND!! ^^\n");
		return false;
	}

	C700BRR* pBRR = new C700BRR(name);
	pBRR->load(path);
	//pBRR->dump();

	mBRRPtrs.push_back(pBRR);
	mBRRPacker.addBRR(pBRR);
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
			if (a_ofs >= 0 && l_ofs >= 0) {
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
	fprintf(stderr, "----+------+------+---------------------\n\n");
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

		fprintf(stderr, "%3d | %4d | %d, %d, %d, %d\n", i, def.srcn, def.adsr.A, def.adsr.D, def.adsr.Slv, def.adsr.R);
	}
}

void array_if(uint8_t& outU8, const picojson::array& a, unsigned int pos) {
	if (pos < a.size()) {
		outU8 = (uint8_t) a.at(pos).get<double>();
	}
}