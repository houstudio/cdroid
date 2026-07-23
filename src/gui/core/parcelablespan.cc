#include "parcelablespan.h"
#include "../utils/textutils.h"

namespace cdroid {

std::wstring CharSequence::utf8tounicode(const std::string& utf8) {
    return TextUtils::utf8tounicode(utf8);
}

std::string CharSequence::unicode2utf8(const std::wstring& u32s) {
    return TextUtils::unicode2utf8(u32s);
}

}  // namespace cdroid
