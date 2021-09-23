#ifndef __NGL_VIEW_H__
#define __NGL_VIEW_H__
#include <core/eventcodes.h>
#include <core/uievents.h>
#include <core/canvas.h>
#include <core/insets.h>
#include <core/viewconfiguration.h>
#include <core/systemclock.h>
#include <widget/layoutparams.h>
#include <widget/measurespec.h>
#include <animation/animation.h>
#include <set>
#include <memory>
#include <vector>
#include <functional>
#include <core/rect.h>
#include <drawables.h>
#include <core/gravity.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <core/velocitytracker.h>

#ifndef _GLIBCXX_FUNCTIONAL
#define DECLARE_UIEVENT(type,name,...) typedef type(*name)(__VA_ARGS__)
#else
#define DECLARE_UIEVENT(type,name,...) typedef std::function< type(__VA_ARGS__) >name
#endif

#define TEXT(x) App::getInstance().getString(x)
#define _(x) TEXT(x)
#define _T(x) TEXT(x)

namespace cdroid{

class ViewGroup;
class View:public Drawable::Callback,public KeyEvent::Callback{
public:
    static bool DEBUG_DRAW;
    constexpr static int DEBUG_CORNERS_COLOR    = 0xFF3f7fff;
    constexpr static int DEBUG_CORNERS_SIZE_DIP = 8;
    constexpr static int NO_ID =-1;
    //TEXT Alignment
    constexpr static int TEXT_ALIGNMENT_INHERIT   = 0;
    constexpr static int TEXT_ALIGNMENT_GRAVITY   = 1;
    constexpr static int TEXT_ALIGNMENT_TEXT_START= 2;
    constexpr static int TEXT_ALIGNMENT_TEXT_END  = 3;
    constexpr static int TEXT_ALIGNMENT_CENTER    = 4;
    constexpr static int TEXT_ALIGNMENT_VIEW_START= 5;
    constexpr static int TEXT_ALIGNMENT_VIEW_END  = 6;
    constexpr static int TEXT_ALIGNMENT_DEFAULT   = TEXT_ALIGNMENT_GRAVITY;
    constexpr static int TEXT_ALIGNMENT_RESOLVED_DEFAULT = TEXT_ALIGNMENT_GRAVITY;

    //TextDirection{
    constexpr static int TEXT_DIRECTION_INHERIT =0;
    constexpr static int TEXT_DIRECTION_FIRST_STRONG=1;
    constexpr static int TEXT_DIRECTION_ANY_RTL =2;
    constexpr static int TEXT_DIRECTION_LTR     =3;
    constexpr static int TEXT_DIRECTION_RTL     =4;
    constexpr static int TEXT_DIRECTION_LOCALE  =5;
    constexpr static int TEXT_DIRECTION_FIRST_STRONG_LTR =6;
    constexpr static int TEXT_DIRECTION_FIRST_STRONG_RTL =7;
    constexpr static int TEXT_DIRECTION_DEFAULT = TEXT_DIRECTION_INHERIT;
    constexpr static int TEXT_DIRECTION_RESOLVED_DEFAULT = TEXT_DIRECTION_FIRST_STRONG;

    class TransformationInfo{
    public:
        Matrix mMatrix;
        Matrix mInverseMatrix;
        float mAlpha = 1.f;
        float mTransitionAlpha = 1.f;
    };
protected:
    //PFLAGS in mPrivateFlags
    constexpr static int PFLAG_WANTS_FOCUS      = 0x01;
    constexpr static int PFLAG_FOCUSED          = 0x02;
    constexpr static int PFLAG_SELECTED         = 0x04;
    constexpr static int PFLAG_IS_ROOT_NAMESPACE= 0x08;
    constexpr static int PFLAG_HAS_BOUNDS       = 0x10;
    constexpr static int PFLAG_DRAWN            = 0x20;
    constexpr static int PFLAG_DRAW_ANIMATION   = 0x40;
    constexpr static int PFLAG_SKIP_DRAW        = 0x80;
    constexpr static int PFLAG_REQUEST_TRANSPARENT_REGIONS=0x200;
    constexpr static int PFLAG_DRAWABLE_STATE_DIRTY  =0x400;
    constexpr static int PFLAG_MEASURED_DIMENSION_SET=0x800;
    constexpr static int PFLAG_FORCE_LAYOUT     =0x1000;
    constexpr static int PFLAG_LAYOUT_REQUIRED  =0x2000;

