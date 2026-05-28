#ifndef __SPANNABLE_STRING_H__
#define __SPANNABLE_STRING_H__
#include <core/predicate.h>
#include <core/textutils.h>
#include <algorithm>
#include <string>
#include <vector>
#include <memory>

namespace cdroid {

class CharSequence {
public:
    virtual ~CharSequence() = default;
    virtual size_t length()const{return 0;}
    virtual int charAt(int)const{return 0;}
    virtual CharSequence*subSequence(int,int){return nullptr;}
    virtual std::string toString() const = 0;
    virtual std::wstring toWString() const = 0;
    // Copies characters from [start, end) into dest starting at destPos.
    // If dest is shorter than destPos, it will be resized.
    virtual void getChars(int start, int end, std::wstring& dest, int destPos) const = 0;
};

class CharacterStyle{
public:
    virtual ~CharacterStyle() = default;
};

class ForegroundColorSpan : public CharacterStyle {
public:
    explicit ForegroundColorSpan(int color) : mColor(color) {}
    int getForegroundColor() const { return mColor; }
private:
    int mColor;
};

class StyleSpan : public CharacterStyle {
public:
    explicit StyleSpan(int style) : mStyle(style) {}
    int getStyle() const { return mStyle; }
private:
    int mStyle;
};

class UnderlineSpan : public CharacterStyle {
};

class StrikethroughSpan : public CharacterStyle {
};
class SubscriptSpan: public CharacterStyle {
};
class SuperscriptSpan: public CharacterStyle {
};
class URLSpan : public CharacterStyle {
public:
    explicit URLSpan(const std::string& url) : mUrl(url) {}
    const std::string& getUrl() const { return mUrl; }
private:
    std::string mUrl;
};

class TypefaceSpan : public CharacterStyle {
public:
    explicit TypefaceSpan(const std::string& family) : mFamily(family) {}
    const std::string& getFamily() const { return mFamily; }
private:
    std::string mFamily;
};

class AbsoluteSizeSpan : public CharacterStyle {
public:
    explicit AbsoluteSizeSpan(int size) : mSize(size) {}
    int getSize() const { return mSize; }
private:
    int mSize;
};

class RelativeSizeSpan : public CharacterStyle {
public:
    explicit RelativeSizeSpan(float proportion) : mProportion(proportion) {}
    float getProportion() const { return mProportion; }
private:
    float mProportion;
};

struct SpanInfo {
    std::shared_ptr<CharacterStyle> what;
    int start;
    int end;
    int flags;

    bool operator==(const SpanInfo& other) const {
        return start == other.start && end == other.end && flags == other.flags && what == other.what;
    }
};
using SpanFilter=Predicate<const CharacterStyle*>;
// Spanned: read-only span-aware CharSequence (similar to Android's Spanned)
class Spanned : public CharSequence {
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
    virtual std::vector<SpanInfo> getSpans(int,int,const SpanFilter&) const = 0;
    virtual int getSpanStart(const std::shared_ptr<CharacterStyle>& what) const = 0;
    virtual int getSpanEnd(const std::shared_ptr<CharacterStyle>& what) const = 0;
    virtual int getSpanFlags(const std::shared_ptr<CharacterStyle>& what) const = 0;
    virtual int nextSpanTransition(int start, int limit, const SpanFilter& kind) const = 0;
};

// Spannable: mutable Spanned (supports setSpan/removeSpan)
class Spannable : public Spanned {
public:
    virtual ~Spannable() = default;
    virtual void setSpan(const std::shared_ptr<CharacterStyle>& what, int start, int end, int flags) = 0;
    virtual void removeSpan(const std::shared_ptr<CharacterStyle>& what) = 0;
};

// Immutable SpannedString implementation
class SpannedString : public Spanned {
public:
    SpannedString() = default;
    explicit SpannedString(const std::string& text)
        : mText(TextUtils::utf8tounicode(text)) {}
    explicit SpannedString(const std::wstring& text)
        : mText(text) {}

    static SpannedString valueOf(const std::string& text) {
        return SpannedString(text);
    }

    // SpannedString is immutable, so no setSpan here. Use SpannableStringBuilder
    SpannedString* subSequence(int start, int end) override{
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return new SpannedString();
        SpannedString* result = new SpannedString(mText.substr(start, end - start));
        for (const SpanInfo& span : mSpans) {
            if (span.end <= start || span.start >= end) continue;
            int spanStart = std::max(span.start, start) - start;
            int spanEnd = std::min(span.end, end) - start;
            result->mSpans.push_back(SpanInfo{span.what, spanStart, spanEnd, span.flags});
        }
        return result;
    }

