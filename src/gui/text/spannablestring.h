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
// Spanned: read-only span-aware CharSequence (similar to Android's Spanned)
class Spanned : virtual public CharSequence {
public:
    enum{
        SPAN_POINT_MARK_MASK = 0x33,
        SPAN_MARK_MARK =   0x11,
        SPAN_MARK_POINT =  0x12,
        SPAN_POINT_MARK =  0x21,
        SPAN_POINT_POINT = 0x22,
        SPAN_PARAGRAPH =   0x33,
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

// Spannable: mutable Spanned (supports setSpan/removeSpan)
class Spannable : virtual public Spanned {
public:
    virtual ~Spannable() = default;
    virtual void setSpan(const ParcelableSpan* what, int start, int end, int flags) = 0;
    virtual void removeSpan(const ParcelableSpan* what) = 0;
};

// Internal base class for SpannedString and SpannableString (similar to Android's SpannableStringInternal)
class SpannableStringInternal : virtual public Spanned {
protected:
    std::u16string mText;
    std::vector<std::tuple<const ParcelableSpan*,int,int,int>> mSpans;
    SpannableStringInternal() = default;
    explicit SpannableStringInternal(const std::u16string& text) : mText(text) {}
public:
    SpannableStringInternal(const CharSequence* source, int start, int end, bool ignoreNoCopySpan){}
    SpannableStringInternal(const CharSequence* source, int start, int end){}
    SpannableStringInternal(const CharSequence*){}
    virtual ~SpannableStringInternal() = default;
    
    std::string toString() const override {
        return "";//TextUtils::unicode2utf8(mText);
    }

    size_t length() const override {
        return mText.length();
    }
    
    int charAt(int idx) const override {
        return mText.at(idx);
    }
    
    std::vector<const ParcelableSpan*> getSpans(int start, int end, const SpanFilter& filter) const override {
        std::vector<const ParcelableSpan*> result;
        for (const auto & t : mSpans) {
            const ParcelableSpan*span;
            int sstart,send,sflags;
            std::tie(span,sstart,send,sflags) = t;
            if (sstart >= end || send <= start) continue;
            if (filter.test(span)) {
                result.push_back(span);
            }
        }
        return result;
    }

    int getSpanStart(const ParcelableSpan* what) const override {
        for (const auto& t : mSpans) {
            const ParcelableSpan*span;
            int sstart,send,sflags;
            std::tie(span,sstart,send,sflags) = t;
            if (span == what) return sstart;
        }
        return -1;
    }
    
    int getSpanEnd(const ParcelableSpan* what) const override {
        for (auto& t : mSpans) {
            const ParcelableSpan*span;
            int sstart,send,sflags;
            std::tie(span,sstart,send,sflags) = t;
            if (span == what) return send;
        }
        return -1;
    }
    
    int getSpanFlags(const ParcelableSpan* what) const override {
        for (auto t : mSpans) {
            const ParcelableSpan*span;
            int sstart,send,sflags;
            std::tie(span,sstart,send,sflags) = t;
            if (span == what) return sflags;
        }
        return 0;
    }
    
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override {
        int edge = limit;
        for (auto t : mSpans) {
            const ParcelableSpan*span;
            int sstart,send,sflags;
            std::tie(span,sstart,send,sflags) = t;
            if (kind.test(span)) {
                if (sstart > start && sstart < edge) edge = sstart;
                if (send > start && send < edge) edge = send;
            }
        }
        return edge;
    }
    
    void getChars(int start, int end, char16_t* dest, int destPos) const override {
        if (start >= end) return;
        //if ((int)dest.size() < destPos) dest.resize(destPos);
        for(int i=0;i<end-start;i++)
            dest[destPos+i]=mText[start+i];
    }
};

// Immutable SpannedString implementation
class SpannedString : public SpannableStringInternal {
public:
    SpannedString() = default;
    explicit SpannedString(const std::u16string& text)
        : SpannableStringInternal(text) {}

    static SpannedString valueOf(const std::u16string& text) {
        return SpannedString(text);
    }

    // SpannedString is immutable, so no setSpan here. Use SpannableStringBuilder
    SpannedString* subSequence(int start, int end)const override{
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return new SpannedString();
        SpannedString* result = new SpannedString();
        result->mText = mText.substr(start, end - start);
        for (auto& t : mSpans) {
            const ParcelableSpan*span;
            int sstart,send,sflags;
            std::tie(span,sstart,send,sflags) = t;
            if (send <= start || sstart >= end) continue;
            int spanStart = std::max(sstart, start) - start;
            int spanEnd = std::min(send, end) - start;
            result->mSpans.push_back({span, spanStart, spanEnd, sflags});
        }
        return result;
    }
};

// Mutable SpannableString: immutable text with mutable spans, matching Android semantics.
class SpannableString : virtual public SpannableStringInternal,virtual public Spannable {
public:
    SpannableString() = default;
    SpannableString(const CharSequence*, bool ignoreNoCopySpan ){}
    explicit SpannableString(const std::u16string& text)
        : SpannableStringInternal(text) {}

    void setSpan(const ParcelableSpan* what, int start, int end, int flags) override {
        if (!what) return;
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return;
        mSpans.push_back({what, start, end, flags});
    }
    void removeSpan(const ParcelableSpan* what) override {
        mSpans.erase(std::remove_if(mSpans.begin(), mSpans.end(), 
            [&](const std::tuple<const ParcelableSpan*,int,int,int>& s){ 
                return std::get<0>(s) == what; 
            }), mSpans.end());
    }
};

}/*endof namespace*/
#endif/*__SPANNABLE_STRING_H__*/
