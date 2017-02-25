#include "isdir.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlwapi.h>
#include <string>

bool checkFileExistsRel(const char* path) {
	return !!(::PathFileExists(path));
}

bool checkFileExistsAbs(const char* path) {
	return !!( ::PathFileExists(path) );
}

bool checkFileExists(const char* path) {
	char buf[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, buf);
	std::string fullpath = buf;
	fullpath += "\\";
	fullpath += path;

	return checkFileExistsAbs(fullpath.c_str());
}

bool isValidDirectory(const char* path) {
	char buf[MAX_PATH];
	::GetCurrentDirectory(MAX_PATH, buf);
	std::string fullpath = buf;
	fullpath += "\\";
	fullpath += path;

	if (!checkFileExistsAbs(fullpath.c_str())) { return false; }

	return !!( ::PathIsDirectory(fullpath.c_str()) );
}
