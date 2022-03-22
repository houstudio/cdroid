#ifndef __ADAPTERVIEW_ANIMATOR_H__
#define __ADAPTERVIEW_ANIMATOR_H__
#include <widget/adapterview.h>
#include <widget/framelayout.h>

namespace cdroid{

class AdapterViewAnimator:public AdapterView{
private:
    static constexpr int DEFAULT_ANIMATION_DURATION = 200;
    /* Private touch states.*/
    static constexpr int TOUCH_MODE_NONE = 0;
    static constexpr int TOUCH_MODE_DOWN_IN_CURRENT_VIEW = 1;
    static constexpr int TOUCH_MODE_HANDLED = 2;
    int mRestoreWhichChild = -1;
    /* Current touch state.*/
    Runnable mPendingCheckForTap;
    int mTouchMode = TOUCH_MODE_NONE;

    class ViewAndMetaData {
    public:
        View* view;
        int relativeIndex;
        int adapterPosition;
        long itemId;
    public:
        ViewAndMetaData(View* view, int relativeIndex, int adapterPosition, long itemId);
    };
protected:
    int mWhichChild = 0;

    /* The index of the child to restore after the asynchronous connection from the
     * RemoteViewsAdapter has been. */

    /* Whether or not the first view(s) should be animated in*/
    bool mAnimateFirstTime = true;

    /*  Represents where the in the current window of
     *  views the current <code>mDisplayedChild</code> sits */
    int mActiveOffset = 0;

    /**The number of views that the {@link AdapterViewAnimator} keeps as children at any
     * given time (not counting views that are pending removal, see {@link #mPreviousViews}). */
    int mMaxNumActiveViews = 1;

    /* Map of the children of the {@link AdapterViewAnimator}.*/
    std::map<int, ViewAndMetaData*> mViewsMap;

    /* List of views pending removal from the {@link AdapterViewAnimator}*/
    std::vector<int> mPreviousViews;

    /* The index, relative to the adapter, of the beginning of the window of views */
    int mCurrentWindowStart = 0;

    /* The index, relative to the adapter, of the end of the window of views */
    int mCurrentWindowEnd = -1;

    /* The same as {@link #mCurrentWindowStart}, except when the we have bounded
     * {@link #mCurrentWindowStart} to be non-negative  */
    int mCurrentWindowStartUnbounded = 0;

    /* Listens for data changes from the adapter */
    DataSetObserver* mDataSetObserver;

    /* The {@link Adapter} for this {@link AdapterViewAnimator} */
    Adapter* mAdapter;

    /* The {@link RemoteViewsAdapter} for this {@link AdapterViewAnimator}*/
    //RemoteViewsAdapter mRemoteViewsAdapter;

    /* The remote adapter containing the data to be displayed by this view to be set */
    bool mDeferNotifyDataSetChanged = false;

    /* Specifies whether this is the first time the animator is showing views */
    bool mFirstTime = true;

    /* Specifies if the animator should wrap from 0 to the end and vice versa
     * or have hard boundaries at the beginning and end */
    bool mLoopViews = true;

    /* The width and height of some child, used as a size reference in-case our
     * dimensions are unspecified by the parent. */
    int mReferenceChildWidth = -1;
    int mReferenceChildHeight = -1;

    /* In and out animations. */
    ObjectAnimator* mInAnimation;
    ObjectAnimator* mOutAnimation;
private:
    void initViewAnimator();
    void setDisplayedChild(int whichChild, bool animate);
    ViewAndMetaData* getMetaDataForChild(View* child);
    void addChild(View* child);
    void measureChildren();
protected:
    void configureViewAnimator(int numVisibleViews, int activeOffset);
    void transformViewForTransition(int fromIndex, int toIndex, View* view, bool animate);
    ObjectAnimator* getDefaultInAnimation();
    ObjectAnimator* getDefaultOutAnimation();
    void applyTransformForChildAtIndex(View* child, int relativeIndex);
    int modulo(int pos, int size);
    View* getViewAtRelativeIndex(int relativeIndex);
    int getNumActiveViews();
    int getWindowSize();
    LayoutParams* createOrReuseLayoutParams(View* v);
    void refreshChildren();
    FrameLayout* getFrameForChild();
    void showOnly(int childIndex, bool animate);
    void showTapFeedback(View* v);
    void hideTapFeedback(View* v);
    void cancelHandleClick();
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void checkForAndHandleDataChanged();
    void onLayout(bool changed, int left, int top, int width, int height)override;
public:
    AdapterViewAnimator(Context* context,const AttributeSet& attrs);
    ~AdapterViewAnimator()override;
    void setDisplayedChild(int whichChild);
    int getDisplayedChild();
    void showNext();
    void showPrevious();
    bool onTouchEvent(MotionEvent& ev)override;
    View* getCurrentView();
    ObjectAnimator* getInAnimation();
    void setInAnimation(ObjectAnimator* inAnimation);
    ObjectAnimator* getOutAnimation();
    void setOutAnimation(ObjectAnimator* outAnimation);
    void setInAnimation(Context* context,const std::string& resourceID);
    void setOutAnimation(Context* context,const std::string& resourceID);
    void setAnimateFirstView(bool animate);
    int getBaseline()override;
    Adapter* getAdapter()override;
    void setAdapter(Adapter* adapter)override;
    void setSelection(int position)override;
    View* getSelectedView()override;
    void deferNotifyDataSetChanged();
    void advance();
};

}//namepace
#endif
