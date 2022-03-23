#ifndef __CDROID_VIEW_H__
#define __CDROID_VIEW_H__
#include <core/eventcodes.h>
#include <core/uievents.h>
#include <core/inputdevice.h>
#include <core/canvas.h>
#include <core/insets.h>
#include <core/systemclock.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <view/gravity.h>
#include <view/layoutparams.h>
#include <view/rendernode.h>
#include <view/velocitytracker.h>
#include <view/layoutinflater.h>
#include <view/viewpropertyanimator.h>
#include <view/viewconfiguration.h>
#include <widget/measurespec.h>
#include <animation/animation.h>
#include <animation/statelistanimator.h>
#include <set>
#include <memory>
#include <vector>
#include <functional>
#include <core/rect.h>
#include <drawables.h>

#ifndef _GLIBCXX_FUNCTIONAL
#define DECLARE_UIEVENT(type,name,...) typedef type(*name)(__VA_ARGS__)
#else
#define DECLARE_UIEVENT(type,name,...) typedef std::function< type(__VA_ARGS__) >name
#endif

namespace cdroid{
class ViewGroup;
class ViewOverlay;
typedef const std::string Intent;
class View:public Drawable::Callback,public KeyEvent::Callback{
public:
    static bool DEBUG_DRAW;
    constexpr static int DEBUG_CORNERS_COLOR    = 0xFF3f7fff;
    constexpr static int DEBUG_CORNERS_SIZE_DIP = 8;
    constexpr static int NO_ID =-1;
    enum TextAlignment{
        TEXT_ALIGNMENT_INHERIT   = 0 ,
        TEXT_ALIGNMENT_GRAVITY   = 1 ,
        TEXT_ALIGNMENT_TEXT_START= 2 ,
        TEXT_ALIGNMENT_TEXT_END  = 3 ,
        TEXT_ALIGNMENT_CENTER    = 4 ,
        TEXT_ALIGNMENT_VIEW_START= 5 ,
        TEXT_ALIGNMENT_VIEW_END  = 6 ,
        TEXT_ALIGNMENT_DEFAULT   = TEXT_ALIGNMENT_GRAVITY ,
        TEXT_ALIGNMENT_RESOLVED_DEFAULT = TEXT_ALIGNMENT_GRAVITY
    };

    enum TextDirection{
        TEXT_DIRECTION_INHERIT =0 ,
        TEXT_DIRECTION_FIRST_STRONG=1,
        TEXT_DIRECTION_ANY_RTL =2 ,
        TEXT_DIRECTION_LTR     =3 ,
        TEXT_DIRECTION_RTL     =4 ,
        TEXT_DIRECTION_LOCALE  =5 ,
        TEXT_DIRECTION_FIRST_STRONG_LTR =6,
        TEXT_DIRECTION_FIRST_STRONG_RTL =7,
        TEXT_DIRECTION_DEFAULT = TEXT_DIRECTION_INHERIT,
        TEXT_DIRECTION_RESOLVED_DEFAULT = TEXT_DIRECTION_FIRST_STRONG
    };

    class TransformationInfo{
    public:
        Matrix mMatrix;
        Matrix mInverseMatrix;
        float mAlpha = 1.f;
        float mTransitionAlpha = 1.f;
        TransformationInfo();
    };
protected:
    enum PFLAGS{//FLAGS in mPrivateFlags
        PFLAG_WANTS_FOCUS      = 0x01 ,
        PFLAG_FOCUSED          = 0x02 ,
        PFLAG_SELECTED         = 0x04 ,
        PFLAG_IS_ROOT_NAMESPACE= 0x08 ,
        PFLAG_HAS_BOUNDS       = 0x10 ,
        PFLAG_DRAWN            = 0x20 ,
        PFLAG_DRAW_ANIMATION   = 0x40 ,
        PFLAG_SKIP_DRAW        = 0x80 ,
        PFLAG_REQUEST_TRANSPARENT_REGIONS=0x200,
        PFLAG_DRAWABLE_STATE_DIRTY  =0x400 ,
        PFLAG_MEASURED_DIMENSION_SET=0x800 ,
        PFLAG_FORCE_LAYOUT     =0x1000 ,
        PFLAG_LAYOUT_REQUIRED  =0x2000 ,

