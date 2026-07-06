#include <text/spannablestring.h>
#include <text/spanwatcher.h>
#include <text/textutils.h>   // TextUtils::utf16_utf8

namespace cdroid {

// --- SpannableStringInternal centralized span mutation (owned/borrowed) -----

void SpannableStringInternal::addSpan(const ParcelableSpan* span, int start, int end, int flags) {
    // The public API is const-correct (Android's setSpan takes a non-const
    // Object; CDROID passes const ParcelableSpan*). Every span is a non-const
    // heap allocation reached through a const pointer only for API convenience,
    // so casting away const here to later manage/delete it is well-defined.
    // This is the ONLY const_cast in the span subsystem.
    ParcelableSpan* s = const_cast<ParcelableSpan*>(span);
    const bool owned = (dynamic_cast<NoCopySpan*>(s) == nullptr);
    mSpans.push_back({s, start, end, flags, owned});
}

bool SpannableStringInternal::removeSpanRecord(const ParcelableSpan* span) {
    for (auto it = mSpans.begin(); it != mSpans.end(); ++it) {
        if (it->span == span) {
            disposeSpan(*it);
            mSpans.erase(it);
            return true;
        }
    }
    return false;
}

void SpannableStringInternal::deleteAllOwnedSpans() {
    for (auto& r : mSpans) {
        disposeSpan(r);
    }
    mSpans.clear();
}

void SpannableStringInternal::appendSpanCopy(std::vector<SpanRecord>& dest,
        const ParcelableSpan* srcSpan, int newStart, int newEnd, int flags,
        bool ignoreNoCopySpan) {
    const bool isNoCopy = (dynamic_cast<const NoCopySpan*>(srcSpan) != nullptr);
    if (ignoreNoCopySpan && isNoCopy) {
        return;  // Android: NoCopySpan is skipped on slice/copy
    }
    if (isNoCopy) {
        // Borrowed: share the raw pointer; never deleted, never cloned.
        dest.push_back({const_cast<ParcelableSpan*>(srcSpan), newStart, newEnd, flags, false});
    } else {
        // Owned: clone so the destination owns an independent copy (avoids two
        // containers deleting the same object).
        ParcelableSpan* c = srcSpan->clone();
        assert(c && "owned span subclass forgot to override clone()");
        dest.push_back({c, newStart, newEnd, flags, true});
    }
}

void SpannableStringInternal::disposeSpan(SpanRecord& r) {
    // Free the span iff this container owns it (i.e. it is a non-NoCopySpan).
    // Borrowed (NoCopySpan) spans — watchers, selection markers — are owned
    // elsewhere and are never deleted here.
    if (r.owned) {
        delete r.span;
        r.span = nullptr;
        r.owned = false;
    }
}

SpannableStringInternal::~SpannableStringInternal() {
    deleteAllOwnedSpans();
}

SpannableStringInternal::SpannableStringInternal(const SpannableStringInternal& o)
    : mText(o.mText) {
    // Clone owned spans, share borrowed (NoCopySpan) ones. ignoreNoCopy=false
    // so borrowed spans are carried along (shared by pointer, never deleted).
    for (const auto& r : o.mSpans) {
        appendSpanCopy(mSpans, r.span, r.start, r.end, r.flags, /*ignoreNoCopy=*/false);
    }
}

SpannableStringInternal& SpannableStringInternal::operator=(const SpannableStringInternal& o) {
    if (this == &o) return *this;
    deleteAllOwnedSpans();   // release our current owned spans first
    mText = o.mText;
    for (const auto& r : o.mSpans) {
        appendSpanCopy(mSpans, r.span, r.start, r.end, r.flags, /*ignoreNoCopy=*/false);
    }
    return *this;
}

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
            int spanStart = spanned->getSpanStart(span);
            int spanEnd = spanned->getSpanEnd(span);
            int spanFlags = spanned->getSpanFlags(span);

            int newStart = std::max(start, spanStart) - start;
            int newEnd = std::min(end, spanEnd) - start;

