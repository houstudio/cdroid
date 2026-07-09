/*********************************************************************************
 * Port of android.text.SegmentFinder (android-36). Finds text segment boundaries
 * within text (grapheme clusters, words, etc.). Only the abstract base +
 * PrescribedSegmentFinder are ported here; the break-iterator-driven SegmentFinders
 * (GraphemeClusterSegmentFinder / WordSegmentFinder) are not ported yet.
 * Implementations live in segmentfinder.cc.
 *
 * NB: android nests PrescribedSegmentFinder inside SegmentFinder and has it `extends
 * SegmentFinder`. In C++ a nested class cannot inherit its enclosing class while the
 * enclosing class is still being defined, so PrescribedSegmentFinder is forward-declared
 * nested and defined out-of-line (qualified name SegmentFinder::PrescribedSegmentFinder
 * is preserved). Implementations live in segmentfinder.cc.
 *********************************************************************************/
#ifndef __SEGMENT_FINDER_H__
#define __SEGMENT_FINDER_H__
#include <vector>

namespace cdroid {

class SegmentFinder {
public:
    /** No boundary of the requested type in the requested direction. */
    static constexpr int DONE = -1;

    virtual ~SegmentFinder() = default;

    /** Offset of the previous segment start boundary before {@code offset}, or DONE. */
    virtual int previousStartBoundary(int offset) = 0;
    /** Offset of the previous segment end boundary before {@code offset}, or DONE. */
    virtual int previousEndBoundary(int offset) = 0;
    /** Offset of the next segment start boundary after {@code offset}, or DONE. */
    virtual int nextStartBoundary(int offset) = 0;
    /** Offset of the next segment end boundary after {@code offset}, or DONE. */
    virtual int nextEndBoundary(int offset) = 0;

    /** SegmentFinder based on given segment ranges. Defined out-of-line below. */
    class PrescribedSegmentFinder;
};

/**
 * SegmentFinder based on given segment ranges. Segments[i*2] is the i-th segment's start,
 * segments[i*2+1] its end. Segments must not overlap and must be sorted by start.
 */
class SegmentFinder::PrescribedSegmentFinder : public SegmentFinder {
public:
    explicit PrescribedSegmentFinder(const std::vector<int>& segments);
    int previousStartBoundary(int offset) override;
    int previousEndBoundary(int offset) override;
    int nextStartBoundary(int offset) override;
    int nextEndBoundary(int offset) override;
private:
    std::vector<int> mSegments;
    int findNext(int offset, bool isStart);
    int findPrevious(int offset, bool isStart);
};

}  // namespace cdroid
#endif  // __SEGMENT_FINDER_H__
