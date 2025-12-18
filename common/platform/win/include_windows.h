#pragma once

#ifdef _WINDOWS

#ifndef NOMINMAX
#define NOMINMAX
#endif 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#ifdef GetClassName
#undef GetClassName
#endif

#endif // _WINDOWS