    constexpr static int PFLAG_PRESSED          = 0x4000;
    constexpr static int PFLAG_DRAWING_CACHE_VALID= 0x8000;
    constexpr static int PFLAG_ANIMATION_STARTED= 0x00010000;
    constexpr static int PFLAG_ALPHA_SET        = 0x00040000;
    constexpr static int PFLAG_DIRTY            = 0x00200000;
    constexpr static int PFLAG_DIRTY_OPAQUE     = 0x00400000;
    constexpr static int PFLAG_DIRTY_MASK       = 0x00600000;
    constexpr static int PFLAG_OPAQUE_BACKGROUND= 0x00800000;
    constexpr static int PFLAG_OPAQUE_SCROLLBARS= 0x01000000;
    constexpr static int PFLAG_OPAQUE_MASK      = 0x01800000;
    constexpr static int PFLAG_PREPRESSED       = 0x02000000;
    constexpr static int PFLAG_CANCEL_NEXT_UP_EVENT=0x04000000;
    constexpr static int PFLAG_HOVERED    = 0x10000000;
    constexpr static int PFLAG_ACTIVATED  = 0x40000000;
    constexpr static int PFLAG_INVALIDATED= 0x80000000;

    //PFLAG2 in mPrivateFlag2
    constexpr static int PFLAG2_TEXT_DIRECTION_MASK_SHIFT =6;
    constexpr static int PFLAG2_TEXT_DIRECTION_MASK      = 0x00000007<< PFLAG2_TEXT_DIRECTION_MASK_SHIFT;
    constexpr static int PFLAG2_TEXT_DIRECTION_RESOLVED  = 0x00000008 << PFLAG2_TEXT_DIRECTION_MASK_SHIFT;
    constexpr static int PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT=10;
    constexpr static int PFLAG2_TEXT_DIRECTION_RESOLVED_MASK = 0x00000007 << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT;
    constexpr static int PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT = TEXT_DIRECTION_RESOLVED_DEFAULT << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT;
    constexpr static int PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT = 13;
    constexpr static int PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT = 17;
    constexpr static int PFLAG2_TEXT_ALIGNMENT_MASK          = 0x00000007 << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT;
    constexpr static int PFLAG2_TEXT_ALIGNMENT_RESOLVED      = 0x00000008 << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT;
    constexpr static int PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK = 0x00000007 << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT;
    constexpr static int PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT = TEXT_ALIGNMENT_RESOLVED_DEFAULT << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT;
    constexpr static int PFLAG2_ACCESSIBILITY_FOCUSED   = 0x04000000;
    constexpr static int PFLAG2_VIEW_QUICK_REJECTED     = 0x10000000;
    constexpr static int PFLAG2_PADDING_RESOLVED        = 0x20000000;
    constexpr static int PFLAG2_DRAWABLE_RESOLVED       = 0x40000000;
    constexpr static int PFLAG2_HAS_TRANSIENT_STATE     = 0x80000000;
    constexpr static int PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT   = 0x02;
    constexpr static int PFLAG2_LAYOUT_DIRECTION_MASK         = 0x00000003 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
    constexpr static int PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL = 4 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
    constexpr static int PFLAG2_LAYOUT_DIRECTION_RESOLVED     = 8 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
    constexpr static int PFLAG2_LAYOUT_DIRECTION_RESOLVED_MASK= 0x0000000C<< PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
    constexpr static int ALL_RTL_PROPERTIES_RESOLVED = PFLAG2_LAYOUT_DIRECTION_RESOLVED |  PFLAG2_TEXT_DIRECTION_RESOLVED 
               | PFLAG2_TEXT_ALIGNMENT_RESOLVED | PFLAG2_PADDING_RESOLVED | PFLAG2_DRAWABLE_RESOLVED;
    