        PFLAG_PRESSED          = 0x4000 ,
        PFLAG_DRAWING_CACHE_VALID= 0x8000 ,
        PFLAG_ANIMATION_STARTED= 0x00010000 ,
        PFLAG_ALPHA_SET        = 0x00040000 ,
        PFLAG_SCROLL_CONTAINER = 0x00080000 ,
        PFLAG_SCROLL_CONTAINER_ADDED= 0x00100000,
        PFLAG_DIRTY            = 0x00200000 ,
        PFLAG_DIRTY_OPAQUE     = 0x00400000 ,
        PFLAG_DIRTY_MASK       = 0x00600000 ,
        PFLAG_OPAQUE_BACKGROUND= 0x00800000 ,
        PFLAG_OPAQUE_SCROLLBARS= 0x01000000 ,
        PFLAG_OPAQUE_MASK      = 0x01800000 ,
        PFLAG_PREPRESSED       = 0x02000000 ,
        PFLAG_CANCEL_NEXT_UP_EVENT=0x04000000,
        PFLAG_HOVERED    = 0x10000000 ,
        PFLAG_ACTIVATED  = 0x40000000 ,
        PFLAG_INVALIDATED= 0x80000000
    };//
    enum PFLAGS2{//FLAG2 in mPrivateFlag2
        PFLAG2_TEXT_DIRECTION_MASK_SHIFT =6 ,
        PFLAG2_TEXT_DIRECTION_MASK      = 0x00000007<< PFLAG2_TEXT_DIRECTION_MASK_SHIFT ,
        PFLAG2_TEXT_DIRECTION_RESOLVED  = 0x00000008 << PFLAG2_TEXT_DIRECTION_MASK_SHIFT ,
        PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT=10 ,
        PFLAG2_TEXT_DIRECTION_RESOLVED_MASK = 0x00000007 << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT ,
        PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT = TEXT_DIRECTION_RESOLVED_DEFAULT << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT ,
        PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT = 13 ,
        PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT = 17 ,
        PFLAG2_TEXT_ALIGNMENT_MASK          = 0x00000007 << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT ,
        PFLAG2_TEXT_ALIGNMENT_RESOLVED      = 0x00000008 << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT ,
        PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK = 0x00000007 << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT ,
        PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT = TEXT_ALIGNMENT_RESOLVED_DEFAULT << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT ,
        PFLAG2_ACCESSIBILITY_FOCUSED   = 0x04000000 ,
        PFLAG2_VIEW_QUICK_REJECTED     = 0x10000000 ,
        PFLAG2_PADDING_RESOLVED        = 0x20000000 ,
        PFLAG2_DRAWABLE_RESOLVED       = 0x40000000 ,
        PFLAG2_HAS_TRANSIENT_STATE     = 0x80000000 ,
        PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT   = 0x02 ,
        PFLAG2_LAYOUT_DIRECTION_MASK         = 0x00000003 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT ,
        PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL = 4 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT ,
        PFLAG2_LAYOUT_DIRECTION_RESOLVED     = 8 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT ,
        PFLAG2_LAYOUT_DIRECTION_RESOLVED_MASK= 0x0000000C<< PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT ,
        ALL_RTL_PROPERTIES_RESOLVED = PFLAG2_LAYOUT_DIRECTION_RESOLVED |  PFLAG2_TEXT_DIRECTION_RESOLVED 
               | PFLAG2_TEXT_ALIGNMENT_RESOLVED | PFLAG2_PADDING_RESOLVED | PFLAG2_DRAWABLE_RESOLVED
    };
    enum PFLAGS3{//FLAGS in mPrivateFlags3
        PFLAG3_VIEW_IS_ANIMATING_TRANSFORM = 0x0001 ,
        PFLAG3_VIEW_IS_ANIMATING_ALPHA     = 0x0002 ,
        PFLAG3_IS_LAID_OUT                 = 0x0004 ,
        PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT= 0x0008 ,
        PFLAG3_NESTED_SCROLLING_ENABLED    = 0x0080 ,
        PFLAG3_SCROLL_INDICATOR_TOP        = 0x0100 ,
        PFLAG3_SCROLL_INDICATOR_BOTTOM     = 0x0200 ,
        PFLAG3_SCROLL_INDICATOR_LEFT       = 0x0400 ,
        PFLAG3_SCROLL_INDICATOR_RIGHT      = 0x0800 ,
        PFLAG3_SCROLL_INDICATOR_START      = 0x1000 ,
        PFLAG3_SCROLL_INDICATOR_END        = 0x2000 ,

        PFLAG3_CLUSTER                 = 0x08000 ,
        PFLAG3_IS_AUTOFILLED           = 0x10000 ,
        PFLAG3_FINGER_DOWN             = 0x20000 ,
        PFLAG3_FOCUSED_BY_DEFAULT      = 0x40000 ,
        PFLAG3_TEMPORARY_DETACH        = 0x2000000 ,
        PFLAG3_NO_REVEAL_ON_FOCUS      = 0x4000000 ,
        PFLAG3_AGGREGATED_VISIBLE      = 0x20000000
    };

    class AttachInfo{
    public:
        View*mRootView;
        bool mHardwareAccelerated;
        float mApplicationScale;
        int mWindowLeft;
        int mWindowTop;
        Rect mOverscanInsets;
        Rect mContentInsets;
        Rect mVisibleInsets;
        Rect mStableInsets;
        Rect mOutsets;
        KeyEvent::DispatcherState mKeyDispatchState;
        bool mAlwaysConsumeNavBar;
        bool mAlwaysConsumeSystemBars;
        bool mHasWindowFocus;
        bool mScalingRequired;
        bool mUse32BitDrawingCache;
        bool mViewVisibilityChanged;
        bool mIgnoreDirtyState;
        int mWindowVisibility;
        long mDrawingTime;
        bool mInTouchMode;
        bool mKeepScreenOn;
        bool mDebugLayout;
        bool mDisplayState;/*true display is on*/
        RefPtr<Canvas> mCanvas;
        Drawable*mAutofilledDrawable;
        View* mTooltipHost;
        View* mViewRequestingLayout;
        std::vector<View*> mScrollContainers;
        AttachInfo(); 
    };
public:
    enum ViewFlags{//public common View Flags
        ENABLED        = 0x00 ,
        DISABLED       = 0x01 ,
        ENABLED_MASK   = 0x01 ,
        VISIBLE        = 0x02 ,
        INVISIBLE      = 0x04 ,
        GONE           = 0x08 ,
        VISIBILITY_MASK= 0x0E ,
        NOT_FOCUSABLE  = 0x00 ,
        FOCUSABLE      = 0x10 ,
        FOCUSABLE_AUTO = 0x20 ,
        FOCUSABLE_MASK = 0x30 ,
        FOCUSABLE_IN_TOUCH_MODE=0x40 ,

        WILL_NOT_DRAW  = 0x80 ,
        DRAW_MASK      = 0x80 ,

        SCROLLBARS_NONE= 0 ,
        SCROLLBARS_HORIZONTAL= 0x100 ,
        SCROLLBARS_VERTICAL  = 0x200 ,
        SCROLLBARS_MASK      = 0x300 ,

        CLIPCHILDREN = 0x400 ,
        TRANSPARENT  = 0x800 ,

        FADING_EDGE_NONE =0x000000 ,
        FADING_EDGE_HORIZONTAL= 0x1000 ,
        FADING_EDGE_VERTICAL  = 0x2000 ,
        FADING_EDGE_MASK      = 0x3000 ,

        CLICKABLE       = 0x4000 ,
        DRAWING_CACHE_ENABLED  = 0x8000 ,
        WILL_NOT_CACHE_DRAWING = 0x000020000 ,
        
        LONG_CLICKABLE = 0x200000 ,
        DUPLICATE_PARENT_STATE=0x10000 ,
        CONTEXT_CLICKABLE=0x20000 ,
        TOOLTIP =0x40000 ,

