#include <widget/view.h>
#include <widget/viewgroup.h>
#include <widget/measurespec.h>
#include <widget/roundscrollbarrenderer.h>
#include <cdlog.h>
#include <string.h>
#include <algorithm>
#include <focusfinder.h>
#include <inputmethodmanager.h>
#include <app.h>
#include <color.h>
#include <systemclock.h>


namespace cdroid{

class TintInfo{
public:
    ColorStateList*mTintList;
    bool mHasTintMode;
    bool mHasTintList;
    int mTintMode;
    TintInfo(){
        mTintList=nullptr;
        mHasTintList=mHasTintMode=false;
        mTintMode=0;
    };
    ~TintInfo(){
        delete mTintList;
    };
};

class ForegroundInfo {
public:
    Drawable* mDrawable;
    TintInfo* mTintInfo;
    int mGravity;
    bool mInsidePadding;
    bool mBoundsChanged;
    RECT mSelfBounds;
    RECT mOverlayBounds;
public:
    ForegroundInfo(){
        mInsidePadding=mBoundsChanged=true;
        mGravity=Gravity::FILL;
        mSelfBounds.set(0,0,0,0);
        mOverlayBounds.set(0,0,0,0);
        mTintInfo=nullptr;
        mDrawable=nullptr;
    }
    ~ForegroundInfo(){
        delete mDrawable;
        delete mTintInfo;
    }
};

class ScrollabilityCache{
public:
    int scrollBarSize;
    int mScrollBarDraggingPos;
    int scrollBarMinTouchTarget;
    int fadingEdgeLength;

    int scrollBarDefaultDelayBeforeFade;
    int scrollBarFadeDuration;
    RECT mScrollBarBounds;
    RECT mScrollBarTouchBounds;
    bool fadeScrollBars;
    ScrollBarDrawable*scrollBar;
public:
    ScrollabilityCache(int sz){
        scrollBarSize=sz;
        fadeScrollBars=true;
        scrollBar=nullptr;
        mScrollBarDraggingPos=0;
        scrollBarMinTouchTarget=8;
        mScrollBarBounds.set(0,0,0,0);
        mScrollBarTouchBounds.set(0,0,0,0);
    }
    virtual ~ScrollabilityCache(){
        delete scrollBar;
    }
};

View::View(int w,int h){
    initView();
    mContext=&App::getInstance();
    mWidth=w;
    mHeight=h;
    mLeft=mTop=0;
    setBackgroundColor(0xFF000000);
    if(ViewConfiguration::isScreenRound())
       mRoundScrollbarRenderer=new RoundScrollbarRenderer(this);
}

View::View(Context*ctx,const AttributeSet&attrs){
    initView();
    mContext=ctx;
    mID=attrs.getInt("id",NO_ID);
    mMinWidth = attrs.getDimensionPixelSize("minWidth",0);
    mMinHeight= attrs.getDimensionPixelSize("minHeight",0);
    setClickable(attrs.getBoolean("clickable",false));
    setLongClickable(attrs.getBoolean("longclickable",false));
    setFocusableInTouchMode(attrs.getBoolean("focusableInTouchMode",false));
    std::string bgtxt=attrs.getString("background");
    if(!bgtxt.empty()){
        if(bgtxt[0]=='#')
            mBackground=new ColorDrawable(Color::parseColor(bgtxt));
         else mBackground=ctx->getDrawable(attrs.getString("background"));
    }else
        mBackground=new ColorDrawable(0xFF222222);
    
    int padding=attrs.getDimensionPixelSize("padding",-1);
    if(padding>=0){
        mPaddingLeft= mPaddingRight =padding;
        mPaddingTop = mPaddingBottom=padding;
    }else{
        int horz=attrs.getDimensionPixelSize("paddingHorizontal",-1);
        int vert=attrs.getDimensionPixelSize("paddingVertical",-1);
        if(horz>=0){
            mPaddingLeft=mPaddingRight=horz;
        }else{
            mPaddingLeft =attrs.getDimensionPixelSize("paddingLeft",0);
            mPaddingRight=attrs.getDimensionPixelSize("paddingRight",0);
        }
        if(vert>=0){
            mPaddingTop = mPaddingBottom=vert;
        }else{
            mPaddingTop   =attrs.getDimensionPixelSize("paddingTop",0);
            mPaddingBottom=attrs.getDimensionPixelSize("paddingBottom",0);
        }
    }
    std::string scbars=attrs.getString("scrollbars","none");
    if(scbars.compare("none")){
        if(scbars.find("vert")!=std::string::npos)
            setVerticalScrollBarEnabled(true);
        if(scbars.find("horiz")!=std::string::npos)
            setHorizontalScrollBarEnabled(true);
        setScrollBarSize(attrs.getDimensionPixelSize("scrollBarSize",10));

        mScrollCache->scrollBar->setHorizontalThumbDrawable(
            Drawable::inflate(mContext,attrs.getString("scrollbarThumbHorizontal")));
        mScrollCache->scrollBar->setHorizontalTrackDrawable(
            Drawable::inflate(mContext,attrs.getString("scrollbarTrackHorizontal")));
        
        mScrollCache->scrollBar->setVerticalThumbDrawable(
            Drawable::inflate(mContext,attrs.getString("scrollbarThumbVertical")));
        mScrollCache->scrollBar->setVerticalTrackDrawable(
            Drawable::inflate(mContext,attrs.getString("scrollbarTrackVertical")));
    } 
}

void View::initView(){
    mContext=nullptr;
    mID=NO_ID;
    mParent=nullptr;
    mScrollX=mScrollY=0;
    mMinWidth=mMinHeight=0;
    mViewFlags=ENABLED|VISIBLE|FOCUSABLE_AUTO;
    mPrivateFlags = mPrivateFlags2 = mPrivateFlags3 =0;
    mScrollCache=nullptr;
    mRoundScrollbarRenderer=nullptr;
    mTop=mLeft=mWidth=mHeight=0;
    mOnClick=mOnLongClick=nullptr;
    mOnFocusChangeListener=nullptr;
    mOnScrollChangeListener=nullptr;
    mOverScrollMode=OVER_SCROLL_NEVER;
    mVerticalScrollbarPosition=0;
    mUserPaddingLeft=mUserPaddingRight=0;
    mUserPaddingTop=mUserPaddingBottom=0;
    mPrivateFlags=mPrivateFlags3=0;
    mPrivateFlags|=PFLAG_DRAWABLE_STATE_DIRTY;
    mNextFocusLeftId= mNextFocusRightId=NO_ID;
    mNextFocusUpId  = mNextFocusDownId=NO_ID;
    mNextFocusForwardId=mNextClusterForwardId=NO_ID;
    mBackgroundTint=nullptr;
    mMeasuredWidth=mMeasuredHeight=0;
    mLayoutParams=nullptr;
    mPaddingLeft=mPaddingTop=0;
    mPaddingRight=mPaddingBottom=0;
    mForegroundInfo=nullptr;
    mScrollIndicatorDrawable=nullptr;
    mBackground=nullptr;
    mDefaultFocusHighlight=nullptr;
    mDefaultFocusHighlightCache=nullptr;
    mCurrentAnimation=nullptr;
    mTransformationInfo=nullptr;
    mMatrix=identity_matrix();
}

View::~View(){
    if(mParent)onDettached();
    if(mBackground)mBackground->setCallback(nullptr);
    delete mScrollIndicatorDrawable;
    delete mDefaultFocusHighlight;
	delete mDefaultFocusHighlightCache;
    delete mScrollCache;
    delete mBackground;
    delete mBackgroundTint;
    delete mLayoutParams;
    delete mRoundScrollbarRenderer;
    delete mCurrentAnimation;
    delete mTransformationInfo;
}

View*View::findViewById(int id)const{
    if(id==mID)return (View*)this;
    return nullptr;
}

View*View::findViewInsideOutShouldExist(View* root, int id)const{
    
    return nullptr;
}

int View::getLeft()const{
    return mLeft;
}

int View::getTop()const{
    return mTop;
}

int View::getRight()const{
    return mLeft+mWidth;
}

int View::getBottom()const{
    return mTop+mHeight;
}

int View::getPaddingTop() {
   return mPaddingTop;
}

int View::getPaddingBottom() {
    return mPaddingBottom;
}

int View::getPaddingLeft() {
   if (!isPaddingResolved()) {
       resolvePadding();
   }
   return mPaddingLeft;
}

int View::getPaddingStart() {
   if (!isPaddingResolved()) resolvePadding();

   return (getLayoutDirection() == LAYOUT_DIRECTION_RTL) ?mPaddingRight : mPaddingLeft;
}

int View::getPaddingRight() {
    if (!isPaddingResolved()) {
        resolvePadding();
    }
    return mPaddingRight;
}

int View::getPaddingEnd() {
   if (!isPaddingResolved())resolvePadding();
   
   return (getLayoutDirection() == LAYOUT_DIRECTION_RTL) ?mPaddingLeft : mPaddingRight;
}

void View::setPadding(int left, int top, int right, int bottom){
    mPaddingLeft=left;
    mPaddingRight=right;
    mPaddingTop=top;
    mPaddingBottom=bottom;
    LOGV("%p padding=%d,%d-%d-%d",this,left,top,right,bottom);
}

bool View::isPaddingResolved()const{
    return (mPrivateFlags2 & PFLAG2_PADDING_RESOLVED) == PFLAG2_PADDING_RESOLVED;
}

bool View::isLayoutDirectionResolved()const{
    return (mPrivateFlags2 & PFLAG2_LAYOUT_DIRECTION_RESOLVED) == PFLAG2_LAYOUT_DIRECTION_RESOLVED;
}

void View::resolvePadding(){
    mPrivateFlags2 |= PFLAG2_PADDING_RESOLVED;
}

bool View::isOpaque()const{
    return (mPrivateFlags & PFLAG_OPAQUE_MASK) == PFLAG_OPAQUE_MASK ;//&&  getFinalAlpha() >= 1.0f;
}

void View::computeOpaqueFlags(){
    if (mBackground  && mBackground->getOpacity() == Drawable::OPAQUE) {
        mPrivateFlags |= PFLAG_OPAQUE_BACKGROUND;
    } else {
        mPrivateFlags &= ~PFLAG_OPAQUE_BACKGROUND;
    }

    int flags = mViewFlags;
    /*if (((flags & SCROLLBARS_VERTICAL) == 0 && (flags & SCROLLBARS_HORIZONTAL) == 0) ||
            (flags & SCROLLBARS_STYLE_MASK) == SCROLLBARS_INSIDE_OVERLAY ||
            (flags & SCROLLBARS_STYLE_MASK) == SCROLLBARS_OUTSIDE_OVERLAY) {
        mPrivateFlags |= PFLAG_OPAQUE_SCROLLBARS;
    } else {
        mPrivateFlags &= ~PFLAG_OPAQUE_SCROLLBARS;
    }*/
}


void View::onAnimationStart() {
    mPrivateFlags |= PFLAG_ANIMATION_STARTED;
}

void View::onAnimationEnd() {
    mPrivateFlags &= ~PFLAG_ANIMATION_STARTED;
}

bool View::onSetAlpha(int alpha) {
    LOGD("");
    return false;
}

Animation* View::getAnimation() {
    return mCurrentAnimation;
}

void View::startAnimation(Animation* animation) {
    animation->setStartTime(Animation::START_ON_FIRST_FRAME);
    setAnimation(animation);
    //invalidateParentCaches();
    invalidate();
}

void View::clearAnimation() {
    if (mCurrentAnimation ) {
        mCurrentAnimation->detach();
    }
    mCurrentAnimation = nullptr;
    //invalidateParentIfNeeded();
    invalidate();
}

void View::setAnimation(Animation* animation) {
    mCurrentAnimation = animation;

    if (animation) {
        // If the screen is off assume the animation start time is now instead of
        // the next frame we draw. Keeping the START_ON_FIRST_FRAME start time
        // would cause the animation to start when the screen turns back on
        if (/*mAttachInfo != null && mAttachInfo.mDisplayState == Display.STATE_OFF
                &&*/ animation->getStartTime() == Animation::START_ON_FIRST_FRAME) {
            animation->setStartTime(SystemClock::uptimeMillis());//AnimationUtils.currentAnimationTimeMillis());
        }
        animation->reset();
    }
}
void View::setDefaultFocusHighlightEnabled(bool defaultFocusHighlightEnabled){
    mDefaultFocusHighlightEnabled = defaultFocusHighlightEnabled;
}

bool View::getDefaultFocusHighlightEnabled()const{
    return mDefaultFocusHighlightEnabled;
}

bool View::isDefaultFocusHighlightNeeded(const Drawable* background,const Drawable* foreground)const{
    bool lackFocusState = (background == nullptr || !background->isStateful()
            || !background->hasFocusStateSpecified())
            && (foreground == nullptr || !foreground->isStateful()
            || !foreground->hasFocusStateSpecified());
    return !isInTouchMode() && getDefaultFocusHighlightEnabled() 
	           && lackFocusState;//&& isAttachedToWindow() && sUseDefaultFocusHighlight;
}

void View::switchDefaultFocusHighlight() {
    if (isFocused()) {
        bool needed = isDefaultFocusHighlightNeeded(mBackground,
                mForegroundInfo == nullptr ? nullptr : mForegroundInfo->mDrawable);
        bool active = mDefaultFocusHighlight != nullptr;
        if (needed && !active) {
            setDefaultFocusHighlight(getDefaultFocusHighlightDrawable());
        } else if (!needed && active) {
            // The highlight is no longer needed, so tear it down.
            setDefaultFocusHighlight(nullptr);
        }
    }
}

void View::drawDefaultFocusHighlight(Canvas& canvas){
    if (mDefaultFocusHighlight != nullptr) {
        if (mDefaultFocusHighlightSizeChanged) {
            mDefaultFocusHighlightSizeChanged = false;
            mDefaultFocusHighlight->setBounds(mScrollX, mScrollY,mWidth,mHeight);
        }
        mDefaultFocusHighlight->draw(canvas);
    }
}

bool View::awakenScrollBars(){
    return mScrollCache != nullptr &&
           awakenScrollBars(mScrollCache->scrollBarDefaultDelayBeforeFade, true);
}
bool View::awakenScrollBars(int startDelay, bool invalidate){
    if (mScrollCache == nullptr || !mScrollCache->fadeScrollBars) {
        return false;
    }
    if (mScrollCache->scrollBar == nullptr) {
        mScrollCache->scrollBar = new ScrollBarDrawable();
        mScrollCache->scrollBar->setState(getDrawableState());
        mScrollCache->scrollBar->setCallback(this);
        return true;
    }
    if (isHorizontalScrollBarEnabled() || isVerticalScrollBarEnabled()) {

        if (invalidate) {
            // Invalidate to show the scrollbars
            postInvalidateOnAnimation();
        }

        /*if (scrollCache.state == ScrollabilityCache.OFF) {
            // FIXME: this is copied from WindowManagerService.
            // We should get this value from the system when it
            // is possible to do so.
            int KEY_REPEAT_FIRST_DELAY = 750;
            startDelay = std::max(KEY_REPEAT_FIRST_DELAY, startDelay);
        }

        // Tell mScrollCache when we should start fading. This may
        // extend the fade start time if one was already scheduled
        long fadeStartTime = SystemClock::uptimeMillis() + startDelay;
        scrollCache->fadeStartTime = fadeStartTime;
        scrollCache->state = ScrollabilityCache.ON;

        // Schedule our fader to run, unscheduling any old ones first
        if (mAttachInfo != null) {
            mAttachInfo.mHandler.removeCallbacks(scrollCache);
            mAttachInfo.mHandler.postAtTime(scrollCache, fadeStartTime);
        }*/

        return true;
    }
    return false;
}

void View::scrollTo(int x,int y){
    if( (mScrollX!=x)|| (mScrollY!=y) ){
        int oX=mScrollX;
        int oY=mScrollY;
        mScrollX=x;
        mScrollY=y;
        onScrollChanged(mScrollX, mScrollY, oX, oY);
        invalidate(nullptr);
        awakenScrollBars(0,true);
    }
}

void View::scrollBy(int dx,int dy){
    scrollTo(mScrollX+dx,mScrollY+dy);
}

int View::getScrollX()const{
    return mScrollX;
}

int View::getScrollY()const{
    return mScrollY;
}

void View::setScrollX(int x){
    scrollTo(x,mScrollY);    
}

void View::setScrollY(int y){
    scrollTo(mScrollX,y);
}

int View::getOverScrollMode()const{
    return mOverScrollMode;
}
void View::setOverScrollMode(int overScrollMode){
    bool rc=overScrollMode != OVER_SCROLL_ALWAYS &&
            overScrollMode != OVER_SCROLL_IF_CONTENT_SCROLLS &&
            overScrollMode != OVER_SCROLL_NEVER;
    LOGE_IF(rc,"Invalid overscroll mode %d" ,overScrollMode);
    mOverScrollMode = overScrollMode;
}

bool View::overScrollBy(int deltaX, int deltaY, int scrollX, int scrollY, int scrollRangeX,
           int scrollRangeY, int maxOverScrollX, int maxOverScrollY, bool isTouchEvent){
    int overScrollMode = mOverScrollMode;
    bool canScrollHorizontal= computeHorizontalScrollRange() > computeHorizontalScrollExtent();
    bool canScrollVertical =  computeVerticalScrollRange() > computeVerticalScrollExtent();
    bool overScrollHorizontal = overScrollMode == OVER_SCROLL_ALWAYS ||
                (overScrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && canScrollHorizontal);
    bool overScrollVertical = overScrollMode == OVER_SCROLL_ALWAYS ||
                (overScrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && canScrollVertical);

    int newScrollX = scrollX + deltaX;
    if (!overScrollHorizontal) maxOverScrollX = 0;

    int newScrollY = scrollY + deltaY;
    if (!overScrollVertical)   maxOverScrollY = 0;

        // Clamp values if at the limits and record
    int left = -maxOverScrollX;
    int right = maxOverScrollX + scrollRangeX;
    int top = -maxOverScrollY;
    int bottom = maxOverScrollY + scrollRangeY;

    bool clampedX = false;
    if (newScrollX > right) {
        newScrollX = right;
        clampedX = true;
    } else if (newScrollX < left) {
        newScrollX = left;
        clampedX = true;
    }

    bool clampedY = false;
    if (newScrollY > bottom) {
        newScrollY = bottom;
        clampedY = true;
    } else if (newScrollY < top) {
        newScrollY = top;
        clampedY = true;
    }

    onOverScrolled(newScrollX, newScrollY, clampedX, clampedY);
    return clampedX || clampedY;
}

void View::onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY){

}

