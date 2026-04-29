#pragma once

#if defined(_MSC_VER)
    #define PUSH_DISABLE_DEPRECATED_WARNINGS() __pragma(warning(push)) \
                                               __pragma(warning(disable:4996))
    #define POP_DISABLE_DEPRECATED_WARNINGS()  __pragma(warning(pop))
#elif defined(__GNUC__) || defined(__clang__)
    #define PUSH_DISABLE_DEPRECATED_WARNINGS() _Pragma("GCC diagnostic push") \
                                               _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
    #define POP_DISABLE_DEPRECATED_WARNINGS()  _Pragma("GCC diagnostic pop")
#else
    #define PUSH_DISABLE_DEPRECATED_WARNINGS()
    #define POP_DISABLE_DEPRECATED_WARNINGS()
#endif

#if defined(_MSC_VER) && !defined(__clang__)
    #define UNREACHABLE() __assume(0)
#else
    #define UNREACHABLE() __builtin_unreachable()
#endif