        DRAWING_CACHE_QUALITY_LOW  = 0x00080000 ,
        DRAWING_CACHE_QUALITY_HIGH = 0x00100000 ,
        DRAWING_CACHE_QUALITY_AUTO = 0x00000000 ,
        DRAWING_CACHE_QUALITY_MASK = 0x00180000 ,

        MEASURED_HEIGHT_STATE_SHIFT=16 ,
        MEASURED_STATE_TOO_SMALL=0x1000000 ,
        MEASURED_SIZE_MASK =0x00ffffff ,
        MEASURED_STATE_MASK=0xff000000 ,

       //FocusDirection{
        FOCUS_BACKWARD=0x01 ,
        FOCUS_FORWARD =0x02 ,
        FOCUS_LEFT    =0x11 ,
        FOCUS_UP      =0x21 ,
        FOCUS_RIGHT   =0x42 ,
        FOCUS_DOWN    =0x82 ,

       //FocusableMode
        FOCUSABLES_ALL = 0 ,
        FOCUSABLES_TOUCH_MODE=1 ,
  
        LAYOUT_DIRECTION_UNDEFINED = LayoutDirection::UNDEFINED ,
        LAYOUT_DIRECTION_LTR=LayoutDirection::LTR ,
        LAYOUT_DIRECTION_RTL=LayoutDirection::RTL ,
        LAYOUT_DIRECTION_INHERIT=LayoutDirection::INHERIT ,
        LAYOUT_DIRECTION_LOCALE=LayoutDirection::LOCAL ,
  
       //ScrollBarPosition
        SCROLLBAR_POSITION_DEFAULT= 0 ,
        SCROLLBAR_POSITION_LEFT   = 1 ,
        SCROLLBAR_POSITION_RIGHT  = 2 ,

       //ScrollIndicators
        SCROLL_INDICATORS_NONE        = 0x0000 ,
        SCROLL_INDICATORS_PFLAG3_MASK = PFLAG3_SCROLL_INDICATOR_TOP | PFLAG3_SCROLL_INDICATOR_BOTTOM 
             | PFLAG3_SCROLL_INDICATOR_LEFT  | PFLAG3_SCROLL_INDICATOR_RIGHT 
             | PFLAG3_SCROLL_INDICATOR_START | PFLAG3_SCROLL_INDICATOR_END ,
        SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT =8 ,
  
        SCROLL_INDICATOR_TOP    = PFLAG3_SCROLL_INDICATOR_TOP >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT ,
        SCROLL_INDICATOR_BOTTOM = PFLAG3_SCROLL_INDICATOR_BOTTOM >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT ,
        SCROLL_INDICATOR_LEFT   = PFLAG3_SCROLL_INDICATOR_LEFT >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT ,
        SCROLL_INDICATOR_RIGHT  = PFLAG3_SCROLL_INDICATOR_RIGHT >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT ,

    /*The scrollbar style to display the scrollbars at the edge of the view,
     * increasing the padding of the view. The scrollbars will only overlap the
     * background, if any*/
        SCROLLBARS_INSIDE_OVERLAY = 0 ,
        SCROLLBARS_INSIDE_INSET   = 0x01000000 ,
        SCROLLBARS_OUTSIDE_OVERLAY= 0x02000000 ,
        SCROLLBARS_OUTSIDE_INSET  = 0x03000000 ,
        SCROLLBARS_INSET_MASK     = 0x01000000 ,
        SCROLLBARS_OUTSIDE_MASK   = 0x02000000 ,
        SCROLLBARS_STYLE_MASK     = 0x03000000 ,

       //Indicates no axis of view scrolling.
        SCROLL_AXIS_NONE      =0 ,
        SCROLL_AXIS_HORIZONTAL=1 ,
        SCROLL_AXIS_VERTICAL  =2 ,

       //OverScrollMode of view
        OVER_SCROLL_ALWAYS =0 ,
        OVER_SCROLL_IF_CONTENT_SCROLLS =1 ,
        OVER_SCROLL_NEVER =2 ,
    };//endof ViewFlags
    enum LayerType{
        LAYER_TYPE_NONE    =0,
        LAYER_TYPE_SOFTWARE=1,
        LAYER_TYPE_HARDWARE=2
    };
    DECLARE_UIEVENT(bool,OnKeyListener,View& v, int keyCode, KeyEvent&);
    DECLARE_UIEVENT(bool,OnTouchListener,View&v, MotionEvent&);
    DECLARE_UIEVENT(bool,OnHoverListener,View&v, MotionEvent&);
    DECLARE_UIEVENT(bool,OnGenericMotionListener,View&v, MotionEvent&);
    DECLARE_UIEVENT(void,OnClickListener,View&);
    DECLARE_UIEVENT(bool,OnLongClickListener,View&);
    DECLARE_UIEVENT(bool,OnContextClickListener,View&);
    DECLARE_UIEVENT(void,OnFocusChangeListener,View&,bool);
    DECLARE_UIEVENT(void,OnScrollChangeListener,View& v, int, int, int, int);
    DECLARE_UIEVENT(void,OnLayoutChangeListener,View* v, int left, int top, int width, int height,
            int oldLeft, int oldTop, int oldWidth, int oldHeight);
    typedef CallbackBase<bool,View&,KeyEvent&>OnUnhandledKeyEventListener;

private:
    friend ViewGroup;
    friend ViewPropertyAnimator;
    int mMinWidth;
    int mMinHeight;
    int mDrawingCacheBackgroundColor;
    int mOldWidthMeasureSpec;
    int mOldHeightMeasureSpec;
    int mVerticalScrollbarPosition;
    int mLongClickX ,mLongClickY;
    int mNextFocusLeftId;
    int mNextFocusRightId;
    int mNextFocusUpId;
    int mNextFocusDownId;
    int mNextFocusForwardId;
    int mNextClusterForwardId;
    Insets mLayoutInsets;
 
    bool mInContextButtonPress;
    bool mHasPerformedLongPress;
    bool mIgnoreNextUpEvent;
    bool mOriginalPressedState;
	
    bool mBackgroundSizeChanged;
    bool mDefaultFocusHighlightSizeChanged;
    bool mDefaultFocusHighlightEnabled;
    bool mBoundsChangedmDefaultFocusHighlightSizeChanged;