    std::wstring toWString() const override {
        return mText;
    }

    std::string toString() const override {
        return TextUtils::unicode2utf8(mText);
    }

    size_t length() const override{
        return mText.length();
    }
    int charAt(int idx)const override{
        return mText.at(idx);
    }
    std::vector<SpanInfo> getSpans(int,int,const SpanFilter&) const override {
        return mSpans;
    }

    int getSpanStart(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) {
            if (s.what == what) return s.start;
        }
        return -1;
    }
    int getSpanEnd(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) {
            if (s.what == what) return s.end;
        }
        return -1;
    }
    int getSpanFlags(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) {
            if (s.what == what) return s.flags;
        }
        return 0;
    }
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override {
        int edge = limit;
        for (const SpanInfo& s : mSpans) {
            if (kind.test(s.what.get())) {
                if (s.start > start && s.start < edge) edge = s.start;
                if (s.end > start && s.end < edge) edge = s.end;
            }
        }
        return edge;
    }
    void getChars(int start, int end, std::wstring& dest, int destPos) const override {
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return;
        std::wstring sub = mText.substr(start, end - start);
        if (destPos < 0) destPos = 0;
        if ((int)dest.length() < destPos) dest.resize(destPos);
        dest.insert(destPos, sub);
    }
private:
    std::wstring mText;
    std::vector<SpanInfo> mSpans;
};

// Mutable SpannableString: immutable text with mutable spans, matching Android semantics.
class SpannableString : public Spannable {
public:
    SpannableString() = default;
    explicit SpannableString(const std::string& text)
        : mText(TextUtils::utf8tounicode(text)) {}
    explicit SpannableString(const std::wstring& text)
        : mText(text) {}

    std::wstring toWString() const override { return mText; }
    std::string toString() const override { return TextUtils::unicode2utf8(mText); }
    size_t length() const { return mText.length(); }

    std::vector<SpanInfo> getSpans(int,int,const SpanFilter&) const override { return mSpans; }
    int getSpanStart(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) if (s.what == what) return s.start;
        return -1;
    }
    int getSpanEnd(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) if (s.what == what) return s.end;
        return -1;
    }
    int getSpanFlags(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) if (s.what == what) return s.flags;
        return 0;
    }
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override {
        int edge = limit;
        for (const SpanInfo& s : mSpans) {
            if (kind.test(s.what.get())) {
                if (s.start > start && s.start < edge) edge = s.start;
                if (s.end > start && s.end < edge) edge = s.end;
            }
        }
        return edge;
    }

    void setSpan(const std::shared_ptr<CharacterStyle>& what, int start, int end, int flags) override {
        if (!what) return;
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return;
        mSpans.push_back(SpanInfo{what, start, end, flags});
    }
    void removeSpan(const std::shared_ptr<CharacterStyle>& what) override {
        mSpans.erase(std::remove_if(mSpans.begin(), mSpans.end(), [&](const SpanInfo& s){ return s.what == what; }), mSpans.end());
    }

private:
    std::wstring mText;
    std::vector<SpanInfo> mSpans;
};

// Mutable SpannableStringBuilder: builder-style mutable spannable (similar to Android's SpannableStringBuilder)
class SpannableStringBuilder : public Spannable {
public:
    SpannableStringBuilder() = default;
    explicit SpannableStringBuilder(const std::string& text) : mText(TextUtils::utf8tounicode(text)) {}

    std::wstring toWString() const override { return mText; }
    std::string toString() const override { return TextUtils::unicode2utf8(mText); }
    size_t length() const { return mText.length(); }

