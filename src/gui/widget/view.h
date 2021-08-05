#ifndef __NGL_VIEW_H__
#define __NGL_VIEW_H__
#include <core/eventcodes.h>
#include <core/uievents.h>
#include <core/canvas.h>
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

#define NO_ID (-1) 

class ViewGroup;
class View:public Drawable::Callback,public KeyEvent::Callback{
public:
    enum{
        TEXT_ALIGNMENT_INHERIT   = 0,
        TEXT_ALIGNMENT_GRAVITY   = 1,
        TEXT_ALIGNMENT_TEXT_START= 2,
        TEXT_ALIGNMENT_TEXT_END  = 3,
        TEXT_ALIGNMENT_CENTER    = 4,
        TEXT_ALIGNMENT_VIEW_START= 5,
        TEXT_ALIGNMENT_VIEW_END  = 6,
        TEXT_ALIGNMENT_DEFAULT   = TEXT_ALIGNMENT_GRAVITY,
        TEXT_ALIGNMENT_RESOLVED_DEFAULT = TEXT_ALIGNMENT_GRAVITY,
    };
    enum TextDirection{
        TEXT_DIRECTION_INHERIT =0,
        TEXT_DIRECTION_FIRST_STRONG=1,
        TEXT_DIRECTION_ANY_RTL =2,
        TEXT_DIRECTION_LTR  =3,
        TEXT_DIRECTION_RTL =4,
        TEXT_DIRECTION_LOCALE =5,
        TEXT_DIRECTION_FIRST_STRONG_LTR =6,
        TEXT_DIRECTION_FIRST_STRONG_RTL =7,
        TEXT_DIRECTION_DEFAULT = TEXT_DIRECTION_INHERIT,
        TEXT_DIRECTION_RESOLVED_DEFAULT = TEXT_DIRECTION_FIRST_STRONG,
    }; 
    class TransformationInfo{
    public:
        Matrix mMatrix;
        Matrix mInverseMatrix;
        float mAlpha = 1.f;
        float mTransitionAlpha = 1.f;
    };
protected:
    enum{//PFLAGS in mPrivateFlags
        PFLAG_WANTS_FOCUS      = 0x01,
        PFLAG_FOCUSED          = 0x02,
        PFLAG_SELECTED         = 0x04,
        PFLAG_IS_ROOT_NAMESPACE= 0x08,
        PFLAG_HAS_BOUNDS       = 0x10,
        PFLAG_DRAWN            = 0x20,
        PFLAG_DRAW_ANIMATION   = 0x40,
        PFLAG_SKIP_DRAW        = 0x80,
        PFLAG_REQUEST_TRANSPARENT_REGIONS=0x200,
        PFLAG_DRAWABLE_STATE_DIRTY  =0x400,
        PFLAG_MEASURED_DIMENSION_SET=0x800,
        PFLAG_FORCE_LAYOUT     =0x1000,
        PFLAG_LAYOUT_REQUIRED  =0x2000,