    ViewOverlay* mOverlay;
    StateListAnimator* mStateListAnimator;
    ViewPropertyAnimator* mAnimator;
    ViewGroup* mNestedScrollingParent;
    std::map<Size,Size>mMeasureCache;
    std::string mStartActivityRequestWho;
    class ScrollabilityCache*mScrollCache;

    Drawable* mBackground;
    Drawable* mDefaultFocusHighlight;
    Drawable* mDefaultFocusHighlightCache;
    Drawable* mScrollIndicatorDrawable;
	
    Runnable* mPendingCheckForTap;
    Runnable* mPerformClick;
    Runnable* mPendingCheckForLongPress;
    Runnable* mUnsetPressedState;	
    class RoundScrollbarRenderer* mRoundScrollbarRenderer;
    class TintInfo* mBackgroundTint;
    class ForegroundInfo* mForegroundInfo;
private:	
    View(const View&) = delete;
    View&operator=(const View&) = delete;
    //Temporary values used to hold (x,y) coordinates when delegating from the
    // two-arg performLongClick() method to the legacy no-arg version
    void checkForTapCallback(int x,int y);//for mPendingCheckForTap
    void checkLongPressCallback(int x,int y);//for mPendingCheckForLongPress
    void unsetPressedCallback();//for mUnsetPressedState

    void removeTapCallback();
    void removeLongPressCallback();
    void removePerformClickCallback();
    void removeUnsetPressCallback();

    void checkForLongClick(int delayOffset,int x,int y);
    bool performClickInternal();
    bool performLongClickInternal(int x,int y);
    void setPressed(bool pressed,int x,int y);

    void resetPressedState();
    void initView();
    void drawBackground(Canvas&canvas);
    void applyBackgroundTint();
    void applyForegroundTint();
    View* findViewInsideOutShouldExist(View* root, int id)const;
    bool requestFocusNoSearch(int direction,Rect*previouslyFocusedRect);
    bool requestFocusFromTouch();
    bool hasAncestorThatBlocksDescendantFocus()const;
	
    void debugDrawFocus(Canvas&canvas);
    Drawable* getDefaultFocusHighlightDrawable();
    void setDefaultFocusHighlight(Drawable* highlight);
    void switchDefaultFocusHighlight();
    void drawDefaultFocusHighlight(Canvas& canvas);

    void sizeChange(int newWidth,int newHeight,int oldWidth,int oldHeight);
    void setMeasuredDimensionRaw(int measuredWidth, int measuredHeight);
    void initializeScrollbarsInternal(const AttributeSet&attrs);
    void initScrollCache();
    ScrollabilityCache* getScrollCache();
    Drawable* getAutofilledDrawable();
    void drawAutofilledHighlight(Canvas& canvas);
    bool isOnVerticalScrollbarThumb(int x,int y);
    bool isOnHorizontalScrollbarThumb(int x,int y);
    bool isHoverable()const;
    bool hasSize()const;
    bool canTakeFocus()const;
    void getRoundVerticalScrollBarBounds(RECT* bounds);
    void getStraightVerticalScrollBarBounds(RECT*drawBounds,RECT*touchBounds=nullptr);
    void getVerticalScrollBarBounds(RECT*bounds,RECT*touchBounds=nullptr);
    void getHorizontalScrollBarBounds(RECT*drawBounds,RECT*touchBounds);
    void initializeScrollIndicatorsInternal();
    void setFocusedInCluster(View* cluster);
    void updateFocusedInCluster(View* oldFocus,int direction);
    bool dispatchGenericMotionEventInternal(MotionEvent& event);
    bool applyLegacyAnimation(ViewGroup* parent, long drawingTime, Animation* a, bool scalingRequired);
    bool needRtlPropertiesResolution()const;
    bool skipInvalidate()const;
    void buildDrawingCache(bool autoScale);
    void buildDrawingCacheImpl(bool autoScale);
    bool hasParentWantsFocus()const;
    void cleanupDraw();
    void invalidateInternal(int l, int t, int r, int b, bool invalidateCache,bool fullInvalidate);
protected:
    int mID;
    int mLayerType;
    int mAutofillViewId;
    int mAccessibilityViewId;
    int mScrollX;
    int mScrollY;
    int mOverScrollMode;
    int mViewFlags;
    int mPrivateFlags;
    int mPrivateFlags2;
    int mPrivateFlags3;
    int mPaddingLeft;
    int mPaddingRight;
    int mPaddingTop;
    int mPaddingBottom;
    int mMeasuredWidth;
    int mMeasuredHeight;

    int mUserPaddingLeft; //set by the user to append to the scrollbar's size.
    int mUserPaddingRight;
    int mUserPaddingTop;
    int mUserPaddingBottom;
    /* Cache the paddingStart set by the user to append to the scrollbar's size. */
    int mUserPaddingStart;
    /* Cache the paddingEnd set by the user to append to the scrollbar's size.*/
    int mUserPaddingEnd;
    /* Cache initial left padding*/
    int mUserPaddingLeftInitial;
    /* Cache initial right padding*/
    int  mUserPaddingRightInitial;
    int  mTransientStateCount;
    bool mLeftPaddingDefined;
    bool mRightPaddingDefined;
    bool mCachingFailed;
    bool mLastIsOpaque;
    std::string mHint;
    std::string mContentDescription;
    RefPtr<ImageSurface> mDrawingCache;
    RefPtr<ImageSurface> mUnscaledDrawingCache;
    void * mTag;
    Context* mContext;
    LayoutParams* mLayoutParams;
    TransformationInfo* mTransformationInfo;
    Animation* mCurrentAnimation;
    std::vector<int> mDrawableState;

    int mTop,mLeft,mRight,mBottom;
    ViewGroup * mParent;
    AttachInfo* mAttachInfo;
    RenderNode* mRenderNode;
    class ListenerInfo* mListenerInfo;
    ListenerInfo*getListenerInfo();
protected:
    virtual void internalSetPadding(int left, int top, int right, int bottom);
    void assignParent(ViewGroup*p);
    bool debugDraw()const;
    int dipsToPixels(int dips)const;
    void computeOpaqueFlags();
    virtual void resolveDrawables();
    bool areDrawablesResolved()const;
    void setDuplicateParentStateEnabled(bool);
    bool isDuplicateParentStateEnabled()const;

