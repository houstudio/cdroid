#include <text/inputfilter.h>
#include <text/character.h>
#include <text/textutils.h>

namespace cdroid {

InputFilter_LengthFilter::InputFilter_LengthFilter(int max)
    : mMax(max) {
}

// android.text.InputFilter.LengthFilter#filter, ported verbatim. Returning a non-null
// empty CharSequence *rejects* the edit; nullptr means keep the original replacement.
CharSequence* InputFilter_LengthFilter::filter(CharSequence* source, int start, int end,
        Spanned* dest, int dstart, int dend) {
    int keep = mMax - ((int)dest->length() - (dend - dstart));
    if (keep <= 0) {
        return new SpannableString(std::u16string()); // reject (empty)
    } else if (keep >= end - start) {
        return nullptr; // keep original
    } else {
        keep += start;
        // Don't split a supplementary character: back off past a high surrogate.
        if (Character::isHighSurrogate((char16_t)source->charAt(keep - 1))) {
            --keep;
            if (keep == start) {
                return new SpannableString(std::u16string()); // reject (empty)
            }
        }
        return source->subSequence(start, keep);
    }
}

int InputFilter_LengthFilter::getMax() const {
    return mMax;
}

InputFilter_AllCaps::InputFilter_AllCaps() {
}

// android.text.InputFilter.AllCaps#filter. CDROID has no CharSequenceWrapper, so the
// [start,end) view is obtained via subSequence (owned, deleted below). Spans are copied
// into the result by TextUtils::toUpperCase when the source is a Spanned.
CharSequence* InputFilter_AllCaps::filter(CharSequence* source, int start, int end,
        Spanned* /*dest*/, int /*dstart*/, int /*dend*/) {
    CharSequence* slice = source->subSequence(start, end);
    if (slice == nullptr) return nullptr; // keep original

    // Only transform if there is at least one lowercase or titlecase letter; otherwise
    // the uppercasing would be a no-op and we keep the original.
    bool lowerOrTitleFound = false;
    for (int i = 0; i < (int)slice->length();) {
        const int cp = Character::codePointAt(slice, i);
        if (Character::isLowerCase(cp) || Character::isTitleCase((char16_t)cp)) {
            lowerOrTitleFound = true;
            break;
        }
        i += Character::charCount(cp);
    }
    if (!lowerOrTitleFound) {
        delete slice;
        return nullptr; // keep original
    }

    const bool copySpans = (dynamic_cast<Spanned*>(source) != nullptr);
    CharSequence* upper = TextUtils::toUpperCase(slice, copySpans);
    delete slice;
    return upper; // toUpperCase returns a fresh object (never == slice)
}

}
