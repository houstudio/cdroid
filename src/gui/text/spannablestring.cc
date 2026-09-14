#include <text/spannablestring.h>
#include <text/String.h>
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
    ++mMutationEpoch;
}

bool SpannableStringInternal::removeSpanRecord(const ParcelableSpan* span) {
    for (auto it = mSpans.begin(); it != mSpans.end(); ++it) {
        if (it->span == span) {
            disposeSpan(*it);
            mSpans.erase(it);
            ++mMutationEpoch;
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
    ++mMutationEpoch;
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

            /*AOSP copySpansFromSpanned keeps EVERYTHING getSpans() returned —
              including ZERO-LENGTH spans (Selection markers) at any offset in
              [start, end]; the old `newStart < newEnd` test silently dropped
              them. getSpans above already applied AOSP's isOutOfCopyRange
              predicate (out-of-range or boundary-touching non-empty spans),
              so no further filtering is needed here.*/
            appendSpanCopy(mSpans, span, newStart, newEnd, spanFlags, ignoreNoCopySpan);
        }
    }
}

SpannableStringInternal::SpannableStringInternal(const CharSequence* source, int start, int end)
    : SpannableStringInternal(source, start, end, false) {}

SpannableStringInternal::SpannableStringInternal(const CharSequence* source)
    : SpannableStringInternal(source, 0, source ? source->length() : 0, false) {}

String* SpannableStringInternal::toString() const {
    return new String(mText);
}

std::string SpannableStringInternal::toUTF8() const {
    return TextUtils::utf16_utf8(mText);   // proper UTF-8 (was lossy: char16-as-byte)
}

std::u16string SpannableStringInternal::toUTF16() const {
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
    /*Zero-priority spans come back in storage order, priority spans jump
      ahead of lower priorities — AOSP walks its interval-tree ARRAY linearly,
      and the tree layout (built by insertions + rotations) yields insertion
      order for the simple cases (its CTS test asserts priority-then-insertion;
      the sortsByPriorityEvenWhenSortParamIsFalse case additionally depends on
      the tree's rotation layout, which the flat vector does not replicate —
      that one test stays a known red until the interval tree is ported).*/
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

/*AOSP's sendSpan* snapshots the watchers and calls them all — safe because
  GC keeps a detached watcher's Java object alive. Under raw pointers a
  watcher removed (and freed) by an earlier callback leaves dangling entries
  in the snapshot, so each recipient is re-checked against the live span set
  first (the same isRecorded guard SpannableStringBuilder uses for its
  TextWatcher phases). Divergence: a watcher detached mid-notification does
  not receive the remaining events; a deleted C++ span cannot be called.
  The O(spans) getSpanStart rescan is skipped while mMutationEpoch is
  unchanged since the snapshot (no callback has touched the span set).*/

void SpannableString::sendSpanAdded(const ParcelableSpan* what, int start, int end) {
    Spannable& self = dynamic_cast<SpannableString&>(*this);
    SpanFilter watcherFilter = make_span_filter<SpanWatcher>();
    auto watchers = getSpans(start, end, watcherFilter);
    const uint64_t epoch0 = mMutationEpoch;
    for (const ParcelableSpan* w : watchers) {
        if (mMutationEpoch != epoch0 && getSpanStart(w) < 0) continue;
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
    const uint64_t epoch0 = mMutationEpoch;
    for (const ParcelableSpan* w : watchers) {
        if (mMutationEpoch != epoch0 && getSpanStart(w) < 0) continue;
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
    const uint64_t epoch0 = mMutationEpoch;
    for (const ParcelableSpan* w : watchers) {
        if (mMutationEpoch != epoch0 && getSpanStart(w) < 0) continue;
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

    // Zero-length spans (start == end) are legal and MUST be stored
    // (SpannableStringInternal.setSpan / checkRange only rejects start > end):
    // Selection's SELECTION_START/SELECTION_END markers are zero-length, so
    // dropping them made every Selection on a SpannableString a silent no-op.

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
            ++mMutationEpoch;
            return;
        }
    }
}

}/*endof namespace*/