    void recomputePadding();
    bool isPaddingOffsetRequired();
    int getLeftPaddingOffset();
    int getRightPaddingOffset();
    int getTopPaddingOffset();
    int getBottomPaddingOffset();
    int getFadeTop(bool offsetRequired);
    int getFadeHeight(bool offsetRequired);
    bool isHardwareAccelerated()const;

    void invalidateParentIfNeededAndWasQuickRejected();
    virtual void invalidateInheritedLayoutMode(int);
    void destroyDrawingCache();
    RefPtr<ImageSurface>getDrawingCache(bool autoScale);
    virtual bool hasWindowFocus()const;

    virtual bool setFrame(int x,int y,int w,int h);
    virtual void resetResolvedDrawables();
    virtual bool verifyDrawable(Drawable*)const;
    virtual void drawableStateChanged();
    virtual std::vector<int> onCreateDrawableState()const;
    virtual View& setFlags(int flag,int mask);
    virtual bool hasFlag(int flag) const;
    virtual void dispatchSetSelected(bool selected);
    virtual void dispatchSetPressed(bool pressed);
    virtual void dispatchVisibilityChanged(View& changedView,int visiblity);
    virtual bool dispatchVisibilityAggregated(bool isVisible);
    virtual void dispatchWindowFocusChanged(bool);
    virtual void onWindowFocusChanged(bool hasWindowFocus);
    virtual void onVisibilityChanged(View& changedView,int visibility);
    virtual void onAttachedToWindow();
    virtual void onDetachedFromWindow();
    void onDetachedFromWindowInternal();
    virtual void  onMeasure(int widthMeasureSpec, int heightMeasureSpec);
    virtual void dispatchDraw(Canvas&);
    virtual void onFocusChanged(bool,int,Rect*);
    virtual void onFocusLost();
    virtual void clearParentsWantFocus();
    virtual void clearFocusInternal(View* focused, bool propagate, bool refocus);
    virtual void handleFocusGainInternal(int direction,Rect*previouslyFocusedRect);
    bool awakenScrollBars();
    bool awakenScrollBars(int startDelay, bool invalidate);

    static int combineVisibility(int vis1, int vis2);
    virtual void onSizeChanged(int w,int h,int oldw,int oldh);
    virtual void onScrollChanged(int l, int t, int oldl, int oldt);
    virtual void onLayout(bool ,int,int,int,int);
    virtual void onDraw(Canvas& ctx);
    virtual void onDrawForeground(Canvas& canvas);
    virtual void onFinishInflate();
    virtual void dispatchSetActivated(bool activated);
    virtual void dispatchAttachedToWindow(AttachInfo*info,int visibility);
    virtual void dispatchDetachedFromWindow();
    bool canReceivePointerEvents()const;
    virtual bool dispatchHoverEvent(MotionEvent&event);
    virtual bool dispatchGenericPointerEvent(MotionEvent& event);
    virtual bool dispatchGenericFocusedEvent(MotionEvent& event);

    static int combineMeasuredStates(int curState, int newState);
    static std::vector<int>& mergeDrawableStates(std::vector<int>&baseState,const std::vector<int>&additionalState);
    static int resolveSize(int size, int measureSpec);
    static int resolveSizeAndState(int size, int measureSpec, int childMeasuredState);
    static int getDefaultSize(int size, int measureSpec);
    void damageInParent();
    bool hasDefaultFocus()const;
    int getSuggestedMinimumWidth();
    int getSuggestedMinimumHeight();
    void setMeasuredDimension(int measuredWidth, int measuredHeight);
    bool handleScrollBarDragging(MotionEvent& event);
    void playSoundEffect(int soundConstant);
    bool performHapticFeedback(int feedbackConstant, int flags=0);
    bool performButtonActionOnTouchDown(MotionEvent&);

    void onAnimationStart();
    void onAnimationEnd();
    bool onSetAlpha(int alpha);

    bool isVerticalScrollBarHidden()const;
    bool shouldDrawRoundScrollbar()const;

    bool isOnScrollbar(int x,int y);
    bool isOnScrollbarThumb(int x,int y);
    bool overScrollBy(int deltaX, int deltaY, int scrollX, int scrollY, int scrollRangeX, 
    int  scrollRangeY, int maxOverScrollX, int maxOverScrollY, bool isTouchEvent);
    virtual void onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY);
    virtual float getTopFadingEdgeStrength();
    virtual float getBottomFadingEdgeStrength();
    virtual float getLeftFadingEdgeStrength();
    virtual float getRightFadingEdgeStrength();
    virtual void getScrollIndicatorBounds(Rect&);
    virtual void onDrawScrollIndicators(Canvas& canvas);
    virtual void onDrawScrollBars(Canvas& canvas);
    void onDrawHorizontalScrollBar(Canvas& canvas, Drawable* scrollBar,int l, int t, int w, int h);
    void onDrawVerticalScrollBar (Canvas& canvas , Drawable* scrollBar,int l, int t, int w, int h);

    void ensureTransformationInfo();
public:
    View(Context*ctx,const AttributeSet&attrs);
    View(int w,int h);
    virtual ~View();
    virtual void draw(Canvas&canvas);
    bool draw(Canvas&canvas,ViewGroup*parent,long drawingTime);

    virtual void invalidateParentCaches();
    virtual void invalidateParentIfNeeded();
    virtual void invalidateViewProperty(bool invalidateParent, bool forceRedraw);
    virtual void invalidate(const Rect&dirty);
    virtual void invalidate(int l,int t,int w,int h);
    virtual void invalidate(bool invalidateCache=true);

    bool isDirty()const;
    void postInvalidate();
    void postInvalidateOnAnimation();
    void postInvalidateOnAnimation(int left, int top, int width, int height);
    void invalidateDrawable(Drawable& who)override;
    int  getLayerType()const;
    void setLayerType(int);
    int  getDrawingCacheBackgroundColor()const;
    void setDrawingCacheBackgroundColor(int);
    void scheduleDrawable(Drawable& who,Runnable what, long when)override;
    void unscheduleDrawable(Drawable& who,Runnable what)override;
    virtual void unscheduleDrawable(Drawable& who);