    //FLAGS in mPrivateFlags3
    constexpr static int PFLAG3_VIEW_IS_ANIMATING_TRANSFORM = 0x0001;
    constexpr static int PFLAG3_VIEW_IS_ANIMATING_ALPHA     = 0x0002;
    constexpr static int PFLAG3_IS_LAID_OUT                 = 0x0004;
    constexpr static int PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT= 0x0008;
    constexpr static int PFLAG3_NESTED_SCROLLING_ENABLED    = 0x0080;
    constexpr static int PFLAG3_SCROLL_INDICATOR_TOP        = 0x0100;
    constexpr static int PFLAG3_SCROLL_INDICATOR_BOTTOM     = 0x0200;
    constexpr static int PFLAG3_SCROLL_INDICATOR_LEFT       = 0x0400;
    constexpr static int PFLAG3_SCROLL_INDICATOR_RIGHT      = 0x0800;
    constexpr static int PFLAG3_SCROLL_INDICATOR_START      = 0x1000;
    constexpr static int PFLAG3_SCROLL_INDICATOR_END        = 0x2000;

    constexpr static int PFLAG3_CLUSTER                 = 0x08000;
    constexpr static int PFLAG3_FINGER_DOWN             = 0x20000;
    constexpr static int PFLAG3_FOCUSED_BY_DEFAULT      = 0x40000;
    constexpr static int PFLAG3_TEMPORARY_DETACH        = 0x2000000;
    constexpr static int PFLAG3_NO_REVEAL_ON_FOCUS      = 0x4000000;
    constexpr static int PFLAG3_AGGREGATED_VISIBLE      = 0x20000000;
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
        bool mAlwaysConsumeNavBar;
        bool mHasWindowFocus;
        bool mViewVisibilityChanged;
        int mWindowVisibility;
        long mDrawingTime;
        bool mInTouchMode;
        bool mKeepScreenOn;
        bool mDebugLayout;
        Canvas* mCanvas;
        View* mTooltipHost;
        AttachInfo(); 
    };
public:
    constexpr static int ENABLED        = 0x00;
    constexpr static int DISABLED       = 0x01;
    constexpr static int ENABLED_MASK   = 0x01;
    constexpr static int VISIBLE        = 0x02;
    constexpr static int INVISIBLE      = 0x04;
    constexpr static int GONE           = 0x08;
    constexpr static int VISIBILITY_MASK= 0x0E;
    constexpr static int NOT_FOCUSABLE  = 0x00;
    constexpr static int FOCUSABLE      = 0x10;
    constexpr static int FOCUSABLE_AUTO = 0x20;
    constexpr static int FOCUSABLE_MASK = 0x30;
    constexpr static int FOCUSABLE_IN_TOUCH_MODE=0x40;

    constexpr static int WILL_NOT_DRAW  = 0x80;
    constexpr static int DRAW_MASK      = 0x80;

    constexpr static int SCROLLBARS_NONE= 0;
    constexpr static int SCROLLBARS_HORIZONTAL= 0x100;
    constexpr static int SCROLLBARS_VERTICAL  = 0x200;
    constexpr static int SCROLLBARS_MASK      = 0x300;

    constexpr static int CLIPCHILDREN = 0x400;
    constexpr static int TRANSPARENT  = 0x800;

