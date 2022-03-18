#ifndef __FASTSCROLLER_H_
#define __FASTSCROLLER_H__
#include <widget/imageview.h>
#include <widget/textview.h>
#include <widget/abslistview.h>
#include <animation/animationset.h>
#include <view/viewoverlay.h>

namespace cdroid{

class FastScroller{
private:
    /** Duration of fade-out animation. */
    static constexpr int DURATION_FADE_OUT = 300;

    /** Duration of fade-in animation. */
    static constexpr int DURATION_FADE_IN = 150;

    /** Duration of transition cross-fade animation. */
    static constexpr int DURATION_CROSS_FADE = 50;

    /** Duration of transition resize animation. */
    static constexpr int DURATION_RESIZE = 100;

    /** Inactivity timeout before fading controls. */
    static constexpr long FADE_TIMEOUT = 1500;

    /** Minimum number of pages to justify showing a fast scroll thumb. */
    static constexpr int MIN_PAGES = 4;

    /** Scroll thumb and preview not showing. */
    static constexpr int STATE_NONE = 0;

    /** Scroll thumb visible and moving along with the scrollbar. */
    static constexpr int STATE_VISIBLE = 1;

    /** Scroll thumb and preview being dragged by user. */
    static constexpr int STATE_DRAGGING = 2;

    // Positions for preview image and text.
    static constexpr int OVERLAY_FLOATING = 0;
    static constexpr int OVERLAY_AT_THUMB = 1;
    static constexpr int OVERLAY_ABOVE_THUMB = 2;

    // Positions for thumb in relation to track.
    static constexpr int THUMB_POSITION_MIDPOINT = 0;
    static constexpr int THUMB_POSITION_INSIDE = 1;

    // Indices for mPreviewResId.
    static constexpr int PREVIEW_LEFT = 0;
    static constexpr int PREVIEW_RIGHT = 1;

    /** Delay before considering a tap in the thumb area to be a drag. */
    static const long TAP_TIMEOUT = 50;//ViewConfiguration.getTapTimeout();

    Rect mTempBounds ;
    Rect mTempMargins;
    Rect mContainerRect;

    AbsListView* mList;
    ViewGroupOverlay* mOverlay;
    TextView* mPrimaryText;
    TextView* mSecondaryText;
    ImageView* mThumbImage;
    ImageView* mTrackImage;
    View* mPreviewImage;
    /**
     * Preview image resource IDs for left- and right-aligned layouts. See
     * {@link #PREVIEW_LEFT} and {@link #PREVIEW_RIGHT}.
     */
    std::string mPreviewResId[2];

    /** The minimum touch target size in pixels. */
    int mMinimumTouchTarget;

    /**
     * Padding in pixels around the preview text. Applied as layout margins to
     * the preview text and padding to the preview image.
     */
    int mPreviewPadding;

    int mPreviewMinWidth;
    int mPreviewMinHeight;
    int mThumbMinWidth;
    int mThumbMinHeight;

    /** Theme-specified text size. Used only if text appearance is not set. */
    float mTextSize;

    /** Theme-specified text color. Used only if text appearance is not set. */
    ColorStateList* mTextColor;

    Drawable* mThumbDrawable;
    Drawable* mTrackDrawable;
    int mTextAppearance;
    int mThumbPosition;

    // Used to convert between y-coordinate and thumb position within track.
    float mThumbOffset;
    float mThumbRange;

    /** Total width of decorations. */
    int mWidth;

    /** Set containing decoration transition animations. */
    Animator*/*AnimatorSet**/ mDecorAnimation;

    /** Set containing preview text transition animations. */
    Animator*/*AnimatorSet**/ mPreviewAnimation;

    /** Whether the primary text is showing. */
    bool mShowingPrimary;

    /** Whether we're waiting for completion of scrollTo(). */
    bool mScrollCompleted;

    /** The position of the first visible item in the list. */
    int mFirstVisibleItem;

    /** The number of headers at the top of the view. */
    int mHeaderCount;

    /** The index of the current section. */
    int mCurrentSection = -1;

    /** The current scrollbar position. */
    int mScrollbarPosition = -1;

    /** Whether the list is long enough to need a fast scroller. */
    bool mLongList;

    std::vector<void*> mSections;

    /** Whether this view is currently performing layout. */
    bool mUpdatingLayout;

    int mState;

    /** Whether the preview image is visible. */
    bool mShowingPreview;

    Adapter* mListAdapter;
    SectionIndexer mSectionIndexer;

    /** Whether decorations should be laid out from right to left. */
    bool mLayoutFromRight;

    /** Whether the fast scroller is enabled. */
    bool mEnabled;

    /** Whether the scrollbar and decorations should always be shown. */
    bool mAlwaysShow;

    int mOverlayPosition;

    /** Current scrollbar style, including inset and overlay properties. */
    int mScrollBarStyle;

    /** Whether to precisely match the thumb position to the list. */
    bool mMatchDragPosition;

    float mInitialTouchY;
    long mPendingDrag = -1;
    int mScaledTouchSlop;

    int mOldItemCount;
    int mOldChildCount;
    Runnable mDeferHide;
private:
    void updateAppearance();
    void updateLongList(int childCount, int itemCount);
    TextView* createPreviewTextView(Context* context);
    void updateContainerRect();
    void measurePreview(View* v, Rect& out);
    void measureViewToSide(View* view, View* adjacent,const Rect* margins, Rect& out);
    void measureFloating(View* preview,const Rect* margins, Rect& out);
    void updateOffsetAndRange();
    void refreshDrawablePressedState();
    void transitionToHidden();
    void transitionToVisible();
    void transitionToDragging();
    void postAutoHide();
    void getSectionsFromIndexer();
    void scrollTo(float position);
    bool transitionPreviewLayout(int sectionIndex);
    void setThumbPos(float position);
    float getPosFromMotionEvent(float y);
    float getPosFromItemCount(int firstVisibleItem, int visibleItemCount, int totalItemCount);
    void cancelFling();
    void cancelPendingDrag();
    void startPendingDrag();
    void beginDrag();
    bool isPointInside(float x, float y);
    bool isPointInsideX(float x);
    bool isPointInsideY(float x);
    void layoutThumb();
    void layoutTrack();
    void setState(int state);
    void applyLayout(View* view,const Rect& bounds);
public:
    FastScroller(AbsListView*,const std::string& scrollstyle);
    ~FastScroller();
    void remove();
    void setEnabled(bool);
    bool isEnabled()const;
    void setAlwaysShow(bool);
    bool isAlwaysShowEnabled()const;
    void onStateDependencyChanged(bool peekIfEnabled);
    void setStyle(const std::string&styleResId);
    void setScrollBarStyle(int);
    void stop();
    void setScrollbarPosition(int position);
    int getWidth()const;
    void onSizeChanged(int w, int h, int oldw, int oldh);
    void onItemCountChanged(int childCount, int itemCount);
    void onScroll(int firstVisibleItem, int visibleItemCount, int totalItemCount);
    void onSectionsChanged();
    bool onInterceptTouchEvent(MotionEvent& ev);
    bool onInterceptHoverEvent(MotionEvent& ev);
    //PointerIcon onResolvePointerIcon(MotionEvent& event, int pointerIndex);
    bool onTouchEvent(MotionEvent& me);
    void updateLayout();
};

}

#endif
