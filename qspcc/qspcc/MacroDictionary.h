#ifndef MACRODICTIONARY_H_INCLUDED
#define MACRODICTIONARY_H_INCLUDED

#include "MMLTokenizer.h"
#include <string>
#include <map>

typedef struct _MacroDefinition {
	MMLTokenList tokenList;
} MacroDefinition;

typedef std::map<std::string, MacroDefinition> MacroDefNameMap;

class MacroDictionary {
public:
	MacroDictionary();
	virtual ~MacroDictionary();

	void registerFromExpr(const MMLExprStruct& srcExpr);
	bool exists(const std::string& name) const;

	const MacroDefinition& referMacroDefinition(const std::string& name) const;
protected:
	MacroDefNameMap mNameMap;
};

#endif