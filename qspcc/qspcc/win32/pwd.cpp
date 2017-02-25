#include "pwd.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <regex>
#include <iostream>

std::string getSelfDir() {
	TCHAR selfPath[MAX_PATH];
	::GetModuleFileName(NULL, selfPath, MAX_PATH);

	std::regex re("[^\\\\/]+$");
	std::regex re_term("[\\\\/]+$");
	const std::string& mids = std::regex_replace(selfPath, re, "");
	std::string dirOnly = std::regex_replace(mids.c_str(), re_term, "");

	return dirOnly;
}