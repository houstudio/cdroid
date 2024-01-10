#ifndef __LAYOUT_STATE_H__
#define __LAYOUT_STATE_H__
namespace cdroid{
class LayoutState {
public:
    static constexpr int LAYOUT_START = -1;
    static constexpr int LAYOUT_END = 1;
    static constexpr int INVALID_LAYOUT = INT_MIN;//Integer.MIN_VALUE;
    static constexpr int ITEM_DIRECTION_HEAD = -1;
    static constexpr int ITEM_DIRECTION_TAIL = 1;

    bool mRecycle = true;

    int mAvailable;
    int mCurrentPosition;
    int mItemDirection;
    int mLayoutDirection;
    int mStartLine = 0;
    int mEndLine = 0;
    bool mStopInFocusable;
    bool mInfinite;
    bool hasMore(RecyclerView::State& state) {
        return mCurrentPosition >= 0 && mCurrentPosition < state.getItemCount();
    }
    View* next(RecyclerView::Recycler& recycler) {
        View* view = recycler.getViewForPosition(mCurrentPosition);
        mCurrentPosition += mItemDirection;
        return view;
    }
};
}/*endof namespace */
#endif/*__LAYOUT_STATE_H__*/
