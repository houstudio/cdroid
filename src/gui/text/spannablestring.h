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
#include <cassert>
#include <stdexcept>

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
    // One stored span plus its [start, end) bounds, flags, and ownership mode.
    // `owned == true`  => this container manages the span's lifetime (it is a
    //                     non-NoCopySpan passed to setSpan): the container deletes
    //                     it on removal/destruction and clones it on copy.
    // `owned == false` => BORROWED (a NoCopySpan: watcher or selection marker):
    //                     never deleted, never cloned; only its pointer is
    //                     (optionally) shared. Mirrors Android's NoCopySpan.
    struct SpanRecord {
        ParcelableSpan* span;   // non-const; const is cast away once in addSpan()
        int start;
        int end;
        int flags;
        bool owned;
    };
    std::u16string mText;
    std::vector<SpanRecord> mSpans;

    // --- centralized span mutation: the owned/borrowed logic lives ONLY here ---
    // Insert. The SINGLE point where const is cast away and `owned` is decided.
    void addSpan(const ParcelableSpan* span, int start, int end, int flags);
    // Remove the first record whose span == `span` (pointer identity) and
    // dispose it. Returns true if a record was removed.
    bool removeSpanRecord(const ParcelableSpan* span);
    // Dispose every owned span, then clear the vector. Used by dtor/clearSpans.
    void deleteAllOwnedSpans();
    // Append a copy of `srcSpan` into `dest` at [newStart, newEnd): clone it if
    // the span is owned, share the raw pointer if borrowed, and honor
    // ignoreNoCopySpan (skip NoCopySpan entirely when true). The single clone
    // decision point, used by the copy-ctor and subSequence.
    static void appendSpanCopy(std::vector<SpanRecord>& dest, const ParcelableSpan* srcSpan,
                               int newStart, int newEnd, int flags, bool ignoreNoCopySpan);
    // Delete the span object iff this record owns it.
    static void disposeSpan(SpanRecord& r);

    SpannableStringInternal() = default;
    explicit SpannableStringInternal(const std::u16string& text) : mText(text) {}
public:
    SpannableStringInternal(const CharSequence* source, int start, int end, bool ignoreNoCopySpan);
    SpannableStringInternal(const CharSequence* source, int start, int end);
    SpannableStringInternal(const CharSequence* source);
    virtual ~SpannableStringInternal() = default;

    std::string toString() const override;
    std::u16string toU16String() const override;   // fast path: return mText directly
    size_t length() const override;
    int charAt(int idx) const override;
    std::vector<const ParcelableSpan*> getSpans(int start, int end, const SpanFilter& filter) const override;
    int getSpanStart(const ParcelableSpan* what) const override;
    int getSpanEnd(const ParcelableSpan* what) const override;
    int getSpanFlags(const ParcelableSpan* what) const override;
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override;
    void getChars(int start, int end, char16_t* dest, int destPos) const override;
    // Each class returns its own type — SSI returns SSI*, SpannedString returns
    // SpannedString* (covariant). No forward declaration needed.
    SpannableStringInternal* subSequence(int start, int end) const override;
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
protected:
    void sendSpanAdded(const ParcelableSpan* what, int start, int end);
    void sendSpanRemoved(const ParcelableSpan* what, int start, int end);
    void sendSpanChanged(const ParcelableSpan* what, int ostart, int oend, int nstart, int nend);
public:
    SpannableString() = default;
    SpannableString(const CharSequence*, bool ignoreNoCopySpan);
    explicit SpannableString(const std::u16string& text) : SpannableStringInternal(text) {}

    void setSpan(const ParcelableSpan* what, int start, int end, int flags) override;
    void removeSpan(const ParcelableSpan* what) override;
};

}/*endof namespace*/
#endif/*__SPANNABLE_STRING_H__*/