        PFLAG_PRESSED          = 0x4000,
        PFLAG_DRAWING_CACHE_VALID= 0x8000,
        PFLAG_ANIMATION_STARTED= 0x00010000,
        PFLAG_DIRTY            = 0x00200000,
        PFLAG_DIRTY_OPAQUE     = 0x00400000,
        PFLAG_DIRTY_MASK       = 0x00600000,
        PFLAG_OPAQUE_BACKGROUND= 0x00800000,
        PFLAG_OPAQUE_SCROLLBARS= 0x01000000,
        PFLAG_OPAQUE_MASK      = 0x01800000,
        PFLAG_PREPRESSED       = 0x02000000,
        PFLAG_CANCEL_NEXT_UP_EVENT=0x04000000,
        PFLAG_HOVERED    = 0x10000000,
        PFLAG_ACTIVATED  = 0x40000000,
        PFLAG_INVALIDATED= 0x80000000, 
    };
    enum{//PFLAG2
        PFLAG2_TEXT_DIRECTION_MASK_SHIFT =6,
        PFLAG2_TEXT_DIRECTION_MASK      = 0x00000007<< PFLAG2_TEXT_DIRECTION_MASK_SHIFT,
        PFLAG2_TEXT_DIRECTION_RESOLVED  = 0x00000008 << PFLAG2_TEXT_DIRECTION_MASK_SHIFT,
        PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT=10,
        PFLAG2_TEXT_DIRECTION_RESOLVED_MASK = 0x00000007 << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT,
        PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT=13,
        PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT = 17,
        PFLAG2_TEXT_ALIGNMENT_MASK      = 0x00000007 << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT,
        PFLAG2_TEXT_ALIGNMENT_RESOLVED  = 0x00000008 << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT,
        PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK = 0x00000007 << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT,
        PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT = TEXT_ALIGNMENT_RESOLVED_DEFAULT << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT,
        PFLAG2_ACCESSIBILITY_FOCUSED   = 0x04000000,
        PFLAG2_PADDING_RESOLVED        = 0x20000000,
        PFLAG2_DRAWABLE_RESOLVED       = 0x40000000,
        PFLAG2_HAS_TRANSIENT_STATE     = 0x80000000,
        PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT=2,
        PFLAG2_LAYOUT_DIRECTION_MASK =0x00000003 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT,
        PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL =4 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT,
        PFLAG2_LAYOUT_DIRECTION_RESOLVED     =8 << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT,
        PFLAG2_LAYOUT_DIRECTION_RESOLVED_MASK= 0x0000000C<< PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT,
    };
    enum{
        PFLAG3_VIEW_IS_ANIMATING_TRANSFORM = 0x01,
        PFLAG3_VIEW_IS_ANIMATING_ALPHA     = 0x2,
        PFLAG3_IS_LAID_OUT                 = 0x04,
        PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT= 0x08,
        PFLAG3_NESTED_SCROLLING_ENABLED    = 0x80,
        PFLAG3_SCROLL_INDICATOR_TOP    = 0x0100,
        PFLAG3_SCROLL_INDICATOR_BOTTOM = 0x0200,
        PFLAG3_SCROLL_INDICATOR_LEFT   = 0x0400,
        PFLAG3_SCROLL_INDICATOR_RIGHT  = 0x0800,
        PFLAG3_SCROLL_INDICATOR_START  = 0x1000,
        PFLAG3_SCROLL_INDICATOR_END    = 0x2000,

        PFLAG3_CLUSTER                 = 0x08000,
        PFLAG3_FINGER_DOWN             = 0x20000,
        PFLAG3_FOCUSED_BY_DEFAULT      = 0x40000,
        PFLAG3_TEMPORARY_DETACH        = 0x2000000,
        PFLAG3_NO_REVEAL_ON_FOCUS      = 0x4000000
    };
public:
    enum{
        ENABLED  =0x00,
        DISABLED =0x01,
        ENABLED_MASK=0x1,

        VISIBLE  =0x02,
        INVISIBLE=0x04,
        GONE     =0x08,
        VISIBILITY_MASK=0x0E,

        NOT_FOCUSABLE=0,
        FOCUSABLE=0x10,
        FOCUSABLE_AUTO=0x20,
        FOCUSABLE_MASK=0x30,
        FOCUSABLE_IN_TOUCH_MODE=0x40,

        WILL_NOT_DRAW=0x80,
        DRAW_MASK    =0x80,

        SCROLLBARS_NONE=0,
        SCROLLBARS_HORIZONTAL=0x100,
        SCROLLBARS_VERTICAL=0x200,
        SCROLLBARS_MASK =0x300,

        CLIPCHILDREN=0x400,
        TRANSPARENT =0x800,

        FADING_EDGE_NONE =0x000000,
        FADING_EDGE_HORIZONTAL =0x1000,
        FADING_EDGE_VERTICAL =0x2000,
        FADING_EDGE_MASK =0x3000,

        CLICKABLE      = 0x4000,
        LONG_CLICKABLE =0x8000,
        DUPLICATE_PARENT_STATE=0x10000,
        CONTEXT_CLICKABLE=0x20000,
        TOOLTIP =0x40000,
        MEASURED_HEIGHT_STATE_SHIFT=16,
        MEASURED_STATE_TOO_SMALL=0x1000000,
        MEASURED_SIZE_MASK =0x00ffffff,
        MEASURED_STATE_MASK=0xff000000
    };
    enum {
        WM_CREATE    =0,
        WM_ACTIVE    =1,/*wp:0-->Active,1-->Deactive*/
        WM_DESTROY   =2,//no param
        WM_INVALIDATE=3,
        WM_TIMER     =4,//wParam it timerid lParam unused
        WM_CLICK     =5,//wParam is view's id
        WM_CHAR      =6,//wparam is unicode char
        WM_FOCUSRECT =7,//move focus rect wParam(x/y)lParam(width,height)
        WM_USER      =0x1000,/*the 1st user defined MESSAGE*/
    };
    enum FocusDirection{
        FOCUS_BACKWARD,
        FOCUS_FORWARD,
        FOCUS_LEFT,
        FOCUS_RIGHT,
        FOCUS_UP,
        FOCUS_DOWN
    };
    enum FocusableMode{
        FOCUSABLES_ALL=0,
        FOCUSABLES_TOUCH_MODE=1
    };
    enum {
        LAYOUT_DIRECTION_LTR=0,
        LAYOUT_DIRECTION_RTL=1,
        LAYOUT_DIRECTION_INHERIT=2,
        LAYOUT_DIRECTION_LOCALE=3
    };
    enum ScrollBarPosition{
        SCROLLBAR_POSITION_DEFAULT=0,
        SCROLLBAR_POSITION_LEFT=1,
        SCROLLBAR_POSITION_RIGHT=2
    };
    enum ScrollIndicators{
        SCROLL_INDICATORS_NONE        = 0x0000,
        SCROLL_INDICATORS_PFLAG3_MASK = PFLAG3_SCROLL_INDICATOR_TOP
             | PFLAG3_SCROLL_INDICATOR_BOTTOM | PFLAG3_SCROLL_INDICATOR_LEFT
             | PFLAG3_SCROLL_INDICATOR_RIGHT | PFLAG3_SCROLL_INDICATOR_START
             | PFLAG3_SCROLL_INDICATOR_END,
        SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT =8,
  
