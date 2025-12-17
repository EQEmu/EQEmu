#pragma once

//headers from the Perl distribution
#include <EXTERN.h>
#define WIN32IO_IS_STDIO

// header to detect if libc++ is the std lib
// this doesn't do shit for C++ but libc++ and GCC 6.1+ use it to define some macros
#include <ciso646>

#ifndef WIN32
extern "C" {	//the perl headers dont do this for us...
#endif
#if _MSC_VER
#define __inline__ __inline
#define __builtin_expect 
#endif
#include <perl.h>
#include <XSUB.h>
#ifndef WIN32
};
#endif

#ifdef WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

//perl defines these macros and dosent clean them up, lazy bastards. -- I hate them too!
#ifdef Copy
#undef Copy
#endif

#ifdef list
#undef list
#endif

#ifdef write
#undef write
#endif

#ifdef bool
#undef bool
#endif

#ifdef Zero
#undef Zero
#endif

#ifdef THIS /* this macro seems to leak out on some systems */
#undef THIS
#endif

//These need to be cleaned up on FreeBSD
#if defined(__FreeBSD__) || defined(_LIBCPP_VERSION)
#ifdef do_open
#undef do_open
#endif

#ifdef do_close
#undef do_close
#endif
#endif
