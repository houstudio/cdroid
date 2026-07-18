#ifndef __CDROID_STRING_H__
#define __CDROID_STRING_H__

#include <string>
#include <cstddef>
#include <cstdint>
#include <text/charsequence.h>

namespace cdroid {

// Minimal, CharSequence-implementing port of java.lang.String.
//
// CDROID's primary text type is std::string (UTF-8, at I/O boundaries) /
// std::u16string (char16_t, internal). This class is NOT a full reimplementation
// of Java's String — only the CharSequence surface plus the common string
// operations the text/widget code reaches for. It replaces the old
// `using String = std::string;` alias in character.h.
//
// Backing store is std::u16string (so length()/charAt()/getChars are UTF-16
// correct, matching the CharSequence contract and Java semantics). UTF-8
// interop (implicit conversions to/from std::string and const char*, plus
// toString()/c_str()) is provided so existing call sites that treated the old
// alias as a std::string keep compiling unchanged.
class String : public CharSequence {
public:
    // ---- construction ----
    String();
    String(const String&);
    String(String&&) noexcept = default;
    String& operator=(const String&);
    String& operator=(String&&) noexcept = default;

    // Primary (internal) form: UTF-16.
    String(const std::u16string& s);
    String(const char16_t* s);
    String(const char16_t* s, int length);

    // I/O-boundary form: UTF-8. Non-explicit so a std::string / string literal
    // can be returned/assigned where a String is expected (backward compat).
    String(const std::string& utf8);
    String(const char* utf8);

    // ---- CharSequence ----
    size_t length() const override;                 // UTF-16 code unit count
    int charAt(int index) const override;           // UTF-16 code unit at index
    String* subSequence(int start, int end) const override; // owned by caller
    void getChars(int start, int end, char16_t* dest, int destPos) const override;
    String* toString() const override;              // owned copy (Android: returns String)
    std::string toUTF8() const override;            // explicit UTF-8
    std::u16string toUTF16() const override;        // explicit UTF-16

    // ---- ParcelableSpan ----
    String* clone() const override;

    // ---- std::string (UTF-8) interop ----
    // No implicit operator std::string() — it would clash with the free
    // operator+(std::string, CharSequence) overloads. Call toUTF8()/toString()
    // (or str()/c_str()) explicitly when you need a std::string.
    String& operator=(const std::string& utf8);
    String& operator=(const char* utf8);
    std::string str() const;                        // explicit UTF-8 copy
    const char* c_str() const;                      // cached UTF-8 c_str()
    const std::u16string& u16str() const;           // raw UTF-16 view

    // ---- common operations (kept from java.lang.String where useful) ----
    bool isEmpty() const;

    bool equals(const String& other) const;
    bool equals(const std::string& utf8) const;
    bool equals(const char* utf8) const;
    bool operator==(const String& other) const;
    bool operator==(const char* utf8) const;
    bool operator==(const std::string& utf8) const;
    bool operator!=(const String& other) const;

    String operator+(const String& other) const;
    String concat(const String& other) const;

    int compareTo(const String& other) const;
    bool operator<(const String& other) const;
    bool operator>(const String& other) const;

    int indexOf(char16_t ch, int fromIndex = 0) const;
    int indexOf(const String& sub, int fromIndex = 0) const;
    int lastIndexOf(char16_t ch, int fromIndex = -1) const;

    String substring(int beginIndex) const;
    String substring(int beginIndex, int endIndex) const;

    bool startsWith(const String& prefix) const;
    bool endsWith(const String& suffix) const;
    bool contains(const String& sub) const;

    String toLowerCase() const;
    String toUpperCase() const;
    String trim() const;
    String replace(char16_t oldChar, char16_t newChar) const;

    int hashCode() const;

    // ---- valueOf / join ----
    static String valueOf(char16_t c);
    static String valueOf(int i);
    static String valueOf(int64_t l);
    static String valueOf(float f);
    static String valueOf(double d);
    static String valueOf(bool b);
    static String valueOf(const std::string& utf8);
    static String valueOf(const char* utf8);
    static String valueOf(const std::u16string& u16);

    static const String EMPTY;

private:
    std::u16string mValue;
    // Lazily-computed caches (mutable so the const CharSequence view stays pure).
    mutable std::string mUtf8;
    mutable bool mUtf8Valid = false;
    mutable int mHash = 0;       // 0 == not yet computed (Java semantics)
};

} // namespace cdroid

#endif // __CDROID_STRING_H__
