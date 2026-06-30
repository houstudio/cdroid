#include <text/inputfilter.h>

namespace cdroid {

InputFilter_LengthFilter::InputFilter_LengthFilter(int max)
    : mMax(max) {
}

CharSequence* InputFilter_LengthFilter::filter(CharSequence* source, int start, int end, Spanned* dest,
        int dstart, int dend) {
    int keep = mMax - (dest->length() - (dend - dstart));
    if (keep <= 0) {
        return nullptr;
    } else if (keep >= end - start) {
        return nullptr;
    } else {
        keep += start;
        return source->subSequence(start, keep);
    }
}

int InputFilter_LengthFilter::getMax() const {
    return mMax;
}

}
