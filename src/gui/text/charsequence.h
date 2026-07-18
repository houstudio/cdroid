#ifndef __CHAR_SEQUENCE_H__
#define __CHAR_SEQUENCE_H__

#include <string>
#include <text/parcelablespan.h>   // ParcelableSpan — CharSequence's base

namespace cdroid {

class String; // forward declaration (text/String.h); toString() returns it.
              // Not #included here to avoid a circular include — String derives
              // from CharSequence, so String.h must include this header.

// A CharSequence is a readable sequence of char16_t code units — the base of
// CDROID's text model (String, Spanned/Spannable, Editable, ...). Mirrors
// java.lang.CharSequence.
class CharSequence : virtual public ParcelableSpan {
public:
    virtual ~CharSequence() = default;
    virtual size_t length() const = 0;
    virtual int charAt(int) const = 0;
    virtual CharSequence* subSequence(int, int) const { return nullptr; }
    virtual void getChars(int start, int end, char16_t* dest, int destPos) const = 0;

    // Android rule: CharSequence.toString() returns a String. The returned
    // pointer is OWNED BY THE CALLER (same contract as subSequence). String is
    // only forward-declared above; it is complete wherever toString() is
    // actually defined or called.
    virtual String* toString() const = 0;

    // Encoding-explicit accessors (by value — no ownership burden). The old
    // toU16String() was removed because it overlapped toUTF16(); call toUTF16()
    // for UTF-16 code units and toUTF8() for UTF-8 bytes.
    virtual std::string toUTF8() const = 0;
    virtual std::u16string toUTF16() const = 0;

    // Implicit conversion so `std::string s = charSeq;` / `std::u16string u = charSeq;`
    // are legal, and so CharSequence composes with std::string concatenation.
    // (Conversion operators must be non-static members per C++ — there is no
    // free-function form.) These coexist with the operator+ free functions below:
    // for `str + charSeq` the explicit operator+(std::string, CharSequence) is a
    // strictly better match (no user-defined conversion) than converting charSeq
    // to std::string and using std::string + std::string, so no ambiguity.
    operator std::string() const { return toUTF8(); }
    operator std::u16string() const { return toUTF16(); }
};

// Free-function concatenation between std::string / literals and any
// CharSequence. Result is UTF-8 (std::string), the common I/O form.
inline std::string operator+(const std::string& lhs, const CharSequence& rhs) {
    return lhs + rhs.toUTF8();
}
inline std::string operator+(const CharSequence& lhs, const std::string& rhs) {
    return lhs.toUTF8() + rhs;
}
inline std::string operator+(const char* lhs, const CharSequence& rhs) {
    return std::string(lhs) + rhs.toUTF8();
}
inline std::string operator+(const CharSequence& lhs, const char* rhs) {
    return lhs.toUTF8() + std::string(rhs);
}

} // namespace cdroid

#endif // __CHAR_SEQUENCE_H__
