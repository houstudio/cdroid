/* This file is part of cairomm. */

/* Defined when the --enable-api-exceptions configure argument was given */
#define CAIROMM_EXCEPTIONS_ENABLED 1

/* Major version number of cairomm. */
#define CAIROMM_MAJOR_VERSION 1

/* Minor version number of cairomm. */
#define CAIROMM_MINOR_VERSION 19 

/* Micro version number of cairomm. */
#define CAIROMM_MICRO_VERSION 0
#ifdef CAIROMM_DLL
# if defined(CAIROMM_BUILD)
#  define CAIROMM_API __declspec(dllexport)
# else
#  define CAIROMM_API __declspec(dllimport)
# endif
/* Build a static or non-native-Windows library */
#else
# define CAIROMM_API
#endif /* CAIROMM_DLL */

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define _ALLOW_KEYWORD_MACROS 1
#define noexcept _NOEXCEPT
#endif