        SCROLL_INDICATOR_TOP    = PFLAG3_SCROLL_INDICATOR_TOP >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT,
        SCROLL_INDICATOR_BOTTOM = PFLAG3_SCROLL_INDICATOR_BOTTOM >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT,
        SCROLL_INDICATOR_LEFT   = PFLAG3_SCROLL_INDICATOR_LEFT >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT,
        SCROLL_INDICATOR_RIGHT  = PFLAG3_SCROLL_INDICATOR_RIGHT >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT,
    };
    enum{
        SCROLL_AXIS_NONE      =0,
        SCROLL_AXIS_HORIZONTAL=1,
        SCROLL_AXIS_VERTICAL  =2
    };
    enum OverScrollMode{
        OVER_SCROLL_ALWAYS =0,
        OVER_SCROLL_IF_CONTENT_SCROLLS =1,
        OVER_SCROLL_NEVER =2
    };
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
    bool mInContextButtonPress;
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
private:
    Drawable* getDefaultFocusHighlightDrawable();
    void setDefaultFocusHighlight(Drawable* highlight);
    void drawDefaultFocusHighlight(Canvas& canvas);

    void clip(RefPtr<Region>rgn);
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
    std::string mHint;
    LayoutParams*mLayoutParams;
    TransformationInfo* mTransformationInfo;
    Matrix mMatrix;
    Context*mContext;
    Animation* mCurrentAnimation;
    std::vector<int>mDrawableState;

    ViewGroup*mParent;
    int mTop,mLeft,mWidth,mHeight;
    OnClickListener mOnClick;
    OnLongClickListener mOnLongClick;
    OnFocusChangeListener mOnFocusChangeListener;
    std::vector<OnLayoutChangeListener> mOnLayoutChangeListeners;
    OnScrollChangeListener mOnScrollChangeListener;

    bool hasIdentityMatrix();
    void computeOpaqueFlags();
    virtual void resolveDrawables();
    void setDuplicateParentStateEnabled(bool);
    bool isDuplicateParentStateEnabled()const;
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
    virtual void onVisibilityChanged(View& changedView,int visibility);
    virtual void onAttached();
    virtual void onDettached();
    virtual Canvas*getCanvas();
    virtual void  onMeasure(int widthMeasureSpec, int heightMeasureSpec);
    virtual void dispatchDraw(Canvas&);
    virtual void onFocusChanged(bool,int,const RECT*);
    virtual void onFocusLost();
    virtual void clearParentsWantFocus();
    virtual void clearFocusInternal(View* focused, bool propagate, bool refocus);
    virtual void handleFocusGainInternal(int direction,const RECT*previouslyFocusedRect);
    bool awakenScrollBars();
    bool awakenScrollBars(int startDelay, bool invalidate);

    void postOnAnimation(Runnable action);
    void postOnAnimationDelayed(Runnable action, long delayMillis);
    virtual void onSizeChanged(int w,int h,int oldw,int oldh);
    virtual void onScrollChanged(int l, int t, int oldl, int oldt);
    virtual void onLayout(bool ,int,int,int,int);
    virtual void onDraw(Canvas& ctx);
    virtual void onDrawForeground(Canvas& canvas);
    virtual void onFinishInflate();
    virtual void dispatchSetActivated(bool activated);

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
    void playSoundEffect(int soundConstant);
    bool performHapticFeedback(int feedbackConstant, int flags=0);

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
    virtual void getScrollIndicatorBounds(RECT&);
    virtual void onDrawScrollIndicators(Canvas& canvas);
    virtual void onDrawScrollBars(Canvas& canvas);
    void onDrawHorizontalScrollBar(Canvas& canvas, Drawable* scrollBar,int l, int t, int w, int h);
    void onDrawVerticalScrollBar (Canvas& canvas , Drawable* scrollBar,int l, int t, int w, int h);

