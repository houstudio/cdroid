#include "unicode/unorm2.h"

struct UNormalizer2 {
    int dummy;
};

U_CAPI const UNormalizer2* U_EXPORT2 unorm2_getNFDInstance(UErrorCode* status) {
    if (status != nullptr && *status > U_ZERO_ERROR) {
        return nullptr;
    }
    static UNormalizer2 dummy = {0};
    return &dummy;
}

U_CAPI int32_t U_EXPORT2 unorm2_getRawDecomposition(const UNormalizer2* n2, UChar32 c, UChar* dest, int32_t destCapacity, UErrorCode* status) {
    (void)n2;
    if (status != nullptr && *status > U_ZERO_ERROR) {
        return 0;
    }
    if (dest == nullptr || destCapacity <= 0) {
        if (status != nullptr) *status = U_ILLEGAL_ARGUMENT_ERROR;
        return 0;
    }
    if (c <= 0xFFFF) {
        dest[0] = (UChar)c;
        return 1;
    }
    if (destCapacity >= 2) {
        dest[0] = U16_LEAD(c);
        dest[1] = U16_TRAIL(c);
        return 2;
    }
    if (status != nullptr) *status = U_STRING_NOT_TERMINATED_WARNING;
    return 0;
}