int View::getVerticalFadingEdgeLength(){
    if (isVerticalFadingEdgeEnabled()) {
         ScrollabilityCache* cache = mScrollCache;
        if (cache != nullptr) {
            return cache->fadingEdgeLength;
        }
    }
    return 0;
}

int View::getHorizontalFadingEdgeLength(){
    if (isHorizontalFadingEdgeEnabled()) {
        ScrollabilityCache* cache = mScrollCache;
        if (cache != nullptr) {
            return cache->fadingEdgeLength;
        }
    }
    return 0;
}

void View::setFadingEdgeLength(int length){
    initScrollCache();
    mScrollCache->fadingEdgeLength = length;
}
void View::transformFromViewToWindowSpace(int*inOutLocation){
    View* view = this;
    double position[2]={(double)inOutLocation[0],(double)inOutLocation[1]};
    if(!hasIdentityMatrix()){
        mMatrix.transform_point(position[0],position[1]);
    }
    while (view) {
        position[0] -= view->mScrollX;
        position[1] -= view->mScrollY;
        if (!view->hasIdentityMatrix()) {
             view->mMatrix.transform_point(position[0],position[1]);
        }
        if(view->mParent){
            position[0] += view->mLeft;
            position[1] += view->mTop;
        }
        view = view->mParent;
    }
    inOutLocation[0]=(int)position[0];
    inOutLocation[1]=(int)position[1];
}

void View::getLocationInWindow(int* outLocation) {
    outLocation[0] = 0;
    outLocation[1] = 0;
    transformFromViewToWindowSpace(outLocation);
}

bool View::startNestedScroll(int axes){
    if (hasNestedScrollingParent()) {
        // Already in progress
        return true;
    }
    if (isNestedScrollingEnabled()) {
        ViewGroup* p = getParent();
        View* child = this;
         while (p != nullptr) {
             if (p->onStartNestedScroll(child, this, axes)) {
                  mNestedScrollingParent = p;
                  p->onNestedScrollAccepted(child, this, axes);
                  return true;
             }
             child = (View*) p;
             p = p->getParent();
        }
    }
    return false;
}
void View::stopNestedScroll() {
    if (mNestedScrollingParent != nullptr) {
        mNestedScrollingParent->onStopNestedScroll(this);
        mNestedScrollingParent = nullptr;
    }
}

bool View::hasNestedScrollingParent()const {
    return mNestedScrollingParent != nullptr;
}
void View::setNestedScrollingEnabled(bool benabled) {
    if (benabled) {
        mPrivateFlags3 |= PFLAG3_NESTED_SCROLLING_ENABLED;
    } else {
        stopNestedScroll();
        mPrivateFlags3 &= ~PFLAG3_NESTED_SCROLLING_ENABLED;
    }
}

bool View::isNestedScrollingEnabled()const{
    return (mPrivateFlags3 & PFLAG3_NESTED_SCROLLING_ENABLED) ==
                PFLAG3_NESTED_SCROLLING_ENABLED;
}

bool View::dispatchNestedScroll(int dxConsumed, int dyConsumed,
    int dxUnconsumed, int dyUnconsumed,int* offsetInWindow){
    if (isNestedScrollingEnabled() && mNestedScrollingParent) {
        if (dxConsumed != 0 || dyConsumed != 0 || dxUnconsumed != 0 || dyUnconsumed != 0) {
            int startX = 0;
            int startY = 0;
            if (offsetInWindow) {
                getLocationInWindow(offsetInWindow);
                startX = offsetInWindow[0];
                startY = offsetInWindow[1];
            }

            mNestedScrollingParent->onNestedScroll(this, dxConsumed, dyConsumed,
                    dxUnconsumed, dyUnconsumed);

            if (offsetInWindow) {
                getLocationInWindow(offsetInWindow);
                offsetInWindow[0] -= startX;
                offsetInWindow[1] -= startY;
            }
            return true;
        } else if (offsetInWindow) {
            // No motion, no dispatch. Keep offsetInWindow up to date.
            offsetInWindow[0] = 0;
            offsetInWindow[1] = 0;
        }
    }
    return false;
}

bool View::dispatchNestedPreScroll(int dx, int dy,int* consumed,int* offsetInWindow){
    if (!isNestedScrollingEnabled() || mNestedScrollingParent==nullptr) 
        return false;
    if (dx != 0 || dy != 0) {
        int startX = 0;
        int startY = 0;
        int mTempNestedScrollConsumed[2];
        if (offsetInWindow) {
            getLocationInWindow(offsetInWindow);
            startX = offsetInWindow[0];
            startY = offsetInWindow[1];
        }

        if (consumed == nullptr) {
            consumed = mTempNestedScrollConsumed;
        }
        consumed[0] = 0;
        consumed[1] = 0;
        mNestedScrollingParent->onNestedPreScroll(this, dx, dy, consumed);

        if (offsetInWindow) {
            getLocationInWindow(offsetInWindow);
            offsetInWindow[0] -= startX;
            offsetInWindow[1] -= startY;
        }
        return consumed[0] != 0 || consumed[1] != 0;
    } else if (offsetInWindow) {
        offsetInWindow[0] = 0;
        offsetInWindow[1] = 0;
    }
    return false;
}

bool View::dispatchNestedFling(float velocityX, float velocityY, bool consumed) {
    if (isNestedScrollingEnabled() && mNestedScrollingParent != nullptr) {
        return mNestedScrollingParent->onNestedFling(this, velocityX, velocityY, consumed);
    }
    return false;
}

bool View::dispatchNestedPreFling(float velocityX, float velocityY) {
    if (isNestedScrollingEnabled() && mNestedScrollingParent != nullptr) {
        return mNestedScrollingParent->onNestedPreFling(this, velocityX, velocityY);
    }
    return false;
}

void View::initScrollCache(){
    if(mScrollCache==nullptr)
        mScrollCache=new ScrollabilityCache(ViewConfiguration::get(mContext).getScaledScrollBarSize());
}

ScrollabilityCache*View::getScrollCache(){
    initScrollCache();
    return mScrollCache;
}

int View::getScrollBarSize()const{
    return mScrollCache == nullptr ? ViewConfiguration::get(mContext).getScaledScrollBarSize() :
                mScrollCache->scrollBarSize;
}

View& View::setScrollBarSize(int scrollBarSize){
    getScrollCache()->scrollBarSize = scrollBarSize;
    return *this;
}

bool View::isVerticalScrollBarHidden()const{
    return false;
}

void View::computeScroll(){
}
bool View::isHorizontalScrollBarEnabled()const{
    return (mViewFlags & SCROLLBARS_HORIZONTAL) == SCROLLBARS_HORIZONTAL;
}

View& View::setHorizontalScrollBarEnabled(bool horizontalScrollBarEnabled){
    getScrollCache();
    if (isHorizontalScrollBarEnabled() != horizontalScrollBarEnabled) {
        mViewFlags ^= SCROLLBARS_HORIZONTAL;
        //computeOpaqueFlags();
        resolvePadding();
        awakenScrollBars(0,false);
    }
    return *this;
}
float View::getTopFadingEdgeStrength(){
    return computeVerticalScrollOffset() > 0 ? 1.0f : 0.0f;
}
float View::getBottomFadingEdgeStrength(){
    return computeVerticalScrollOffset() + computeVerticalScrollExtent() <
             computeVerticalScrollRange() ? 1.0f : 0.0f;
}
float View::getLeftFadingEdgeStrength(){
    return computeHorizontalScrollOffset() > 0 ? 1.0f : 0.0f;
}
float View::getRightFadingEdgeStrength(){
    return computeHorizontalScrollOffset() + computeHorizontalScrollExtent() <
             computeHorizontalScrollRange() ? 1.0f : 0.0f;
}

bool View::isVerticalScrollBarEnabled()const{
    return (mViewFlags & SCROLLBARS_VERTICAL) == SCROLLBARS_VERTICAL;
}

View& View::setVerticalScrollBarEnabled(bool verticalScrollBarEnabled){
    getScrollCache();
    if (isVerticalScrollBarEnabled() != verticalScrollBarEnabled) {
        mViewFlags ^= SCROLLBARS_VERTICAL;
        //computeOpaqueFlags();
        resolvePadding();
        awakenScrollBars(0,false);
    }
    return *this;
}

void View::getVerticalScrollBarBounds(RECT*bounds,RECT*touchBounds){
    if (mRoundScrollbarRenderer == nullptr) {
        getStraightVerticalScrollBarBounds(bounds,touchBounds);
    } else {
        getRoundVerticalScrollBarBounds(bounds != nullptr ? bounds : touchBounds);
    }
}

void View::getHorizontalScrollBarBounds(RECT*drawBounds,RECT*touchBounds){
    RECT* bounds = drawBounds != nullptr ? drawBounds : touchBounds;
    if (bounds == nullptr)return;

    int inside =~0;// (mViewFlags & SCROLLBARS_OUTSIDE_MASK) == 0 ? ~0 : 0;
    bool drawVerticalScrollBar = isVerticalScrollBarEnabled()&& !isVerticalScrollBarHidden();
    int size = getHorizontalScrollbarHeight();
    int verticalScrollBarGap = drawVerticalScrollBar ? getVerticalScrollbarWidth() : 0;
    int width = getWidth();
    int height=getHeight();
    bounds->y = mScrollY + height - size - (mUserPaddingBottom & inside);
    bounds->x = mScrollX + (mPaddingLeft & inside);
    bounds->width =  width - (mUserPaddingRight & inside) - verticalScrollBarGap;
    bounds->height=  size;

    if (touchBounds == nullptr)return;
    if (*touchBounds != *bounds) {
        *touchBounds=*bounds;
    }
    int minTouchTarget = mScrollCache->scrollBarMinTouchTarget;
    if (touchBounds->height < minTouchTarget) {
        int adjust = (minTouchTarget - touchBounds->height) / 2;
        touchBounds->height = std::min(touchBounds->height + adjust,height);
        touchBounds->y = touchBounds->bottom() - minTouchTarget;
    }
    if (touchBounds->width < minTouchTarget) {
        int adjust = (minTouchTarget - touchBounds->width) / 2;
        touchBounds->x -= adjust;
        touchBounds->width =  minTouchTarget;
    }
    LOGD("scrollbar's drawbound=%d,%d-%d,%d",bounds->x,bounds->y,bounds->width,bounds->height);
}

bool View::isHorizontalFadingEdgeEnabled()const{
    return (mViewFlags & FADING_EDGE_HORIZONTAL) == FADING_EDGE_HORIZONTAL;
}
void View::setHorizontalFadingEdgeEnabled(bool horizontalFadingEdgeEnabled){
    if (isHorizontalFadingEdgeEnabled() != horizontalFadingEdgeEnabled) {
        if (horizontalFadingEdgeEnabled) {
            initScrollCache();
        }
        mViewFlags ^= FADING_EDGE_HORIZONTAL;
    }
}
bool View::isVerticalFadingEdgeEnabled()const{
    return (mViewFlags & FADING_EDGE_VERTICAL) == FADING_EDGE_VERTICAL;
}
void View::setVerticalFadingEdgeEnabled(bool verticalFadingEdgeEnabled){
    if (isVerticalFadingEdgeEnabled() != verticalFadingEdgeEnabled) {
        if (verticalFadingEdgeEnabled) {
            initScrollCache();
        }

        mViewFlags ^= FADING_EDGE_VERTICAL;
    }
}

void View::getStraightVerticalScrollBarBounds(RECT*drawBounds,RECT*touchBounds){
    RECT*bounds = drawBounds != nullptr ? drawBounds : touchBounds;
    if (bounds == nullptr) return;
    int inside =~0;// (mViewFlags & SCROLLBARS_OUTSIDE_MASK) == 0 ? ~0 : 0;
    int size = getVerticalScrollbarWidth();
    int verticalScrollbarPosition = mVerticalScrollbarPosition;
    if (verticalScrollbarPosition ==SCROLLBAR_POSITION_DEFAULT) {
        verticalScrollbarPosition = isLayoutRtl() ? SCROLLBAR_POSITION_LEFT : SCROLLBAR_POSITION_RIGHT;
    }
    switch (verticalScrollbarPosition) {
    default:
    case SCROLLBAR_POSITION_RIGHT:
        bounds->x = mScrollX + getWidth() - size - (mUserPaddingRight & inside);
        break;
    case SCROLLBAR_POSITION_LEFT:
        bounds->x = mScrollX + (mUserPaddingLeft & inside);
        break;
    }
    bounds->y = mScrollY+ (mPaddingTop & inside);
    bounds->width =  size;
    bounds->height= getHeight() - (mUserPaddingBottom & inside);

    if (touchBounds == nullptr) return;
    if (touchBounds != bounds) {
        *touchBounds=*bounds;
    }
    int minTouchTarget = mScrollCache->scrollBarMinTouchTarget;
    if (touchBounds->width < minTouchTarget) {
        int adjust = (minTouchTarget - touchBounds->width) / 2;
        if (verticalScrollbarPosition == SCROLLBAR_POSITION_RIGHT) {
            touchBounds->width= std::min(touchBounds->width + adjust, getWidth());
            touchBounds->x = touchBounds->width - minTouchTarget;
        } else {
            touchBounds->x = std::max(touchBounds->x+ adjust, mScrollX);
            touchBounds->width= touchBounds->x + minTouchTarget;
        }
    }
    if (touchBounds->height < minTouchTarget) {
        int adjust = (minTouchTarget - touchBounds->height) / 2;
        touchBounds->y -= adjust;
        touchBounds->height = touchBounds->y + minTouchTarget;
    }
}

void View::getRoundVerticalScrollBarBounds(RECT* bounds){
    // Do not take padding into account as we always want the scrollbars
    // to hug the screen for round wearable devices.
    *bounds=getBound();
    bounds->offset(mScrollX,mScrollY);
}

int View::getHorizontalScrollbarHeight()const{
    ScrollabilityCache* cache = mScrollCache;
    if (cache != nullptr) {
        ScrollBarDrawable* scrollBar = cache->scrollBar;
        if (scrollBar != nullptr) {
            int size = scrollBar->getSize(false);
            if (size <= 0) {
                size = cache->scrollBarSize;
            }
            return size;
        }
        return 0;
    }
    return 0;
}

int View::getVerticalScrollbarWidth()const{
    ScrollabilityCache* cache = mScrollCache;
    if (cache != nullptr) {
        ScrollBarDrawable* scrollBar = cache->scrollBar;
        if (scrollBar != nullptr) {
            int size = scrollBar->getSize(true);
            if (size <= 0) {
                size = cache->scrollBarSize;
            }
            return size;
        }
        return 0;
    }
    return 0;
}

int View::getVerticalScrollbarPosition()const{
    return mVerticalScrollbarPosition;
}

View& View::setVerticalScrollbarPosition(int position){
    if (mVerticalScrollbarPosition != position) {
        mVerticalScrollbarPosition = position;
        //computeOpaqueFlags();
        resolvePadding();
    }
    return *this;
}

int View::computeHorizontalScrollRange(){
    return getWidth();
}

int View::computeHorizontalScrollOffset(){
    return mScrollX;
}

int View::computeHorizontalScrollExtent(){
    return getWidth();
}

int View::computeVerticalScrollRange(){
    return getHeight();
}

int View::computeVerticalScrollOffset(){
    return mScrollY;
}

int View::computeVerticalScrollExtent(){
    return getHeight();
}