    const Rect getBound()const;
    void getHitRect(Rect&);
    bool pointInView(int localX,int localY,int slop);
    const Rect getDrawingRect()const;
    long getDrawingTime()const;
    virtual void getFocusedRect(Rect&r);
    virtual View& setPos(int x,int y);
    virtual View& setSize(int x,int y);
    void getDrawingRect(Rect& outRect);
    void offsetTopAndBottom(int offset);
    void offsetLeftAndRight(int offset);
    void setLeft(int left);
    void setTop(int top);
    void setRight(int right);
    void setBottom(int bottom);
    int getLeft()const;
    int getTop()const;
    int getRight()const;
    int getBottom()const;
    int getWidth()const;
    int getHeight()const;
    int getPaddingTop();
    int getPaddingBottom();
    int getPaddingLeft();
    int getPaddingStart();
    int getPaddingRight();
    int getPaddingEnd();
    bool isPaddingRelative()const;
    Insets computeOpticalInsets();
    Insets getOpticalInsets();
    void resetPaddingToInitialValues();
    void setOpticalInsets(const Insets& insets);
    void setPadding(int left, int top, int right, int bottom);
    void setPaddingRelative(int start,int top,int end,int bottom);
    bool isPaddingResolved()const;
    virtual void resolvePadding();
    virtual void onRtlPropertiesChanged(int layoutDirection);
    bool resolveLayoutDirection();
    bool canResolveTextDirection();
    bool canResolveLayoutDirection();
    int getMinimumHeight();
    void setMinimumHeight(int minHeight);
    int getMinimumWidth();
    void setMinimumWidth(int minWidth);
    Animation* getAnimation()const;
    void startAnimation(Animation* animation);
    void clearAnimation();
    void setAnimation(Animation* animation);

    void setDrawingCacheEnabled(bool);
    bool isDrawingCacheEnabled()const;
    
    void setDefaultFocusHighlightEnabled(bool defaultFocusHighlightEnabled);
    bool getDefaultFocusHighlightEnabled()const;
    bool isLayoutDirectionResolved()const;
    int getLayoutDirection()const;
    virtual bool isOpaque()const;
    View&setLayoutDirection(int layoutDirection);
    bool isLayoutRtl()const;
    bool isFocusableInTouchMode()const;
    virtual void setFocusable(int focusable);
    virtual void setFocusableInTouchMode(bool focusableInTouchMode);
    virtual void drawableHotspotChanged(float x, float y);
    virtual void dispatchDrawableHotspotChanged(float x,float y);
    void refreshDrawableState();
    bool isDefaultFocusHighlightNeeded(const Drawable* background,const Drawable* foreground)const;
    virtual const std::vector<int>getDrawableState();

    int getNextFocusLeftId()const{return mNextFocusLeftId;}
    View& setNextFocusLeftId(int id){mNextFocusLeftId=id;return *this;}
    int getNextFocusRightId()const{return mNextFocusRightId;}
    View& setNextFocusRightId(int id){mNextFocusRightId=id;return *this;}
    int getNextFocusUpId()const{return mNextFocusUpId;}
    View& setNextFocusUpId(int id){mNextFocusUpId=id;return *this;}
    int getNextFocusDownId()const{return mNextFocusDownId;}
    View& setNextFocusDownId(int id){mNextFocusUpId=id;return *this;}
    int getNextFocusForwardId()const{return mNextFocusForwardId;}
    View& setNextFocusForwardId(int id){mNextFocusForwardId=id;return *this;}

    int getScrollBarSize()const;
    View& setScrollBarSize(int scrollBarSize);
    virtual void setScrollBarStyle(int style);
    int  getScrollBarStyle()const;
    bool isHorizontalScrollBarEnabled()const;
    View& setHorizontalScrollBarEnabled(bool);
    bool isVerticalScrollBarEnabled()const;
    View& setVerticalScrollBarEnabled(bool);
    virtual int getHorizontalScrollbarHeight()const;
    virtual int getVerticalScrollbarWidth()const;
    virtual int getVerticalScrollbarPosition()const;
    bool isScrollContainer()const;
    void setScrollContainer(bool isScrollContainer);
    bool isHorizontalFadingEdgeEnabled()const;
    void setHorizontalFadingEdgeEnabled(bool horizontalFadingEdgeEnabled);
    bool isVerticalFadingEdgeEnabled()const;
    void setVerticalFadingEdgeEnabled(bool verticalFadingEdgeEnabled);

    virtual View& setVerticalScrollbarPosition(int position);

    void setScrollbarFadingEnabled(bool fadeScrollbars);
    bool isScrollbarFadingEnabled();
    int  getScrollBarDefaultDelayBeforeFade();
    void setScrollBarDefaultDelayBeforeFade(int scrollBarDefaultDelayBeforeFade);
    int  getScrollBarFadeDuration();
    void setScrollBarFadeDuration(int scrollBarFadeDuration);

    int getScrollIndicators()const;
    virtual void setScrollIndicators(int indicators,int mask=SCROLL_INDICATORS_PFLAG3_MASK >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT);
    virtual void computeScroll();
    virtual int  computeHorizontalScrollRange();
    virtual int  computeHorizontalScrollOffset();
    virtual int  computeHorizontalScrollExtent();
    virtual int  computeVerticalScrollRange();
    virtual int  computeVerticalScrollOffset();
    virtual int  computeVerticalScrollExtent();
    virtual bool canScrollHorizontally(int direction);
    virtual bool canScrollVertically(int direction);