    std::vector<SpanInfo> getSpans(int,int,const SpanFilter&) const override { return mSpans; }
    int getSpanStart(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) if (s.what == what) return s.start;
        return -1;
    }
    int getSpanEnd(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) if (s.what == what) return s.end;
        return -1;
    }
    int getSpanFlags(const std::shared_ptr<CharacterStyle>& what) const override {
        for (const SpanInfo& s : mSpans) if (s.what == what) return s.flags;
        return 0;
    }
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override {
        int edge = limit;
        for (const SpanInfo& s : mSpans) {
            if (kind.test(s.what.get())) {
                if (s.start > start && s.start < edge) edge = s.start;
                if (s.end > start && s.end < edge) edge = s.end;
            }
        }
        return edge;
    }

    void setSpan(const std::shared_ptr<CharacterStyle>& what, int start, int end, int flags) override {
        if (!what) return;
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return;
        SpanInfo info{what, start, end, flags};
        mSpans.push_back(std::move(info));
    }
    void removeSpan(const std::shared_ptr<CharacterStyle>& what) override {
        mSpans.erase(std::remove_if(mSpans.begin(), mSpans.end(), [&](const SpanInfo& s){ return s.what == what; }), mSpans.end());
    }

    void shiftSpans(int index, int delta) {
        for (SpanInfo& span : mSpans) {
            if (span.start >= index) {
                span.start += delta;
                span.end += delta;
            } else if (span.end > index) {
                span.end += delta;
            }
        }
    }

    void adjustSpansForReplace(int start, int end, int delta) {
        std::vector<SpanInfo> result;
        for (SpanInfo span : mSpans) {
            if (span.end <= start) {
                result.push_back(span);
            } else if (span.start >= end) {
                span.start += delta;
                span.end += delta;
                result.push_back(span);
            } else if (span.start < start && span.end > end) {
                span.end = start;
                result.push_back(span);
            } else if (span.start < start && span.end > start) {
                span.end = start;
                result.push_back(span);
            } else if (span.start < end && span.end > end) {
                span.start = start + delta;
                span.end += delta;
                result.push_back(span);
            }
        }
        mSpans.swap(result);
    }

    SpannableStringBuilder& append(const std::string& utf8) { mText += TextUtils::utf8tounicode(utf8); return *this; }
    SpannableStringBuilder& append(const std::string& utf8, const std::shared_ptr<CharacterStyle>& what, int flags) {
        int start = (int)mText.length();
        append(utf8);
        setSpan(what, start, (int)mText.length(), flags);
        return *this;
    }
    SpannableStringBuilder& append(const std::string& utf8, const std::vector<std::shared_ptr<CharacterStyle>>& whats, int flags) {
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
    SpannableStringBuilder& append(const CharSequence& text, const std::shared_ptr<CharacterStyle>& what, int flags) {
        return append(text.toString(), what, flags);
    }
    SpannableStringBuilder& append(const CharSequence& text, const std::vector<std::shared_ptr<CharacterStyle>>& whats, int flags) {
        return append(text.toString(), whats, flags);
    }
    SpannableStringBuilder& append(const std::string& utf8, int start, int end) {
        std::wstring source = TextUtils::utf8tounicode(utf8);
        if (start < 0) start = 0;
        if (end > (int)source.length()) end = (int)source.length();
        if (start >= end) return *this;
        mText += source.substr(start, end - start);
        return *this;
    }
    SpannableStringBuilder& append(const CharSequence& text, int start, int end) {
        std::wstring source = text.toWString();
        if (start < 0) start = 0;
        if (end > (int)source.length()) end = (int)source.length();
        if (start >= end) return *this;
        mText += source.substr(start, end - start);
        return *this;
    }
    SpannableStringBuilder& insert(int where, const std::string& utf8) {
        std::wstring insertText = TextUtils::utf8tounicode(utf8);
        if (where < 0) where = 0;
        if (where > (int)mText.length()) where = (int)mText.length();
        mText.insert(where, insertText);
        shiftSpans(where, (int)insertText.length());
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
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return *this;
        std::wstring insertText = TextUtils::utf8tounicode(utf8);
        int oldLen = end - start;
        mText.replace(start, oldLen, insertText);
        int delta = (int)insertText.length() - oldLen;
        adjustSpansForReplace(start, end, delta);
        return *this;
    }
    SpannableStringBuilder& replace(int start, int end, const CharSequence& text, int tbeg, int tend) {
        std::wstring source = text.toWString();
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
        adjustSpansForReplace(start, end, delta);
        return *this;
    }
    SpannableStringBuilder& clear() {
        mText.clear();
        mSpans.clear();
        return *this;
    }
    void setSpanForText(const std::string& targetUtf8, const std::shared_ptr<CharacterStyle>& what, int flags, bool firstOnly = true) {
        if (!what) return;
        std::wstring target = TextUtils::utf8tounicode(targetUtf8);
        size_t pos = 0;
        while (pos < mText.size()) {
            size_t found = mText.find(target, pos);
            if (found == std::wstring::npos) break;
            setSpan(what, (int)found, (int)(found + target.length()), flags);
            if (firstOnly) break;
            pos = found + 1;
        }
    }
    void getChars(int start, int end, std::wstring& dest, int destPos) const override {
        if (start < 0) start = 0;
        if (end > (int)mText.length()) end = (int)mText.length();
        if (start >= end) return;
        std::wstring sub = mText.substr(start, end - start);
        if (destPos < 0) destPos = 0;
        if ((int)dest.length() < destPos) dest.resize(destPos);
        dest.insert(destPos, sub);
    }
private:
    std::wstring mText;
    std::vector<SpanInfo> mSpans;
};

}/*endof namespace*/
#endif/*__SPANNABLE_STRING_H__*/