bool View::canScrollHorizontally(int direction){
    int offset = computeHorizontalScrollOffset();
    int range = computeHorizontalScrollRange() - computeHorizontalScrollExtent();
    if (range == 0) return false;
    if (direction < 0) {
        return offset > 0;
    } else {
        return offset < range - 1;
    }
}

bool View::canScrollVertically(int direction){
    int offset = computeVerticalScrollOffset();
    int range = computeVerticalScrollRange() - computeVerticalScrollExtent();
    if (range == 0) return false;
    if (direction < 0) {
        return offset > 0;
    } else {
        return offset < range - 1;
    }
}

bool View::isOnScrollbar(int x,int y){
    if (mScrollCache == nullptr) return false;
    x += getScrollX();
    y += getScrollY();
    if (isVerticalScrollBarEnabled() && !isVerticalScrollBarHidden()) {
        RECT& touchBounds = mScrollCache->mScrollBarTouchBounds;
        getVerticalScrollBarBounds(nullptr,&touchBounds);
        if (touchBounds.contains(x,y)) {
            return true;
        }
    }
    if (isHorizontalScrollBarEnabled()) {
        RECT& touchBounds = mScrollCache->mScrollBarTouchBounds;
        getHorizontalScrollBarBounds(nullptr, &touchBounds);
        if (touchBounds.contains( x, y)) {
            return true;
        }
    }
    return false;
}

bool View::isOnScrollbarThumb(int x,int y){
    return isOnVerticalScrollbarThumb(x, y) || isOnHorizontalScrollbarThumb(x, y);
}

bool View::isOnVerticalScrollbarThumb(int x,int y){
    if (mScrollCache == nullptr) return false;
    
    if (isVerticalScrollBarEnabled() && !isVerticalScrollBarHidden()) {
        x += getScrollX();
        y += getScrollY();
        RECT bounds = mScrollCache->mScrollBarBounds;
        RECT touchBounds = mScrollCache->mScrollBarTouchBounds;
        getVerticalScrollBarBounds(&bounds, &touchBounds);
        int range = computeVerticalScrollRange();
        int offset = computeVerticalScrollOffset();
        int extent = computeVerticalScrollExtent();
        int thumbLength = ViewConfiguration::getThumbLength(bounds.height, bounds.width,extent, range);
        int thumbOffset = ViewConfiguration::getThumbOffset(bounds.height, thumbLength, extent, range, offset);
        int thumbTop = bounds.y + thumbOffset;
        int adjust = std::max(mScrollCache->scrollBarMinTouchTarget - thumbLength, 0) / 2;
        if (x >= touchBounds.x && x <= touchBounds.right()
                && y >= thumbTop - adjust && y <= thumbTop + thumbLength + adjust) {
            return true;
        }
    }
    return false;
}

bool View::isOnHorizontalScrollbarThumb(int x,int y){
    if (mScrollCache == nullptr)return false;
    
    if (isHorizontalScrollBarEnabled()) {
        x += getScrollX();
        y += getScrollY();
        RECT bounds = mScrollCache->mScrollBarBounds;
        RECT touchBounds = mScrollCache->mScrollBarTouchBounds;
        getHorizontalScrollBarBounds(&bounds, &touchBounds);
        int range = computeHorizontalScrollRange();
        int offset = computeHorizontalScrollOffset();
        int extent = computeHorizontalScrollExtent();
        int thumbLength = ViewConfiguration::getThumbLength(bounds.width, bounds.height,extent, range);
        int thumbOffset = ViewConfiguration::getThumbOffset(bounds.width, thumbLength,extent, range, offset);
        int thumbLeft = bounds.x + thumbOffset;
        int adjust = std::max(mScrollCache->scrollBarMinTouchTarget - thumbLength, 0) / 2;
        if (x >= thumbLeft - adjust && x <= thumbLeft + thumbLength + adjust
                && y >= touchBounds.y && y <= touchBounds.bottom()) {
            return true;
        }
    }
    return false;
}

void View::initializeScrollIndicatorsInternal(){
    if (mScrollIndicatorDrawable == nullptr) {
        mScrollIndicatorDrawable = mContext?mContext->getDrawable("scroll_indicator_material.xml"):nullptr;
    }
    if( mScrollIndicatorDrawable == nullptr){
        Shape*sp=new RectShape();
        sp->setSolidColor(0x80FFFFFF);
        ShapeDrawable* sd=new ShapeDrawable();
        sd->setShape(sp);
        mScrollIndicatorDrawable =sd;
        sd->setIntrinsicWidth(2);
        sd->setIntrinsicHeight(2);
    }
}

int View::getScrollIndicators()const {
    return (mPrivateFlags3 & SCROLL_INDICATORS_PFLAG3_MASK)
            >> SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
}

void View::setScrollIndicators(int indicators,int mask) {
    // Shift and sanitize mask.
    mask <<= SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    mask &= SCROLL_INDICATORS_PFLAG3_MASK;

    // Shift and mask indicators.
    indicators <<= SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT;
    indicators &= mask;

    // Merge with non-masked flags.
    int updatedFlags = indicators | (mPrivateFlags3 & ~mask);

    if (mPrivateFlags3 != updatedFlags) {
        mPrivateFlags3 = updatedFlags;

        if (indicators != 0) {
            initializeScrollIndicatorsInternal();
        }
        invalidate(nullptr);
    }
}

void View::getScrollIndicatorBounds(RECT&out) {
    out.x     = mScrollX;
    out.width = getWidth();
    out.y     = mScrollY;
    out.height= getHeight();
}

void View::onDrawScrollIndicators(Canvas& canvas){
    if ((mPrivateFlags3 & SCROLL_INDICATORS_PFLAG3_MASK) == 0)
        return;// No scroll indicators enabled.

    Drawable* dr = mScrollIndicatorDrawable;
    if (dr == nullptr)
        return;// Scroll indicators aren't supported here.

    RECT rect ;
    int h = dr->getIntrinsicHeight();
    int w = dr->getIntrinsicWidth();

    getScrollIndicatorBounds(rect);
    if ((mPrivateFlags3 & PFLAG3_SCROLL_INDICATOR_TOP) != 0) {
        bool canScrollUp = canScrollVertically(-1);
        if (canScrollUp) {
            dr->setBounds(rect.x, rect.y, rect.width,h);
            dr->draw(canvas);
        }
    }

    if ((mPrivateFlags3 & PFLAG3_SCROLL_INDICATOR_BOTTOM) != 0) {
        bool canScrollDown = canScrollVertically(1);
        if (canScrollDown) {
            dr->setBounds(rect.x, rect.bottom() - h, rect.width, rect.height);
            dr->draw(canvas);
        }
    }

    int leftRtl;
    int rightRtl;
    if (getLayoutDirection() == LAYOUT_DIRECTION_RTL) {
        leftRtl = PFLAG3_SCROLL_INDICATOR_END;
        rightRtl = PFLAG3_SCROLL_INDICATOR_START;
    } else {
        leftRtl = PFLAG3_SCROLL_INDICATOR_START;
        rightRtl = PFLAG3_SCROLL_INDICATOR_END;
    }

    int leftMask = PFLAG3_SCROLL_INDICATOR_LEFT | leftRtl;
    if ((mPrivateFlags3 & leftMask) != 0) {
        bool canScrollLeft = canScrollHorizontally(-1);
        if (canScrollLeft) {
            dr->setBounds(rect.x, rect.y, w, rect.height);
            dr->draw(canvas);
        }
    }

    int rightMask = PFLAG3_SCROLL_INDICATOR_RIGHT | rightRtl;
    if ((mPrivateFlags3 & rightMask) != 0) {
        bool canScrollRight = canScrollHorizontally(1);
        if (canScrollRight) {
            dr->setBounds(rect.right() - w, rect.y,w, rect.height);
            dr->draw(canvas);
        }
    }
    rect=dr->getBounds();
}

void View::onDrawScrollBars(Canvas& canvas){
    bool drawHorizontalScrollBar = isHorizontalScrollBarEnabled();
    bool drawVerticalScrollBar = isVerticalScrollBarEnabled() && !isVerticalScrollBarHidden();
    
    if(mScrollCache == nullptr) return;

    // Fork out the scroll bar drawing for round wearable devices.
    if (mRoundScrollbarRenderer != nullptr) {
        if (drawVerticalScrollBar) {
            RECT bounds = mScrollCache->mScrollBarBounds;
            getVerticalScrollBarBounds(&bounds, nullptr);
            mRoundScrollbarRenderer->drawRoundScrollbars(
                canvas, (float)mScrollCache->scrollBar->getAlpha() / 255.f, bounds);
            //if (binvalidate) invalidate(nullptr);
        }
        // Do not draw horizontal scroll bars for round wearable devices.
    } else if ( drawVerticalScrollBar || drawHorizontalScrollBar) {
        RECT bounds;
        ScrollBarDrawable* scrollBar = mScrollCache->scrollBar;
        if (drawHorizontalScrollBar) {
            scrollBar->setParameters(computeHorizontalScrollRange(),
                    computeHorizontalScrollOffset(),
                    computeHorizontalScrollExtent(), false);
            getHorizontalScrollBarBounds(&bounds, nullptr);
            onDrawHorizontalScrollBar(canvas,scrollBar, bounds.x, bounds.y,bounds.width, bounds.height);
        }
        if (drawVerticalScrollBar) {
            scrollBar->setParameters(computeVerticalScrollRange(),
                    computeVerticalScrollOffset(),computeVerticalScrollExtent(), true);
            getVerticalScrollBarBounds(&bounds, nullptr);
            onDrawVerticalScrollBar(canvas, scrollBar, bounds.x, bounds.y,bounds.width, bounds.height);
        }
    }
}

void View::onDrawHorizontalScrollBar(Canvas& canvas, Drawable* scrollBar,int l, int t, int w, int h){
    scrollBar->setBounds(l, t, w, h);
    scrollBar->draw(canvas);
}

void View::onDrawVerticalScrollBar (Canvas& canvas , Drawable* scrollBar,int l, int t, int w, int h){
    scrollBar->setBounds(l, t, w, h);
    scrollBar->draw(canvas);
}

bool View::isInScrollingContainer()const{
    ViewGroup* p = getParent();
    while(p != nullptr) {
        if (((ViewGroup*) p)->shouldDelayChildPressedState()) {
            return true;
        }
        p = p->getParent();
    }
    return false;
}

void View::setOnClickListener(OnClickListener l){
    if(!isClickable())
        setClickable(true);
    mOnClick=l;
}

bool View::hasClickListener()const{
    return mOnClick!=nullptr;
}

void View::setOnLongClickListener(OnLongClickListener l){
    if(!isLongClickable())
        setLongClickable(true);
    mOnLongClick=l;
}

void View::setOnFocusChangeListener(OnFocusChangeListener listtener){
    mOnFocusChangeListener=listtener;
}

void View::addOnLayoutChangeListener(OnLayoutChangeListener listener){
    mOnLayoutChangeListeners.push_back(listener);
}

void View::removeOnLayoutChangeListener(OnLayoutChangeListener listener){
    //mOnLayoutChangeListeners.push_back(listener);
}

void View::setOnScrollChangeListener(OnScrollChangeListener l){
    mOnScrollChangeListener=l;
}

void View::clip(RefPtr<Region>rgn){
    if(mParent){//clip sliblings
        BOOL afterthis=FALSE;
        RECT rect=getBound();
        for(int i=0;i<mParent->getChildCount();i++){
            View*s=mParent->getChildAt(i);
            RECT rc,r=s->getBound();
            if( afterthis==FALSE ){
                afterthis=(s==this);
                continue;
            }
            if(s->hasFlag(VISIBLE)==false)continue;
            if(false==rc.intersect(rect,r))continue;
            rc.offset(-rect.x,-rect.y);
            rgn->subtract((const RectangleInt&)rc);
        }
        rect=getDrawingRect();
        getLocationInWindow((int*)&rect);
        View*c=mParent;
        do{
            RECT rc=c->getDrawingRect();
            c->getLocationInWindow((int*)&rc);
            rect.intersect(rc);
            c=c->mParent;
        }while(c);
        getRootView()->offsetRectIntoDescendantCoords(this,rect);
        rgn->intersect((const RectangleInt&)rect);
        LOGV("%p:%d scrolled:%d,%d aftercliped(%d,%d,%d,%d)at:%d,%d",this,mID,mScrollX,mScrollY,
            rect.x,rect.y,rect.width,rect.height,mLeft,mTop); 
    }
}

Canvas*View::getCanvas(){
    if(mParent){
        Canvas*parentCanvas=getRootView()->getCanvas();
        RECT rect=getClientRect();
        RefPtr<Region>rgn=getRootView()->mInvalidRgn->copy();
        
        getLocationInWindow((int*)&rect);
       
        rgn->intersect((const RectangleInt&)rect);
        rgn->translate(-rect.x,-rect.y);

        Canvas*c=parentCanvas->subContext(rect.x,rect.y,mWidth,mHeight);
        clip(rgn);//

        int num=rgn->get_num_rectangles();
        LOGV_IF(num==0,"%p:%d Location=%d,%d %d,%d num=%d",this,mID,rect.x,rect.y,rect.width,rect.height,num);
        for(int i=0;i<num;i++){
            RectangleInt r=rgn->get_rectangle(i);
            c->rectangle(r.x,r.y,r.width,r.height);
            LOGV("%p:%d clips(%d,%d %d,%d)",this,mID,r.x,r.y,r.width,r.height);
        }
        c->clip();
        return c;
    }
    return nullptr;
}

void View::dispatchDraw(Canvas&){
    //for inherited view(container) to draw children...
}

void View::drawBackground(Canvas&canvas){
    if(mBackground==nullptr)
        return ;
    mBackground->setBounds(0, 0,getWidth(),getHeight());
    if(mScrollX||mScrollY){
        canvas.translate(mScrollX,mScrollY);
        mBackground->draw(canvas);
        canvas.translate(-mScrollX,-mScrollY);
    }else{
        mBackground->draw(canvas);
    }
}

void View::onDrawForeground(Canvas& canvas){
    onDrawScrollIndicators(canvas);
    if(mScrollCache){
        onDrawScrollBars(canvas);
        return;
    }
    Drawable*foreground=mForegroundInfo != nullptr ? mForegroundInfo->mDrawable : nullptr;
    if(foreground){
        if (mForegroundInfo->mBoundsChanged) {
            mForegroundInfo->mBoundsChanged = false;
            RECT& selfBounds = mForegroundInfo->mSelfBounds;
            RECT& overlayBounds = mForegroundInfo->mOverlayBounds;

            if (mForegroundInfo->mInsidePadding) {
                selfBounds.set(0, 0, getWidth(), getHeight());
            } else {
                selfBounds.set(getPaddingLeft(), getPaddingTop(),
                    getWidth() - getPaddingRight(), getHeight() - getPaddingBottom());
            }

            int ld = getLayoutDirection();
            Gravity::apply(mForegroundInfo->mGravity, foreground->getIntrinsicWidth(),
                    foreground->getIntrinsicHeight(), selfBounds, overlayBounds, ld);
            foreground->setBounds(overlayBounds);
        }
        foreground->draw(canvas);    
    }
}

bool View::applyLegacyAnimation(ViewGroup* parent, long drawingTime, Animation* a, bool scalingRequired) {
    Transformation* invalidationTransform;
    int flags = parent->mGroupFlags;
    bool initialized = a->isInitialized();
    if (!initialized) {
        a->initialize(mWidth, mHeight, parent->getWidth(), parent->getHeight());
        a->initializeInvalidateRegion(0, 0, mWidth, mHeight);
        //if (mAttachInfo != null) a.setListenerHandler(mAttachInfo.mHandler);
        onAnimationStart();
    }

    Transformation* t = parent->getChildTransformation();
    bool more = a->getTransformation(drawingTime, *t, 1.f);
    if (scalingRequired /*&& mAttachInfo.mApplicationScale != 1.f*/) {
        if (parent->mInvalidationTransformation == nullptr) {
            parent->mInvalidationTransformation = new Transformation();
        }
        invalidationTransform = parent->mInvalidationTransformation;
        a->getTransformation(drawingTime, *invalidationTransform, 1.f);
        Matrix*m=invalidationTransform->getMatrix();
        LOGV("matrix=%f,%f,%f,%f,%f,%f",m->xx,m->yy,m->xy,m->yx,m->x0,m->y0);
    } else {
        invalidationTransform = t;
    }

    if (more) {
        if (!a->willChangeBounds()) {
            if ((flags & (ViewGroup::FLAG_OPTIMIZE_INVALIDATE | ViewGroup::FLAG_ANIMATION_DONE)) ==
                        ViewGroup::FLAG_OPTIMIZE_INVALIDATE) {
                parent->mGroupFlags |= ViewGroup::FLAG_INVALIDATE_REQUIRED;
            } else if ((flags & ViewGroup::FLAG_INVALIDATE_REQUIRED) == 0) {
                // The child need to draw an animation, potentially offscreen, so
                // make sure we do not cancel invalidate requests
                parent->mPrivateFlags |= PFLAG_DRAW_ANIMATION;
                parent->invalidate(mLeft, mTop, mWidth, mHeight);
            }
        } else {
            //if (parent->mInvalidateRegion == nullptr) {
            //    parent->mInvalidateRegion = new RectF();
            //}
            Rect region ;//= parent->mInvalidateRegion;
            a->getInvalidateRegion(0, 0, mWidth, mHeight, region,*invalidationTransform);

            // The child need to draw an animation, potentially offscreen, so
            // make sure we do not cancel invalidate requests
            parent->mPrivateFlags |= PFLAG_DRAW_ANIMATION;
            int left = mLeft + (int) region.x;
            int top = mTop + (int) region.y;
            parent->invalidate(left, top, region.width+1,region.height+1);
       }
    }
    return more;
}

