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
using SpanFilter=Predicate<const ParcelableSpan*>;

class CharSequence : virtual public ParcelableSpan {
public:
    virtual ~CharSequence() = default;
    virtual size_t length()const{return 0;}
    virtual int charAt(int)const{return 0;}
    virtual CharSequence*subSequence(int,int)const{return nullptr;}
    virtual std::string toString() const = 0;
    virtual std::wstring toWString() const = 0;
    // Copies characters from [start, end) into dest starting at destPos.
    // If dest is shorter than destPos, it will be resized.
    virtual void getChars(int start, int end, std::vector<char32_t>& dest, int destPos) const = 0;
    static std::wstring utf8tounicode(const std::string&);
    static std::string unicode2utf8(const std::wstring&);
};

class ParagraphStyle : public ParcelableSpan {
public:
    virtual ~ParagraphStyle() = default;
};

class CharacterStyle : virtual public ParcelableSpan {
public:
    virtual ~CharacterStyle() = default;
    virtual void updateDrawState(const Paint& paint){};
};

}/*endof namespace*/
#endif/*__PARCELABLE_SPAN_H__*/
