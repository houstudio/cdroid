#include "unicode/uscript.h"
#include "unicode_range_data.h"

U_CAPI UScriptCode U_EXPORT2 uscript_getScript(UChar32 c, UErrorCode* status) {
    if (status && U_FAILURE(*status)) {
        return USCRIPT_INVALID_CODE;
    }
    
    // 直接使用数据表中的 script 属性
    const auto* range = findUnicodeRange(c);
    
    if (status) {
        *status = U_ZERO_ERROR;
    }
    
    return (UScriptCode)range->script;
}