void View::draw(Canvas*context){
    Transformation* transformToApply=nullptr;
    if(false==isShown()/*||0==(mPrivateFlags&PFLAG_DIRTY_MASK)*/)
        return;
    Canvas*canvas=context?context:getCanvas();
    if(canvas==nullptr)return;

    canvas->set_antialias(ANTIALIAS_GRAY);
    Animation* a = getAnimation(); 
    bool more=false;
    bool concatMatrix = false;
    if(a){
        more=applyLegacyAnimation(mParent, /*drawingTime*/SystemClock::uptimeMillis(), a, /*scalingRequired*/true);
        concatMatrix = a->willChangeTransformationMatrix(); 
        if (concatMatrix) mPrivateFlags3 |= PFLAG3_VIEW_IS_ANIMATING_TRANSFORM;
        transformToApply = mParent->getChildTransformation();
        Transformation at;
        a->getTransformation(SystemClock::uptimeMillis(),at);
        canvas->transform(*at.getMatrix());
    }else{
        mPrivateFlags3 &= ~PFLAG3_VIEW_IS_ANIMATING_TRANSFORM;
    }
    if(transformToApply|| (mPrivateFlags3 & PFLAG3_VIEW_IS_ANIMATING_ALPHA)){
        //TODO:    
    } 
    drawBackground(*canvas);
    if(mScrollX|mScrollY) canvas->translate(-mScrollX,-mScrollY);
    onDraw(*canvas);
    dispatchDraw(*canvas);//draw children
    onDrawForeground(*canvas);
    // Step 7, draw the default focus highlight
    drawDefaultFocusHighlight(*canvas);
    if(mScrollX|mScrollY) canvas->translate(mScrollX,mScrollY);


    mPrivateFlags&=~PFLAG_DIRTY_MASK;
    if(mParent&&context==nullptr)delete canvas;
    if(getRootView()==this){
        auto re=((ViewGroup*)this)->mInvalidRgn->get_extents();
        LOGV("clip2dirty(%d,%d %d,%d)",re.x,re.y,re.width,re.height);
        canvas->clip2dirty();
        getRootView()->mInvalidRgn->subtract(getRootView()->mInvalidRgn);
    }
    if(more)postInvalidate();
}

void View::onDraw(Canvas&canvas){
}

const RECT View::getBound()const{
    return RECT::Make(mLeft,mTop,mWidth,mHeight);
}

const RECT View::getDrawingRect()const{
    RECT ret;
    ret.set(mScrollX,mScrollY,mScrollX+getWidth(),mScrollY+getHeight());
    return ret;
}

void View::getFocusedRect(RECT&r){
    r.set(mLeft,mTop,mWidth,mHeight);
}

View& View::setId(int id){
    mID=id;
    return *this;
}

int View::getId() const{
    return mID;
}

View& View::setHint(const std::string&hint){
    mHint=hint;
    invalidate(nullptr);
    return *this;
}
const std::string&View::getHint()const{
    return mHint;
}

bool View::isTemporarilyDetached()const{
    return (mPrivateFlags3 & PFLAG3_TEMPORARY_DETACH) != 0;
}
void View::dispatchFinishTemporaryDetach(){
    mPrivateFlags3 &= ~PFLAG3_TEMPORARY_DETACH;
    onFinishTemporaryDetach();
    /*if (hasWindowFocus() && hasFocus()) {
        InputMethodManager.getInstance().focusIn(this);
    }
    notifyEnterOrExitForAutoFillIfNeeded(true);*/
}

void View::onFinishTemporaryDetach(){
    //NOTHING
}

void View::dispatchStartTemporaryDetach(){
    mPrivateFlags3 |= PFLAG3_TEMPORARY_DETACH;
    //notifyEnterOrExitForAutoFillIfNeeded(false);
    onStartTemporaryDetach();
}
void View::onStartTemporaryDetach() {
    removeUnsetPressCallback();
    mPrivateFlags |= PFLAG_CANCEL_NEXT_UP_EVENT;
}

bool View::hasTransientState(){
    return (mPrivateFlags2 & PFLAG2_HAS_TRANSIENT_STATE) == PFLAG2_HAS_TRANSIENT_STATE;
}

void View::setHasTransientState(bool hasTransientState){
    
}

void View::setIsRootNamespace(bool isRoot){
    if (isRoot) {
        mPrivateFlags |= PFLAG_IS_ROOT_NAMESPACE;
    } else {
        mPrivateFlags &= ~PFLAG_IS_ROOT_NAMESPACE;
    }
}

bool View::isRootNamespace()const {
    return (mPrivateFlags&PFLAG_IS_ROOT_NAMESPACE) != 0;
}

Context*View::getContext()const{
    return mContext;
}

int View::getWidth()const{
    return mWidth;
}

int View::getHeight()const{
    return mHeight;
}

int View::getX()const{
    return mLeft;
}

int View::getY()const{
    return mTop;
}

void View::offsetTopAndBottom(int offset){
    mTop+=offset;
}

void View::offsetLeftAndRight(int offset){
    mLeft+=offset;
}

void View::setTextDirection(int dir){
}

int View::getTextDirection()const{
    return (mPrivateFlags2 & PFLAG2_TEXT_DIRECTION_RESOLVED_MASK) >> PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT;
}

void View::setWillNotDraw(bool willNotDraw) {
    setFlags(willNotDraw ? WILL_NOT_DRAW : 0, DRAW_MASK);
}

bool View::canResolveTextAlignment()const {
    switch (getRawTextAlignment()) {
    case TEXT_DIRECTION_INHERIT:
         return mParent && mParent->canResolveTextAlignment();
    default: return true;
   }
}

void View::resetResolvedTextAlignment() {
    // Reset any previous text alignment resolution
    mPrivateFlags2 &= ~(PFLAG2_TEXT_ALIGNMENT_RESOLVED | PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK);
    // Set to default
    mPrivateFlags2 |= PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT;
}

bool View::isTextAlignmentInherited()const{
    return (getRawTextAlignment() == TEXT_ALIGNMENT_INHERIT);
}

bool View::isTextAlignmentResolved()const{
    return (mPrivateFlags2 & PFLAG2_TEXT_ALIGNMENT_RESOLVED) == PFLAG2_TEXT_ALIGNMENT_RESOLVED;
}

bool View::resolveTextAlignment() {
    // Reset any previous text alignment resolution
    mPrivateFlags2 &= ~(PFLAG2_TEXT_ALIGNMENT_RESOLVED | PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK);

    if (true/*hasRtlSupport()*/) {
        // Set resolved text alignment flag depending on text alignment flag
        int textAlignment = getRawTextAlignment();
        int parentResolvedTextAlignment;
        switch (textAlignment) {
        case TEXT_ALIGNMENT_INHERIT:
             // Check if we can resolve the text alignment
            if (!canResolveTextAlignment()) {
                // We cannot do the resolution if there is no parent so use the default
                mPrivateFlags2 |= PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT;
                // Resolution will need to happen again later
                return false;
            }

            // Parent has not yet resolved, so we still return the default
            if (!mParent->isTextAlignmentResolved()) {
                mPrivateFlags2 |= PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT;
                // Resolution will need to happen again later
                return false;
            }
            parentResolvedTextAlignment = mParent->getTextAlignment();
            switch (parentResolvedTextAlignment) {
            case TEXT_ALIGNMENT_GRAVITY:
            case TEXT_ALIGNMENT_TEXT_START:
            case TEXT_ALIGNMENT_TEXT_END:
            case TEXT_ALIGNMENT_CENTER:
            case TEXT_ALIGNMENT_VIEW_START:
            case TEXT_ALIGNMENT_VIEW_END:
                // Resolved text alignment is the same as the parent resolved
                // text alignment
                mPrivateFlags2 |=(parentResolvedTextAlignment << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT);
                break;
            default: // Use default resolved text alignment
                mPrivateFlags2 |= PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT;
            }
            break;
        case TEXT_ALIGNMENT_GRAVITY:
        case TEXT_ALIGNMENT_TEXT_START:
        case TEXT_ALIGNMENT_TEXT_END:
        case TEXT_ALIGNMENT_CENTER:
        case TEXT_ALIGNMENT_VIEW_START:
        case TEXT_ALIGNMENT_VIEW_END:
            // Resolved text alignment is the same as text alignment
            mPrivateFlags2 |= (textAlignment << PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT);
            break;
        default: // Use default resolved text alignment
            mPrivateFlags2 |= PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT;
        }
    } else {
        // Use default resolved text alignment
        mPrivateFlags2 |= PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT;
    }

    // Set the resolved
    mPrivateFlags2 |= PFLAG2_TEXT_ALIGNMENT_RESOLVED;
    return true;
}

int View::getRawTextAlignment()const{
    return (mPrivateFlags2 & PFLAG2_TEXT_ALIGNMENT_MASK) >> PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT;
}

void View::setTextAlignment(int textAlignment){
    if(textAlignment != getRawTextAlignment()){
        mPrivateFlags2 &= ~PFLAG2_TEXT_ALIGNMENT_MASK;
        mPrivateFlags2 |= ((textAlignment << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT) & PFLAG2_TEXT_ALIGNMENT_MASK);
        resolveTextAlignment();
        requestLayout();
    }
}

int View::getTextAlignment()const{
    return (mPrivateFlags2 & PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK) >> PFLAG2_TEXT_ALIGNMENT_RESOLVED_MASK_SHIFT;
}

bool View::willNotDraw()const{
    return (mViewFlags & DRAW_MASK) == WILL_NOT_DRAW;
}

View& View::setLayoutDirection(int layoutDirection){
    if (getRawLayoutDirection() != layoutDirection) {
        // Reset the current layout direction and the resolved one
        mPrivateFlags2 &= ~PFLAG2_LAYOUT_DIRECTION_MASK;
        resetRtlProperties();
        // Set the new layout direction (filtered)
        mPrivateFlags2 |=
            ((layoutDirection << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT) & PFLAG2_LAYOUT_DIRECTION_MASK);
        // We need to resolve all RTL properties as they all depend on layout direction
        resolveRtlPropertiesIfNeeded();
        requestLayout();
        invalidate(nullptr);
    }
    return *this;
}

int View::getLayoutDirection()const{
    return LAYOUT_DIRECTION_LTR;
}

bool View::isLayoutRtl()const{
    return getLayoutDirection() == LAYOUT_DIRECTION_RTL;
}

View& View::setBound(const RECT&b){
    if(mParent){
        RECT ro=getBound();
        mParent->invalidate(&ro);
        mParent->invalidate(&b);
    }
    if( (b.width!=mWidth) || (b.height!=mHeight) )
        onSizeChanged(b.width,b.height,mWidth,mHeight);
    mLeft=b.x;
    mTop=b.y;
    mWidth=b.width;
    mHeight=b.height;
    mPrivateFlags |= PFLAG_HAS_BOUNDS;
    invalidate(nullptr);
    return *this;
}

bool View::setFrame(int left,int top,int width,int height){
    bool changed = false;
    if (mLeft != left || mWidth != width || mTop != top || mHeight != height) {
        changed = true;

        // Remember our drawn bit
        int drawn = mPrivateFlags & PFLAG_DRAWN;

        int oldWidth = mWidth;
        int oldHeight = mHeight;
        int newWidth = width;
        int newHeight = height;
        bool sizeChanged = (newWidth != oldWidth) || (newHeight != oldHeight);

        // Invalidate our old position
        invalidate();

        mLeft = left;
        mTop = top;
        mWidth = width;
        mHeight = height;

        LOGV("%p:%d (%d,%d %d,%d)",this,mID,left,top,width,height);
        mPrivateFlags |= PFLAG_HAS_BOUNDS;

        if (sizeChanged) {
            onSizeChanged(newWidth, newHeight, oldWidth, oldHeight);
            //sizeChange(newWidth, newHeight, oldWidth, oldHeight);
        }

        if ((mViewFlags & VISIBILITY_MASK) == VISIBLE/*|| mGhostView != null*/) {
            // If we are visible, force the DRAWN bit to on so that
            // this invalidate will go through (at least to our parent).
            // This is because someone may have invalidated this view
            // before this call to setFrame came in, thereby clearing
            // the DRAWN bit.
            mPrivateFlags |= PFLAG_DRAWN;
            invalidate();//sizeChanged);
            // parent display list may need to be recreated based on a change in the bounds
            // of any child
            //invalidateParentCaches();
        }

        // Reset drawn bit to original value (invalidate turns it off)
        mPrivateFlags |= drawn;

        mBackgroundSizeChanged = true;
        mDefaultFocusHighlightSizeChanged = true;
        if (mForegroundInfo != nullptr) {
            mForegroundInfo->mBoundsChanged = true;
        }
        //notifySubtreeAccessibilityStateChangedIfNeeded();
    }
    return changed;
}

View& View::setPos(int x,int y){
    setFrame(x,y,mWidth,mHeight);
    return *this;
}

View& View::setSize(int w,int h){
    if( (mWidth!=w)|| (mHeight!=h) ){
        setFrame(mLeft,mTop,w,h);
        mWidth=w;
        mHeight=h;
        onSizeChanged(w,h,mWidth,mHeight);
    }
    return *this;
}

const RECT View::getClientRect()const{
    return RECT::Make(0,0,mWidth,mHeight);
}

void View::getHitRect(RECT& outRect){
    outRect.set(mLeft,mTop,mWidth,mHeight);
}

bool View::pointInView(int localX,int localY, int slop) {
    return localX >= -slop && localY >= -slop && localX < (getWidth() + slop) &&
            localY < (getHeight() + slop);
}

void View::onResolveDrawables(int layoutDirection){
}

void View::resolveDrawables(){
    if (!isLayoutDirectionResolved() &&
         getRawLayoutDirection() == View::LAYOUT_DIRECTION_INHERIT) {
         return;
    }

    int layoutDirection = isLayoutDirectionResolved() ?
            getLayoutDirection() : getRawLayoutDirection();

    if (mBackground)  mBackground->setLayoutDirection(layoutDirection);

    if (mForegroundInfo  && mForegroundInfo->mDrawable )
        mForegroundInfo->mDrawable->setLayoutDirection(layoutDirection);
    if (mDefaultFocusHighlight ) mDefaultFocusHighlight->setLayoutDirection(layoutDirection);
    
    mPrivateFlags2 |= PFLAG2_DRAWABLE_RESOLVED;
    onResolveDrawables(layoutDirection);
}

void View::resetResolvedDrawables(){
}

bool View::verifyDrawable(Drawable*who)const{
    return who == mBackground || (mForegroundInfo  && mForegroundInfo->mDrawable == who)|| (mDefaultFocusHighlight == who);
}

void View::jumpDrawablesToCurrentState(){
    if (mBackground) mBackground->jumpToCurrentState();

    //if (mStateListAnimator) mStateListAnimator->jumpToCurrentState();

    if (mDefaultFocusHighlight)mDefaultFocusHighlight->jumpToCurrentState();

    if (mForegroundInfo  && mForegroundInfo->mDrawable )mForegroundInfo->mDrawable->jumpToCurrentState();
}

std::vector<int>View::onCreateDrawableState()const{
    int viewStateIndex = 0;
    std::vector<int>states;
#if 0
    if(mPrivateFlags & PFLAG_PRESSED)states.push_back(StateSet::PRESSED);
    if(isEnabled())states.push_back(StateSet::ENABLED);
    if(mPrivateFlags & PFLAG_HOVERED)states.push_back(StateSet::HOVERED);
    if(mPrivateFlags & PFLAG_SELECTED)states.push_back(StateSet::SELECTED);
    if(isFocused())states.push_back(StateSet::FOCUSED);

#else
    if(mPrivateFlags & PFLAG_PRESSED)          viewStateIndex =StateSet::VIEW_STATE_PRESSED;
    if((mViewFlags & ENABLED_MASK) == ENABLED) viewStateIndex|=StateSet::VIEW_STATE_ENABLED;
    if(isFocused()) viewStateIndex |= StateSet::VIEW_STATE_FOCUSED;
    if(mPrivateFlags & PFLAG_SELECTED) viewStateIndex |= StateSet::VIEW_STATE_SELECTED;
    if ((mPrivateFlags & PFLAG_ACTIVATED) != 0) viewStateIndex |= StateSet::VIEW_STATE_ACTIVATED;
    //LOGV("**** %p:%d isFocused=%d enabled=%d pressed=%d",this,getId(),isFocused(),isEnabled(),isPressed());
    
    if(mPrivateFlags & PFLAG_HOVERED ) viewStateIndex |= StateSet::VIEW_STATE_HOVERED;

    states = StateSet::get(viewStateIndex);
#endif
    return states;
}

