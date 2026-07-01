#ifndef __PARCELABLE_SPAN_H__
#define __PARCELABLE_SPAN_H__
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <core/predicate.h>
namespace cdroid{

class Paint;
class TextPaint;
class CharacterStylePassthrough;

class ParcelableSpan {
public:
    virtual ~ParcelableSpan() = default;
    // Deep copy used when a Spanned carrying OWNED spans is copied/sliced
    // (see SpannableStringInternal). NON-pure: the root is directly instantiated
    // (Selection markers do `new ParcelableSpan()`), so it returns nullptr by
    // default. Concrete owned value-spans override this covariantly, returning
    // their own type; the copy path asserts non-null to catch a forgotten
    // override. NoCopySpan/borrowed spans inherit the default and are never
    // cloned (they are skipped on copy, matching Android's ignoreNoCopySpan).
    virtual ParcelableSpan* clone() const { return nullptr; }
};

class CharSequence : virtual public ParcelableSpan {
public:
    virtual ~CharSequence() = default;
    virtual size_t length() const = 0;
    virtual int charAt(int) const = 0;
    virtual CharSequence* subSequence(int, int) const { return nullptr; }
    virtual void getChars(int start, int end, char16_t* dest, int destPos) const = 0;

    virtual std::string toString() const = 0;
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

class UpdateAppearance : public ParcelableSpan {
public:
    virtual ~UpdateAppearance() = default;
};

class UpdateLayout : public UpdateAppearance {
public:
    virtual ~UpdateLayout() = default;
};

class CharacterStyle : virtual public ParcelableSpan {
public:
    virtual ~CharacterStyle() = default;
    virtual void updateDrawState(TextPaint& paint) const {}

    static CharacterStyle* wrap(CharacterStyle* cs);

    virtual CharacterStyle* getUnderlying() const {
        return const_cast<CharacterStyle*>(this);
    }
};

class SuggestionSpan : public CharacterStyle {
public:
    virtual ~SuggestionSpan() = default;
    void updateDrawState(TextPaint& paint) const override {}
    SuggestionSpan* clone() const override { return new SuggestionSpan(*this); }
};

class SpellCheckSpan : public ParcelableSpan {
public:
    virtual ~SpellCheckSpan() = default;
};

class CharacterStylePassthrough : public CharacterStyle {
public:
    explicit CharacterStylePassthrough(CharacterStyle* cs) : mStyle(cs) {}
    void updateDrawState(TextPaint& paint) const override {
        if (mStyle) mStyle->updateDrawState(paint);
    }
    CharacterStyle* getUnderlying() const override {
        return mStyle ? mStyle->getUnderlying() : nullptr;
    }
    // mStyle is BORROWED (owned elsewhere, e.g. by CharacterStyle::wrap caller);
    // the copy shallow-copies the pointer, preserving the borrow — it must NOT
    // transfer or clone mStyle's ownership.
    CharacterStylePassthrough* clone() const override { return new CharacterStylePassthrough(*this); }
private:
    CharacterStyle* mStyle;
};

class NoCopySpan:virtual public ParcelableSpan{
};

inline CharacterStyle* CharacterStyle::wrap(CharacterStyle* cs) {
    if (cs == nullptr) return nullptr;
    return new CharacterStylePassthrough(cs);
}

using SpanFilter=Predicate<const ParcelableSpan*>;

template<typename T>
SpanFilter make_span_filter() {
    return SpanFilter([](const ParcelableSpan* o) -> bool {
        return dynamic_cast<const T*>(o) != nullptr;
    });
}

}/*endof namespace*/
#endif/*__PARCELABLE_SPAN_H__*/
