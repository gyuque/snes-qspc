#ifndef MMLERRORS_H_INCLUDED
#define MMLERRORS_H_INCLUDED

#include <string>

class MMLErrors
{
public:
	static std::string getErrorString(int errorId);

	static const int E_UNKNOWN_CHAR    = 1;
	static const int E_UNKNOWN_EXPR    = 11;
	static const int E_NESTED_TUP      = 21;
	static const int E_NESTED_REPEAT   = 22;
	static const int E_RECURSIVE_MACRO = 25;
protected:
	MMLErrors();
	virtual ~MMLErrors();
};

#endif