std::vector<int>& View::mergeDrawableStates(std::vector<int>&baseState,const std::vector<int>&additionalState) {
    const int N = baseState.size();
    const int M = additionalState.size();
    for(int j=0;j<M;j++)
        baseState.push_back(additionalState[j]);
    return baseState;
}

void View::drawableStateChanged(){
    if(mBackground&&mBackground->isStateful()){
        const std::vector<int>state=getDrawableState();
        mBackground->setState(state);
    }
    invalidate(nullptr);
}

void View::refreshDrawableState(){
    mPrivateFlags |= PFLAG_DRAWABLE_STATE_DIRTY;
    drawableStateChanged();
}

const std::vector<int>View::getDrawableState(){
    if(!(mPrivateFlags & PFLAG_DRAWABLE_STATE_DIRTY))
        return mDrawableState;
    mDrawableState=onCreateDrawableState();
    mPrivateFlags &= ~PFLAG_DRAWABLE_STATE_DIRTY;
    return mDrawableState;
}

Drawable*View::getBackground()const{
    return mBackground;
}

View& View::setBackgroundResource(const std::string&resid){
    Drawable*d=Drawable::inflate(mContext,resid);
    return setBackgroundDrawable(d);
}

View& View::setBackgroundDrawable(Drawable*background){
    if(mBackground!=nullptr){
        mBackground->setCallback(nullptr);
        delete mBackground;
    }
    mBackground = background;
    if(mBackground!=nullptr){
        RECT padding;
        if(background->getPadding(padding)){
            setPadding(padding.x,padding.y,padding.width,padding.height);
        }
        if(background->isStateful())
            background->setState(getDrawableState());
        background->setCallback(this);
    }
    return *this;
}

void View::applyBackgroundTint() {
    if (mBackground != nullptr && mBackgroundTint != nullptr) {
        const TintInfo*tintInfo = mBackgroundTint;
        if (tintInfo->mHasTintList || tintInfo->mHasTintMode) {
            mBackground = mBackground->mutate();

            if (tintInfo->mHasTintList)mBackground->setTintList(tintInfo->mTintList);
            if (tintInfo->mHasTintMode)mBackground->setTintMode(tintInfo->mTintMode);

            // The drawable (or one of its children) may not have been
            // stateful before applying the tint, so let's try again.
            if (mBackground->isStateful()) {
                mBackground->setState(getDrawableState());
            }
        }
    }
}

View& View::setBackgroundColor(int color){
    if(dynamic_cast<ColorDrawable*>(mBackground)){
        ((ColorDrawable*)mBackground)->setColor(color);
    }else
        setBackgroundDrawable(new ColorDrawable(color));
    return *this;
}

View& View::setBackgroundTintList(ColorStateList* tint){
    if (mBackgroundTint == nullptr) {
        mBackgroundTint = new TintInfo();
    }
    mBackgroundTint->mTintList = tint;
    mBackgroundTint->mHasTintList = true;

    applyBackgroundTint();
    return *this;
}

ColorStateList* View::getBackgroundTintList()const{
    return mBackgroundTint != nullptr ? mBackgroundTint->mTintList : nullptr;
}

void View::onFocusChanged(bool gainFocus,int direct,const RECT*previouslyFocusedRect){
    if(mOnFocusChangeListener)mOnFocusChangeListener(*this,gainFocus);
    switchDefaultFocusHighlight();
    if(!gainFocus){
        if(isPressed())setPressed(false);
        onFocusLost();
    }
    refreshDrawableState();
}

Drawable* View::getForeground()const{
    return mForegroundInfo != nullptr ? mForegroundInfo->mDrawable : nullptr;
}

View& View::setForeground(Drawable* foreground){
    if (mForegroundInfo == nullptr) {
        if (foreground == nullptr) {
            // Nothing to do.
            return *this;
        }
        mForegroundInfo = new ForegroundInfo();
    }

    if (foreground == mForegroundInfo->mDrawable) {
        // Nothing to do
        return *this;
    }

    if (mForegroundInfo->mDrawable != nullptr) {
        if (true/*isAttachedToWindow()*/) {
            mForegroundInfo->mDrawable->setVisible(false, false);
        }
        mForegroundInfo->mDrawable->setCallback(nullptr);
        unscheduleDrawable(*mForegroundInfo->mDrawable);
    }

    mForegroundInfo->mDrawable = foreground;
    mForegroundInfo->mBoundsChanged = true;
    if (foreground != nullptr) {
        if ((mPrivateFlags & PFLAG_SKIP_DRAW) != 0) {
            mPrivateFlags &= ~PFLAG_SKIP_DRAW;
        }
        foreground->setLayoutDirection(getLayoutDirection());
        if (foreground->isStateful()) {
            foreground->setState(getDrawableState());
        }
        applyForegroundTint();
        if (true/*isAttachedToWindow()*/) {
            foreground->setVisible(isShown(), false);
        }
            // Set callback last, since the view may still be initializing.
        foreground->setCallback(this);
    } else if ((mViewFlags & WILL_NOT_DRAW) != 0 && mBackground == nullptr && (mDefaultFocusHighlight == nullptr)) {
        mPrivateFlags |= PFLAG_SKIP_DRAW;
    }
    requestLayout();
    invalidate(nullptr);
    return *this;
}

bool View::isForegroundInsidePadding()const{
    return mForegroundInfo != nullptr ? mForegroundInfo->mInsidePadding : true;
}

int View::getForegroundGravity()const{
    return mForegroundInfo != nullptr ? mForegroundInfo->mGravity : Gravity::START | Gravity::TOP;
}

View& View::setForegroundGravity(int gravity){
    if (mForegroundInfo == nullptr) {
        mForegroundInfo = new ForegroundInfo();
    }

    if (mForegroundInfo->mGravity != gravity) {
        if ((gravity & Gravity::RELATIVE_HORIZONTAL_GRAVITY_MASK) == 0) {
            gravity |= Gravity::START;
        }

        if ((gravity & Gravity::VERTICAL_GRAVITY_MASK) == 0) {
            gravity |= Gravity::TOP;
        }

        mForegroundInfo->mGravity = gravity;
        requestLayout();
    }
    return *this;
}

View& View::setForegroundTintList(ColorStateList* tint){
    if (mForegroundInfo == nullptr) {
        mForegroundInfo = new ForegroundInfo();
    }
    if (mForegroundInfo->mTintInfo == nullptr) {
        mForegroundInfo->mTintInfo = new TintInfo();
    }
    mForegroundInfo->mTintInfo->mTintList = tint;
    mForegroundInfo->mTintInfo->mHasTintList = true;

    applyForegroundTint();
    return *this;
}

View& View::setForegroundTintMode(int tintMode){
    if (mForegroundInfo == nullptr) {
        mForegroundInfo = new ForegroundInfo();
    }
    if (mForegroundInfo->mTintInfo == nullptr) {
        mForegroundInfo->mTintInfo = new TintInfo();
    }
    mForegroundInfo->mTintInfo->mTintMode = tintMode;
    mForegroundInfo->mTintInfo->mHasTintMode = true;

    applyForegroundTint();
    return *this;
}

ColorStateList* View::getForegroundTintList(){
    return mForegroundInfo != nullptr && mForegroundInfo->mTintInfo != nullptr
                ? mForegroundInfo->mTintInfo->mTintList : nullptr;
}

void View::applyForegroundTint() {
    if (mForegroundInfo != nullptr && mForegroundInfo->mDrawable != nullptr
            && mForegroundInfo->mTintInfo != nullptr) {
        TintInfo* tintInfo = mForegroundInfo->mTintInfo;
        if (tintInfo->mHasTintList || tintInfo->mHasTintMode) {
            mForegroundInfo->mDrawable = mForegroundInfo->mDrawable->mutate();

            if (tintInfo->mHasTintList) {
                mForegroundInfo->mDrawable->setTintList(tintInfo->mTintList);
            }

            if (tintInfo->mHasTintMode) {
                mForegroundInfo->mDrawable->setTintMode(tintInfo->mTintMode);
            }

            // The drawable (or one of its children) may not have been
            // stateful before applying the tint, so let's try again.
            if (mForegroundInfo->mDrawable->isStateful()) {
                mForegroundInfo->mDrawable->setState(getDrawableState());
            }
        }
    }
}

/** Create a default focus highlight if it doesn't exist.
 * @return a default focus highlight.*/
Drawable* View::getDefaultFocusHighlightDrawable() {
    if (mDefaultFocusHighlightCache == nullptr) {
        if (mContext) {
             //int[] attrs = new int[] { android.R.attr.selectableItemBackground };
             mDefaultFocusHighlightCache=new ColorDrawable(0x80FF0000);//todo loadresource;
        }
    }
    return mDefaultFocusHighlightCache;
}

void View::setDefaultFocusHighlight(Drawable* highlight){
    mDefaultFocusHighlight = highlight;
    mDefaultFocusHighlightSizeChanged = true;
    if (highlight != nullptr) {
        if ((mPrivateFlags & PFLAG_SKIP_DRAW) != 0) {
            mPrivateFlags &= ~PFLAG_SKIP_DRAW;
        }
        highlight->setLayoutDirection(getLayoutDirection());
        if (highlight->isStateful()) {
            highlight->setState(getDrawableState());
        }
        /*if (isAttachedToWindow())*/highlight->setVisible(isShown(), false);
        // Set callback last, since the view may still be initializing.
        highlight->setCallback(this);
    } else if ((mViewFlags & WILL_NOT_DRAW) != 0 && mBackground == nullptr
            && (mForegroundInfo == nullptr || mForegroundInfo->mDrawable == nullptr)) {
        mPrivateFlags |= PFLAG_SKIP_DRAW;
    }
    invalidate();
}

bool View::hasSize()const {
    return mWidth>0 && mHeight>0;
}

bool View::canTakeFocus()const{
    return ((mViewFlags & VISIBILITY_MASK) == VISIBLE)
            && ((mViewFlags & FOCUSABLE) == FOCUSABLE)
            && ((mViewFlags & ENABLED_MASK) == ENABLED)
            && (/*sCanFocusZeroSized ||*/ !isLayoutValid() || hasSize());
}

View& View::setFlags(int flags,int mask) {
    int old = mViewFlags;
    mViewFlags = (mViewFlags & ~mask) | (flags & mask);
    int changed = mViewFlags ^ old;
    if(changed==0)return *this;

    int privateFlags = mPrivateFlags;
    bool shouldNotifyFocusableAvailable = false;

    // If focusable is auto, update the FOCUSABLE bit.
    int focusableChangedByAuto = 0;
    if (((mViewFlags & FOCUSABLE_AUTO) != 0)
            && (changed & (FOCUSABLE_MASK | CLICKABLE)) != 0) {
        // Heuristic only takes into account whether view is clickable.
        int newFocus=(mViewFlags & CLICKABLE)?FOCUSABLE:NOT_FOCUSABLE;
        mViewFlags = (mViewFlags & ~FOCUSABLE) | newFocus;
        focusableChangedByAuto = (old & FOCUSABLE) ^ (newFocus & FOCUSABLE);
        changed = (changed & ~FOCUSABLE) | focusableChangedByAuto;
    }

    /* Check if the FOCUSABLE bit has changed */
    if (((changed & FOCUSABLE) != 0) && ((privateFlags & PFLAG_HAS_BOUNDS) != 0)) {
        if (((old & FOCUSABLE) == FOCUSABLE) && ((privateFlags & PFLAG_FOCUSED) != 0)) {
            /* Give up focus if we are no longer focusable */
            clearFocus();
            mParent->clearFocusedInCluster();
        } else if (((old & FOCUSABLE) == NOT_FOCUSABLE)
                && ((privateFlags & PFLAG_FOCUSED) == 0)) {
            /* Tell the view system that we are now available to take focus
             * if no one else already has it.*/
            /*if (mParent != null) {
                ViewRootImpl viewRootImpl = getViewRootImpl();
                if (!sAutoFocusableOffUIThreadWontNotifyParents
                        || focusableChangedByAuto == 0
                        || viewRootImpl == null
                        || viewRootImpl.mThread == Thread.currentThread()) {
                    shouldNotifyFocusableAvailable = canTakeFocus();
                }
            }*/
        }
    }

    const int newVisibility = flags & VISIBILITY_MASK;
    if (newVisibility == VISIBLE) {
        if ((changed & VISIBILITY_MASK) != 0) {
            /* If this view is becoming visible, invalidate it in case it changed while
             * it was not visible. Marking it drawn ensures that the invalidation will
             * go through.*/
            mPrivateFlags |= PFLAG_DRAWN;
            invalidate();

            //needGlobalAttributesUpdate(true);

            // a view becoming visible is worth notifying the parent about in case nothing has
            // focus. Even if this specific view isn't focusable, it may contain something that
            // is, so let the root view try to give this focus if nothing else does.
            shouldNotifyFocusableAvailable = hasSize();
        }
    }

    if ((changed & ENABLED_MASK) != 0) {
        if ((mViewFlags & ENABLED_MASK) == ENABLED) {
            // a view becoming enabled should notify the parent as long as the view is also
            // visible and the parent wasn't already notified by becoming visible during this
            // setFlags invocation.
            shouldNotifyFocusableAvailable = canTakeFocus();
        } else {
            if (isFocused()) clearFocus();
        }
    }

    if (shouldNotifyFocusableAvailable && mParent ) {
        mParent->focusableViewAvailable(this);
    }

    /* Check if the GONE bit has changed */
    if ((changed & GONE) != 0) {
        //needGlobalAttributesUpdate(false);
        requestLayout();

        if (((mViewFlags & VISIBILITY_MASK) == GONE)) {
            if (hasFocus()) {
                clearFocus();
                mParent->clearFocusedInCluster();
            }
            //clearAccessibilityFocus();
            //destroyDrawingCache();
            // GONE views noop invalidation, so invalidate the parent
            if(mParent)mParent->invalidate();
            // Mark the view drawn to ensure that it gets invalidated properly the next
            // time it is visible and gets invalidated
            mPrivateFlags |= PFLAG_DRAWN;
        }
        //if (mAttachInfo != null) mAttachInfo.mViewVisibilityChanged = true;
    }

    /* Check if the VISIBLE bit has changed */
    if ((changed & INVISIBLE) != 0) {
        //needGlobalAttributesUpdate(false);
        /* If this view is becoming invisible, set the DRAWN flag so that
         * the next invalidate() will not be skipped.*/
        mPrivateFlags |= PFLAG_DRAWN;

        if (((mViewFlags & VISIBILITY_MASK) == INVISIBLE)) {
            // root view becoming invisible shouldn't clear focus and accessibility focus
            if (getRootView() != this) {
                if (hasFocus()) {
                    clearFocus();
                    mParent->clearFocusedInCluster();
                }
                //clearAccessibilityFocus();
            }
        }
        //if (mAttachInfo != null)mAttachInfo.mViewVisibilityChanged = true;
    }

    if ((changed & VISIBILITY_MASK) != 0) {
        // If the view is invisible, cleanup its display list to free up resources
        if (newVisibility != VISIBLE /*&& mAttachInfo != null*/) {
            //cleanupDraw();
        }

        if (mParent) {
            mParent->onChildVisibilityChanged(this,(changed & VISIBILITY_MASK), newVisibility);
            mParent->invalidate();
        }

        if (true/*mAttachInfo != null*/) {
            dispatchVisibilityChanged(*this, newVisibility);

            // Aggregated visibility changes are dispatched to attached views
            // in visible windows where the parent is currently shown/drawn
            // or the parent is not a ViewGroup (and therefore assumed to be a ViewRoot),
            // discounting clipping or overlapping. This makes it a good place
            // to change animation states.
            if (mParent  && mParent->isShown()) {
                //dispatchVisibilityAggregated(newVisibility == VISIBLE);
            }
            //notifySubtreeAccessibilityStateChangedIfNeeded();
        }
    }

    /*if ((changed & WILL_NOT_CACHE_DRAWING) != 0) destroyDrawingCache();

    if ((changed & DRAWING_CACHE_ENABLED) != 0) {
        destroyDrawingCache();
        mPrivateFlags &= ~PFLAG_DRAWING_CACHE_VALID;
        invalidateParentCaches();
    }

    if ((changed & DRAWING_CACHE_QUALITY_MASK) != 0) {
        destroyDrawingCache();
        mPrivateFlags &= ~PFLAG_DRAWING_CACHE_VALID;
    }*/

    if ((changed & DRAW_MASK) != 0) {
        if ((mViewFlags & WILL_NOT_DRAW) != 0) {
            if (mBackground != nullptr || mDefaultFocusHighlight != nullptr
                    || (mForegroundInfo != nullptr && mForegroundInfo->mDrawable != nullptr)) {
                mPrivateFlags &= ~PFLAG_SKIP_DRAW;
            } else {
                mPrivateFlags |= PFLAG_SKIP_DRAW;
            }
        } else {
            mPrivateFlags &= ~PFLAG_SKIP_DRAW;
        }
        requestLayout();
        invalidate();
    }
#if 0
    if ((changed & KEEP_SCREEN_ON) != 0) {
        if (mParent != null && mAttachInfo != null && !mAttachInfo.mRecomputeGlobalAttributes) {
            mParent.recomputeViewAttributes(this);
        }
    }

    if (accessibilityEnabled) {
        // If we're an accessibility pane and the visibility changed, we already have sent
        // a state change, so we really don't need to report other changes.
        if (isAccessibilityPane()) {
            changed &= ~VISIBILITY_MASK;
        }
        if ((changed & FOCUSABLE) != 0 || (changed & VISIBILITY_MASK) != 0
                || (changed & CLICKABLE) != 0 || (changed & LONG_CLICKABLE) != 0
                || (changed & CONTEXT_CLICKABLE) != 0) {
            if (oldIncludeForAccessibility != includeForAccessibility()) {
                //notifySubtreeAccessibilityStateChangedIfNeeded();
            } else {
                //notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent.CONTENT_CHANGE_TYPE_UNDEFINED);
            }
        } else if ((changed & ENABLED_MASK) != 0) {
            //notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent.CONTENT_CHANGE_TYPE_UNDEFINED);
        }
    }
#endif
    invalidate(nullptr);
    return *this;
}

