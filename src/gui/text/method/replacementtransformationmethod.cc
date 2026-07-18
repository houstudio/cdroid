/*
 * Ported from Android android.text.method.ReplacementTransformationMethod (Apache 2.0).
 *
 * Two inner CharSequence impls (ReplacementCharSequence / SpannedReplacementCharSequence)
 * mirror the source with selected characters replaced. CharSequence is inherited
 * VIRTUALLY so SpannedReplacementCharSequence can add Spanned without a diamond.
 */
#include <text/method/replacementtransformationmethod.h>
#include <text/String.h>
#include <text/spannablestring.h>   // Spanned, SpannedString
#include <text/textutils.h>

namespace cdroid {
namespace {

// Mirrors `source` but reads each char in `original` as its `replacement` peer.
class ReplacementCharSequence : public virtual CharSequence {
public:
    ReplacementCharSequence(CharSequence* source, const std::u16string& original,
                            const std::u16string& replacement)
        : mSource(source), mOriginal(original), mReplacement(replacement) {}

    size_t length() const override { return mSource->length(); }

    int charAt(int i) const override {
        const char16_t c = (char16_t)mSource->charAt(i);
        const size_t pos = mOriginal.find(c);
        return (pos != std::u16string::npos) ? (int)mReplacement[pos] : (int)c;
    }

    CharSequence* subSequence(int start, int end) const override {
        std::u16string out;
        out.reserve(end - start);
        for (int i = start; i < end; i++) out += (char16_t)charAt(i);
        return new SpannedString(out);
    }

    void getChars(int start, int end, char16_t* dest, int destPos) const override {
        mSource->getChars(start, end, dest, destPos);
        const int offend = destPos + (end - start);
        for (int i = destPos; i < offend; i++) {
            const size_t pos = mOriginal.find(dest[i]);
            if (pos != std::u16string::npos) dest[i] = mReplacement[pos];
        }
    }

    String* toString() const override {
        return new String(toUTF16());
    }

    std::string toUTF8() const override {
        return TextUtils::utf16_utf8(toUTF16());
    }

    std::u16string toUTF16() const override {
        std::u16string out;
        const int n = (int)length();
        out.reserve(n);
        for (int i = 0; i < n; i++) out += (char16_t)charAt(i);
        return out;
    }

protected:
    CharSequence* mSource;
    std::u16string mOriginal;
    std::u16string mReplacement;
};

// Adds Spanned forwarding so spans (selection, watchers, styles) are preserved
// across the replacement — needed when the source is an Editable/Spanned.
class SpannedReplacementCharSequence
        : public ReplacementCharSequence, public virtual Spanned {
public:
    SpannedReplacementCharSequence(Spanned* source, const std::u16string& original,
                                   const std::u16string& replacement)
        : ReplacementCharSequence(source, original, replacement), mSpanned(source) {}

    CharSequence* subSequence(int start, int end) const override {
        // Android: new SpannedString(this).subSequence(start, end) — materialize the
        // replaced text (via this->getChars) + forwarded spans into a SpannedString.
        SpannedString* copy = new SpannedString(this);
        CharSequence* sub = copy->subSequence(start, end);
        delete copy;
        return sub;
    }

    std::vector<const ParcelableSpan*> getSpans(int start, int end,
            const SpanFilter& kind) const override {
        return mSpanned->getSpans(start, end, kind);
    }
    int getSpanStart(const ParcelableSpan* what) const override {
        return mSpanned->getSpanStart(what);
    }
    int getSpanEnd(const ParcelableSpan* what) const override {
        return mSpanned->getSpanEnd(what);
    }
    int getSpanFlags(const ParcelableSpan* what) const override {
        return mSpanned->getSpanFlags(what);
    }
    int nextSpanTransition(int start, int limit, const SpanFilter& kind) const override {
        return mSpanned->nextSpanTransition(start, limit, kind);
    }

private:
    Spanned* mSpanned;
};

} // namespace

CharSequence* ReplacementTransformationMethod::getTransformation(CharSequence& source, View& /*view*/) {
    const std::u16string original = getOriginal();
    const std::u16string replacement = getReplacement();
    if (Spanned* spanned = dynamic_cast<Spanned*>(&source)) {
        return new SpannedReplacementCharSequence(spanned, original, replacement);
    }
    return new ReplacementCharSequence(&source, original, replacement);
}

} // namespace cdroid
