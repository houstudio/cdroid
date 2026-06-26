#include <text/spannablestring.h>
#include <text/spanwatcher.h>
#include <text/textutils.h>   // TextUtils::utf16_utf8

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
            if (ignoreNoCopySpan && dynamic_cast<const NoCopySpan*>(span)) {
                continue;
            }
            int spanStart = spanned->getSpanStart(span);
            int spanEnd = spanned->getSpanEnd(span);
            int spanFlags = spanned->getSpanFlags(span);
            
            int newStart = std::max(start, spanStart) - start;
            int newEnd = std::min(end, spanEnd) - start;
            
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
    return TextUtils::utf16_utf8(mText);   // proper UTF-8 (was lossy: char16-as-byte)
}

std::u16string SpannableStringInternal::toU16String() const {
    return mText;   // fast path — mText IS the char16 buffer
}

SpannableStringInternal* SpannableStringInternal::subSequence(int start, int end) const {
    return new SpannableStringInternal(this, start, end);
}

size_t SpannableStringInternal::length() const {
    return mText.length();
}

int SpannableStringInternal::charAt(int idx) const {
    return mText.at(idx);
}

std::vector<const ParcelableSpan*> SpannableStringInternal::getSpans(int queryStart, int queryEnd, const SpanFilter& filter) const {
    std::vector<const ParcelableSpan*> result;
    
    for (const auto& t : mSpans) {
        const ParcelableSpan* span;
        int sstart, send, sflags;
        std::tie(span, sstart, send, sflags) = t;
        
        if (sstart > queryEnd || send < queryStart) {
            continue;
        }
        
        if (sstart != send && queryStart != queryEnd) {
            if (sstart == queryEnd || send == queryStart) {
                continue;
            }
        }
        
        if (!filter.test(span)) {
            continue;
        }
        
        const int priority = sflags & Spanned::SPAN_PRIORITY;
        auto it = result.begin();
        for (; it != result.end(); ++it) {
            int existingPriority = getSpanFlags(*it) & Spanned::SPAN_PRIORITY;
            if (priority > existingPriority) {
                break;
            }
        }
        result.insert(it, span);
    }
    
    return result;
}

int SpannableStringInternal::getSpanStart(const ParcelableSpan* what) const {
    for (auto it = mSpans.rbegin(); it != mSpans.rend(); ++it) {
        const ParcelableSpan* span;
        int sstart, send, sflags;
        std::tie(span, sstart, send, sflags) = *it;
        if (span == what) return sstart;
    }
    return -1;
}

int SpannableStringInternal::getSpanEnd(const ParcelableSpan* what) const {
    for (auto it = mSpans.rbegin(); it != mSpans.rend(); ++it) {
        const ParcelableSpan* span;
        int sstart, send, sflags;
        std::tie(span, sstart, send, sflags) = *it;
        if (span == what) return send;
    }
    return -1;
}

int SpannableStringInternal::getSpanFlags(const ParcelableSpan* what) const {
    for (auto it = mSpans.rbegin(); it != mSpans.rend(); ++it) {
        const ParcelableSpan* span;
        int sstart, send, sflags;
        std::tie(span, sstart, send, sflags) = *it;
        if (span == what) return sflags;
    }
    return 0;
}

int SpannableStringInternal::nextSpanTransition(int start, int limit, const SpanFilter& kind) const {
    int edge = limit;
    for (const auto& t : mSpans) {
        const ParcelableSpan* span;
        int sstart, send, sflags;
        std::tie(span, sstart, send, sflags) = t;
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

void SpannableStringInternal::sendSpanAdded(const ParcelableSpan* what, int start, int end) {
    Spanned& self = const_cast<SpannableStringInternal&>(*this);
    SpanFilter watcherFilter = make_span_filter<SpanWatcher>();
    auto watchers = getSpans(start, end, watcherFilter);
    for (const ParcelableSpan* w : watchers) {
        SpanWatcher* watcher = const_cast<SpanWatcher*>(dynamic_cast<const SpanWatcher*>(w));
        if (watcher) {
            watcher->onSpanAdded(self, what, start, end);
        }
    }
}

void SpannableStringInternal::sendSpanRemoved(const ParcelableSpan* what, int start, int end) {
    Spanned& self = const_cast<SpannableStringInternal&>(*this);
    SpanFilter watcherFilter = make_span_filter<SpanWatcher>();
    auto watchers = getSpans(start, end, watcherFilter);
    for (const ParcelableSpan* w : watchers) {
        SpanWatcher* watcher = const_cast<SpanWatcher*>(dynamic_cast<const SpanWatcher*>(w));
        if (watcher) {
            watcher->onSpanRemoved(self, what, start, end);
        }
    }
}

void SpannableStringInternal::sendSpanChanged(const ParcelableSpan* what, int ostart, int oend, int nstart, int nend) {
    Spanned& self = const_cast<SpannableStringInternal&>(*this);
    int start = std::min(ostart, nstart);
    int end = std::max(oend, nend);
    SpanFilter watcherFilter = make_span_filter<SpanWatcher>();
    auto watchers = getSpans(start, end, watcherFilter);
    for (const ParcelableSpan* w : watchers) {
        SpanWatcher* watcher = const_cast<SpanWatcher*>(dynamic_cast<const SpanWatcher*>(w));
        if (watcher) {
            watcher->onSpanChanged(self, what, ostart, oend, nstart, nend);
        }
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
        const ParcelableSpan* span;
        int sstart, send, sflags;
        std::tie(span, sstart, send, sflags) = t;
        if (send <= start || sstart >= end) continue;
        int spanStart = std::max(sstart, start) - start;
        int spanEnd = std::min(send, end) - start;
        result->mSpans.push_back({span, spanStart, spanEnd, sflags});
    }
    return result;
}

// SpannableString implementations
SpannableString::SpannableString(const CharSequence* source, bool ignoreNoCopySpan) 
    : SpannableStringInternal(source, 0, source ? source->length() : 0, ignoreNoCopySpan) {}

void SpannableString::setSpan(const ParcelableSpan* what, int start, int end, int flags) {
    if (!what) return;
    
    const int len = (int)mText.length();
    if (end < start) {
        throw std::out_of_range("setSpan has end before start");
    }
    if (start > len || end > len) {
        throw std::out_of_range("setSpan ends beyond length");
    }
    if (start < 0 || end < 0) {
        throw std::out_of_range("setSpan starts before 0");
    }
    
    if (start >= end) return;
    
    for (auto& t : mSpans) {
        const ParcelableSpan* span;
        int sstart, send, sflags;
        std::tie(span, sstart, send, sflags) = t;
        if (span == what) {
            this->sendSpanChanged(what, sstart, send, start, end);
            std::get<1>(t) = start;
            std::get<2>(t) = end;
            std::get<3>(t) = flags;
            return;
        }
    }
    
    mSpans.push_back({what, start, end, flags});
    this->sendSpanAdded(what, start, end);
}

void SpannableString::removeSpan(const ParcelableSpan* what) {
    for (auto it = mSpans.begin(); it != mSpans.end(); ++it) {
        const ParcelableSpan* span;
        int sstart, send, sflags;
        std::tie(span, sstart, send, sflags) = *it;
        if (span == what) {
            this->sendSpanRemoved(what, sstart, send);
            mSpans.erase(it);
            return;
        }
    }
}

}/*endof namespace*/
