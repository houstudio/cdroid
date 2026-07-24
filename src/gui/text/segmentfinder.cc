#include <text/segmentfinder.h>
#include <algorithm>
#include <climits>
#include <stdexcept>
#include <cstdint>

namespace cdroid {

namespace {

// Mimics java.util.Arrays.binarySearch: returns the index of key if present,
// otherwise (-(insertion point) - 1).
int binarySearch(const std::vector<int>& a, int key) {
    auto it = std::lower_bound(a.begin(), a.end(), key);
    const int idx = static_cast<int>(it - a.begin());
    if (it != a.end() && *it == key) return idx;       // found
    return -idx - 1;                                     // not found: -(insertion) - 1
}

void checkSegmentsValid(const std::vector<int>& segments) {
    if (segments.size() % 2 != 0) {
        throw std::invalid_argument("the length of segments must be even");
    }
    if (segments.empty()) return;
    int lastSegmentEnd = INT_MIN;
    for (size_t index = 0; index < segments.size(); index += 2) {
        if (segments[index] < lastSegmentEnd) {
            throw std::invalid_argument("segments can't overlap");
        }
        if (segments[index] >= segments[index + 1]) {
            throw std::invalid_argument("the segment range can't be empty");
        }
        lastSegmentEnd = segments[index + 1];
    }
}

}  // namespace

/////////////////////////////////////////////////////////////////////////////////////////////////

SegmentFinder::PrescribedSegmentFinder::PrescribedSegmentFinder(const std::vector<int>& segments) {
    checkSegmentsValid(segments);
    mSegments = segments;
}

int SegmentFinder::PrescribedSegmentFinder::previousStartBoundary(int offset) {
    return findPrevious(offset, /* isStart = */ true);
}

int SegmentFinder::PrescribedSegmentFinder::previousEndBoundary(int offset) {
    return findPrevious(offset, /* isStart = */ false);
}

int SegmentFinder::PrescribedSegmentFinder::nextStartBoundary(int offset) {
    return findNext(offset, /* isStart = */ true);
}

int SegmentFinder::PrescribedSegmentFinder::nextEndBoundary(int offset) {
    return findNext(offset, /* isStart = */ false);
}

int SegmentFinder::PrescribedSegmentFinder::findNext(int offset, bool isStart) {
    if (offset < 0) return DONE;
    if (mSegments.empty() || offset > mSegments.back()) return DONE;

    if (offset < mSegments.front()) {
        return isStart ? mSegments[0] : mSegments[1];
    }

    int index = binarySearch(mSegments, offset);
    if (index >= 0) {
        // mSegments may have duplicate elements (a segment end equals the next segment's start).
        // Move the index forwards since we are searching for the next segment.
        if (index + 1 < (int)mSegments.size() && mSegments[index + 1] == offset) {
            index = index + 1;
        }
        // Point the index to the first segment boundary larger than the given offset.
        index += 1;
    } else {
        // binarySearch returns the insertion point, the first segment boundary larger than offset.
        index = -(index + 1);
    }
    if (index >= (int)mSegments.size()) return DONE;

    //  +---------------------------------------+
    //  |               | isStart   | isEnd     |
    //  |---------------+-----------+-----------|
    //  | indexIsStart  | index     | index + 1 |
    //  |---------------+-----------+-----------|
    //  | indexIsEnd    | index + 1 | index     |
    //  +---------------------------------------+
    const bool indexIsStart = index % 2 == 0;
    if (isStart != indexIsStart) {
        return (index + 1 < (int)mSegments.size()) ? mSegments[index + 1] : DONE;
    }
    return mSegments[index];
}

int SegmentFinder::PrescribedSegmentFinder::findPrevious(int offset, bool isStart) {
    if (mSegments.empty() || offset < mSegments.front()) return DONE;

    if (offset > mSegments.back()) {
        return isStart ? mSegments[mSegments.size() - 2] : mSegments[mSegments.size() - 1];
    }

    int index = binarySearch(mSegments, offset);
    if (index >= 0) {
        // mSegments may have duplicate elements (a segment end equals the next segment's start).
        // Move the index backwards since we are searching for the previous segment.
        if (index > 0 && mSegments[index - 1] == offset) {
            index = index - 1;
        }
        // Point the index to the first segment boundary smaller than the given offset.
        index -= 1;
    } else {
        // binarySearch returns the insertion point; insertionPoint - 1 is the first segment
        // boundary smaller than the given offset.
        index = -(index + 1) - 1;
    }
    if (index < 0) return DONE;

    //  +---------------------------------------+
    //  |               | isStart   | isEnd     |
    //  |---------------+-----------+-----------|
    //  | indexIsStart  | index     | index - 1 |
    //  |---------------+-----------+-----------|
    //  | indexIsEnd    | index - 1 | index     |
    //  +---------------------------------------+
    const bool indexIsStart = index % 2 == 0;
    if (isStart != indexIsStart) {
        return (index > 0) ? mSegments[index - 1] : DONE;
    }
    return mSegments[index];
}

}  // namespace cdroid