    constexpr static int FADING_EDGE_NONE =0x000000;
    constexpr static int FADING_EDGE_HORIZONTAL= 0x1000;
    constexpr static int FADING_EDGE_VERTICAL  = 0x2000;
    constexpr static int FADING_EDGE_MASK      = 0x3000;

    constexpr static int CLICKABLE       = 0x4000;
    constexpr static int DRAWING_CACHE_ENABLED = 0x8000;
    constexpr static int WILL_NOT_CACHE_DRAWING = 0x000020000;
        
    constexpr static int LONG_CLICKABLE = 0x200000;
    constexpr static int DUPLICATE_PARENT_STATE=0x10000;
    constexpr static int CONTEXT_CLICKABLE=0x20000;
    constexpr static int TOOLTIP =0x40000;

    constexpr static int DRAWING_CACHE_QUALITY_LOW  = 0x00080000;
    constexpr static int DRAWING_CACHE_QUALITY_HIGH = 0x00100000;
    constexpr static int DRAWING_CACHE_QUALITY_AUTO = 0x00000000;
    constexpr static int DRAWING_CACHE_QUALITY_MASK = 0x00180000;

    constexpr static int MEASURED_HEIGHT_STATE_SHIFT=16;
    constexpr static int MEASURED_STATE_TOO_SMALL=0x1000000;
    constexpr static int MEASURED_SIZE_MASK =0x00ffffff;
    constexpr static int MEASURED_STATE_MASK=0xff000000;

    //FocusDirection{
    constexpr static int FOCUS_BACKWARD=0x01;
    constexpr static int FOCUS_FORWARD =0x02;
    constexpr static int FOCUS_LEFT    =0x11;
    constexpr static int FOCUS_UP      =0x21;
    constexpr static int FOCUS_RIGHT   =0x42;
    constexpr static int FOCUS_DOWN    =0x82;

    //FocusableMode
    constexpr static int FOCUSABLES_ALL = 0;
    constexpr static int FOCUSABLES_TOUCH_MODE=1;
  
    constexpr static int LAYOUT_DIRECTION_UNDEFINED = LayoutDirection::UNDEFINED;
    constexpr static int LAYOUT_DIRECTION_LTR=LayoutDirection::LTR;
    constexpr static int LAYOUT_DIRECTION_RTL=LayoutDirection::RTL;
    constexpr static int LAYOUT_DIRECTION_INHERIT=LayoutDirection::INHERIT;
    constexpr static int LAYOUT_DIRECTION_LOCALE=LayoutDirection::LOCAL;
  
    //ScrollBarPosition
    constexpr static int SCROLLBAR_POSITION_DEFAULT= 0;
    constexpr static int SCROLLBAR_POSITION_LEFT   = 1;
    constexpr static int SCROLLBAR_POSITION_RIGHT  = 2;

    //ScrollIndicators
    constexpr static int SCROLL_INDICATORS_NONE        = 0x0000;
    constexpr static int SCROLL_INDICATORS_PFLAG3_MASK = PFLAG3_SCROLL_INDICATOR_TOP | PFLAG3_SCROLL_INDICATOR_BOTTOM 
             | PFLAG3_SCROLL_INDICATOR_LEFT  | PFLAG3_SCROLL_INDICATOR_RIGHT 
             | PFLAG3_SCROLL_INDICATOR_START | PFLAG3_SCROLL_INDICATOR_END;
    constexpr static int SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT =8;
  
    constexpr static int SCROLL_INDICATOR_TOP    = PFLAG3_SCROLL_INDICATOR_TOP >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    constexpr static int SCROLL_INDICATOR_BOTTOM = PFLAG3_SCROLL_INDICATOR_BOTTOM >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    constexpr static int SCROLL_INDICATOR_LEFT   = PFLAG3_SCROLL_INDICATOR_LEFT >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    constexpr static int SCROLL_INDICATOR_RIGHT  = PFLAG3_SCROLL_INDICATOR_RIGHT >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;