View& View::clearFlag(int flag) {
    mViewFlags&=(~flag);
    invalidate(nullptr);
    return *this;
}

bool View::hasFlag(int flag) const {
    return (mViewFlags&flag)==flag;
}

void View::onAttached(){
    onSizeChanged(mWidth,mHeight,-1,-1);
    refreshDrawableState();
    invalidate(nullptr);
}

void View::onDettached(){
    InputMethodManager::getInstance().onViewDetachedFromWindow((View*)this); 
}

void View::setDuplicateParentStateEnabled(bool enabled){
    setFlags(enabled ? DUPLICATE_PARENT_STATE : 0, DUPLICATE_PARENT_STATE);
}

bool View::isDuplicateParentStateEnabled()const{
    return (mViewFlags & DUPLICATE_PARENT_STATE) == DUPLICATE_PARENT_STATE;
}

bool View::isFocused()const {
    return (mPrivateFlags & PFLAG_FOCUSED) != 0;
}

bool View::isFocusedByDefault()const{
    return (mPrivateFlags3 & PFLAG3_FOCUSED_BY_DEFAULT) != 0;
}

bool View::isAccessibilityFocused()const{
    return (mPrivateFlags2 & PFLAG2_ACCESSIBILITY_FOCUSED) != 0;
}

bool View::requestAccessibilityFocus(){
    if ((mViewFlags & VISIBILITY_MASK) != VISIBLE) {
        return false;
    }
    if ((mPrivateFlags2 & PFLAG2_ACCESSIBILITY_FOCUSED) == 0) {
        mPrivateFlags2 |= PFLAG2_ACCESSIBILITY_FOCUSED;
        ViewGroup* viewRootImpl = getRootView();
        //if (viewRootImpl) viewRootImpl->setAccessibilityFocus(this, nullptr);
        invalidate();
        //sendAccessibilityEvent(AccessibilityEvent.TYPE_VIEW_ACCESSIBILITY_FOCUSED);
        return true;
    }
    return false;
}

bool View::isAccessibilityFocusedViewOrHost() {
    ViewGroup* viewRootImpl = getRootView();
    return isAccessibilityFocused();// || (viewRootImpl && viewRootImpl->getAccessibilityFocusedHost() == this);
}

bool View::hasDefaultFocus()const{
    return isFocusedByDefault();
}

void View::setFocusedByDefault(bool isFocusedByDefault){
    if (isFocusedByDefault == ((mPrivateFlags3 & PFLAG3_FOCUSED_BY_DEFAULT) != 0)) {
        return;
    }

    if (isFocusedByDefault) 
        mPrivateFlags3 |= PFLAG3_FOCUSED_BY_DEFAULT;
    else
        mPrivateFlags3 &= ~PFLAG3_FOCUSED_BY_DEFAULT;

    if (mParent) {
        if (isFocusedByDefault)
            mParent->setDefaultFocus(this);
        else 
            mParent->clearDefaultFocus(this);
    }
}

View& View::setVisibility(int visibility) {
    setFlags(visibility, VISIBILITY_MASK);
    return *this;
}

int View::getVisibility() const {
   return mViewFlags&VISIBILITY_MASK;
}

bool View::isShown()const{
    const View* current = this;
    //noinspection ConstantConditions
    do {
        if ((current->mViewFlags & VISIBILITY_MASK) != VISIBLE) {
            return false;
        }
        ViewGroup* parent = current->mParent;
        if (parent == nullptr) {
            return (current->mViewFlags&VISIBILITY_MASK)==VISIBLE;
        }
        if (dynamic_cast<View*>(parent)==nullptr) {
            return true;
        }
        current = (const View*) parent;
    } while (current != nullptr);
    return true;
}

View& View::setEnabled(bool enable) {
    setFlags(enable?ENABLED:DISABLED,ENABLED_MASK);
    refreshDrawableState();
    return *this;
}

bool View::isEnabled() const {
    return (mViewFlags&ENABLED_MASK)==ENABLED;
}

void View::setActivated(bool activated){
     if (((mPrivateFlags & PFLAG_ACTIVATED) != 0) != activated) {
        mPrivateFlags = (mPrivateFlags & ~PFLAG_ACTIVATED) | (activated ? PFLAG_ACTIVATED : 0);
        invalidate(nullptr);
        refreshDrawableState();
        dispatchSetActivated(activated);
     }
}

void View::dispatchSetActivated(bool activated){
}

bool View::isActivated()const{
    return (mPrivateFlags & PFLAG_ACTIVATED) != 0;
}

void View::setSelected(bool selected){
    if (((mPrivateFlags & PFLAG_SELECTED) != 0) != selected){
        mPrivateFlags = (mPrivateFlags & ~PFLAG_SELECTED) | (selected ? PFLAG_SELECTED : 0);
        if (!selected) resetPressedState();
        refreshDrawableState();
        dispatchSetSelected(selected);
    }
}

bool View::isSelected()const{
    return (mPrivateFlags & PFLAG_SELECTED) != 0;
}

bool View::isClickable()const{
    return (mViewFlags & CLICKABLE) == CLICKABLE;
}

void View::setClickable(bool clickable){
    setFlags(clickable ? CLICKABLE : 0, CLICKABLE);
}

bool View::isLongClickable()const{
    return (mViewFlags & LONG_CLICKABLE) == LONG_CLICKABLE;
}

void View::setLongClickable(bool longClickable){
    setFlags(longClickable ? LONG_CLICKABLE : 0, LONG_CLICKABLE);
}

bool View::isContextClickable()const{
    return (mViewFlags & CONTEXT_CLICKABLE) == CONTEXT_CLICKABLE;
}

void View::setContextClickable(bool contextClickable) {
    setFlags(contextClickable ? CONTEXT_CLICKABLE : 0, CONTEXT_CLICKABLE);
}

void View::dispatchSetSelected(bool selected){
}

void View::onFocusLost() {
    resetPressedState();
}

void View::resetPressedState(){
    if ((mViewFlags & ENABLED_MASK) == DISABLED)return;
    if (isPressed()) {
        setPressed(false);
        if (!mHasPerformedLongPress) removeLongPressCallback();
    }
}

void View::setPressed(bool pressed){
    const bool needsRefresh = pressed != ((mPrivateFlags & PFLAG_PRESSED) == PFLAG_PRESSED);
    if (pressed) 
        mPrivateFlags |= PFLAG_PRESSED;
    else 
        mPrivateFlags &= ~PFLAG_PRESSED;
    if (needsRefresh) refreshDrawableState();
    dispatchSetPressed(pressed);
}

void View::setPressed(bool pressed,int x,int y){
    //if(pressed)drawableHotspotChanged(x,y);
    setPressed(pressed);
}

bool View::isPressed()const{
    return (mPrivateFlags & PFLAG_PRESSED) == PFLAG_PRESSED;
}

void View::dispatchSetPressed(bool pressed){
}

void View::dispatchVisibilityChanged(View& changedView,int visibility){
    onVisibilityChanged(changedView, visibility); 
}

void View::onVisibilityChanged(View& changedView,int visibility){
}

bool View::isDirty()const{
    return (mPrivateFlags&PFLAG_DIRTY_MASK)!=0;
}
/*param:rect is views logical area,maybe it is large than views'bounds,function invalidate must convert it to bound area*/
void View::invalidate(const RECT*rect){
#if 1
    RECT rcInvalid=rect?*rect:getDrawingRect();
    ViewGroup*rv=getRootView();
    if(rv){
        rv->mPrivateFlags|=PFLAG_DIRTY;
        rv->offsetDescendantRectToMyCoords(this,rcInvalid);
        rv->mInvalidRgn->do_union((const RectangleInt&)rcInvalid);
    }
    mPrivateFlags|=PFLAG_DIRTY;
#else
    RECT rcInvalid=rect?*rect:getClientRect();

    if(rect){
	RECT rectw=*rect;
        View*c=this;
        getLocationInWindow((int*)&rectw);
        rectw.offset(-mScrollX,-mScrollY);//convert logical coord to visual coord
        do{
            RECT rc=c->getClientRect();
            c->getLocationInWindow((int*)&rc);
            rectw.intersect(rc);
            c=c->mParent;
        }while(c);
        getRootView()->offsetRectIntoDescendantCoords(this,rectw);
        rcInvalid.intersect(rectw);
    }
    
    if(rcInvalid.empty())return;

    mPrivateFlags|=PFLAG_DIRTY;
    //LOGV("%p.%p:%d (%d,%d %d,%d)",mParent,this,mID,rcInvalid.x,rcInvalid.y,rcInvalid.width,rcInvalid.height);
    if(mParent)mParent->invalidateChild(this,&rcInvalid);
#endif
}

void View::invalidate(int l,int t,int w,int h){
    Rect rect;
    rect.set(l,t,w,h);
    invalidate(&rect);
}

void View::postInvalidate(){
    postDelayed([this](){ invalidate(nullptr);},30);
}

void View::postInvalidateOnAnimation(){
    invalidate(nullptr);
}

void View::invalidateDrawable(Drawable& who){
    if(verifyDrawable(&who))
        invalidate(nullptr);
}

void View::scheduleDrawable(Drawable& who,Runnable what, long when){
    long delay = when - SystemClock::uptimeMillis();
    postDelayed(what,delay);
}

void View::unscheduleDrawable(Drawable& who,Runnable what){
    //LOGV(" %p unschedule %p",&who,addr_of(what));
}

void View::unscheduleDrawable(Drawable& who){
    LOGV(" %p ",&who);
}

ViewGroup*View::getParent()const{
    return mParent;
}

View& View::setParent(ViewGroup*p){
    mParent=p;
    onAttached();
    return *this;
}

ViewGroup*View::getRootView()const{
    View* parent = (View*)this;
    while (parent->mParent != nullptr) {
        parent =parent->mParent;
    }
    return dynamic_cast<ViewGroup*>(parent);
}

View*View::focusSearch(int direction)const{
    if(mParent)
        mParent->focusSearch((View*)this,direction);
    return nullptr;
}
bool View::requestFocus(int direction){
    return requestFocus(direction,nullptr);
}

void View::clearFocus(){
    clearFocusInternal(nullptr, true,isInTouchMode());
}

bool View::restoreFocusInCluster(int direction){
    if (restoreDefaultFocus()) {
        return true;
    }
    return requestFocus(direction);
}

bool View::restoreFocusNotInCluster(){
    return requestFocus(View::FOCUS_DOWN);
}

bool View::restoreDefaultFocus() {
    return requestFocus(View::FOCUS_DOWN);
}

View*View::findFocus(){
    return (mPrivateFlags & PFLAG_FOCUSED) != 0 ? this : nullptr;
}

bool View::requestFocus(int direction,const RECT* previouslyFocusedRect){
    return requestFocusNoSearch(direction, previouslyFocusedRect);
}

void View::clearParentsWantFocus(){
    if(mParent!=nullptr){
        mParent->mPrivateFlags &= ~PFLAG_WANTS_FOCUS;
        mParent->clearParentsWantFocus();
    }
}

bool View::requestFocusNoSearch(int direction,const RECT*previouslyFocusedRect) {
    // need to be focusable
    //if (!canTakeFocus())  return false;
    // need to be focusable in touch mode if in touch mode
    if (isInTouchMode() && (FOCUSABLE_IN_TOUCH_MODE != (mViewFlags & FOCUSABLE_IN_TOUCH_MODE))) {
        return false;
    }
    // need to not have any parents blocking us
    if (hasAncestorThatBlocksDescendantFocus()) return false;
   
    if (!isLayoutValid()) {
        mPrivateFlags |= PFLAG_WANTS_FOCUS;
    } else {
        clearParentsWantFocus();
    }
    handleFocusGainInternal(direction, previouslyFocusedRect);
    return true;
}

bool View::requestFocusFromTouch(){
   if(isInTouchMode()){
       ViewGroup* viewRoot = (ViewGroup*)getRootView();
       //if(viewRoot)viewRoot->ensureTouchMode(false);
   }
   return requestFocus(View::FOCUS_DOWN);
}

bool View::hasAncestorThatBlocksDescendantFocus(){
   const  bool focusableInTouchMode = isFocusableInTouchMode();
   ViewGroup* ancestor =mParent;
   while (ancestor) {
       const ViewGroup*vgAncestor =ancestor;
       if (vgAncestor->getDescendantFocusability() == ViewGroup::FOCUS_BLOCK_DESCENDANTS
                 || (!focusableInTouchMode && vgAncestor->shouldBlockFocusForTouchscreen())) {
            return true;
       } else {
            ancestor = vgAncestor->getParent();
       }
   }
   return false;
}

bool View::hasFocus()const{
    return (mPrivateFlags & PFLAG_FOCUSED) != 0;
}

void View::unFocus(View*focused){
    clearFocusInternal(focused, false, false);
}


void View::clearFocusInternal(View* focused, bool propagate, bool refocus){
    if ((mPrivateFlags & PFLAG_FOCUSED) != 0) {
         mPrivateFlags &= ~PFLAG_FOCUSED;
         clearParentsWantFocus();

         if (propagate && mParent != nullptr) {
             mParent->clearChildFocus(this);
         }

         onFocusChanged(false, 0, nullptr);
         invalidate(nullptr);
         refreshDrawableState();

         //if (propagate && (!refocus || !rootViewRequestFocus())) notifyGlobalFocusCleared(this);
         
    }
}

void View::handleFocusGainInternal(int direction,const RECT*previouslyFocusedRect){
    if ((mPrivateFlags & PFLAG_FOCUSED) == 0) {
        mPrivateFlags |= PFLAG_FOCUSED;
        View* oldFocus =getRootView()?getRootView()->findFocus():nullptr;
        LOGV("%p :%d gained focused oldFocus:%p:%d",this,mID,oldFocus,(oldFocus?oldFocus->mID:-1));
        if (mParent != nullptr) {
            mParent->requestChildFocus(this, this);
            updateFocusedInCluster(oldFocus, direction);
        }

        //if (mAttachInfo != null) mAttachInfo.mTreeObserver.dispatchOnGlobalFocusChange(oldFocus, this);

        onFocusChanged(true, direction, previouslyFocusedRect);
        invalidate(nullptr);
        refreshDrawableState();
    }
}

void View::setRevealOnFocusHint(bool revealOnFocus){
    if (revealOnFocus) {
        mPrivateFlags3 &= ~PFLAG3_NO_REVEAL_ON_FOCUS;
    } else {
        mPrivateFlags3 |= PFLAG3_NO_REVEAL_ON_FOCUS;
    }
}

bool View::getRevealOnFocusHint()const{
    return (mPrivateFlags3 & PFLAG3_NO_REVEAL_ON_FOCUS) == 0;
}

void View::setFocusedInCluster(View* cluster){
    if (dynamic_cast<ViewGroup*>(this)) {
        ((ViewGroup*) this)->mFocusedInCluster = nullptr;
    }
    if (cluster == this) {
        return;
    }
    ViewGroup* parent = mParent;
    View* child = this;
    while (parent) {
        parent->mFocusedInCluster = child;
        if (parent == cluster) {
            break;
        }
        child = (View*) parent;
        parent = parent->getParent();
    }
}

