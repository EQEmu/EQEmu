#pragma once

#include "common/strings.h"

#include <cstdio>

#if _WIN32
#define popen _popen
#define pclose _pclose
#endif

class Process {
public:
	static std::string execute(const std::string &cmd);
};