    /*The scrollbar style to display the scrollbars at the edge of the view,
     * increasing the padding of the view. The scrollbars will only overlap the
     * background, if any*/
    constexpr static int SCROLLBARS_INSIDE_OVERLAY = 0;
    constexpr static int SCROLLBARS_INSIDE_INSET   = 0x01000000;
    constexpr static int SCROLLBARS_OUTSIDE_OVERLAY= 0x02000000;
    constexpr static int SCROLLBARS_OUTSIDE_INSET  = 0x03000000;
    constexpr static int SCROLLBARS_INSET_MASK     = 0x01000000;
    constexpr static int SCROLLBARS_OUTSIDE_MASK   = 0x02000000;
    constexpr static int SCROLLBARS_STYLE_MASK     = 0x03000000;

    //Indicates no axis of view scrolling.
    constexpr static int SCROLL_AXIS_NONE      =0;
    constexpr static int SCROLL_AXIS_HORIZONTAL=1;
    constexpr static int SCROLL_AXIS_VERTICAL  =2;

    //OverScrollMode of view
    constexpr static int OVER_SCROLL_ALWAYS =0;
    constexpr static int OVER_SCROLL_IF_CONTENT_SCROLLS =1;
    constexpr static int OVER_SCROLL_NEVER =2;

    DECLARE_UIEVENT(void,OnClickListener,View&);
    DECLARE_UIEVENT(bool,OnLongClickListener,View&);
    DECLARE_UIEVENT(void,OnFocusChangeListener,View&,bool);
    DECLARE_UIEVENT(void,OnScrollChangeListener,View& v, int, int, int, int);
    DECLARE_UIEVENT(void,OnLayoutChangeListener,View* v, int left, int top, int width, int height,
            int oldLeft, int oldTop, int oldWidth, int oldHeight);
private:
    int mMinWidth;
    int mMinHeight;
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
 
    Runnable mPendingCheckForTap;
    bool mInContextButtonPress;
    bool mHasPerformedLongPress;
    bool mIgnoreNextUpEvent;
    bool mOriginalPressedState;

    ViewGroup*mNestedScrollingParent;
    std::map<int,int>mMeasureCache;
    class ScrollabilityCache*mScrollCache;

    Drawable*mBackground;
    bool mBackgroundSizeChanged;
    Drawable*mDefaultFocusHighlight;
    Drawable*mDefaultFocusHighlightCache;
    bool mDefaultFocusHighlightSizeChanged;
    bool mDefaultFocusHighlightEnabled;
    bool mBoundsChangedmDefaultFocusHighlightSizeChanged;
    Drawable*mScrollIndicatorDrawable;
    class RoundScrollbarRenderer* mRoundScrollbarRenderer;
    class TintInfo*mBackgroundTint;
    class ForegroundInfo*mForegroundInfo;
    KeyEvent::DispatcherState mKeyDispatchState;
private:
    void debugDrawFocus(Canvas&canvas);
    Drawable* getDefaultFocusHighlightDrawable();
    void setDefaultFocusHighlight(Drawable* highlight);
    void switchDefaultFocusHighlight();
    void drawDefaultFocusHighlight(Canvas& canvas);

    void sizeChange(int newWidth,int newHeight,int oldWidth,int oldHeight);
    void setMeasuredDimensionRaw(int measuredWidth, int measuredHeight);
    void initScrollCache();
    ScrollabilityCache* getScrollCache();
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
    bool hasParentWantsFocus()const;
    void invalidateInternal(int l, int t, int r, int b, bool invalidateCache,bool fullInvalidate);
protected:
    int mID;
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
    int mUserPaddingRightInitial;

    std::string mHint;
    bool mCachingFailed;
    RefPtr<ImageSurface>mDrawingCache;
    RefPtr<ImageSurface>mUnscaledDrawingCache;
    LayoutParams*mLayoutParams;
    TransformationInfo* mTransformationInfo;
    Matrix mMatrix;
    Context*mContext;
    Animation* mCurrentAnimation;
    std::vector<int>mDrawableState;

