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

// Mutable SpannableStringBuilder: builder-style mutable spannable (similar to Android's SpannableStringBuilder)
class SpannableStringBuilder : public SpannableStringInternal, virtual public Spannable {
public:
    SpannableStringBuilder() = default;
    explicit SpannableStringBuilder(const std::u16string& text)
        : SpannableStringInternal(text) {}
    SpannableStringBuilder(const CharSequence*){}
    
    void setSpan(const ParcelableSpan* what, int start, int end, int flags) override {
        if (!what) return;
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return;
        mSpans.push_back({what,start,end,flags});
    }
    void removeSpan(const ParcelableSpan* what) override {
        mSpans.erase(std::remove_if(mSpans.begin(), mSpans.end(), 
            [&](const std::tuple<const ParcelableSpan*,int,int,int>& s){ 
                return std::get<0>(s) == what; 
            }), mSpans.end());
    }

    void shiftSpans(int index, int delta) {
        for (auto t : mSpans) {
            const ParcelableSpan*span;
            int sstart,send,sflags;
            std::tie(span,sstart,send,sflags) = t;
            if (sstart >= index) {
                sstart += delta;
                send += delta;
            } else if (send > index) {
                send += delta;
            }
        }
    }

    void adjustSpansForReplace(int start, int end, int delta) {
        std::vector<std::tuple<const ParcelableSpan*,int,int,int>> result;
        for (auto& t : mSpans) {
            const ParcelableSpan*span;
            int sstart,send,sflags;
            std::tie(span,sstart,send,sflags) = t;
            if (send <= start) {
                result.push_back({span,sstart,send,sflags});
            } else if (sstart >= end) {
                sstart += delta;
                send += delta;
                result.push_back({span,sstart,send,sflags});
            } else if (sstart < start && send > end) {
                send = start;
                result.push_back({span,sstart,send,sflags});
            } else if (sstart < start && send > start) {
                send = start;
                result.push_back({span,sstart,send,sflags});
            } else if (sstart < end && send > end) {
                sstart = start + delta;
                send += delta;
                result.push_back({span,sstart,send,sflags});
            }
        }
        mSpans.swap(result);
    }

    SpannableStringBuilder& append(const std::string& utf8) { mText += TextUtils::utf8_utf16(utf8); return *this; }
    SpannableStringBuilder& append(const std::u16string& utf16) {mText+=utf16;return *this;}
    SpannableStringBuilder& append(const char16_t*utf16,int start,int count){
        mText+=std::u16string(utf16+start,count);
        return *this;
    }
    SpannableStringBuilder& append(const std::string& utf8,const ParcelableSpan* what, int flags) {
        int start = (int)mText.length();
        append(utf8);
        setSpan(what, start, (int)mText.length(), flags);
        return *this;
    }
    SpannableStringBuilder& append(const std::string& utf8, const std::vector<const ParcelableSpan*>& whats, int flags) {
        int start = (int)mText.length();
        append(utf8);
        int end = (int)mText.length();
        for (const auto& what : whats) {
            if (what) {
                setSpan(what, start, end, flags);
            }
        }
        return *this;
    }
    SpannableStringBuilder& append(const CharSequence& text) {
        return append(text.toString());
    }
    SpannableStringBuilder& append(const CharSequence& text, const ParcelableSpan* what, int flags) {
        return append(text.toString(), what, flags);
    }
    SpannableStringBuilder& append(const CharSequence& text, const std::vector<const ParcelableSpan*>& whats, int flags) {
        return append(text.toString(), whats, flags);
    }
    SpannableStringBuilder& append(const std::string& utf8, int start, int end) {
        std::u16string source = TextUtils::utf8_utf16(utf8);
        if (start < 0) start = 0;
        if (end > (int)source.length()) end = (int)source.length();
        if (start >= end) return *this;
        mText += source.substr(start, end - start);
        return *this;
    }
    SpannableStringBuilder& append(const CharSequence& text, int start, int end) {
        /*std::u16string source = text.toWString();
        if (start < 0) start = 0;
        if (end > (int)source.length()) end = (int)source.length();
        if (start >= end) return *this;
        mText += source.substr(start, end - start);*/
        return *this;
    }
    SpannableStringBuilder& insert(int where, const std::string& utf8) {
        /*std::wstring insertText = TextUtils::utf8tounicode(utf8);
        if (where < 0) where = 0;
        if (where > (int)mText.length()) where = (int)mText.length();
        mText.insert(where, insertText);
        shiftSpans(where, (int)insertText.length());*/
        return *this;
    }
    SpannableStringBuilder& insert(int where, const CharSequence& text) {
        return insert(where, text.toString());
    }
    // Android equivalent: delete(start, end)
    // C++ cannot use the keyword `delete` as a method name.
    SpannableStringBuilder& deleteText(int start, int end) {
        return replace(start, end, std::string());
    }
    SpannableStringBuilder& replace(int start, int end, const std::string& utf8) {
        /*if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return *this;
        std::wstring insertText = TextUtils::utf8tounicode(utf8);
        int oldLen = end - start;
        mText.replace(start, oldLen, insertText);
        int delta = (int)insertText.length() - oldLen;
        adjustSpansForReplace(start, end, delta);*/
        return *this;
    }
    SpannableStringBuilder& replace(int start, int end, const CharSequence& text, int tbeg, int tend) {
        /*std::wstring source = text.toWString();
        if (tbeg < 0) tbeg = 0;
        if (tend > (int)source.length()) tend = (int)source.length();
        if (tbeg >= tend) return replace(start, end, std::string());
        std::wstring insertText = source.substr(tbeg, tend - tbeg);
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return *this;
        int oldLen = end - start;
        mText.replace(start, oldLen, insertText);
        int delta = (int)insertText.length() - oldLen;
        adjustSpansForReplace(start, end, delta);*/
        return *this;
    }
    SpannableStringBuilder& clear() {
        mText.clear();
        mSpans.clear();
        return *this;
    }
    void setSpanForText(const std::string& targetUtf8, const ParcelableSpan* what, int flags, bool firstOnly = true) {
        if (!what) return;
        /*std::wstring target = TextUtils::utf8tounicode(targetUtf8);
        size_t pos = 0;
        while (pos < mText.size()) {
            size_t found = mText.find(target, pos);
            if (found == std::wstring::npos) break;
            setSpan(what, (int)found, (int)(found + target.length()), flags);
            if (firstOnly) break;
            pos = found + 1;
        }*/
    }
    void getChars(int start, int end, char16_t* dest, int destPos) const override {
        for(int i=start;i<end;i++)
            dest[i+destPos]=mText[start+i];
    }
};

}/*endof namespace*/
#endif/*__SPANNABLE_STRING_H__*/
