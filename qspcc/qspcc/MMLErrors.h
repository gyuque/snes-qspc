#ifndef MMLERRORS_H_INCLUDED
#define MMLERRORS_H_INCLUDED

#include <string>

#define MSGLNG_UNKNOWN  0
#define MSGLNG_ENGLISH  1
#define MSGLNG_JAPANESE 2

class MMLErrors
{
public:
	static int getCurrentLanguage();

	static std::string getErrorString(int errorId);

	static const int E_UNKNOWN_CHAR    = 1;
	static const int E_UNKNOWN_EXPR    = 11;
	static const int E_NESTED_TUP      = 21;
	static const int E_NESTED_REPEAT   = 22;
	static const int E_RECURSIVE_MACRO = 25;
	static const int E_NOTFOUND_MACRO  = 26;
	static const int E_INST_NOTSET     = 80;
	static const int E_INST_NOTFOUND   = 81;
	static const int E_INST_M_NOTFOUND = 82;
	static const int E_INST_M_BAD      = 83;
	static const int E_INST_B_NOTFOUND = 84;
	static const int E_INST_MULTI_FIX  = 85;
	static const int E_INST_EXCEED_FIX = 86;
	static const int E_INST_BAD_FIXBRR = 87;
	static const int E_EMB_CAPACITY_EX = 90;
	static const int E_TOO_MANY_FILES  = 92;

	static const int MSG_COMPILING  = 101;
	static const int MSG_EMBEDEXP   = 102;
	static const int MSG_EXPORTING  = 103;
	static const int MSG_FIXENABLED = 104;
	static const int MSG_SUCCESSS   = 110;

protected:
	MMLErrors();
	virtual ~MMLErrors();
};

#endif