    ViewGroup*mParent;
    AttachInfo* mAttachInfo;
    int mTop,mLeft,mRight,mBottom;
    float mX,mY,mZ,mScaleX,mScaleY;
    float mRotationX,mRotationY,mRotation;
    float mPivotX,mPivotY,mAlpha;
    float mTranslationX,mTranslationY,mTranslationZ;
    OnClickListener mOnClick;
    OnLongClickListener mOnLongClick;
    OnFocusChangeListener mOnFocusChangeListener;
    std::vector<OnLayoutChangeListener> mOnLayoutChangeListeners;
    OnScrollChangeListener mOnScrollChangeListener;
    void assignParent(ViewGroup*p);
    bool debugDraw()const;
    int dipsToPixels(int dips)const;
    bool hasIdentityMatrix();
    void computeOpaqueFlags();
    virtual void resolveDrawables();
    bool areDrawablesResolved();
    void setDuplicateParentStateEnabled(bool);
    bool isDuplicateParentStateEnabled()const;

    bool isPaddingOffsetRequired();
    int getLeftPaddingOffset();
    int getRightPaddingOffset();
    int getTopPaddingOffset();
    int getBottomPaddingOffset();
    int getFadeTop(bool offsetRequired);
    int getFadeHeight(bool offsetRequired);
    bool isHardwareAccelerated()const;

    void invalidateViewProperty(bool invalidateParent, bool forceRedraw);
    void invalidateParentCaches();
    void invalidateParentIfNeeded();
    void invalidateParentIfNeededAndWasQuickRejected();
    void destroyDrawingCache();
    RefPtr<ImageSurface>getDrawingCache(bool autoScale);
    bool hasWindowFocus()const;
    int getWindowVisibility()const;

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

    void postOnAnimation(Runnable& action);
    void postOnAnimationDelayed(Runnable& action, uint32_t delayMillis);
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

    virtual bool dispatchHoverEvent(MotionEvent&event);
    virtual bool dispatchGenericPointerEvent(MotionEvent& event);
    virtual bool dispatchGenericFocusedEvent(MotionEvent& event);