void View::updateFocusedInCluster(View* oldFocus,int direction){
    if (oldFocus != nullptr) {
        View* oldCluster = oldFocus->findKeyboardNavigationCluster();
        View* cluster = findKeyboardNavigationCluster();
        if (oldCluster != cluster) {
            // Going from one cluster to another, so save last-focused.
            // This covers cluster jumps because they are always FOCUS_DOWN
            oldFocus->setFocusedInCluster(oldCluster);
            if ( oldFocus->mParent==nullptr)return ;

            if (direction == FOCUS_FORWARD || direction == FOCUS_BACKWARD) {
                // This is a result of ordered navigation so consider navigation through
                // the previous cluster "complete" and clear its last-focused memory.
                oldFocus->mParent->clearFocusedInCluster(oldFocus);
            } else if ( dynamic_cast<ViewGroup*>(oldFocus)
                    && ((ViewGroup*) oldFocus)->getDescendantFocusability()
                            == ViewGroup::FOCUS_AFTER_DESCENDANTS
                    /*&& ViewRootImpl.isViewDescendantOf(this, oldFocus)*/) {
                // This means oldFocus is not focusable since it obviously has a focusable
                // child (this). Don't restore focus to it in the future.
                oldFocus->mParent->clearFocusedInCluster(oldFocus);
            }
        }
    }
}

void View::addTouchables(std::vector<View*>& views)const {
    const int viewFlags = mViewFlags;
    if (((viewFlags & CLICKABLE) == CLICKABLE || (viewFlags & LONG_CLICKABLE) == LONG_CLICKABLE
            || (viewFlags & CONTEXT_CLICKABLE) == CONTEXT_CLICKABLE)
            && (viewFlags & ENABLED_MASK) == ENABLED) {
        views.push_back((View*)this);
    }
}

void View::addFocusables(std::vector<View*>& views,int direction)const{
    addFocusables(views, direction, isInTouchMode() ? FOCUSABLES_TOUCH_MODE : FOCUSABLES_ALL);
}

void View::addFocusables(std::vector<View*>& views,int direction,int focusableMode)const{
    if (!isFocusable()) {
        return;
    }
    if ((focusableMode & FOCUSABLES_TOUCH_MODE) == FOCUSABLES_TOUCH_MODE
           && !isFocusableInTouchMode()) {
        return;
    }
    views.push_back((View*)this);
}

void View::setKeyboardNavigationCluster(bool isCluster){
    if (isCluster) {
        mPrivateFlags3 |= PFLAG3_CLUSTER;
    } else {
        mPrivateFlags3 &= ~PFLAG3_CLUSTER;
    }
}

bool View::isKeyboardNavigationCluster()const{
    return (mPrivateFlags3 & PFLAG3_CLUSTER) != 0;
}

bool View::isInTouchMode()const{
    return false;
}

bool View::isFocusable()const{
    return FOCUSABLE == (mViewFlags & FOCUSABLE);
}

void View::setFocusable(bool focusable){
    setFocusable((int)(focusable ? FOCUSABLE : NOT_FOCUSABLE));
}

void View::setFocusable(int focusable){
    if ((focusable & (FOCUSABLE_AUTO | FOCUSABLE)) == 0) {
        setFlags(0, FOCUSABLE_IN_TOUCH_MODE);
    }
    setFlags(focusable, FOCUSABLE_MASK);    
}

bool View::isFocusableInTouchMode()const{
    return FOCUSABLE_IN_TOUCH_MODE == (mViewFlags & FOCUSABLE_IN_TOUCH_MODE);
}

void View::setFocusableInTouchMode(bool focusableInTouchMode){
    setFlags(focusableInTouchMode ? FOCUSABLE_IN_TOUCH_MODE : 0, FOCUSABLE_IN_TOUCH_MODE);
    // Clear FOCUSABLE_AUTO if set.
    if (focusableInTouchMode) {
        // Clears FOCUSABLE_AUTO if set.
        setFlags(FOCUSABLE, FOCUSABLE_MASK);
    }
}

void View::addKeyboardNavigationClusters(std::vector<View*>&views,int drection)const{
    
}

std::vector<View*>View::getFocusables(int direction)const{
    std::vector<View*> result;
    addFocusables(result, direction);
    return result;
}

int View::getFocusable()const{
   return (mViewFlags & FOCUSABLE_AUTO) > 0 ? FOCUSABLE_AUTO : (mViewFlags & FOCUSABLE);
}

bool View::hasExplicitFocusable()const {
    return hasFocusable(false, true);
}

bool View::hasFocusable(bool allowAutoFocus, bool dispatchExplicit)const{
    if (!isFocusableInTouchMode()) {
        for (ViewGroup* p = mParent; p; p = p->mParent) {
            if (p->shouldBlockFocusForTouchscreen()) {
                return false;
            }
        }
    }

    // Invisible and gone views are never focusable.
    if ((mViewFlags & VISIBILITY_MASK) != VISIBLE) {
        return false;
    }

        // Only use effective focusable value when allowed.
    if ((allowAutoFocus || getFocusable() != FOCUSABLE_AUTO) && isFocusable()) {
        return true;
    }
    return false;
}

View*View::keyboardNavigationClusterSearch(View* currentCluster,int direction){
    if (isKeyboardNavigationCluster()) {
        currentCluster = this;
    }
    if (isRootNamespace()) {
        // Root namespace means we should consider ourselves the top of the
        // tree for group searching; otherwise we could be group searching
        // into other tabs.  see LocalActivityManager and TabHost for more info.
        return FocusFinder::getInstance().findNextKeyboardNavigationCluster(
                this, currentCluster, direction);
    } else if (mParent != nullptr) {
        return mParent->keyboardNavigationClusterSearch(currentCluster, direction);
    }
    return nullptr;
}

View* View::findUserSetNextFocus(View*root,int direction)const{
    switch (direction) {
    case FOCUS_LEFT:
         if (mNextFocusLeftId == NO_ID) return nullptr;
             return findViewInsideOutShouldExist(root, mNextFocusLeftId);
    case FOCUS_RIGHT:
         if (mNextFocusRightId==NO_ID) return nullptr;
             return findViewInsideOutShouldExist(root, mNextFocusRightId);
    case FOCUS_UP:
         if (mNextFocusUpId == NO_ID) return nullptr;
             return findViewInsideOutShouldExist(root, mNextFocusUpId);
    case FOCUS_DOWN:
         if (mNextFocusDownId==NO_ID) return nullptr;
            return findViewInsideOutShouldExist(root, mNextFocusDownId);
    case FOCUS_FORWARD:
         if (mNextFocusForwardId==NO_ID) return nullptr;
            return findViewInsideOutShouldExist(root, mNextFocusForwardId);
    /*case FOCUS_BACKWARD: {
         if (mID == -1) return nullptr;
              int id = mID;
              return root.findViewByPredicateInsideOut(this, new Predicate<View>() {
                    @Override
                    public boolean test(View t) {
                        return t.mNextFocusForwardId == id;
                    }
                });
            }*/
     }
     return nullptr;
}

View*View::findUserSetNextKeyboardNavigationCluster(View*root,int direction)const{
    switch (direction) {
    case FOCUS_FORWARD:
        if (mNextClusterForwardId == NO_ID) return nullptr;
            return findViewInsideOutShouldExist(root, mNextClusterForwardId);
    case FOCUS_BACKWARD: {
        if (mID == NO_ID) return nullptr;
            //const int id = mID;
        return nullptr;//root->findViewByPredicateInsideOut(this,(Predicate<View>) t -> t.mNextClusterForwardId == id);
       }
    }
    return nullptr;
}

View*View::findKeyboardNavigationCluster()const{
    if(mParent){
        View* cluster=mParent->findKeyboardNavigationCluster();
        if (cluster != nullptr) {
            return cluster;
        } else if (isKeyboardNavigationCluster()) {
            return (View*)this;
        }
    }
    return nullptr;
}

void View::onLayout(bool change,int l,int t,int w,int h){
}

bool View::shouldDrawRoundScrollbar()const{
    return ViewConfiguration::isScreenRound();
}

void View::layout(int l, int t, int w, int h){
    if ((mPrivateFlags3 & PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT) != 0) {
        onMeasure(mOldWidthMeasureSpec, mOldHeightMeasureSpec);
        mPrivateFlags3 &= ~PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT;
    }
    int oldL = mLeft;
    int oldT = mTop;
    int oldW = mWidth;
    int oldH = mHeight;
    mPrivateFlags &= ~PFLAG_FORCE_LAYOUT;
    mPrivateFlags3 |= PFLAG3_IS_LAID_OUT;
    bool changed=setFrame(l,t,w,h);
    if(changed||true){
        if (shouldDrawRoundScrollbar()) {
            if(mRoundScrollbarRenderer == nullptr)
                mRoundScrollbarRenderer = new RoundScrollbarRenderer(this);
        } else {
            mRoundScrollbarRenderer = nullptr;
        }
        onLayout(true, l, t, w, h);
        for(auto ls:mOnLayoutChangeListeners){
            ls(this,l, t, w, h,oldL,oldT,oldW,oldH);
        }
    }
}

void View::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    setMeasuredDimension(getDefaultSize(getSuggestedMinimumWidth(), widthMeasureSpec),
                getDefaultSize(getSuggestedMinimumHeight(), heightMeasureSpec));
}

void View::onSizeChanged(int w,int h,int ow,int oh){
}

void View::onScrollChanged(int l, int t, int oldl, int oldt){
    mBackgroundSizeChanged = true;
    mBoundsChangedmDefaultFocusHighlightSizeChanged = true;
    if (mForegroundInfo != nullptr) {
        mForegroundInfo->mBoundsChanged = true;
    }
    if(mOnScrollChangeListener)
        mOnScrollChangeListener(*this,l,t,oldl,oldt);
}

void View::onFinishInflate(){
}

bool View::dispatchKeyEvent(KeyEvent&event){
    bool res=event.dispatch(this,nullptr,this);
    LOGV("%s.%s=%d",event.getLabel(event.getKeyCode()),KeyEvent::actionToString(event.getAction()).c_str(),res);    
    return res;
}

/** This method is the last chance for the focused view and its ancestors to
  * respond to an arrow key. This is called when the focused view did not
  * consume the key internally, nor could the view system find a new view in
  * the requested direction to give focus to.*/
bool View::dispatchUnhandledMove(View* focused,int direction){
    return false;
}

bool View::onKeyDown(int keyCode,KeyEvent& evt){
    //int mc=InputMethodManager::getInstance().getCharacter(keyCode,evt.getMetaState());
    if (KeyEvent::isConfirmKey(keyCode)) {
        if ((mViewFlags & ENABLED_MASK) == DISABLED)return true;

        if (evt.getRepeatCount()== 0){// Long clickable items don't necessarily have to be clickable.
            const bool clickable =isClickable()||isLongClickable();
            if (clickable || (mViewFlags & TOOLTIP) == TOOLTIP) {
                // For the purposes of menu anchoring and drawable hotspots,
                // key events are considered to be at the center of the view.
                const int x = getWidth() / 2;
                const int y = getHeight()/ 2;
                if (clickable) setPressed(true, x, y);
                checkForLongClick(0, x, y);
                LOGD("%p[%d] clickable=%d",this,mID,clickable,isPressed());
                return true;
            }
        }
    }

    return false;
}

bool View::onKeyUp(int keycode,KeyEvent& event){
    if(KeyEvent::isConfirmKey(event.getKeyCode())){
        if ((mViewFlags & ENABLED_MASK) == DISABLED) return true;
        if (isClickable() && isPressed()) {
            setPressed(false);
            if (!mHasPerformedLongPress) {
                // This is a tap, so remove the longpress check
                removeLongPressCallback();
                //if (!event.isCanceled()) return performClickInternal();
                performClickInternal();return true;
            }
        }

    }
    return false;
}

bool View::onKeyLongPress(int keyCode, KeyEvent& event){
    return false;
}

bool View::onKeyMultiple(int keyCode, int count, KeyEvent& event){
    return false;
}

int View::commitText(const std::wstring&ws){
    return 0;
}

bool View::dispatchGenericMotionEventInternal(MotionEvent& event){
    /*if (li != null && li.mOnGenericMotionListener != null
                && (mViewFlags & ENABLED_MASK) == ENABLED
                && li.mOnGenericMotionListener.onGenericMotion(this, event)) {
            return true;
    }*/
    if (onGenericMotionEvent(event)) {
        return true;
    }

    int actionButton = event.getActionButton();
    switch (event.getActionMasked()) {
    case MotionEvent::ACTION_BUTTON_PRESS:
        if (isContextClickable() && !mInContextButtonPress && !mHasPerformedLongPress
                && (actionButton == MotionEvent::BUTTON_STYLUS_PRIMARY
                || actionButton == MotionEvent::BUTTON_SECONDARY)) {
            if (performContextClick(event.getX(), event.getY())) {
                mInContextButtonPress = true;
                setPressed(true, event.getX(), event.getY());
                removeTapCallback();
                removeLongPressCallback();
                return true;
            }
        }
        break;

    case MotionEvent::ACTION_BUTTON_RELEASE:
        if (mInContextButtonPress && (actionButton == MotionEvent::BUTTON_STYLUS_PRIMARY
                || actionButton == MotionEvent::BUTTON_SECONDARY)) {
            mInContextButtonPress = false;
            mIgnoreNextUpEvent = true;
        }
        break;
    }
    return false;
}

bool View::dispatchGenericPointerEvent(MotionEvent& event) {
    return false;
}

bool View::dispatchGenericFocusedEvent(MotionEvent& event) {
    return false;
}

bool View::dispatchHoverEvent(MotionEvent& event){
    /*if (li != null && li.mOnHoverListener != null
                && (mViewFlags & ENABLED_MASK) == ENABLED
                && li.mOnHoverListener.onHover(this, event)) {
            return true;
    }*/
    return onHoverEvent(event);
}

bool View::dispatchGenericMotionEvent(MotionEvent&event){
    int source = event.getSource();
    if ((source & InputEvent::SOURCE_CLASS_POINTER) != 0) {
        int action = event.getAction();
        if (action == MotionEvent::ACTION_HOVER_ENTER
                || action == MotionEvent::ACTION_HOVER_MOVE
                || action == MotionEvent::ACTION_HOVER_EXIT) {
            if (dispatchHoverEvent(event)) {
                return true;
            }
        } else if (dispatchGenericPointerEvent(event)) {
            return true;
        }
    } else if (dispatchGenericFocusedEvent(event)) {
        return true;
    }
    if (dispatchGenericMotionEventInternal(event)) {
        return true;
    }
    return false;
}

bool View::dispatchTouchEvent(MotionEvent&event){
    return onTouchEvent(event);
}

bool View::onInterceptTouchEvent(MotionEvent&event){
    return false;
}

bool View::onGenericMotionEvent(MotionEvent& event){
    return false;
}

bool View::onHoverEvent(MotionEvent& evt){
    return false;
}

void View::onHoverChanged(bool hovered){
}

bool View::isHoverable()const{
    int viewFlags = mViewFlags;
    if ((viewFlags & ENABLED_MASK) == DISABLED) {
        return false;
    }

    return (viewFlags & CLICKABLE) == CLICKABLE
            || (viewFlags & LONG_CLICKABLE) == LONG_CLICKABLE
            || (viewFlags & CONTEXT_CLICKABLE) == CONTEXT_CLICKABLE;
}

bool View::isHovered()const {
    return (mPrivateFlags & PFLAG_HOVERED) != 0;
}

void View::setHovered(bool hovered) {
    if (hovered) {
        if ((mPrivateFlags & PFLAG_HOVERED) == 0) {
            mPrivateFlags |= PFLAG_HOVERED;
            refreshDrawableState();
            onHoverChanged(true);
        }
    } else {
        if ((mPrivateFlags & PFLAG_HOVERED) != 0) {
            mPrivateFlags &= ~PFLAG_HOVERED;
            refreshDrawableState();
            onHoverChanged(false);
        }
    }
}

bool View::performClick(){
    if(mOnClick) mOnClick(*this);
    return mOnClick!=nullptr;
}

bool View::performLongClickInternal(int x, int y){
    bool handled=false;
    if(mOnLongClick)handled=mOnLongClick(*this);
    return handled;
}

bool View::performLongClick(){
   return performLongClickInternal(mLongClickX,mLongClickY);
}

bool View::performLongClick(int x,int y){
    mLongClickX = x;
    mLongClickY = y;
    const bool handled = performLongClick();
    mLongClickX = INT_MIN;
    mLongClickY = INT_MIN;
    return handled;
}

bool View::performClickInternal(){
    return performClick();
}

bool View::performContextClick(int x, int y) {
    return performContextClick();
}

bool View::performContextClick() {
    //sendAccessibilityEvent(AccessibilityEvent.TYPE_VIEW_CONTEXT_CLICKED);
    bool handled = false;
    /*ListenerInfo li = mListenerInfo;
    if (li != null && li.mOnContextClickListener != null) {
        handled = li.mOnContextClickListener.onContextClick(View.this);
    }
    if (handled) {
        performHapticFeedback(HapticFeedbackConstants.CONTEXT_CLICK);
    }*/
    return handled;
}