    void  setRevealOnFocusHint(bool revealOnFocus);
    bool  getRevealOnFocusHint()const;
    bool  isAttachedToWindow()const;
    bool  isLaidOut()const;
    bool  willNotDraw()const;
    void  setWillNotDraw(bool willNotDraw);
    const Rect getClientRect()const;
    bool  hasClickListener()const;
    virtual void setOnClickListener(OnClickListener l);
    virtual void setOnLongClickListener(OnLongClickListener l);
    virtual void setOnFocusChangeListener(OnFocusChangeListener listtener); 
    virtual void setOnScrollChangeListener(OnScrollChangeListener l);
    void  addOnLayoutChangeListener(OnLayoutChangeListener listener);
    void  removeOnLayoutChangeListener(OnLayoutChangeListener listener);
    virtual bool performClick();
    virtual bool performLongClick();
    virtual bool performLongClick(int x,int y);
    bool  performContextClick(int x, int y);
    bool  performContextClick();
    bool showContextMenu();
    bool showContextMenu(float x, float y);
    void startActivityForResult(Intent intent, int requestCode);
    virtual bool dispatchActivityResult(const std::string& who, int requestCode, int resultCode, Intent data);
    virtual void onActivityResult(int requestCode, int resultCode, Intent data);
    void setOnKeyListener(OnKeyListener l);
    void setOnTouchListener(OnTouchListener l);
    void setOnGenericMotionListener(OnGenericMotionListener l);
    void setOnHoverListener(OnHoverListener l);
    //void setOnDragListener(OnDragListener&l);
    // Foreground color

    //foreground/background
    Drawable* getForeground()const;
    View& setForeground(Drawable* foreground);
    bool isForegroundInsidePadding()const;
    int getForegroundGravity()const;
    View& setForegroundGravity(int gravity);
    View& setForegroundTintList(ColorStateList* tint);
    View& setForegroundTintMode(int tintMode);
    ColorStateList* getForegroundTintList();
    virtual void onResolveDrawables(int layoutDirection);

    virtual void jumpDrawablesToCurrentState();
    Drawable*getBackground()const;
    virtual View& setBackground(Drawable*background);
    View& setBackgroundColor(int color);
    View& setBackgroundResource(const std::string&resid);
    View& setBackgroundTintList(ColorStateList* tint);
    View& setBackgroundTintMode(int tintMode);
    ColorStateList* getBackgroundTintList()const;
    virtual int getSolidColor()const;

    bool isTemporarilyDetached()const;
    virtual void dispatchStartTemporaryDetach();
    virtual void dispatchFinishTemporaryDetach();
    virtual void onFinishTemporaryDetach();
    virtual void onStartTemporaryDetach();
    virtual bool hasTransientState();
    void setHasTransientState(bool hasTransientState);

    View& setId(int id);
    int  getId()const;
    int  getAccessibilityViewId();
    int  getAutoFillViewId();
    void setTag(void*);
    void*getTag()const;
    void setTag(int key,void*tag);
    void*getTag(int key)const;
    virtual View& setHint(const std::string&hint);
    const std::string&getHint()const;
    void setContentDescription(const std::string&);
    std::string getContentDescription()const;
    void setIsRootNamespace(bool);
    bool isRootNamespace()const;
    Context*getContext()const;
    virtual void scrollTo(int x,int y);
    virtual void scrollBy(int dx,int dy);
    void setScrollX(int x);
    void setScrollY(int y);
    int getScrollX()const;
    int getScrollY()const;
    int getOverScrollMode()const;
    virtual void setOverScrollMode(int overScrollMode);
    int getVerticalFadingEdgeLength();
    int getHorizontalFadingEdgeLength();
    void setFadingEdgeLength(int length);
    void transformFromViewToWindowSpace(int*);
    void mapRectFromViewToScreenCoords(Rect& rect, bool clipToParent);
    void getLocationOnScreen(int*);
    void getLocationInWindow(int*);
    bool startNestedScroll(int axes);
    void stopNestedScroll();
    void setNestedScrollingEnabled(bool benabled);
    bool isNestedScrollingEnabled()const;
    bool hasNestedScrollingParent()const;
    bool dispatchNestedScroll(int dxConsumed, int dyConsumed,
            int dxUnconsumed, int dyUnconsumed,int* offsetInWindow);
    bool dispatchNestedPreScroll(int dx, int dy,int* consumed,int* offsetInWindow);
    bool dispatchNestedFling(float velocityX, float velocityY, bool consumed);
    bool dispatchNestedPreFling(float velocityX, float velocityY);
    int  getRawTextDirection()const;
    void setTextDirection(int textDirection);
    int  getTextDirection()const;
    bool canResolveTextAlignment()const;
    void resetResolvedTextAlignment();
    bool isTextAlignmentInherited()const;
    bool isTextAlignmentResolved()const;
    virtual bool resolveTextAlignment();
    int  getRawTextAlignment()const;
    void setTextAlignment(int textAlignment);
    int  getTextAlignment()const;

   // Attribute
    virtual View& clearFlag(int flag);
    bool isAccessibilityFocused()const;
    bool requestAccessibilityFocus();
    bool isAccessibilityFocusedViewOrHost();
    virtual bool isFocused()const;
    virtual bool isInEditMode()const;
    bool isFocusedByDefault()const;
    void setFocusedByDefault(bool isFocusedByDefault);
    // Enable & Visible
    virtual View& setVisibility(int visable);
    virtual int getVisibility() const;
    int  getWindowVisibility()const;
    void getWindowVisibleDisplayFrame(Rect& outRect);
    void getWindowDisplayFrame(Rect& outRect);
    bool isShown()const;
    virtual View& setEnabled(bool enable);
    virtual bool isEnabled() const;
    virtual void setSelected(bool);
    bool isSelected()const;
    void setPressed(bool);
    bool isPressed()const;
    void setActivated(bool activated);
    bool isActivated()const;
    bool isHovered()const;
    void setHovered(bool hovered);
    bool isAutofilled()const;
    void setAutofilled(bool);

    bool isClickable()const;
    void setClickable(bool clickable);
    bool isLongClickable()const;
    void setLongClickable(bool longClickable);
    bool isContextClickable()const;
    void setContextClickable(bool contextClickable);

