#ifndef ANDROID_GUI_TEXT_INPUT_FILTER_H
#define ANDROID_GUI_TEXT_INPUT_FILTER_H

#include <text/spannablestring.h>

namespace cdroid {

class InputFilter {
public:
    virtual ~InputFilter() = default;
    virtual CharSequence* filter(CharSequence* source, int start, int end, Spanned* dest, int dstart, int dend) = 0;
};

class InputFilter_LengthFilter : public InputFilter {
private:
    int mMax;
public:
    InputFilter_LengthFilter(int max);
    CharSequence* filter(CharSequence* source, int start, int end, Spanned* dest, int dstart, int dend) override;
    int getMax() const;
};

}

#endif
