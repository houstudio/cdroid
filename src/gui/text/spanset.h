#ifndef __SPAN_SET_H__
#define __SPAN_SET_H__
#include <core/predicate.h>
namespace cdroid{
class SpanSet{
public:
    int numberOfSpans;
    std::vector<ParcelableSpan*> spans;
    std::vector<int> spanStarts;
    std::vector<int> spanEnds;
    std::vector<int> spanFlags;
public:
    SpanSet(/*const SpanFilter&type*/) {
        //classType = type;
        numberOfSpans = 0;
    }

    void init(Spanned* spanned, int start, int limit,const SpanFilter& classType) {
        auto allSpans = spanned->getSpans(start, limit, classType);
        const int length = allSpans.size();

        if (length > 0 && (spans.empty() || spans.size() < length)) {
            // These arrays may end up being too large because of the discarded empty spans
            spans.resize(length);// = (E[]) Array.newInstance(classType, length);
            spanStarts.resize(length);// = new int[length];
            spanEnds.resize(length) ;// = new int[length];
            spanFlags.resize(length);// = new int[length];
        }

        const int prevNumberOfSpans = numberOfSpans;
        numberOfSpans = 0;
        for (int i = 0; i < length; i++) {
            auto span = allSpans[i];

            const int spanStart = spanned->getSpanStart(span);
            const int spanEnd = spanned->getSpanEnd(span);
            if (spanStart == spanEnd) continue;

            const int spanFlag = spanned->getSpanFlags(span);

            spans[numberOfSpans] = span;
            spanStarts[numberOfSpans] = spanStart;
            spanEnds[numberOfSpans] = spanEnd;
            spanFlags[numberOfSpans] = spanFlag;

            numberOfSpans++;
        }

        // cleanup extra spans left over from previous init() call
        if (numberOfSpans < prevNumberOfSpans) {
            // prevNumberofSpans was > 0, therefore spans != null
            //Arrays.fill(spans, numberOfSpans, prevNumberOfSpans, null);
        }
    }

    bool hasSpansIntersecting(int start, int end) {
        for (int i = 0; i < numberOfSpans; i++) {
            // equal test is valid since both intervals are not empty by construction
            if (spanStarts[i] >= end || spanEnds[i] <= start) continue;
            return true;
        }
        return false;
    }

    int getNextTransition(int start, int limit) {
        for (int i = 0; i < numberOfSpans; i++) {
            const int spanStart = spanStarts[i];
            const int spanEnd = spanEnds[i];
            if (spanStart > start && spanStart < limit) limit = spanStart;
            if (spanEnd > start && spanEnd < limit) limit = spanEnd;
        }
        return limit;
    }

    void recycle() {
        if (!spans.empty()) {
            //Arrays.fill(spans, 0, numberOfSpans, null);
        }
    }
};
}/**/
#endif/*__SPAN_SET_H__*/