    void ensureTransformationInfo();
public:
    View(Context*ctx,const AttributeSet&attrs);
    View(int w,int h);
    virtual ~View();
    virtual void draw(Canvas*canvas=nullptr);
    virtual void invalidate(const RECT*rect=nullptr);
    void invalidate(int l,int t,int w,int h);
    bool isDirty()const;
    void postInvalidate();
    void postInvalidateOnAnimation();
    void invalidateDrawable(Drawable& who)override;
    void scheduleDrawable(Drawable& who,Runnable what, long when)override;
    void unscheduleDrawable(Drawable& who,Runnable what)override;
    virtual void unscheduleDrawable(Drawable& who);

    const RECT getBound()const;
    void getHitRect(RECT&);
    bool pointInView(int localX,int localY,int slop);
    const RECT getDrawingRect()const;
    virtual void getFocusedRect(RECT&r);
    virtual View& setBound(const RECT&);
    virtual View& setPos(int x,int y);
    virtual View& setSize(int x,int y);
    int getX()const;//x pos to screen
    int getY()const;//y pos to screen
    void getDrawingRect(RECT& outRect);
    void offsetTopAndBottom(int offset);
    void offsetLeftAndRight(int offset);
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
    void setPadding(int left, int top, int right, int bottom);
    bool isPaddingResolved()const;
    virtual void resolvePadding();

    int getMinimumHeight();
    void setMinimumHeight(int minHeight);
    int getMinimumWidth();
    void setMinimumWidth(int minWidth);
    Animation* getAnimation();
    void startAnimation(Animation* animation);
    void clearAnimation();
    void setAnimation(Animation* animation);
    
    void setDefaultFocusHighlightEnabled(bool defaultFocusHighlightEnabled);
    bool getDefaultFocusHighlightEnabled()const;
    bool isLayoutDirectionResolved()const;
    int getLayoutDirection()const;
    bool isOpaque()const;
    View&setLayoutDirection(int layoutDirection);
    bool isLayoutRtl()const;
    bool isFocusableInTouchMode()const;
    virtual void setFocusable(int focusable);
    virtual void setFocusableInTouchMode(bool focusableInTouchMode);
    void refreshDrawableState();
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
    virtual int computeHorizontalScrollRange();
    virtual int computeHorizontalScrollOffset();
    virtual int computeHorizontalScrollExtent();
    virtual int computeVerticalScrollRange();
    virtual int computeVerticalScrollOffset();
    virtual int computeVerticalScrollExtent();
    virtual bool canScrollHorizontally(int direction);
    virtual bool canScrollVertically(int direction);

    void setRevealOnFocusHint(bool revealOnFocus);
    bool getRevealOnFocusHint()const;
    bool willNotDraw()const;
    void setWillNotDraw(bool willNotDraw);
    const RECT getClientRect()const;
    bool hasClickListener()const;
    virtual void setOnClickListener(OnClickListener ls);
    virtual void setOnLongClickListener(OnLongClickListener l);
    virtual void setOnFocusChangeListener(OnFocusChangeListener listtener); 
    virtual void setOnScrollChangeListener(OnScrollChangeListener l);
    void addOnLayoutChangeListener(OnLayoutChangeListener listener);
    void removeOnLayoutChangeListener(OnLayoutChangeListener listener);
    virtual bool performClick();
    virtual bool performLongClick();
    virtual bool performLongClick(int x,int y);
    bool performContextClick(int x, int y);
    bool performContextClick();
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
    void setTextDirection(int textDirection);
    int getTextDirection()const;
    bool canResolveTextAlignment()const;
    void resetResolvedTextAlignment();
    bool isTextAlignmentInherited()const;
    bool isTextAlignmentResolved()const;
    virtual bool resolveTextAlignment();
    int getRawTextAlignment()const;
    void setTextAlignment(int textAlignment);
    int getTextAlignment()const;

