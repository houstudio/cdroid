#include "unicode/unorm2.h"
#include "unicode_range_data.h"

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

    // Look up canonical decomposition in the shared NFD table.
    const NfdMapping* entry = findNfdMapping(c);
    if (entry != nullptr && entry->decompLen > 0) {
        int32_t outLen = entry->decompLen;
        if (outLen > destCapacity) {
            if (status != nullptr) *status = U_STRING_NOT_TERMINATED_WARNING;
            return outLen;
        }
        for (int i = 0; i < outLen; ++i) {
            dest[i] = entry->decomp[i];
        }
        return outLen;
    }

    // No canonical decomposition: return 0 (per ICU spec).
    // Characters without decomposition (like control characters, basic ASCII)
    // should return 0, not the character itself.
    return 0;
}
