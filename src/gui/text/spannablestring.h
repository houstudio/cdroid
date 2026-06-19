#ifndef __SPANNABLE_STRING_H__
#define __SPANNABLE_STRING_H__
#include <core/predicate.h>
#include <text/textutils.h>
#include <text/textpaint.h>
#include <text/parcelablespan.h>
#include <algorithm>
#include <string>
#include <vector>
#include <tuple>
#include <memory>

namespace cdroid {

class Spanned : virtual public CharSequence {
public:
    enum {
        SPAN_POINT_MARK_MASK = 0x33,
        SPAN_MARK_MARK = 0x11,
        SPAN_MARK_POINT = 0x12,
        SPAN_POINT_MARK = 0x21,
        SPAN_POINT_POINT = 0x22,
        SPAN_PARAGRAPH = 0x33,
        SPAN_INCLUSIVE_EXCLUSIVE = SPAN_MARK_MARK,
        SPAN_INCLUSIVE_INCLUSIVE = SPAN_MARK_POINT,
        SPAN_EXCLUSIVE_EXCLUSIVE = SPAN_POINT_MARK,
        SPAN_EXCLUSIVE_INCLUSIVE = SPAN_POINT_POINT,
        SPAN_COMPOSING = 0x100,
        SPAN_INTERMEDIATE = 0x200,
        SPAN_USER_SHIFT = 24,
        SPAN_USER = 0xFFFFFFFF << SPAN_USER_SHIFT,
        SPAN_PRIORITY_SHIFT = 16,
        SPAN_PRIORITY = 0xFF << SPAN_PRIORITY_SHIFT
    };
public:
    virtual ~Spanned() = default;
    virtual std::vector<const ParcelableSpan*> getSpans(int start, int end, const SpanFilter&) const = 0;
    virtual int getSpanStart(const ParcelableSpan* what) const = 0;
    virtual int getSpanEnd(const ParcelableSpan* what) const = 0;
    virtual int getSpanFlags(const ParcelableSpan* what) const = 0;
    virtual int nextSpanTransition(int start, int limit, const SpanFilter& kind) const = 0;
};

class Spannable : virtual public Spanned {
public:
    virtual ~Spannable() = default;
    virtual void setSpan(const ParcelableSpan* what, int start, int end, int flags) = 0;
    virtual void removeSpan(const ParcelableSpan* what) = 0;
};

class SpannableStringInternal : virtual public Spanned {
protected:
    std::u16string mText;
    std::vector<std::tuple<const ParcelableSpan*, int, int, int>> mSpans;
    SpannableStringInternal() = default;
    explicit SpannableStringInternal(const std::u16string& text) : mText(text) {}
public:
    SpannableStringInternal(const CharSequence* source, int start, int end, bool ignoreNoCopySpan);
    SpannableStringInternal(const CharSequence* source, int start, int end);
    SpannableStringInternal(const CharSequence* source);
    virtual ~SpannableStringInternal() = default;

    std::string toString() const override;
    size_t length() const override;
    int charAt(int idx) const override;
    std::vector<const ParcelableSpan*> getSpans(int start, int end, const SpanFilter& filter) const override;
    int getSpanStart(const ParcelableSpan* what) const override;
    int getSpanEnd(const ParcelableSpan* what) const override;
    int getSpanFlags(const ParcelableSpan* what) const override;
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override;
    void getChars(int start, int end, char16_t* dest, int destPos) const override;
};

class SpannedString : public SpannableStringInternal {
public:
    SpannedString() = default;
    explicit SpannedString(const std::u16string& text) : SpannableStringInternal(text) {}

    static SpannedString valueOf(const std::u16string& text) {
        return SpannedString(text);
    }

    SpannedString* subSequence(int start, int end) const override;
};

class SpannableString : virtual public SpannableStringInternal, virtual public Spannable {
public:
    SpannableString() = default;
    SpannableString(const CharSequence*, bool ignoreNoCopySpan);
    explicit SpannableString(const std::u16string& text) : SpannableStringInternal(text) {}

    void setSpan(const ParcelableSpan* what, int start, int end, int flags) override;
    void removeSpan(const ParcelableSpan* what) override;
};

}/*endof namespace*/
#endif/*__SPANNABLE_STRING_H__*/