void View::checkLongPressCallback(int x,int y){
    LOGV("checkLongPressCallback(%d,%d)  pressstate=%d,%d",x,y,mOriginalPressedState,isPressed());
    if(mOriginalPressedState==isPressed()){
        if (performLongClick(x, y)) {
            mHasPerformedLongPress = true;
        }
    }
}

void View::checkForLongClick(int delayOffset,int x,int y){
    LOGV("checkForLongClick longclickable=%d",(mViewFlags & LONG_CLICKABLE) == LONG_CLICKABLE );
    if ((mViewFlags & LONG_CLICKABLE) == LONG_CLICKABLE || (mViewFlags & TOOLTIP) == TOOLTIP) {
        mHasPerformedLongPress = false;
        mOriginalPressedState=isPressed();
        mPendingCheckForLongPress=std::bind(&View::checkLongPressCallback,this,x,y);
        postDelayed(mPendingCheckForLongPress,ViewConfiguration::getLongPressTimeout()-delayOffset);
    }
}

void View::checkForTapCallback(int x,int y){
    mPrivateFlags &= ~PFLAG_PREPRESSED;
    setPressed(true,x,y);
    checkForLongClick(ViewConfiguration::getTapTimeout(), x, y);
}

void View::unsetPressedCallback(){
    LOGV("unsetPressedCallback pressed=%x",(mPrivateFlags & PFLAG_PRESSED));
    if ((mPrivateFlags & PFLAG_PRESSED) != 0 && mUnsetPressedState != nullptr) {
        setPressed(false);
        removeCallbacks(mUnsetPressedState);
        mUnsetPressedState=nullptr;
    }
}

void View::removeTapCallback() {
    if (mPendingCheckForTap != nullptr) {
        mPrivateFlags &= ~PFLAG_PREPRESSED;
        removeCallbacks(mPendingCheckForTap);
        mPendingCheckForTap=nullptr;
    }
}

void View::removeLongPressCallback() {
    if (mPendingCheckForLongPress != nullptr) {
        removeCallbacks(mPendingCheckForLongPress);
        mPendingCheckForLongPress=nullptr;
    }
}

void View::removeUnsetPressCallback() {
    LOGV("removeUnsetPressCallback pressed=%d",mPrivateFlags & PFLAG_PRESSED);
    if ((mPrivateFlags & PFLAG_PRESSED) != 0 && mUnsetPressedState != nullptr) {
        setPressed(false);
        removeCallbacks(mUnsetPressedState);
        mUnsetPressedState=nullptr;
    }
}

bool View::onTouchEvent(MotionEvent& mt){
    const int x=mt.getX();
    const int y=mt.getY();
    const int action=mt.getAction();
    const bool prepressed = (mPrivateFlags & PFLAG_PREPRESSED) != 0;
    const bool clickable=((mViewFlags&CLICKABLE) == CLICKABLE  || (mViewFlags&LONG_CLICKABLE) == LONG_CLICKABLE);

    if ((mViewFlags & ENABLED_MASK) == DISABLED) {
        if (action == MotionEvent::ACTION_UP && (mPrivateFlags & PFLAG_PRESSED) != 0) {
            setPressed(false);
        }
        mPrivateFlags3 &= ~PFLAG3_FINGER_DOWN;
        // A disabled view that is clickable still consumes the touch
        // events, it just doesn't respond to them.
        return clickable;
    }
    //if(!clickable)return false;
    switch(action){
    case MotionEvent::ACTION_UP:
        if(mPrivateFlags&PFLAG_PRESSED);
        if (!clickable){
            removeTapCallback();
            removeLongPressCallback();
            mHasPerformedLongPress=false;
            mIgnoreNextUpEvent=false;
            break;
        }
        if ((mPrivateFlags & PFLAG_PRESSED) != 0 || prepressed) {
            bool focusTaken = false;
            if (isFocusable() && isFocusableInTouchMode() && !isFocused()) {
                focusTaken = requestFocus();
            }

            if (prepressed)setPressed(true);

            if(!mHasPerformedLongPress ){//&& !mIgnoreNextUpEvent){
                removeLongPressCallback();
                if (!focusTaken){
                    performClickInternal();
                }
            }
            if(prepressed){
                mUnsetPressedState=std::bind(&View::unsetPressedCallback,this);
                postDelayed(mUnsetPressedState,ViewConfiguration::getPressedStateDuration());
            }else{
                postDelayed(mUnsetPressedState,10);
            }
            removeTapCallback();
        }
        mIgnoreNextUpEvent=true;
        break; 
    case MotionEvent::ACTION_DOWN:
        mHasPerformedLongPress=false;
        if (!clickable) {
            checkForLongClick(0, x, y);
            break;
        }
        if(isInScrollingContainer()){
            mPrivateFlags |= PFLAG_PREPRESSED;
            mPendingCheckForTap=std::bind(&View::checkForTapCallback,this,x,y);
            postDelayed(mPendingCheckForTap,ViewConfiguration::getTapTimeout());
        }else{
            setPressed(true,x,y);
            checkForLongClick(0, x, y);
        }
        break;
    case MotionEvent::ACTION_MOVE:
        break;
    case MotionEvent::ACTION_CANCEL:break;
    }
    return false;
}

void View::postOnAnimation(const Runnable& action){
    postDelayed(action,10);
}

void View::postOnAnimationDelayed(const Runnable& action, uint32_t delayMillis){
    postDelayed(action,delayMillis);
}

void View::post(const Runnable& what){
    postDelayed(what,0);
}

void View::postDelayed(const Runnable& what,uint32_t delay){
    View*root=getRootView();
    if(root&&(root!=this))root->postDelayed(what,delay);
}

void View::post(const std::function<void()>&what){
    Runnable r;
    r=what;
    post(r);
}

void View::postDelayed(const std::function<void()>&what,uint32_t delay){
    Runnable r;
    r=what;
    postDelayed(r,delay);    
}

void View::removeCallbacks(const Runnable& what){
    View*root=getRootView();
    if(root&&(root!=this))root->removeCallbacks(what);
}

///////////////////////////////////////////////////////////////////////////////////////
//   For Layout support

void View::requestLayout(){
    LOGV("%p:%d parent requested=%d",this,mID,(mParent&&mParent->isLayoutRequested()));
    mPrivateFlags |= PFLAG_FORCE_LAYOUT;
    mPrivateFlags |= PFLAG_INVALIDATED;
    if (mParent != nullptr && !mParent->isLayoutRequested()) {
        mParent->requestLayout();
    }
}

void View::forceLayout(){
    //if (mMeasureCache != nullptr) mMeasureCache.clear();
    mPrivateFlags |= PFLAG_FORCE_LAYOUT;
    mPrivateFlags |= PFLAG_INVALIDATED;
}

bool View::isLaidOut()const{
    return (mPrivateFlags3 & PFLAG3_IS_LAID_OUT) == PFLAG3_IS_LAID_OUT;
}

bool View::isLayoutValid()const{
    return isLaidOut() && ((mPrivateFlags & PFLAG_FORCE_LAYOUT) == 0);
}

bool View::isLayoutRequested()const{
    return (mPrivateFlags & PFLAG_FORCE_LAYOUT) == PFLAG_FORCE_LAYOUT;
}

bool View::isLayoutModeOptical(View*p){
    if(dynamic_cast<ViewGroup*>(p)){
        return ((ViewGroup*)p)->isLayoutModeOptical();
    }
    return false;
}

bool View::resolveRtlPropertiesIfNeeded(){
    return false;
}

void View::resetRtlProperties(){
}

int View::getDefaultSize(int size, int measureSpec) {
    int result = size;
    int specMode = MeasureSpec::getMode(measureSpec);
    int specSize = MeasureSpec::getSize(measureSpec);

    switch (specMode) {
    case MeasureSpec::UNSPECIFIED:  result = size;  break;
    case MeasureSpec::AT_MOST:
    case MeasureSpec::EXACTLY: result = specSize;   break;
    }
    return result;
}

int View::getSuggestedMinimumHeight() {
    return (mBackground == nullptr) ? mMinHeight : std::max(mMinHeight, mBackground->getMinimumHeight());
}


int View::getSuggestedMinimumWidth() {
    return (mBackground == nullptr) ? mMinWidth : std::max(mMinWidth, mBackground->getMinimumWidth());
}

int View::getMinimumHeight() {
    return mMinHeight;
}

void View::setMinimumHeight(int minHeight) {
    mMinHeight = minHeight;
    requestLayout();
}

int View::getMinimumWidth() {
    return mMinWidth;
}

void View::setMinimumWidth(int minWidth) {
    mMinWidth = minWidth;
    requestLayout();
}

void View::playSoundEffect(int soundConstant){
}
bool View::performHapticFeedback(int feedbackConstant, int flags){
    return false;
}

void View::setMeasuredDimensionRaw(int measuredWidth, int measuredHeight) {
    mMeasuredWidth = measuredWidth;
    mMeasuredHeight = measuredHeight;
    mPrivateFlags |= PFLAG_MEASURED_DIMENSION_SET;
}

void View::setMeasuredDimension(int measuredWidth, int measuredHeight) {
    /*bool optical = isLayoutModeOptical(this);
    if (optical != isLayoutModeOptical(mParent)) {
        Insets insets = getOpticalInsets();
        int opticalWidth  = insets.left + insets.right;
        int opticalHeight = insets.top  + insets.bottom;

        measuredWidth  += optical ? opticalWidth  : -opticalWidth;
        measuredHeight += optical ? opticalHeight : -opticalHeight;
    }*/
    setMeasuredDimensionRaw(measuredWidth, measuredHeight);
}

int View::getBaseline(){
    return 0;
}

void View::getDrawingRect(RECT& outRect) {
    outRect.x = mScrollX;
    outRect.y = mScrollY;
    outRect.width=mWidth;
    outRect.height=mHeight;
}

int View::getMeasuredWidth()const{
    return  mMeasuredWidth & MEASURED_SIZE_MASK;
}
int View::getMeasuredWidthAndState()const{
    return mMeasuredWidth;
}
int View::getMeasuredHeight()const{
    return  mMeasuredHeight & MEASURED_SIZE_MASK;
}

int View::getMeasuredState()const{
    return (mMeasuredWidth&MEASURED_STATE_MASK)
            | ((mMeasuredHeight>>MEASURED_HEIGHT_STATE_SHIFT)
            & (MEASURED_STATE_MASK>>MEASURED_HEIGHT_STATE_SHIFT));
}

int View::getMeasuredHeightAndState()const{
    return mMeasuredHeight;
}

int View::combineMeasuredStates(int curState, int newState){
    return curState | newState;
}

int View::resolveSize(int size, int measureSpec){
    return resolveSizeAndState(size, measureSpec, 0) & MEASURED_SIZE_MASK;
}

int View::resolveSizeAndState(int size, int measureSpec, int childMeasuredState){
    int specMode = MeasureSpec::getMode(measureSpec);
    int specSize = MeasureSpec::getSize(measureSpec);
    int result;
    switch (specMode) {
    case MeasureSpec::AT_MOST:
        if (specSize < size) {
            result = specSize | MEASURED_STATE_TOO_SMALL;
        } else {
            result = size;
        }
        break;
    case MeasureSpec::EXACTLY:
        result = specSize;
        break;
    case MeasureSpec::UNSPECIFIED:
    default:
        result = size;
    }
    return result | (childMeasuredState & MEASURED_STATE_MASK);
}

void View::ensureTransformationInfo(){
    if (mTransformationInfo == nullptr) {
        mTransformationInfo = new TransformationInfo();
    }
}

bool View::hasIdentityMatrix(){
    return (mMatrix.xx==1.f) && (mMatrix.yx==0) && (mMatrix.xy==0) && (mMatrix.yy==1.f) && (mMatrix.x0==0)&&(mMatrix.y0==0);
}
Matrix View::getMatrix() {
    ensureTransformationInfo();
    Matrix matrix = mTransformationInfo->mMatrix;
    //mRenderNode.getMatrix(matrix);
    return mMatrix;
}

Matrix View::getInverseMatrix() {
    ensureTransformationInfo();
    Matrix matrix = mTransformationInfo->mInverseMatrix;
    //mRenderNode.getInverseMatrix(matrix);
    return matrix;
}

float View::getRotation(){
    return .0f;
}

void View::setRotation(float rotation){

}

float View::getRotationX(){
    return .0f;
}

void View::setRotationX(float){

}

float View::getRotationY(){
    return .0f;
}

void View::setRotationY(float){
    
}

float View::getScaleX(){
    return 1.f;
}

void View::setScaleX(float){

}

float View::getScaleY(){
    return 1.f;
}

void View::setScaleY(float){
}

float View::getPivotX(){
    return 1.f;
}

void View::setPivotX(float){

}

float View::getPivotY(){
    return .0f;
}

void View::setPivotY(float){

}

bool View::isPivotSet(){
    return false;
}

void View::resetPivot(){

}

float View::getAlpha(){
    return mTransformationInfo  ? mTransformationInfo->mAlpha : 1.f;
}

void View::setAlpha(float){

}


LayoutParams*View::getLayoutParams(){
    return mLayoutParams;
}

void View::setLayoutParams(LayoutParams*params){
    mLayoutParams = params;
    resolveLayoutParams();
    if(mParent)((ViewGroup*) mParent)->onSetLayoutParams(this,params);
    requestLayout();
}

int View::getRawLayoutDirection()const{
    return (mPrivateFlags2 & PFLAG2_LAYOUT_DIRECTION_MASK) >> PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
}

bool View::isLayoutDirectionInherited()const{
    return (getRawLayoutDirection() == LAYOUT_DIRECTION_INHERIT);
}

void View::resolveLayoutParams() {
    if(mLayoutParams)
        mLayoutParams->resolveLayoutDirection(getLayoutDirection());
}

void View::measure(int widthMeasureSpec, int heightMeasureSpec){
    bool optical = isLayoutModeOptical(this);
    if (optical != isLayoutModeOptical(mParent)) {
        /*Insets insets = getOpticalInsets();
        int oWidth  = insets.left + insets.right;
        int oHeight = insets.top  + insets.bottom;
        widthMeasureSpec  = MeasureSpec::adjust(widthMeasureSpec,  optical ? -oWidth  : oWidth);
        heightMeasureSpec = MeasureSpec::adjust(heightMeasureSpec, optical ? -oHeight : oHeight);*/
    }

    // Suppress sign extension for the low bytes
    long key = (long) widthMeasureSpec << 32 | (long) heightMeasureSpec & 0xffffffffL;

    bool forceLayout = (mPrivateFlags & PFLAG_FORCE_LAYOUT) == PFLAG_FORCE_LAYOUT;

    // Optimize layout by avoiding an extra EXACTLY pass when the view is
    // already measured as the correct size. In API 23 and below, this
    // extra pass is required to make LinearLayout re-distribute weight.
    bool specChanged = widthMeasureSpec != mOldWidthMeasureSpec  || heightMeasureSpec != mOldHeightMeasureSpec;
    bool isSpecExactly = MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::EXACTLY
                && MeasureSpec::getMode(heightMeasureSpec) == MeasureSpec::EXACTLY;
    bool matchesSpecSize = getMeasuredWidth() == MeasureSpec::getSize(widthMeasureSpec)
                && getMeasuredHeight() == MeasureSpec::getSize(heightMeasureSpec);
    bool needsLayout = specChanged  && (/*sAlwaysRemeasureExactly ||*/ !isSpecExactly || !matchesSpecSize);

    if (forceLayout || needsLayout) {
        // first clears the measured dimension flag
        mPrivateFlags &= ~PFLAG_MEASURED_DIMENSION_SET;

        resolveRtlPropertiesIfNeeded();
        int cacheIndex =-1;  
        if(mMeasureCache.find(key)!=mMeasureCache.end())
            cacheIndex=mMeasureCache[key];
        if(forceLayout)cacheIndex=-1;
        if (cacheIndex < 0 || true/*sIgnoreMeasureCache*/) {
            // measure ourselves, this should set the measured dimension flag back
            onMeasure(widthMeasureSpec, heightMeasureSpec);
            mPrivateFlags3 &= ~PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT;
        } else {
            long value =0;// mMeasureCache.valueAt(cacheIndex);
            // Casting a long to int drops the high 32 bits, no mask needed
            setMeasuredDimensionRaw((int) (value >> 32), (int) value);
            mPrivateFlags3 |= PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT;
        }

        // flag not set, setMeasuredDimension() was not invoked, we raise
        // an exception to warn the developer
        if ((mPrivateFlags & PFLAG_MEASURED_DIMENSION_SET) != PFLAG_MEASURED_DIMENSION_SET) {
            LOGE("View with id %d : #onMeasure() did not set the"
                        " measured dimension by calling setMeasuredDimension()",mID);
        }
        mPrivateFlags |= PFLAG_LAYOUT_REQUIRED;
    }

    mOldWidthMeasureSpec = widthMeasureSpec;
    mOldHeightMeasureSpec = heightMeasureSpec;

    //mMeasureCache.put(key, ((long) mMeasuredWidth) << 32 |
    //        (long) mMeasuredHeight & 0xffffffffL); // suppress sign extension
}

}//endof namespace
