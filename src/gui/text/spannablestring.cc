#include <text/spannablestring.h>

namespace cdroid {

// SpannableStringInternal implementations
SpannableStringInternal::SpannableStringInternal(const CharSequence* source, int start, int end, bool ignoreNoCopySpan) {
    if (source == nullptr) return;
    const int len = source->length();
    if (start < 0) start = 0;
    if (end > len) end = len;
    if (start >= end) return;
    
    mText.resize(end - start);
    for (int i = 0; i < end - start; i++) {
        mText[i] = source->charAt(start + i);
    }
    
    const Spanned* spanned = dynamic_cast<const Spanned*>(source);
    if (spanned != nullptr) {
        const SpanFilter filter;
        auto spans = spanned->getSpans(start, end, filter);
        for (const ParcelableSpan* span : spans) {
            (void)ignoreNoCopySpan;
            int spanStart = spanned->getSpanStart(span);
            int spanEnd = spanned->getSpanEnd(span);
            int spanFlags = spanned->getSpanFlags(span);
            
            int newStart = std::max(0, spanStart - start);
            int newEnd = std::min(end - start, spanEnd - start);
            
            if (newStart < newEnd) {
                mSpans.emplace_back(span, newStart, newEnd, spanFlags);
            }
        }
    }
}

SpannableStringInternal::SpannableStringInternal(const CharSequence* source, int start, int end)
    : SpannableStringInternal(source, start, end, false) {}

SpannableStringInternal::SpannableStringInternal(const CharSequence* source)
    : SpannableStringInternal(source, 0, source ? source->length() : 0, false) {}

std::string SpannableStringInternal::toString() const {
    return "";
}

size_t SpannableStringInternal::length() const {
    return mText.length();
}

int SpannableStringInternal::charAt(int idx) const {
    return mText.at(idx);
}

std::vector<const ParcelableSpan*> SpannableStringInternal::getSpans(int start, int end, const SpanFilter& filter) const {
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

int SpannableStringInternal::getSpanStart(const ParcelableSpan* what) const {
    for (const auto& t : mSpans) {
        const ParcelableSpan*span;
        int sstart,send,sflags;
        std::tie(span,sstart,send,sflags) = t;
        if (span == what) return sstart;
    }
    return -1;
}

int SpannableStringInternal::getSpanEnd(const ParcelableSpan* what) const {
    for (auto& t : mSpans) {
        const ParcelableSpan*span;
        int sstart,send,sflags;
        std::tie(span,sstart,send,sflags) = t;
        if (span == what) return send;
    }
    return -1;
}

int SpannableStringInternal::getSpanFlags(const ParcelableSpan* what) const {
    for (auto t : mSpans) {
        const ParcelableSpan*span;
        int sstart,send,sflags;
        std::tie(span,sstart,send,sflags) = t;
        if (span == what) return sflags;
    }
    return 0;
}

int SpannableStringInternal::nextSpanTransition(int start, int limit, const SpanFilter& kind) const {
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

void SpannableStringInternal::getChars(int start, int end, char16_t* dest, int destPos) const {
    if (start >= end) return;
    for(int i=0; i<end-start; i++) {
        dest[destPos+i] = mText[start+i];
    }
}

// SpannedString implementations
SpannedString* SpannedString::subSequence(int start, int end) const {
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

// SpannableString implementations
SpannableString::SpannableString(const CharSequence*, bool ignoreNoCopySpan) {}

void SpannableString::setSpan(const ParcelableSpan* what, int start, int end, int flags) {
    if (!what) return;
    if (start < 0) start = 0;
    if (end > (int)mText.length()) end = (int)mText.length();
    if (start >= end) return;
    mSpans.push_back({what, start, end, flags});
}

void SpannableString::removeSpan(const ParcelableSpan* what) {
    mSpans.erase(std::remove_if(mSpans.begin(), mSpans.end(), 
        [&](const std::tuple<const ParcelableSpan*,int,int,int>& s){ 
            return std::get<0>(s) == what; 
        }), mSpans.end());
}

}/*endof namespace*/