            if (newStart < newEnd) {
                appendSpanCopy(mSpans, span, newStart, newEnd, spanFlags, ignoreNoCopySpan);
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

    for (const auto& r : mSpans) {
        if (r.start > queryEnd || r.end < queryStart) {
            continue;
        }

        if (r.start != r.end && queryStart != queryEnd) {
            if (r.start == queryEnd || r.end == queryStart) {
                continue;
            }
        }

        if (!filter.test(r.span)) {
            continue;
        }

        const int priority = r.flags & Spanned::SPAN_PRIORITY;
        auto it = result.begin();
        for (; it != result.end(); ++it) {
            int existingPriority = getSpanFlags(*it) & Spanned::SPAN_PRIORITY;
            if (priority > existingPriority) {
                break;
            }
        }
        result.insert(it, r.span);
    }

    return result;
}

int SpannableStringInternal::getSpanStart(const ParcelableSpan* what) const {
    for (auto it = mSpans.rbegin(); it != mSpans.rend(); ++it) {
        if (it->span == what) return it->start;
    }
    return -1;
}

int SpannableStringInternal::getSpanEnd(const ParcelableSpan* what) const {
    for (auto it = mSpans.rbegin(); it != mSpans.rend(); ++it) {
        if (it->span == what) return it->end;
    }
    return -1;
}

int SpannableStringInternal::getSpanFlags(const ParcelableSpan* what) const {
    for (auto it = mSpans.rbegin(); it != mSpans.rend(); ++it) {
        if (it->span == what) return it->flags;
    }
    return 0;
}

int SpannableStringInternal::nextSpanTransition(int start, int limit, const SpanFilter& kind) const {
    int edge = limit;
    for (const auto& r : mSpans) {
        if (kind.test(r.span)) {
            if (r.start > start && r.start < edge) edge = r.start;
            if (r.end > start && r.end < edge) edge = r.end;
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

void SpannableString::sendSpanAdded(const ParcelableSpan* what, int start, int end) {
    Spannable& self = dynamic_cast<SpannableString&>(*this);
    SpanFilter watcherFilter = make_span_filter<SpanWatcher>();
    auto watchers = getSpans(start, end, watcherFilter);
    for (const ParcelableSpan* w : watchers) {
        SpanWatcher* watcher = const_cast<SpanWatcher*>(dynamic_cast<const SpanWatcher*>(w));
        if (watcher) {
            watcher->onSpanAdded(self, what, start, end);
        }
    }
}

void SpannableString::sendSpanRemoved(const ParcelableSpan* what, int start, int end) {
    Spannable& self = dynamic_cast<SpannableString&>(*this);
    SpanFilter watcherFilter = make_span_filter<SpanWatcher>();
    auto watchers = getSpans(start, end, watcherFilter);
    for (const ParcelableSpan* w : watchers) {
        SpanWatcher* watcher = const_cast<SpanWatcher*>(dynamic_cast<const SpanWatcher*>(w));
        if (watcher) {
            watcher->onSpanRemoved(self, what, start, end);
        }
    }
}

void SpannableString::sendSpanChanged(const ParcelableSpan* what, int ostart, int oend, int nstart, int nend) {
    Spannable& self = dynamic_cast<SpannableString&>(*this);
    SpanFilter watcherFilter = make_span_filter<SpanWatcher>();
    auto watchers = getSpans(std::min(ostart,nstart), std::max(oend,nend), watcherFilter);
    for (const ParcelableSpan* w : watchers) {
        SpanWatcher* watcher = const_cast<SpanWatcher*>(dynamic_cast<const SpanWatcher*>(w));
        if (watcher) {
            watcher->onSpanChanged(self, what, ostart, oend, nstart, nend);
        }
    }
}

SpannedString::SpannedString(const CharSequence* source)
    : SpannableStringInternal(source) {
}
// SpannedString implementations
SpannedString* SpannedString::subSequence(int start, int end) const {
    if (start < 0) start = 0;
    if (end > (int)mText.length()) end = (int)mText.length();
    if (start >= end) return new SpannedString();
    SpannedString* result = new SpannedString();
    result->mText = mText.substr(start, end - start);
    for (const auto& r : mSpans) {
        if (r.end <= start || r.start >= end) continue;
        int spanStart = std::max(r.start, start) - start;
        int spanEnd = std::min(r.end, end) - start;
        appendSpanCopy(result->mSpans, r.span, spanStart, spanEnd, r.flags, /*ignoreNoCopy=*/false);
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

    for (auto& r : mSpans) {
        if (r.span == what) {
            this->sendSpanChanged(what, r.start, r.end, start, end);
            r.start = start;
            r.end = end;
            r.flags = flags;
            return;
        }
    }

    addSpan(what, start, end, flags);
    this->sendSpanAdded(what, start, end);
}

void SpannableString::removeSpan(const ParcelableSpan* what) {
    for (auto it = mSpans.begin(); it != mSpans.end(); ++it) {
        if (it->span == what) {
            this->sendSpanRemoved(what, it->start, it->end);
            disposeSpan(*it);
            mSpans.erase(it);
            return;
        }
    }
}

}/*endof namespace*/