    bool isInScrollingContainer()const;
    virtual bool isInTouchMode()const;
    bool isFocusable()const;
    void setFocusable(bool);
    int  getFocusable()const;
    virtual void unFocus(View*);
    virtual bool hasFocus()const;
    virtual bool restoreFocusInCluster(int direction);
    virtual bool restoreFocusNotInCluster();
    virtual bool restoreDefaultFocus();
    void clearFocus();
    virtual View*findFocus();
    bool requestFocus(int direction=FOCUS_DOWN);
    virtual bool requestFocus(int direction,Rect* previouslyFocusedRect);
    bool hasFocusable()const{ return hasFocusable(true, false); }
    virtual bool hasFocusable(bool allowAutoFocus, bool dispatchExplicit)const;
    bool hasExplicitFocusable()const;
    virtual View*keyboardNavigationClusterSearch(View* currentCluster,int direction);
    // Parent and children views
    virtual ViewGroup*getParent()const;
    ViewGroup*getRootView()const;
    void bringToFront();

    virtual View* findViewById(int id)const;
    virtual View* findViewWithTag(void*)const;
    virtual View* findViewByPredicateTraversal(std::function<bool(const View*)>,View* childToSkip)const;
    virtual View* findViewWithTagTraversal(void* tag)const;
    View* findViewByPredicate(std::function<bool(const View*)>)const;
    View* findViewByPredicateInsideOut(View*start,std::function<bool(const View*)>)const;

    virtual View*focusSearch(int direction)const;
    View*findUserSetNextFocus(View*root,int direction)const;
    View*findUserSetNextKeyboardNavigationCluster(View*root,int direction)const;
    View*findKeyboardNavigationCluster()const;
    void addTouchables(std::vector<View*>& views)const;
    virtual void addFocusables(std::vector<View*>& views,int direction);
    virtual void addFocusables(std::vector<View*>& views,int direction,int focusableMode);

    std::vector<View*>getFocusables(int direction);
    void setKeyboardNavigationCluster(bool);
    bool isKeyboardNavigationCluster()const;
    virtual void addKeyboardNavigationClusters(std::vector<View*>&views,int drection);
    virtual bool dispatchTouchEvent(MotionEvent& event);
    virtual bool dispatchGenericMotionEvent(MotionEvent& event);

    KeyEvent::DispatcherState* getKeyDispatcherState()const;
    virtual bool dispatchKeyEvent(KeyEvent&event);
    virtual View* dispatchUnhandledKeyEvent(KeyEvent& evt);
    virtual bool dispatchUnhandledMove(View* focused,int direction);
    bool onKeyUp(int keycode,KeyEvent& evt)override;
    bool onKeyDown(int keycode,KeyEvent& evt)override;
    bool onKeyLongPress(int keyCode, KeyEvent& event)override;
    bool onKeyMultiple(int keyCode, int count, KeyEvent& event)override;
    virtual bool onUnhandledKeyEvent(KeyEvent& event);
    virtual bool hasUnhandledKeyListener()const;
    void addOnUnhandledKeyEventListener(OnUnhandledKeyEventListener listener);
    void removeOnUnhandledKeyEventListener(OnUnhandledKeyEventListener listener);

    virtual int  commitText(const std::wstring&);
    virtual void onWindowVisibilityChanged(int);
    virtual void onVisibilityAggregated(bool isVisible);
    virtual bool onInterceptTouchEvent(MotionEvent& evt);
    virtual bool onTouchEvent(MotionEvent& evt);
    virtual bool onHoverEvent(MotionEvent& evt);
    virtual bool onGenericMotionEvent(MotionEvent& event);
    virtual void onHoverChanged(bool hovered);
	
    void postOnAnimation(Runnable& action);
    void postOnAnimationDelayed(Runnable& action, uint32_t delayMillis);
    bool post(Runnable& what);
    bool post(const std::function<void()>&what);
    bool postDelayed(const std::function<void()>&what,uint32_t delay=0);
    virtual bool postDelayed(Runnable& what,uint32_t delay=0);
    virtual bool removeCallbacks(const Runnable& what);

    virtual int getBaseline();
    static bool isLayoutModeOptical(View*);
    bool resolveRtlPropertiesIfNeeded();
    void resetRtlProperties();
    void resetResolvedTextDirection();
    void resetResolvedLayoutDirection();
    void resetResolvedPadding();
    void measure(int widthMeasureSpec, int heightMeasureSpec);
    int  getMeasuredWidth()const;
    int  getMeasuredWidthAndState()const;
    int  getMeasuredHeight()const;
    int  getMeasuredState()const;
    int  getMeasuredHeightAndState()const;

    Matrix getMatrix();
    Matrix getInverseMatrix();
    bool hasIdentityMatrix()const;

    void setX(float);
    void setY(float);
    void setZ(float);
    float getX()const;//x pos to screen
    float getY()const;//y pos to screen
    float getZ()const;
    float getElevation()const;
    void setElevation(float elevation);
    float getTranslationX()const;
    float getTranslationY()const;
    float getTranslationZ()const;
    void setTranslationX(float x);
    void setTranslationY(float y);
    void setTranslationZ(float z);

    float getScaleX()const;
    void  setScaleX(float);
    float getScaleY()const;
    void  setScaleY(float);
    float getPivotX()const;
    void  setPivotX(float);
    float getPivotY()const;
    void  setPivotY(float);
    bool  isPivotSet()const;
    void  resetPivot();
    float getAlpha()const;
    void  setAlpha(float);

    float getRotation()const;
    void  setRotation(float rotation);
    float getRotationX()const;
    void  setRotationX(float);
    float getRotationY()const;
    void  setRotationY(float);
    StateListAnimator* getStateListAnimator()const;
    void setStateListAnimator(StateListAnimator*);

    LayoutParams*getLayoutParams();
    int getRawLayoutDirection()const;
    bool isLayoutDirectionInherited()const;
    void setLayoutParams(LayoutParams*lp);
    virtual ViewOverlay*getOverlay();
    virtual bool isLayoutRequested()const;
    virtual bool isInLayout()const;
    bool isLayoutValid()const;
    bool hasRtlSupport()const;
    bool isRtlCompatibilityMode()const;
    bool isTextDirectionInherited()const;
    bool isTextDirectionResolved()const;
    bool resolveTextDirection();
    virtual void requestLayout();
    void forceLayout();
    virtual void resolveLayoutParams();
    void layout(int l, int t, int r, int b);
};
}//endof namespace cdroid

using namespace cdroid;

#endif
