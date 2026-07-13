#ifndef ANDROID_GUI_TEXT_INPUT_FILTER_H
#define ANDROID_GUI_TEXT_INPUT_FILTER_H

#include <text/spannablestring.h>

namespace cdroid {

// android.text.InputFilter — faithful C++ port. Filters are attached to an Editable
// to constrain edits; each filter() may return an owned replacement CharSequence or
// nullptr to accept the original replacement. (Empty CharSequence return = reject.)
class InputFilter {
public:
    class AllCaps;
    class LengthFilter;
    virtual ~InputFilter() = default;
    virtual CharSequence* filter(CharSequence* source, int start, int end,
            Spanned* dest, int dstart, int dend) = 0;
};

// android.text.InputFilter.LengthFilter — caps the total text length at mMax.
class InputFilter::LengthFilter : public InputFilter {
private:
    int mMax;
public:
    explicit LengthFilter(int max);
    CharSequence* filter(CharSequence* source, int start, int end,
            Spanned* dest, int dstart, int dend) override;
    int getMax() const;
};

// android.text.InputFilter.AllCaps — uppercases lowercase/titlecase letters added
// through edits. Locale-insensitive in CDROID (no Locale wiring yet), matching
// AllCapsTransformationMethod; spans are preserved when the source is a Spanned.
class InputFilter::AllCaps : public InputFilter {
public:
    AllCaps();
    CharSequence* filter(CharSequence* source, int start, int end,
            Spanned* dest, int dstart, int dend) override;
};

}

#endif
