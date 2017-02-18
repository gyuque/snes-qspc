#ifndef IMMLERROR_H_INCLUDED
#define IMMLERROR_H_INCLUDED
#include <string>

class IMMLError {
public:
	virtual void raiseError(int lineno, int charno, const char* relStr, const std::string& message) = 0;
};

#endif