   // Attribute
    virtual View& clearFlag(int flag);
    bool isAccessibilityFocused()const;
    bool requestAccessibilityFocus();
    bool isAccessibilityFocusedViewOrHost();
    virtual bool isFocused()const;
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
    virtual bool requestFocus(int direction,const RECT* previouslyFocusedRect);
    bool hasFocusable()const{ return hasFocusable(true, false); }
    virtual bool hasFocusable(bool allowAutoFocus, bool dispatchExplicit)const;
    bool hasExplicitFocusable()const;
    virtual View*keyboardNavigationClusterSearch(View* currentCluster,int direction);
    // Parent and children views
    virtual ViewGroup*getParent()const;
    ViewGroup*getRootView()const;
    virtual View& setParent(ViewGroup*p);
    virtual View*findViewById(int id)const;

    virtual View*focusSearch(int direction)const;
    View*findUserSetNextFocus(View*root,int direction)const;
    View*findUserSetNextKeyboardNavigationCluster(View*root,int direction)const;
    View*findKeyboardNavigationCluster()const;
    void addTouchables(std::vector<View*>& views)const;
    virtual void addFocusables(std::vector<View*>& views,int direction)const;
    virtual void addFocusables(std::vector<View*>& views,int direction,int focusableMode)const;

    std::vector<View*>getFocusables(int direction)const;
    void setKeyboardNavigationCluster(bool);
    bool isKeyboardNavigationCluster()const;
    virtual void addKeyboardNavigationClusters(std::vector<View*>&views,int drection)const;
    virtual bool dispatchTouchEvent(MotionEvent& event);
    virtual bool dispatchGenericMotionEvent(MotionEvent& event);

    virtual bool dispatchKeyEvent(KeyEvent&event);
    virtual bool dispatchUnhandledMove(View* focused,int direction);
    bool onKeyUp(int keycode,KeyEvent& evt)override;
    bool onKeyDown(int keycode,KeyEvent& evt)override;
    bool onKeyLongPress(int keyCode, KeyEvent& event)override;
    bool onKeyMultiple(int keyCode, int count, KeyEvent& event)override;
    virtual int commitText(const std::wstring&);
    virtual bool onInterceptTouchEvent(MotionEvent& evt);
    virtual bool onTouchEvent(MotionEvent& evt);
    virtual bool onHoverEvent(MotionEvent& evt);
    virtual bool onGenericMotionEvent(MotionEvent& event);
    virtual void onHoverChanged(bool hovered);
	
    void post(Runnable& what);
    virtual void postDelayed(Runnable& what,uint32_t delay=0);
    virtual void removeCallbacks(const Runnable& what);

    virtual int getBaseline();
    static bool isLayoutModeOptical(View*);
    bool resolveRtlPropertiesIfNeeded();
    void resetRtlProperties();
    void measure(int widthMeasureSpec, int heightMeasureSpec);
    int getMeasuredWidth()const;
    int getMeasuredWidthAndState()const;
    int getMeasuredHeight()const;
    int getMeasuredState()const;
    int getMeasuredHeightAndState()const;

    Matrix getMatrix();
    Matrix getInverseMatrix();

    float getRotation();
    void setRotation(float rotation);
    float getRotationX();
    void setRotationX(float);
    float getRotationY();
    void setRotationY(float);
    float getScaleX();
    void setScaleX(float);
    float getScaleY();
    void setScaleY(float);
    float getPivotX();
    void setPivotX(float);
    float getPivotY();
    void setPivotY(float);
    bool isPivotSet();
    void resetPivot();
    float getAlpha();
    void setAlpha(float);

    LayoutParams*getLayoutParams();
    int getRawLayoutDirection()const;
    bool isLayoutDirectionInherited()const;
    void setLayoutParams(LayoutParams*lp);
    virtual bool isLayoutRequested()const;
    bool isLaidOut()const;
    bool isLayoutValid()const;
    virtual void requestLayout();
    void forceLayout();
    virtual void resolveLayoutParams();
    void layout(int l, int t, int r, int b);
private:
    friend  ViewGroup;
    //Temporary values used to hold (x,y) coordinates when delegating from the
    // two-arg performLongClick() method to the legacy no-arg version
    int mLongClickX ,mLongClickY;
    int mNextFocusLeftId;
    int mNextFocusRightId;
    int mNextFocusUpId;
    int mNextFocusDownId;
    int mNextFocusForwardId;
    int mNextClusterForwardId;
    bool mHasPerformedLongPress;
    bool mIgnoreNextUpEvent;
    bool mOriginalPressedState;

    Runnable mPendingCheckForTap;
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
    bool requestFocusNoSearch(int direction,const RECT*previouslyFocusedRect);
    bool requestFocusFromTouch();
    bool hasAncestorThatBlocksDescendantFocus();
    View(const View&)=delete;
    View&operator=(const View&)=delete;
};
}//endof namespace cdroid

using namespace cdroid;

#endif
