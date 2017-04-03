#include "MacroDictionary.h"



MacroDictionary::MacroDictionary() {

}

MacroDictionary::~MacroDictionary() {

}

void MacroDictionary::registerFromExpr(const MMLExprStruct& srcExpr) {
	const int n = (int)(srcExpr.tokenList.size());
	if (n < 2) {
		return;
	}

	const std::string& name = srcExpr.tokenList.at(0).rawStr;

	const int numInner = n - 2;
	fprintf(stderr, "[[ %d toks ]]\n", numInner);

	MacroDefinition& def = mNameMap[name];
	def.tokenList.clear();
	for (int i = 0; i < numInner; ++i) {
		const MMLTokenStruct& tok = srcExpr.tokenList.at(i+2);

		def.tokenList.push_back(tok);

		fprintf(stderr, "  [[ %s ]]\n", tok.rawStr.c_str());
	}
}

bool MacroDictionary::exists(const std::string& name) const {
	return mNameMap.find(name) != mNameMap.end();
}

const MacroDefinition& MacroDictionary::referMacroDefinition(const std::string& name) const {
	return mNameMap.at(name);
}