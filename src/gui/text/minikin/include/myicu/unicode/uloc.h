#ifndef __ULOC_H__
#define __ULOC_H__
#include "unicode/utypes.h"

#define ULOC_FULLNAME_CAPACITY 156
#define ULOC_LANG_CAPACITY 80

U_CAPI int32_t U_EXPORT2
uloc_canonicalize(const char* locale, char* result, int32_t maxResultSize, UErrorCode* status);

U_CAPI int32_t U_EXPORT2
uloc_addLikelySubtags(const char* locale, char* result, int32_t maxResultSize, UErrorCode* status);

U_CAPI int32_t U_EXPORT2
uloc_toLanguageTag(const char* locale, char* result, int32_t maxResultSize, UBool rfc3066, UErrorCode* status);

U_CAPI int32_t U_EXPORT2
uloc_forLanguageTag(const char* languageTag, char* result, int32_t maxResultSize, void* options, UErrorCode* status);

U_CAPI const char* U_EXPORT2
u_errorName(UErrorCode code);

#endif