    static int combineMeasuredStates(int curState, int newState);
    static std::vector<int>& mergeDrawableStates(std::vector<int>&baseState,const std::vector<int>&additionalState);
    static int resolveSize(int size, int measureSpec);
    static int resolveSizeAndState(int size, int measureSpec, int childMeasuredState);
    static int getDefaultSize(int size, int measureSpec);
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
    int scrollRangeY, int maxOverScrollX, int maxOverScrollY, bool isTouchEvent);
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
    void invalidate(const Rect&dirty);
    void invalidate(int l,int t,int w,int h);
    void invalidate(bool invalidateCache=true);
    bool isDirty()const;
    void postInvalidate();
    void postInvalidateOnAnimation();
    void invalidateDrawable(Drawable& who)override;
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
    void setOpticalInsets(const Insets& insets);
    void setPadding(int left, int top, int right, int bottom);
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
    Animation* getAnimation();
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
    bool isHorizontalScrollBarEnabled()const;
    View& setHorizontalScrollBarEnabled(bool);
    bool isVerticalScrollBarEnabled()const;
    View& setVerticalScrollBarEnabled(bool);
    int getHorizontalScrollbarHeight()const;
    int getVerticalScrollbarWidth()const;
    int getVerticalScrollbarPosition()const;

    bool isHorizontalFadingEdgeEnabled()const;
    void setHorizontalFadingEdgeEnabled(bool horizontalFadingEdgeEnabled);
    bool isVerticalFadingEdgeEnabled()const;
    void setVerticalFadingEdgeEnabled(bool verticalFadingEdgeEnabled);

    View& setVerticalScrollbarPosition(int position);

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
    virtual void setOnClickListener(OnClickListener ls);
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
    virtual View& setBackgroundDrawable(Drawable*drawable);
    View& setBackgroundColor(int color);
    View& setBackgroundResource(const std::string&resid);
    View& setBackgroundTintList(ColorStateList* tint);
    View& setBackgroundTintMode(int tintMode);
    ColorStateList* getBackgroundTintList()const;
    virtual int getSolidColor()const;

    bool isTemporarilyDetached()const;
    void dispatchFinishTemporaryDetach();
    virtual void onFinishTemporaryDetach();
    void dispatchStartTemporaryDetach();
    virtual void onStartTemporaryDetach();
    virtual bool hasTransientState();
    void setHasTransientState(bool hasTransientState);

    View& setId(int id);
    int getId()const;
    virtual View& setHint(const std::string&hint);
    const std::string&getHint()const;
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
    int getFocusable()const;
    virtual void unFocus(View*);
    bool hasFocus()const;
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

    virtual View*findViewById(int id)const;
    virtual View* findViewByPredicateTraversal(std::function<bool(const View*)>,View* childToSkip)const;
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

    virtual bool dispatchKeyEvent(KeyEvent&event);
    virtual bool dispatchUnhandledMove(View* focused,int direction);
    bool onKeyUp(int keycode,KeyEvent& evt)override;
    bool onKeyDown(int keycode,KeyEvent& evt)override;
    bool onKeyLongPress(int keyCode, KeyEvent& event)override;
    bool onKeyMultiple(int keyCode, int count, KeyEvent& event)override;
    virtual int commitText(const std::wstring&);
    virtual void onWindowVisibilityChanged(int);
    virtual void onVisibilityAggregated(bool isVisible);
    virtual bool onInterceptTouchEvent(MotionEvent& evt);
    virtual bool onTouchEvent(MotionEvent& evt);
    virtual bool onHoverEvent(MotionEvent& evt);
    virtual bool onGenericMotionEvent(MotionEvent& event);
    virtual void onHoverChanged(bool hovered);
	
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
    void setScaleX(float);
    float getScaleY()const;
    void setScaleY(float);
    float getPivotX()const;
    void setPivotX(float);
    float getPivotY()const;
    void setPivotY(float);
    bool isPivotSet()const;
    void resetPivot();
    float getAlpha()const;
    void setAlpha(float);

    float getRotation()const;
    void setRotation(float rotation);
    float getRotationX()const;
    void setRotationX(float);
    float getRotationY()const;
    void setRotationY(float);

    LayoutParams*getLayoutParams();
    int getRawLayoutDirection()const;
    bool isLayoutDirectionInherited()const;
    void setLayoutParams(LayoutParams*lp);
    virtual bool isLayoutRequested()const;
    virtual bool isInLayout()const;
    bool isLayoutValid()const;
    bool hasRtlSupport()const;
    bool isTextDirectionInherited()const;
    bool isTextDirectionResolved()const;
    bool resolveTextDirection();
    virtual void requestLayout();
    void forceLayout();
    virtual void resolveLayoutParams();
    void layout(int l, int t, int r, int b);
private:
    friend  ViewGroup;
    //Temporary values used to hold (x,y) coordinates when delegating from the
    // two-arg performLongClick() method to the legacy no-arg version
    void checkForTapCallback(int x,int y);//for mPendingCheckForTap

    Runnable mPendingCheckForLongPress;
    void checkLongPressCallback(int x,int y);//for mPendingCheckForLongPress

    Runnable mUnsetPressedState;
    void unsetPressedCallback();//for mUnsetPressedState

    void removeTapCallback();
    void removeLongPressCallback();
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
    bool hasAncestorThatBlocksDescendantFocus();
    View(const View&)=delete;
    View&operator=(const View&)=delete;
};
}//endof namespace cdroid

using namespace cdroid;

#endif
