#include "unicode/uloc.h"
#include <cstring>

U_CAPI int32_t U_EXPORT2 uloc_canonicalize(const char* locale, char* result, int32_t maxResultSize, UErrorCode* status) {
    if (status != nullptr && *status > U_ZERO_ERROR) {
        return 0;
    }
    if (locale == nullptr || result == nullptr || maxResultSize <= 0) {
        if (status != nullptr) *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    size_t len = strlen(locale);
    if (len >= (size_t)maxResultSize) {
        if (status != nullptr) *status = U_STRING_NOT_TERMINATED_WARNING;
        return 0;
    }
    strcpy(result, locale);
    return (int32_t)len;
}

U_CAPI int32_t U_EXPORT2 uloc_addLikelySubtags(const char* locale, char* result, int32_t maxResultSize, UErrorCode* status) {
    if (status != nullptr && *status > U_ZERO_ERROR) {
        return 0;
    }
    if (locale == nullptr || result == nullptr || maxResultSize <= 0) {
        if (status != nullptr) *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    size_t len = strlen(locale);
    if (len >= (size_t)maxResultSize) {
        if (status != nullptr) *status = U_STRING_NOT_TERMINATED_WARNING;
        return 0;
    }
    strcpy(result, locale);
    return (int32_t)len;
}

U_CAPI int32_t U_EXPORT2 uloc_toLanguageTag(const char* locale, char* result, int32_t maxResultSize, UBool rfc3066, UErrorCode* status) {
    (void)rfc3066;
    if (status != nullptr && *status > U_ZERO_ERROR) {
        return 0;
    }
    if (locale == nullptr || result == nullptr || maxResultSize <= 0) {
        if (status != nullptr) *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    size_t len = strlen(locale);
    if (len >= (size_t)maxResultSize) {
        if (status != nullptr) *status = U_STRING_NOT_TERMINATED_WARNING;
        return 0;
    }
    strcpy(result, locale);
    for (char* p = result; *p; p++) {
        if (*p == '_') *p = '-';
    }
    return (int32_t)strlen(result);
}

U_CAPI int32_t U_EXPORT2 uloc_forLanguageTag(const char* languageTag, char* result, int32_t maxResultSize, void* options, UErrorCode* status) {
    (void)options;
    if (status != nullptr && *status > U_ZERO_ERROR) {
        return 0;
    }
    if (languageTag == nullptr || result == nullptr || maxResultSize <= 0) {
        if (status != nullptr) *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    size_t len = strlen(languageTag);
    if (len >= (size_t)maxResultSize) {
        if (status != nullptr) *status = U_STRING_NOT_TERMINATED_WARNING;
        return 0;
    }
    strcpy(result, languageTag);
    for (char* p = result; *p; p++) {
        if (*p == '-') *p = '_';
    }
    return (int32_t)strlen(result);
}

U_CAPI const char* U_EXPORT2 u_errorName(UErrorCode code) {
    static const char* names[] = {
        "U_ZERO_ERROR",
        "U_ILLEGAL_ARGUMENT_ERROR",
        "U_STRING_NOT_TERMINATED_WARNING"
    };
    if (code == U_ZERO_ERROR) return names[0];
    if (code > 0) return names[1];
    if (code == -124) return names[2];
    return "U_UNKNOWN_ERROR";
}