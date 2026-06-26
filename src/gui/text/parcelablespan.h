#ifndef __PARCELABLE_SPAN_H__
#define __PARCELABLE_SPAN_H__
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <core/predicate.h>
namespace cdroid{

class Paint;

class ParcelableSpan {
public:
    virtual ~ParcelableSpan() = default;
};

class CharSequence : virtual public ParcelableSpan {
public:
    virtual ~CharSequence() = default;
    virtual size_t length() const = 0;
    virtual int charAt(int) const = 0;
    virtual CharSequence* subSequence(int, int) const { return nullptr; }
    virtual std::string toString() const = 0;
    virtual void getChars(int start, int end, char16_t* dest, int destPos) const = 0;
    virtual std::u16string toU16String() const = 0;

    // Convenience: implicit conversion to either string type (non-virtual; delegates
    // to the pure virtuals). Lets you write `std::string s = *charSeq;` or
    // `std::u16string u = *charSeq;` — perfect backward compat for getText().
    operator std::string() const { return toString(); }
    operator std::u16string() const { return toU16String(); }
};

class ParagraphStyle : public ParcelableSpan {
public:
    virtual ~ParagraphStyle() = default;
};

class CharacterStyle : virtual public ParcelableSpan {
public:
    virtual ~CharacterStyle() = default;
    virtual void updateDrawState(const Paint& paint)const{};
};

class NoCopySpan:virtual public ParcelableSpan{
};

using SpanFilter=Predicate<const ParcelableSpan*>;

template<typename T>
SpanFilter make_span_filter() {
    return SpanFilter([](const ParcelableSpan* o) -> bool {
        return dynamic_cast<const T*>(o) != nullptr;
    });
}

}/*endof namespace*/
#endif/*__PARCELABLE_SPAN_H__*/
