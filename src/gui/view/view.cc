#include <view/view.h>
#include <view/viewgroup.h>
#include <view/viewoverlay.h>
#include <view/roundscrollbarrenderer.h>
#include <widget/edgeeffect.h>
#include <animation/animationutils.h>
#include <cdlog.h>
#include <string.h>
#include <algorithm>
#include <focusfinder.h>
#include <inputmethodmanager.h>
#include <app.h>
#include <color.h>
#include <systemclock.h>

#define UNDEFINED_PADDING INT_MIN
using namespace Cairo;
namespace cdroid{

DECLARE_WIDGET(View)

bool View::sIgnoreMeasureCache = false;//targetSdkVersion < Build.VERSION_CODES.KITKAT;
bool View::sAlwaysRemeasureExactly = false;//targetSdkVersion <= Build.VERSION_CODES.M;
bool View::sPreserveMarginParamsInLayoutParamConversion = true;

bool View::VIEW_DEBUG = false;
int View::mViewCount = 0;
View::View(int w,int h){
    initView();
    mContext=&App::getInstance();

    mRight  = w;
    mBottom = h;
    mLeft = mTop =0;
    setBackgroundColor(0xFF000000);
    if(ViewConfiguration::isScreenRound())
        mRoundScrollbarRenderer = new RoundScrollbarRenderer(this);
}

View::View(Context*ctx,const AttributeSet&attrs){
    int viewFlagValues= 0;
    int viewFlagMasks = 0;
    initView();

    mContext = ctx;
    mID = ctx->getId(attrs.getString("id"));
    mMinWidth  = attrs.getDimensionPixelSize("minWidth",0);
    mMinHeight = attrs.getDimensionPixelSize("minHeight",0);
    setLayerType(attrs.getInt("layerType",std::map<const std::string,int>{
           {"software",LAYER_TYPE_SOFTWARE},{"hardware",LAYER_TYPE_HARDWARE}
        },LAYER_TYPE_NONE));
    const int quality=attrs.getInt("drawingCacheQuality",std::map<const std::string,int>{
           {"auto",(int)DRAWING_CACHE_QUALITY_AUTO},
           {"low" ,(int)DRAWING_CACHE_QUALITY_LOW },
           {"high",(int)DRAWING_CACHE_QUALITY_HIGH}
    },DRAWING_CACHE_QUALITY_AUTO);
    if(quality){
        viewFlagValues |= quality;
        viewFlagMasks  |= DRAWING_CACHE_QUALITY_MASK;
    }
    mContentDescription = attrs.getString("contentDescription");
    setVisibility(attrs.getInt("visibility",std::map<const std::string,int>{
           {"gone",(int)GONE},{"invisible",(int)INVISIBLE},{"visible",(int)VISIBLE}   },(int)VISIBLE));

    if(!attrs.getBoolean("soundEffectsEnabled",true)){
        viewFlagValues &= ~SOUND_EFFECTS_ENABLED;
        viewFlagMasks |= SOUND_EFFECTS_ENABLED;
    }
    if(!attrs.getBoolean("hapticFeedbackEnabled",true)){
        viewFlagValues &= ~HAPTIC_FEEDBACK_ENABLED;
        viewFlagMasks |= HAPTIC_FEEDBACK_ENABLED;
    }
    mPrivateFlags2 &= ~(PFLAG2_LAYOUT_DIRECTION_MASK | PFLAG2_LAYOUT_DIRECTION_RESOLVED_MASK);
    const int layoutDirection = attrs.getInt("layoutDirection",std::map<const std::string,int>{
           {"ltr", (int)LAYOUT_DIRECTION_LTR},{"rtl",(int)LAYOUT_DIRECTION_RTL},
           {"inherit",(int)LAYOUT_DIRECTION_INHERIT},{"local",(int)LAYOUT_DIRECTION_LOCALE}
	},LAYOUT_DIRECTION_DEFAULT);
    mPrivateFlags2 |= (layoutDirection << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT);

    const int textDirection = attrs.getInt("textDirection",std::map<const std::string,int>{
		 {"inherit",TEXT_DIRECTION_INHERIT}, {"locale",TEXT_DIRECTION_LOCALE},
		 {"anyRtl",TEXT_DIRECTION_ANY_RTL},  {"ltr",TEXT_DIRECTION_LTR},  {"rtl",TEXT_DIRECTION_RTL},
		 {"firstStrong",TEXT_DIRECTION_FIRST_STRONG}, {"fisrtStringLtr",TEXT_DIRECTION_FIRST_STRONG_LTR},
		 {"firstStrongRtl",TEXT_DIRECTION_FIRST_STRONG_RTL}},-1);
    if (textDirection != -1) {
        mPrivateFlags2 |= textDirection<< PFLAG2_TEXT_DIRECTION_MASK_SHIFT;
    }
    const int textAlignment = attrs.getInt("textAlignment",std::map<const std::string,int>{
        {"inherit" , TEXT_ALIGNMENT_INHERIT},    {"gravity" , TEXT_ALIGNMENT_GRAVITY},
        {"textStart",TEXT_ALIGNMENT_TEXT_START}, {"textEnd" , TEXT_ALIGNMENT_TEXT_END},
        {"center"  , TEXT_ALIGNMENT_CENTER},     {"viewStart",TEXT_ALIGNMENT_VIEW_START},
        {"viewEnd"  ,TEXT_ALIGNMENT_VIEW_END}    },TEXT_ALIGNMENT_DEFAULT);
    // Clear any text alignment flag already set
    mPrivateFlags2 &= ~PFLAG2_TEXT_ALIGNMENT_MASK;
    // Set the text alignment flag depending on the value of the attribute
    mPrivateFlags2 |= (textAlignment<<PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT);
    //setTextAlignment( textAlignment );
    setForegroundGravity( attrs.getGravity("foregroundGravity",Gravity::NO_GRAVITY) );
    setForegroundTintList(attrs.getColorStateList("foregroundTint"));

    setClickable( attrs.getBoolean("clickable",false) );
    setLongClickable( attrs.getBoolean("longclickable",false) );
    setFocusableInTouchMode( attrs.getBoolean("focusableInTouchMode",false) );
    setFocusedByDefault( attrs.getBoolean("focusedByDefault",false) );
  
    mNextFocusLeftId = ctx->getId( attrs.getString("nextFocusLeft") );
    mNextFocusRightId= ctx->getId( attrs.getString("nextFocusRight") );
    mNextFocusUpId   = ctx->getId( attrs.getString("nextFocusUp") );
    mNextFocusDownId = ctx->getId( attrs.getString("nextFocusDown") );
    mNextFocusForwardId  = ctx->getId( attrs.getString("nextFocusForward") );
    mNextClusterForwardId= ctx->getId( attrs.getString("nextClusterFoward") );

    setRotation( attrs.getFloat("rotation",0) );
    setTranslationX( attrs.getDimensionPixelSize("translationX",0) );
    setTranslationY( attrs.getDimensionPixelSize("translationY",0) );
    setTranslationZ( attrs.getDimensionPixelSize("translationZ",0) );
    setRotationX( attrs.getFloat("rotationX",0) );
    setRotationY( attrs.getFloat("rotationY",0) );
    setScaleX( attrs.getFloat("scaleX",1.f) );
    setScaleY( attrs.getFloat("scaleY",1.f) );
    setPivotX( attrs.getFloat("transformPivotX",0.5));
    setPivotY( attrs.getFloat("transformPivotY",0.5));

    setKeyboardNavigationCluster( attrs.getBoolean("keyboardNavigationCluster",false) );
 
    if( attrs.getBoolean( "focusableInTouchMode" , false ) ){
        viewFlagValues &= ~FOCUSABLE_AUTO;
        viewFlagValues |= FOCUSABLE_IN_TOUCH_MODE | FOCUSABLE;
        viewFlagMasks  |= FOCUSABLE_IN_TOUCH_MODE | FOCUSABLE_MASK;
    }
    if( attrs.hasAttribute("focusable") ){
	viewFlagValues|= attrs.getBoolean("focusable",false)?FOCUSABLE : NOT_FOCUSABLE;
	viewFlagMasks |= FOCUSABLE_MASK;
    }
    if( attrs.getBoolean("clickable",false) ){
        viewFlagValues |= CLICKABLE;
        viewFlagMasks  |= CLICKABLE;
    }
    if( attrs.getBoolean("longClickable",false) ){
        viewFlagValues |= LONG_CLICKABLE;
        viewFlagMasks  |= LONG_CLICKABLE;
    }
    if(attrs.getBoolean("duplicateParentState",false)){
        viewFlagValues |= DUPLICATE_PARENT_STATE;
        viewFlagMasks  |= DUPLICATE_PARENT_STATE;
    }

    const int fadingEdges = attrs.getInt("requiresFadingEdge",std::map<const std::string,int>({
	   {"none",FADING_EDGE_NONE},
	   {"horizontal",FADING_EDGE_HORIZONTAL},
	   {"vertical",FADING_EDGE_VERTICAL}
	}),FADING_EDGE_NONE);
    if( fadingEdges != FADING_EDGE_NONE ){
        viewFlagValues |= fadingEdges;
        viewFlagMasks |= FADING_EDGE_MASK;
        initScrollCache();
        mScrollCache->fadingEdgeLength = attrs.getInt("fadingEdgeLength",ViewConfiguration::get(mContext).getScaledFadingEdgeLength());
    }


    const int scrollbars = attrs.getInt("scrollbars",std::map<const std::string,int>({
           {"none",(int)SCROLLBARS_NONE}, {"horizontal",(int)SCROLLBARS_HORIZONTAL},
           {"vertical",(int)SCROLLBARS_VERTICAL} }),SCROLLBARS_NONE);
    if(scrollbars != SCROLLBARS_NONE){
        viewFlagValues |= scrollbars;
        viewFlagMasks  |= SCROLLBARS_MASK;
    }
    const int scrollbarStyle = attrs.getInt("scrollbarStyle",std::map<const std::string,int>({ 
        {"insideOverlay" ,(int)SCROLLBARS_INSIDE_OVERLAY },
        {"insideInset"   ,(int)SCROLLBARS_INSIDE_OVERLAY },
        {"outsideOverlay",(int)SCROLLBARS_OUTSIDE_OVERLAY},
        {"outsideInset"  ,(int)SCROLLBARS_OUTSIDE_OVERLAY} }),SCROLLBARS_INSIDE_OVERLAY);

    mOverScrollMode = attrs.getInt("overScrollMode",std::map<const std::string,int>{
           {"never",(int)OVER_SCROLL_NEVER} , {"always",(int)OVER_SCROLL_ALWAYS},
           {"ifContentScrolls",(int)OVER_SCROLL_IF_CONTENT_SCROLLS}
         },mOverScrollMode);

    mVerticalScrollbarPosition = attrs.getInt("verticalScrollbarPosition",std::map<const std::string,int>{
           {"defaultPosition",SCROLLBAR_POSITION_DEFAULT}, {"left",SCROLLBAR_POSITION_LEFT},
           {"right",SCROLLBAR_POSITION_RIGHT} },SCROLLBAR_POSITION_DEFAULT);

    if (scrollbarStyle != SCROLLBARS_INSIDE_OVERLAY) {
        viewFlagValues |= scrollbarStyle & SCROLLBARS_STYLE_MASK;
        viewFlagMasks  |= SCROLLBARS_STYLE_MASK;
    }

    const int scrollIndicators = (attrs.getInt("scrollIndicators",std::map<const std::string,int>({
           {"top",SCROLL_INDICATOR_TOP}, 	  {"left",SCROLL_INDICATOR_LEFT},
           {"right",SCROLL_INDICATOR_RIGHT}, {"bottom",SCROLL_INDICATOR_BOTTOM}
           }),0)<<SCROLL_INDICATORS_TO_PFLAGS3_LSHIFT)&SCROLL_INDICATORS_PFLAG3_MASK;
    if(scrollIndicators)mPrivateFlags3 |= scrollIndicators;

    if(viewFlagMasks)
        setFlags(viewFlagValues, viewFlagMasks);

    ColorStateList*csl = attrs.getColorStateList("backgroundTint");
    if( (mBackgroundTint == nullptr) && csl){
        mBackgroundTint = new TintInfo;
        mBackgroundTint->mTintList = csl;
        mBackgroundTint->mHasTintList = true;
    }
    const std::map<const std::string,int>tintModes={
           {"add",PorterDuff::Mode::ADD},       {"multiply",PorterDuff::Mode::MULTIPLY},
           {"screen",PorterDuff::Mode::SCREEN}, {"src_atop",PorterDuff::Mode::SRC_ATOP},
           {"src_in",PorterDuff::Mode::SRC_IN}, {"src_over",PorterDuff::Mode::SRC_OVER} };

    const int bgTintMode = Drawable::parseTintMode(attrs.getInt("backgroundTintMode",tintModes,PorterDuff::Mode::NOOP),PorterDuff::Mode::NOOP);
    if( bgTintMode != PorterDuff::Mode::NOOP ){
        if(mBackgroundTint == nullptr) mBackgroundTint=new TintInfo;
        mBackgroundTint->mTintMode = bgTintMode;
        mBackgroundTint->mHasTintMode = true;
    }
    setBackground(attrs.getDrawable("background"));

    setForeground(attrs.getDrawable("foreground"));
    //setForegroundGravity(attrs.getGravity("foregroundGravity",Gravity::NO_GRAVITY));
    const int fgTintMode = Drawable::parseTintMode(attrs.getInt("foregroundTintMode",tintModes,PorterDuff::Mode::NOOP),PorterDuff::Mode::NOOP);
    setForegroundTintMode(fgTintMode);
    mForegroundInfo->mInsidePadding = attrs.getBoolean("foregroundInsidePadding",mForegroundInfo->mInsidePadding);

    int leftPadding,topPadding,rightPadding,bottomPadding;
    const int padding = attrs.getDimensionPixelSize("padding",-1);
    if( padding >= 0 ){
        leftPadding= rightPadding = padding;
        topPadding = bottomPadding= padding;
        mUserPaddingLeftInitial   = padding;
        mUserPaddingRightInitial  = padding;
        mLeftPaddingDefined = true;
        mRightPaddingDefined = true;
    }else{
        const int paddingHorizontal = attrs.getDimensionPixelSize("paddingHorizontal",-1);
        const int paddingVertical   = attrs.getDimensionPixelSize("paddingVertical",-1);
        if( paddingHorizontal >= 0){
            leftPadding = rightPadding = paddingHorizontal;
            mLeftPaddingDefined = mRightPaddingDefined = true;
            mUserPaddingLeftInitial = mUserPaddingRightInitial = paddingHorizontal;
        }else{
            leftPadding  = attrs.getDimensionPixelSize("paddingLeft",0);
            rightPadding = attrs.getDimensionPixelSize("paddingRight",0);
            mLeftPaddingDefined  = (leftPadding != 0);
            mRightPaddingDefined = (rightPadding != 0);
            mUserPaddingLeftInitial = mLeftPaddingDefined?leftPadding:0;
            mUserPaddingRightInitial= mRightPaddingDefined?rightPadding:0;
        }
        if( paddingVertical >= 0){
            topPadding = bottomPadding = paddingVertical;
        }else{
            topPadding   = attrs.getDimensionPixelSize("paddingTop",0);
            bottomPadding= attrs.getDimensionPixelSize("paddingBottom",0);
        }
    }

    mUserPaddingStart= attrs.getDimensionPixelSize("paddingStart",UNDEFINED_PADDING);
    mUserPaddingEnd  = attrs.getDimensionPixelSize("paddingEnd", UNDEFINED_PADDING);
    const bool startPaddingDefined= (mUserPaddingStart != UNDEFINED_PADDING);
    const bool endPaddingDefined  = (mUserPaddingEnd != UNDEFINED_PADDING);
    if (isRtlCompatibilityMode()) {
        // RTL compatibility mode: pre Jelly Bean MR1 case OR no RTL support case.
        // left / right padding are used if defined (meaning here nothing to do). If they are not
        // defined and start / end padding are defined (e.g. in Frameworks resources), then we use
        // start / end and resolve them as left / right (layout direction is not taken into account).
        // Padding from the background drawable is stored at this point in mUserPaddingLeftInitial
        // and mUserPaddingRightInitial) so drawable padding will be used as ultimate default if
        // defined.
        if (!mLeftPaddingDefined && startPaddingDefined) {
            leftPadding  = mUserPaddingStart;
        }
        mUserPaddingLeftInitial = (leftPadding >= 0) ? leftPadding : mUserPaddingLeftInitial;
        if (!mRightPaddingDefined && endPaddingDefined) {
            rightPadding = mUserPaddingEnd;
        }
        mUserPaddingRightInitial = (rightPadding >= 0) ? rightPadding : mUserPaddingRightInitial;
    } else {
        // Jelly Bean MR1 and after case: if start/end defined, they will override any left/right
        // values defined. Otherwise, left /right values are used.
        // Padding from the background drawable is stored at this point in mUserPaddingLeftInitial
        // and mUserPaddingRightInitial) so drawable padding will be used as ultimate default if
        // defined.
        const bool hasRelativePadding = startPaddingDefined || endPaddingDefined;

        if (mLeftPaddingDefined && !hasRelativePadding) {
            mUserPaddingLeftInitial = leftPadding;
        }
        if (mRightPaddingDefined && !hasRelativePadding) {
            mUserPaddingRightInitial = rightPadding;
        }
    }

    internalSetPadding( mUserPaddingLeftInitial, topPadding > 0 ? topPadding : mPaddingTop,
                mUserPaddingRightInitial, bottomPadding > 0 ? bottomPadding : mPaddingBottom);
    const int x = attrs.getInt("scrollX",0);
    const int y = attrs.getInt("scrollY",0);
    if( x || y ) scrollTo(x,y);
    if(scrollbars != SCROLLBARS_NONE) initializeScrollbarsInternal(attrs);
    if(scrollIndicators) initializeScrollIndicatorsInternal();
    if(scrollbarStyle != SCROLLBARS_INSIDE_OVERLAY) recomputePadding();
    computeOpaqueFlags();
}

void View::initView(){
    mViewCount ++;
    LOGV_IF(View::VIEW_DEBUG,"mViewCount=%d",mViewCount);
    mID = NO_ID;
    mAutofillViewId =NO_ID;
    mAccessibilityViewId = NO_ID;
    mDrawingCacheBackgroundColor = 0;
    mContext  = nullptr;
    mParent   = nullptr;
    mKeyedTags = nullptr;
    mAttachInfo  = nullptr;
    mListenerInfo= nullptr;
    mTooltipInfo = nullptr;
    mOverlay  = nullptr;
    mAnimator = nullptr;
    mTag = nullptr;
    mStateListAnimator = nullptr;
    mPerformClick = nullptr;
    mPendingCheckForTap = nullptr;
    mUnsetPressedState = nullptr;;
    mPendingCheckForLongPress = nullptr;
    mInputEventConsistencyVerifier = nullptr;
    if(InputEventConsistencyVerifier::isInstrumentationEnabled()&&View::VIEW_DEBUG)
        mInputEventConsistencyVerifier = new InputEventConsistencyVerifier(nullptr,0);

    mRenderNode  = new  RenderNode();
    mScrollX  = mScrollY = 0;
    mMinWidth = mMinHeight = 0;
    mLayerType= LAYER_TYPE_NONE;
    mTransientStateCount = 0;
    mWindowAttachCount = 0;
    mClipBounds.setEmpty();

    mLeftPaddingDefined = mRightPaddingDefined =false;
    mUserPaddingLeftInitial = mUserPaddingRightInitial =0;
    mUserPaddingStart = mUserPaddingEnd = UNDEFINED_PADDING;
    mUserPaddingRight = mUserPaddingLeft= UNDEFINED_PADDING;

    mLastIsOpaque = false;
    mHasPerformedLongPress = false;
    mInContextButtonPress  = false;
    mIgnoreNextUpEvent     = false;
    mDefaultFocusHighlightEnabled = false;
    mDefaultFocusHighlightSizeChanged = false;
    mBoundsChangedmDefaultFocusHighlightSizeChanged = false;

    mOldWidthMeasureSpec = mOldHeightMeasureSpec = INT_MIN;
    mViewFlags = SOUND_EFFECTS_ENABLED | HAPTIC_FEEDBACK_ENABLED | ENABLED|VISIBLE|FOCUSABLE_AUTO;
    mPrivateFlags = mPrivateFlags2 = mPrivateFlags3 = 0;
    mPrivateFlags2 = (LAYOUT_DIRECTION_DEFAULT << PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT) |
                 (TEXT_DIRECTION_DEFAULT << PFLAG2_TEXT_DIRECTION_MASK_SHIFT) |
                 (PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT) |
                 (TEXT_ALIGNMENT_DEFAULT << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT) |
                 (PFLAG2_TEXT_ALIGNMENT_RESOLVED_DEFAULT);
                 //(IMPORTANT_FOR_ACCESSIBILITY_DEFAULT << PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_SHIFT);
    mScrollCache  = nullptr;
    mRoundScrollbarRenderer=nullptr;
    mTop = mLeft = mRight = mBottom = 0;
    mOverScrollMode = OVER_SCROLL_NEVER;
    mVerticalScrollbarPosition = 0;
    mUserPaddingLeft = mUserPaddingRight  = 0;
    mUserPaddingTop  = mUserPaddingBottom = 0;
    mPrivateFlags  = mPrivateFlags3 = 0;
    mPrivateFlags |= PFLAG_DRAWABLE_STATE_DIRTY;
    mNextFocusLeftId= mNextFocusRightId= NO_ID;
    mNextFocusUpId  = mNextFocusDownId = NO_ID;
    mNextFocusForwardId=mNextClusterForwardId = NO_ID;
    mBackgroundTint= nullptr;
    mMeasuredWidth = mMeasuredHeight = 0;
    mLayoutParams  = nullptr;
    mPaddingLeft   = mPaddingTop    = 0;
    mPaddingRight  = mPaddingBottom = 0;
    mForegroundInfo= nullptr;
    mScrollIndicatorDrawable = nullptr;
    mBackground = nullptr;
    mDefaultFocusHighlight = nullptr;
    mDefaultFocusHighlightCache = nullptr;
    mCurrentAnimation = nullptr;
    mTransformationInfo = nullptr;
    mNestedScrollingParent = nullptr;
    mFloatingTreeObserver = nullptr;
}

View::~View(){
    mViewCount --;
    LOGD_IF(View::VIEW_DEBUG||(mViewCount>1000),"%p:%d mViewCount=%d",this,mID,mViewCount);

    delete mScrollCache;
    mScrollCache = nullptr;
    if(mParent)
        mParent->removeViewInternal(this);
    if(isAttachedToWindow())onDetachedFromWindow();
    if(mBackground)mBackground->setCallback(nullptr);

    delete mKeyedTags;
    delete mForegroundInfo;
    delete mPendingCheckForTap;
    delete mPendingCheckForLongPress;
    delete mRenderNode;
    delete mListenerInfo;
    delete mTooltipInfo;
    delete mScrollIndicatorDrawable;
    delete mDefaultFocusHighlight;
    delete mInputEventConsistencyVerifier;

    delete mBackground;
    delete mBackgroundTint;
    delete mLayoutParams;
    delete mRoundScrollbarRenderer;
    delete mCurrentAnimation;
    delete mTransformationInfo;
    delete mOverlay;
    delete mAnimator;
    delete mFloatingTreeObserver;
}

bool View::isShowingLayoutBounds()const{
    return View::VIEW_DEBUG || mAttachInfo && mAttachInfo->mDebugLayout;
}

void View::setShowingLayoutBounds(bool debugLayout){
    if (mAttachInfo) {
        mAttachInfo->mDebugLayout = debugLayout;
    }
}

bool View::debugDraw()const {
    return View::VIEW_DEBUG|| (mAttachInfo && mAttachInfo->mDebugLayout);
}

int View::dipsToPixels(int dips)const{
    const float scale = getContext()->getDisplayMetrics().density;
    return (int)(dips*scale + 0.5f);
}

View* View::findViewById(int id){
    if( id == mID )return (View*)this;
    return nullptr;
}

View* View::findViewTraversal(int id){
    if( id == mID )return this;
    return nullptr;
}

View* View::findViewWithTag(void*tag){
    return findViewWithTagTraversal(tag);
}

View* View::findViewInsideOutShouldExist(View* root, int id)const{
    View* result = root->findViewByPredicateInsideOut((View*)this,[id](const View*v)->bool{
        return v->mID == id;
    }); 
    return result;
}

View* View::findViewByPredicateTraversal(std::function<bool(const View*)>predicate,View* childToSkip){
    return predicate(this)?(View*)this:nullptr;
}

View* View::findViewByPredicate(std::function<bool(const View*)>predicate){
    return findViewByPredicateTraversal(predicate,nullptr);
}

View* View::findViewWithTagTraversal(void* tag){
    return nullptr;
}

View* View::findViewByPredicateInsideOut(View*start,std::function<bool(const View*)>predicate){
    View* childToSkip = nullptr;
    for (;;) {
        View*view = start->findViewByPredicateTraversal(predicate, childToSkip);
        if (view || (start == this)) {
            return view;
        }

        ViewGroup* parent = start->getParent();
        if (parent == nullptr){
            return nullptr;
        }
        childToSkip = start;
        start = (View*) parent;
    }
}

void View::setLeft(int left){
    if (left == mLeft)return;
    const bool matrixIsIdentity = hasIdentityMatrix();
    if (matrixIsIdentity) {
        if (mAttachInfo != nullptr) {
            int minLeft;
            int xLoc;
            if (left < mLeft) {
                minLeft = left;
                xLoc = left - mLeft;
            } else {
                minLeft = mLeft;
                xLoc = 0;
            }
            invalidate(xLoc, 0, mRight - minLeft, mBottom - mTop);
        }
    } else {
        // Double-invalidation is necessary to capture view's old and new areas
        invalidate(true);
    }

    const int oldWidth = mRight - mLeft;
    const int height = mBottom - mTop;

    mLeft = left;
    mRenderNode->setLeft(left);

    sizeChange(mRight - mLeft, height, oldWidth, height);

    if (!matrixIsIdentity) {
        mPrivateFlags |= PFLAG_DRAWN; // force another invalidation with the new orientation
        invalidate(true);
    }
    mBackgroundSizeChanged = true;
    mDefaultFocusHighlightSizeChanged = true;
    if (mForegroundInfo) {
        mForegroundInfo->mBoundsChanged = true;
    }
    invalidateParentIfNeeded();
    if ((mPrivateFlags2 & PFLAG2_VIEW_QUICK_REJECTED) == PFLAG2_VIEW_QUICK_REJECTED) {
        // View was rejected last time it was drawn by its parent; this may have changed
        invalidateParentIfNeeded();
    }
}

void View::setTop(int top){
    if (top == mTop) return;
    const bool matrixIsIdentity = hasIdentityMatrix();
    if (matrixIsIdentity) {
        if (mAttachInfo != nullptr) {
            int minTop;
            int yLoc;
            if (top < mTop) {
                minTop = top;
                yLoc = top - mTop;
            } else {
                minTop = mTop;
                yLoc = 0;
            }
            invalidate(0, yLoc, mRight - mLeft, mBottom - minTop);
        }
    } else {
        // Double-invalidation is necessary to capture view's old and new areas
        invalidate(true);
    }

    const int width = mRight - mLeft;
    const int oldHeight = mBottom - mTop;

    mTop = top;
    mRenderNode->setTop(mTop);

    sizeChange(width, mBottom - mTop, width, oldHeight);

    if (!matrixIsIdentity) {
        mPrivateFlags |= PFLAG_DRAWN; // force another invalidation with the new orientation
        invalidate(true);
    }
    mBackgroundSizeChanged = true;
    mDefaultFocusHighlightSizeChanged = true;
    if (mForegroundInfo) {
        mForegroundInfo->mBoundsChanged = true;
    }
    invalidateParentIfNeeded();
    if ((mPrivateFlags2 & PFLAG2_VIEW_QUICK_REJECTED) == PFLAG2_VIEW_QUICK_REJECTED) {
        // View was rejected last time it was drawn by its parent; this may have changed
        invalidateParentIfNeeded();
    }
}

void View::setRight(int right){
    if(mRight==right)return;
    const bool matrixIsIdentity = hasIdentityMatrix();
    if (matrixIsIdentity) {
        if (mAttachInfo != nullptr) {
            int maxRight;
            if (right < mRight) {
                maxRight = mRight;
            } else {
                maxRight = right;
            }
            invalidate(0, 0, maxRight - mLeft, mBottom - mTop);
        }
    } else {
        // Double-invalidation is necessary to capture view's old and new areas
        invalidate(true);
    }

    const int oldWidth = mRight - mLeft;
    const int height = mBottom - mTop;

    mRight = right;
    mRenderNode->setRight(mRight);

    sizeChange(mRight - mLeft, height, oldWidth, height);

    if (!matrixIsIdentity) {
        mPrivateFlags |= PFLAG_DRAWN; // force another invalidation with the new orientation
        invalidate(true);
    }
    mBackgroundSizeChanged = true;
    mDefaultFocusHighlightSizeChanged = true;
    if (mForegroundInfo != nullptr) {
        mForegroundInfo->mBoundsChanged = true;
    }
    invalidateParentIfNeeded();
    if ((mPrivateFlags2 & PFLAG2_VIEW_QUICK_REJECTED) == PFLAG2_VIEW_QUICK_REJECTED) {
        // View was rejected last time it was drawn by its parent; this may have changed
        invalidateParentIfNeeded();
    }
}

void View::setBottom(int bottom){
    if (bottom == mBottom)return;
    bool matrixIsIdentity = hasIdentityMatrix();
    if (matrixIsIdentity) {
        if (mAttachInfo) {
            int maxBottom;
            if (bottom < mBottom) {
                maxBottom = mBottom;
            } else {
                maxBottom = bottom;
            }
            invalidate(0, 0, mRight - mLeft, maxBottom - mTop);
        }
    } else {
        // Double-invalidation is necessary to capture view's old and new areas
        invalidate(true);
    }
    
    const int width = mRight - mLeft;
    const int oldHeight = mBottom - mTop;
    
    mBottom = bottom;
    mRenderNode->setBottom(mBottom);
    
    sizeChange(width, mBottom - mTop, width, oldHeight);
    
    if (!matrixIsIdentity) {
        mPrivateFlags |= PFLAG_DRAWN; // force another invalidation with the new orientation
        invalidate(true);
    }
    mBackgroundSizeChanged = true;
    mDefaultFocusHighlightSizeChanged = true;
    if (mForegroundInfo) {
        mForegroundInfo->mBoundsChanged = true;
    }
    invalidateParentIfNeeded();
    if ((mPrivateFlags2 & PFLAG2_VIEW_QUICK_REJECTED) == PFLAG2_VIEW_QUICK_REJECTED) {
        // View was rejected last time it was drawn by its parent; this may have changed
        invalidateParentIfNeeded();
    }
}

int View::getLeft()const{
    return mLeft;
}

int View::getTop()const{
    return mTop;
}

int View::getRight()const{
    return mRight;
}

int View::getBottom()const{
    return mBottom;
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

bool View::isPaddingRelative()const{
    return (mUserPaddingStart != UNDEFINED_PADDING || mUserPaddingEnd != UNDEFINED_PADDING);
}

Insets View::computeOpticalInsets() {
    return (mBackground == nullptr) ? Insets() : mBackground->getOpticalInsets();
}

void View::resetPaddingToInitialValues() {
    if (isRtlCompatibilityMode()) {
        mPaddingLeft = mUserPaddingLeftInitial;
        mPaddingRight = mUserPaddingRightInitial;
        return;
    }
    if (isLayoutRtl()) {
        mPaddingLeft = (mUserPaddingEnd >= 0) ? mUserPaddingEnd : mUserPaddingLeftInitial;
        mPaddingRight = (mUserPaddingStart >= 0) ? mUserPaddingStart : mUserPaddingRightInitial;
    } else {
        mPaddingLeft = (mUserPaddingStart >= 0) ? mUserPaddingStart : mUserPaddingLeftInitial;
        mPaddingRight = (mUserPaddingEnd >= 0) ? mUserPaddingEnd : mUserPaddingRightInitial;
    }
}

Insets View::getOpticalInsets() {
    mLayoutInsets = computeOpticalInsets();
    return mLayoutInsets;
}

void View::setOpticalInsets(const Insets& insets) {
    mLayoutInsets = insets;
}

void View::setPadding(int left, int top, int right, int bottom){
    resetResolvedPaddingInternal();
    mUserPaddingStart = UNDEFINED_PADDING;
    mUserPaddingEnd = UNDEFINED_PADDING;
    mUserPaddingLeftInitial = left;
    mUserPaddingRightInitial = right;
    mLeftPaddingDefined = true;
    mRightPaddingDefined = true;

    internalSetPadding(left,top,right,bottom);
}

void View::setPaddingRelative(int start,int top,int end,int bottom){
    resetResolvedPaddingInternal();
    mUserPaddingStart = start;
    mUserPaddingEnd = end;
    mLeftPaddingDefined  = true;
    mRightPaddingDefined = true;
    switch(getLayoutDirection()){
    case LAYOUT_DIRECTION_RTL:
        mUserPaddingLeftInitial  = end;
        mUserPaddingRightInitial = start;
        internalSetPadding(end,top,start,bottom);
        break;
    case LAYOUT_DIRECTION_LTR:
    default:
        mUserPaddingLeftInitial = start;
        mUserPaddingRightInitial= end;
        internalSetPadding(start,top,end,bottom);
        break;
    }
}

void View::recomputePadding() {
    internalSetPadding(mUserPaddingLeft, mPaddingTop, mUserPaddingRight, mUserPaddingBottom);
}

void View::internalSetPadding(int left, int top, int right, int bottom){
    mUserPaddingLeft = left;
    mUserPaddingRight = right;
    mUserPaddingBottom = bottom;

    bool changed = false;

    // Common case is there are no scroll bars.
    if ((mViewFlags & (SCROLLBARS_VERTICAL|SCROLLBARS_HORIZONTAL)) != 0) {
        if ((mViewFlags & SCROLLBARS_VERTICAL) != 0) {
            const  int offset = (mViewFlags & SCROLLBARS_INSET_MASK) == 0
                    ? 0 : getVerticalScrollbarWidth();
            switch (mVerticalScrollbarPosition) {
            case SCROLLBAR_POSITION_DEFAULT:
                 if (isLayoutRtl()) {
                     left += offset;
                 } else {
                     right += offset;
                 }
                 break;
             case SCROLLBAR_POSITION_RIGHT:
                 right += offset;
                 break;
             case SCROLLBAR_POSITION_LEFT:
                 left += offset;
                 break;
            }
        }
        if ((mViewFlags & SCROLLBARS_HORIZONTAL) != 0) {
            bottom += (mViewFlags & SCROLLBARS_INSET_MASK) == 0
                    ? 0 : getHorizontalScrollbarHeight();
        }
    }

    if (mPaddingLeft != left) {
        changed = true;
        mPaddingLeft = left;
    }
    if (mPaddingTop != top) {
        changed = true;
        mPaddingTop = top;
    }
    if (mPaddingRight != right) {
        changed = true;
        mPaddingRight = right;
    }
    if (mPaddingBottom != bottom) {
        changed = true;
        mPaddingBottom = bottom;
    }
    if (changed) {
        requestLayout();
        invalidate();
    }
}

bool View::isPaddingResolved()const{
    return (mPrivateFlags2 & PFLAG2_PADDING_RESOLVED) == PFLAG2_PADDING_RESOLVED;
}

bool View::isLayoutDirectionResolved()const{
    return (mPrivateFlags2 & PFLAG2_LAYOUT_DIRECTION_RESOLVED) == PFLAG2_LAYOUT_DIRECTION_RESOLVED;
}

void View::resolvePadding(){
    const int resolvedLayoutDirection = getLayoutDirection();

    if (!isRtlCompatibilityMode()) {
        // Post Jelly Bean MR1 case: we need to take the resolved layout direction into account.
        // If start / end padding are defined, they will be resolved (hence overriding) to
        // left / right or right / left depending on the resolved layout direction.
        // If start / end padding are not defined, use the left / right ones.
        if (mBackground  && (!mLeftPaddingDefined || !mRightPaddingDefined)) {
            Rect padding;
            mBackground->getPadding(padding);
            if (!mLeftPaddingDefined) {
                mUserPaddingLeftInitial = padding.left;
            }
            if (!mRightPaddingDefined) {
                mUserPaddingRightInitial = padding.width;
            }
        }
        switch (resolvedLayoutDirection) {
        case LAYOUT_DIRECTION_RTL:
            if (mUserPaddingStart != UNDEFINED_PADDING) {
                mUserPaddingRight = mUserPaddingStart;
            } else {
                mUserPaddingRight = mUserPaddingRightInitial;
            }
            if (mUserPaddingEnd != UNDEFINED_PADDING) {
                mUserPaddingLeft = mUserPaddingEnd;
            } else {
                mUserPaddingLeft = mUserPaddingLeftInitial;
            }
            break;
        case LAYOUT_DIRECTION_LTR:
        default:
            if (mUserPaddingStart != UNDEFINED_PADDING) {
                mUserPaddingLeft = mUserPaddingStart;
            } else {
                mUserPaddingLeft = mUserPaddingLeftInitial;
            }
            if (mUserPaddingEnd != UNDEFINED_PADDING) {
                mUserPaddingRight = mUserPaddingEnd;
            } else {
                mUserPaddingRight = mUserPaddingRightInitial;
            }
        }

        mUserPaddingBottom = (mUserPaddingBottom >= 0) ? mUserPaddingBottom : mPaddingBottom;
    }

    internalSetPadding(mUserPaddingLeft, mPaddingTop, mUserPaddingRight, mUserPaddingBottom);
    onRtlPropertiesChanged(resolvedLayoutDirection);
    mPrivateFlags2 |= PFLAG2_PADDING_RESOLVED;
}

bool View::isOpaque()const{
    return (mPrivateFlags & PFLAG_OPAQUE_MASK) == PFLAG_OPAQUE_MASK ;//&&  getFinalAlpha() >= 1.0f;
}

void View::computeOpaqueFlags(){
    if (mBackground && mBackground->getOpacity() == PixelFormat::OPAQUE) {
        mPrivateFlags |= PFLAG_OPAQUE_BACKGROUND;
    } else {
        mPrivateFlags &= ~PFLAG_OPAQUE_BACKGROUND;
    }

    if (((mViewFlags & SCROLLBARS_VERTICAL) == 0 && (mViewFlags & SCROLLBARS_HORIZONTAL) == 0) ||
            (mViewFlags & SCROLLBARS_STYLE_MASK) == SCROLLBARS_INSIDE_OVERLAY ||
            (mViewFlags & SCROLLBARS_STYLE_MASK) == SCROLLBARS_OUTSIDE_OVERLAY) {
        mPrivateFlags |= PFLAG_OPAQUE_SCROLLBARS;
    } else {
        mPrivateFlags &= ~PFLAG_OPAQUE_SCROLLBARS;
    }
}

bool View::hasOpaqueScrollbars()const{
    return (mPrivateFlags & PFLAG_OPAQUE_SCROLLBARS) == PFLAG_OPAQUE_SCROLLBARS;
}

int View::getLayerType()const{
    return mLayerType;
}

void View::setLayerType(int type){
    mLayerType= type;
}

void View::onAnimationStart() {
    mPrivateFlags |= PFLAG_ANIMATION_STARTED;
}

void View::onAnimationEnd() {
    mPrivateFlags &= ~PFLAG_ANIMATION_STARTED;
}

bool View::onSetAlpha(int alpha) {
    return false;
}

Animation* View::getAnimation()const{
    return mCurrentAnimation;
}

void View::startAnimation(Animation* animation) {
    animation->setStartTime(Animation::START_ON_FIRST_FRAME);
    setAnimation(animation);
    invalidateParentCaches();
    invalidate();
}

void View::clearAnimation() {
    if (mCurrentAnimation ) {
        mCurrentAnimation->detach();
    }
    delete mCurrentAnimation;
    mCurrentAnimation = nullptr;
    invalidateParentIfNeeded();
    invalidate();
}

void View::setAnimation(Animation* animation) {
    delete mCurrentAnimation;
    mCurrentAnimation = animation;
    if (animation) {
        // If the screen is off assume the animation start time is now instead of
        // the next frame we draw. Keeping the START_ON_FIRST_FRAME start time
        // would cause the animation to start when the screen turns back on
        if (mAttachInfo != nullptr && mAttachInfo->mDisplayState //== Display.STATE_OFF
                && animation->getStartTime() == Animation::START_ON_FIRST_FRAME) {
            animation->setStartTime(AnimationUtils::currentAnimationTimeMillis());
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
    const bool lackFocusState = 
	    ((background == nullptr) || (false==background->isStateful())|| !background->hasFocusStateSpecified())
            && ((foreground == nullptr) || (false==foreground->isStateful()) || (false==foreground->hasFocusStateSpecified()));
    return !isInTouchMode() && getDefaultFocusHighlightEnabled() 
	           && lackFocusState && isAttachedToWindow();// && sUseDefaultFocusHighlight;
}

void View::switchDefaultFocusHighlight() {
    if (isFocused()) {
        const bool needed = isDefaultFocusHighlightNeeded(mBackground,
                mForegroundInfo ? mForegroundInfo->mDrawable:nullptr);
        const bool active = mDefaultFocusHighlight != nullptr;
        if (needed && !active) {
            setDefaultFocusHighlight(getDefaultFocusHighlightDrawable());
        } else if (!needed && active) {
            // The highlight is no longer needed, so tear it down.
            setDefaultFocusHighlight(nullptr);
        }
    }
}

void View::debugDrawFocus(Canvas&canvas){
    if (!isFocused()) return;
    const int cornerSquareSize = dipsToPixels(DEBUG_CORNERS_SIZE_DIP);
    const int l = mScrollX;
    const int r = l + mRight-mLeft;
    const int t = mScrollY;
    const int b = t + mBottom-mTop;

    canvas.set_color(DEBUG_CORNERS_COLOR);

    // Draw squares in corners.
    canvas.rectangle(l, t, cornerSquareSize , cornerSquareSize);
    canvas.rectangle(r - cornerSquareSize, t, cornerSquareSize,cornerSquareSize);
    canvas.rectangle(l, b - cornerSquareSize, cornerSquareSize, cornerSquareSize);
    canvas.rectangle(r - cornerSquareSize, b - cornerSquareSize, cornerSquareSize, cornerSquareSize);
    canvas.fill();

    // Draw big X across the view.
    canvas.move_to(l, t);
    canvas.line_to(r, b);
    canvas.move_to(l, b);
    canvas.line_to(r, t);
    canvas.stroke();
}

void View::drawDefaultFocusHighlight(Canvas& canvas){
    if (mDefaultFocusHighlight != nullptr) {
        if (mDefaultFocusHighlightSizeChanged) {
            mDefaultFocusHighlightSizeChanged = false;
            mDefaultFocusHighlight->setBounds(mScrollX, mScrollY,mRight-mLeft,mBottom-mTop);
        }
        mDefaultFocusHighlight->draw(canvas);
    }
}

bool View::awakenScrollBars(){
    return mScrollCache && awakenScrollBars(
        mScrollCache->scrollBarDefaultDelayBeforeFade, true);
}

bool View::initialAwakenScrollBars() {
    return mScrollCache && awakenScrollBars(
        mScrollCache->scrollBarDefaultDelayBeforeFade * 4, true);
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

        if (mScrollCache->state == ScrollabilityCache::OFF) {
            // FIXME: this is copied from WindowManagerService. We should 
            // get this value from the system when it is possible to do so.
            const int KEY_REPEAT_FIRST_DELAY = 750;
            startDelay = std::max(KEY_REPEAT_FIRST_DELAY, startDelay);
        }

        // Tell mScrollCache when we should start fading. This may
        // extend the fade start time if one was already scheduled
        long fadeStartTime = AnimationUtils::currentAnimationTimeMillis() + startDelay;
        mScrollCache->fadeStartTime = fadeStartTime;
        mScrollCache->state = ScrollabilityCache::ON;

        // Schedule our fader to run, unscheduling any old ones first
        if (mAttachInfo) {
            removeCallbacks(mScrollCache->mRunner);
            postDelayed(mScrollCache->mRunner,startDelay);
        }
        return true;
    }
    return false;
}

void View::scrollTo(int x,int y){
    if( (mScrollX!=x) || (mScrollY!=y) ){
        const int oX = mScrollX;
        const int oY = mScrollY;
        mScrollX = x;
        mScrollY = y;
        onScrollChanged(mScrollX, mScrollY, oX, oY);
        invalidate(true);
        if(!awakenScrollBars(0,true))
            postInvalidateOnAnimation();
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
    const int overScrollMode = mOverScrollMode;
    const bool canScrollHorizontal= computeHorizontalScrollRange() > computeHorizontalScrollExtent();
    const bool canScrollVertical =  computeVerticalScrollRange() > computeVerticalScrollExtent();
    const bool overScrollHorizontal = overScrollMode == OVER_SCROLL_ALWAYS ||
                (overScrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && canScrollHorizontal);
    const bool overScrollVertical = overScrollMode == OVER_SCROLL_ALWAYS ||
                (overScrollMode == OVER_SCROLL_IF_CONTENT_SCROLLS && canScrollVertical);

    int newScrollX = scrollX + deltaX;
    if (!overScrollHorizontal) maxOverScrollX = 0;

    int newScrollY = scrollY + deltaY;
    if (!overScrollVertical)   maxOverScrollY = 0;

        // Clamp values if at the limits and record
    const int left = -maxOverScrollX;
    const int right = maxOverScrollX + scrollRangeX;
    const int top = -maxOverScrollY;
    const int bottom = maxOverScrollY + scrollRangeY;

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
    Matrix matrix;
    double position[2]={(double)inOutLocation[0],(double)inOutLocation[1]};
    if(!hasIdentityMatrix()){
        matrix = getMatrix();
        matrix.transform_point(position[0],position[1]);
    }
    while (view) {
        position[0] -= view->mScrollX;
        position[1] -= view->mScrollY;
        if (!view->hasIdentityMatrix()) {
             matrix = view->getMatrix();
             matrix.transform_point(position[0],position[1]);
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

void View::mapRectFromViewToScreenCoords(Rect& rect, bool clipToParent){
    if (!hasIdentityMatrix()) {
        getMatrix().transform_rectangle((RectangleInt&)rect);//mapRect(rect);
    }
    rect.offset(mLeft, mTop);

    View* parent = mParent;
    while (parent){// instanceof View) {
        View* parentView = (View*) parent;
        rect.offset(-parentView->mScrollX, -parentView->mScrollY);

        if (clipToParent) {
            rect.left = std::max(rect.left, 0);
            rect.top  = std::max(rect.top, 0);
            rect.width= std::min(rect.width,parentView->getWidth());//rect.right = std::min(rect.right, parentView->getWidth());
            rect.height=std::min(rect.height,parentView->getHeight());//rect.bottom = std::min(rect.bottom, parentView->getHeight());
        }
        if (!parentView->hasIdentityMatrix()) {
            parentView->getMatrix().transform_rectangle((RectangleInt&)rect);//mapRect(rect);
        }
        rect.offset(parentView->mLeft, parentView->mTop);
        parent = parentView->mParent;
    }
    /*if (parent instanceof ViewRootImpl) {
        ViewRootImpl viewRootImpl = (ViewRootImpl) parent;
        rect.offset(0, -viewRootImpl.mCurScrollY);
    }*/
    rect.offset(mAttachInfo->mWindowLeft, mAttachInfo->mWindowTop);
}

void View::getLocationOnScreen(int*outLocation){
    getLocationInWindow(outLocation);

    if (mAttachInfo) {
        outLocation[0] += mAttachInfo->mWindowLeft;
        outLocation[1] += mAttachInfo->mWindowTop;
    }
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

int View::combineVisibility(int vis1, int vis2) {
    // This works because VISIBLE < INVISIBLE < GONE.
    return std::max(vis1, vis2);
}

const Display* View::getDisplay() const{
    return mAttachInfo ? mAttachInfo->mDisplay : nullptr;
}

void View::dispatchAttachedToWindow(AttachInfo*info,int visibility){
    mAttachInfo = info;
    if(mOverlay)
        mOverlay->getOverlayView()->dispatchAttachedToWindow(info,visibility);

    mWindowAttachCount++;
    mPrivateFlags |= PFLAG_DRAWABLE_STATE_DIRTY;
    if(mFloatingTreeObserver){
        info->mTreeObserver->merge(*mFloatingTreeObserver);
        delete mFloatingTreeObserver;
        mFloatingTreeObserver = nullptr;
    }
    onAttachedToWindow();
    int vis = info->mWindowVisibility;
    if (vis != GONE) {
        onWindowVisibilityChanged(vis);
        if (isShown()) {
            // Calling onVisibilityAggregated directly here since the subtree will also
            // receive dispatchAttachedToWindow and this same call
            onVisibilityAggregated(vis == VISIBLE);
        }
    }

    if ((mPrivateFlags&PFLAG_SCROLL_CONTAINER) != 0) {
        mAttachInfo->mScrollContainers.push_back(this);
        mPrivateFlags |= PFLAG_SCROLL_CONTAINER_ADDED;
    }
    // Send onVisibilityChanged directly instead of dispatchVisibilityChanged.
    // As all views in the subtree will already receive dispatchAttachedToWindow
    // traversing the subtree again here is not desired.
    onVisibilityChanged(*this, visibility);

    if ((mPrivateFlags&PFLAG_DRAWABLE_STATE_DIRTY) != 0) {
        // If nobody has evaluated the drawable state yet, then do it now.
        refreshDrawableState();
    }
}

void View::dispatchDetachedFromWindow(){
    if(mAttachInfo!=nullptr){
        const int vis = mAttachInfo->mWindowVisibility;
        if(vis !=GONE){
            onWindowVisibilityChanged(GONE);
            if(isShown()){
                onVisibilityAggregated(false);
            }
        }
    }
    onDetachedFromWindow();
    onDetachedFromWindowInternal();
    InputMethodManager&imm=InputMethodManager::getInstance();
    imm.onViewDetachedFromWindow(this);

    if(mListenerInfo){
        for(auto l:mListenerInfo->mOnAttachStateChangeListeners){
            if(l.onViewDetachedFromWindow)l.onViewDetachedFromWindow(*this);
        }
    }
	
    if ((mPrivateFlags & PFLAG_SCROLL_CONTAINER_ADDED) != 0) {
        std::vector<View*>&conts=mAttachInfo->mScrollContainers;
        auto it=std::find(conts.begin(),conts.end(),this);
        if(it!=conts.end())
            conts.erase(it);
        mPrivateFlags &= ~PFLAG_SCROLL_CONTAINER_ADDED;
    }
    if(mScrollCache&&mScrollCache->mRunner){
        removeCallbacks(mScrollCache->mRunner);
        //mScrollCache->mRunner.reset();
        /*reset will caused crash on remove views in some case*/
    }
    mAttachInfo = nullptr;
    if(mOverlay)
        mOverlay->getOverlayView()->dispatchDetachedFromWindow();
}

void View::onDetachedFromWindowInternal() {
    mPrivateFlags  &= ~PFLAG_CANCEL_NEXT_UP_EVENT;
    mPrivateFlags3 &= ~PFLAG3_IS_LAID_OUT;
    mPrivateFlags3 &= ~PFLAG3_TEMPORARY_DETACH;

    removeTapCallback();
    removeUnsetPressCallback();
    removeLongPressCallback();
    removePerformClickCallback();
    //cancel(mSendViewScrolledAccessibilityEvent);
    stopNestedScroll();

    // Anything that started animating right before detach should already
    // be in its final state when re-attached.
    jumpDrawablesToCurrentState();

    destroyDrawingCache();

    cleanupDraw();
    delete mCurrentAnimation;
    mCurrentAnimation = nullptr;

    if ((mViewFlags & TOOLTIP) == TOOLTIP) {
        hideTooltip();
    }
}

void View::cancelPendingInputEvents(){
    dispatchCancelPendingInputEvents();
}

void View::dispatchCancelPendingInputEvents() {
    mPrivateFlags3 &= ~PFLAG3_CALLED_SUPER;
    onCancelPendingInputEvents();
    if ((mPrivateFlags3 & PFLAG3_CALLED_SUPER) != PFLAG3_CALLED_SUPER) {
        "View did not call through to super.onCancelPendingInputEvents()";
    }
}

void View::onCancelPendingInputEvents() {
    removePerformClickCallback();
    cancelLongPress();
    mPrivateFlags3 |= PFLAG3_CALLED_SUPER;
}

void View::saveHierarchyState(SparseArray<Parcelable*>& container){
    dispatchSaveInstanceState(container);
}

void View::dispatchSaveInstanceState(SparseArray<Parcelable*>& container){
    if (mID != NO_ID && (mViewFlags & SAVE_DISABLED_MASK) == 0) {
        mPrivateFlags &= ~PFLAG_SAVE_STATE_CALLED;
        Parcelable* state = onSaveInstanceState();
        if ((mPrivateFlags & PFLAG_SAVE_STATE_CALLED) == 0) {
            LOGE("Derived class did not call super.onSaveInstanceState()");
        }
        if (state!=nullptr){
            LOGI("Freezing %d %p",mID,state);
            container.put(mID, state);
        }
    }
}

Parcelable* View::onSaveInstanceState(){
    mPrivateFlags |= PFLAG_SAVE_STATE_CALLED;
    if (mStartActivityRequestWho.size() || isAutofilled()
            || mAutofillViewId > LAST_APP_AUTOFILL_ID) {
        BaseSavedState* state = new BaseSavedState(&AbsSavedState::EMPTY_STATE);

        if (mStartActivityRequestWho.size()) {
            state->mSavedData |= BaseSavedState::START_ACTIVITY_REQUESTED_WHO_SAVED;
        }

        if (isAutofilled()) {
            state->mSavedData |= BaseSavedState::IS_AUTOFILLED;
        }

        if (mAutofillViewId > LAST_APP_AUTOFILL_ID) {
            state->mSavedData |= BaseSavedState::AUTOFILL_ID;
        }

        state->mStartActivityRequestWhoSaved = mStartActivityRequestWho;
        state->mIsAutofilled = isAutofilled();
        state->mAutofillViewId = mAutofillViewId;
        return state;
    }
    return &BaseSavedState::EMPTY_STATE;
}

void View::restoreHierarchyState(SparseArray<Parcelable*>& container){
    dispatchRestoreInstanceState(container);
}

void View::dispatchRestoreInstanceState(SparseArray<Parcelable*>& container){
    if (mID != NO_ID) {
        auto state= container.get(mID);
        if (state != nullptr) {
            LOGD("View %p:%d Restoreing #",this,mID,state);
            mPrivateFlags &= ~PFLAG_SAVE_STATE_CALLED;
            onRestoreInstanceState(*state);
            if ((mPrivateFlags & PFLAG_SAVE_STATE_CALLED) == 0) {
                FATAL("Derived class did not call super.onRestoreInstanceState()");
            }
        }
    }
}

void View::onRestoreInstanceState(Parcelable& state){
#if 0
    mPrivateFlags |= PFLAG_SAVE_STATE_CALLED;
    if (state != null && !(state instanceof AbsSavedState)) {
        throw new IllegalArgumentException("Wrong state class, expecting View State but "
                + "received " + state.getClass().toString() + " instead. This usually happens "
                + "when two views of different type have the same id in the same hierarchy. "
                + "This view's id is " + ViewDebug.resolveId(mContext, getId()) + ". Make sure "
                + "other views do not use the same id.");
    }
    if (state != null && state instanceof BaseSavedState) {
        BaseSavedState baseState = (BaseSavedState) state;

        if ((baseState.mSavedData & BaseSavedState.START_ACTIVITY_REQUESTED_WHO_SAVED) != 0) {
            mStartActivityRequestWho = baseState.mStartActivityRequestWhoSaved;
        }
        if ((baseState.mSavedData & BaseSavedState.IS_AUTOFILLED) != 0) {
            setAutofilled(baseState.mIsAutofilled);
        }
        if ((baseState.mSavedData & BaseSavedState.AUTOFILL_ID) != 0) {
            mAutofillViewId = baseState.mAutofillViewId;
        }
    }
#endif
}

void View::onWindowVisibilityChanged(int visibility) {
    if (visibility == VISIBLE) {
        initialAwakenScrollBars();
    }
}

bool View::dispatchVisibilityAggregated(bool isVisible) {
    bool thisVisible = getVisibility() == VISIBLE;
    // If we're not visible but something is telling us we are, ignore it.
    if (thisVisible || !isVisible) {
        onVisibilityAggregated(isVisible);
    }
    return thisVisible && isVisible;
}

void View::onVisibilityAggregated(bool isVisible) {
    // Update our internal visibility tracking so we can detect changes
    bool oldVisible = (mPrivateFlags3 & PFLAG3_AGGREGATED_VISIBLE) != 0;
    mPrivateFlags3 = isVisible ? (mPrivateFlags3 | PFLAG3_AGGREGATED_VISIBLE)
            : (mPrivateFlags3 & ~PFLAG3_AGGREGATED_VISIBLE);
    if (isVisible && mAttachInfo != nullptr) {
        initialAwakenScrollBars();
    }

    Drawable*dr= mBackground;
    if (dr && isVisible != dr->isVisible()) {
        dr->setVisible(isVisible, false);
    }
    dr = mDefaultFocusHighlight;
    if (dr && isVisible != dr->isVisible()) {
        dr->setVisible(isVisible, false);
    }
    dr = mForegroundInfo ? mForegroundInfo->mDrawable : nullptr;
    if (dr && isVisible != dr->isVisible()) {
        dr->setVisible(isVisible, false);
    }
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

void View::initializeScrollbarsInternal(const AttributeSet&a){
    initScrollCache();

    ScrollabilityCache* scrollabilityCache = mScrollCache;
 
    if (scrollabilityCache->scrollBar == nullptr) {
        scrollabilityCache->scrollBar = new ScrollBarDrawable();
        scrollabilityCache->scrollBar->setState(getDrawableState());
        scrollabilityCache->scrollBar->setCallback(this);
    }
 
    scrollabilityCache->fadeScrollBars = a.getBoolean("fadeScrollbars", true);
 
    if (!scrollabilityCache->fadeScrollBars) {
        scrollabilityCache->state = ScrollabilityCache::ON;
    }
 
    scrollabilityCache->scrollBarFadeDuration = a.getInt("scrollbarFadeDuration", ViewConfiguration::getScrollBarFadeDuration());
    scrollabilityCache->scrollBarDefaultDelayBeforeFade = a.getInt("scrollbarDefaultDelayBeforeFade",ViewConfiguration::getScrollDefaultDelay());
 
 
    scrollabilityCache->scrollBarSize = a.getDimensionPixelSize("scrollbarSize",ViewConfiguration::get(mContext).getScaledScrollBarSize());
 
    Drawable* track = a.getDrawable("scrollbarTrackHorizontal");
    scrollabilityCache->scrollBar->setHorizontalTrackDrawable(track);
 
    Drawable* thumb = a.getDrawable("scrollbarThumbHorizontal");
    if (thumb) {
        scrollabilityCache->scrollBar->setHorizontalThumbDrawable(thumb);
    }
 
    bool alwaysDraw = a.getBoolean("scrollbarAlwaysDrawHorizontalTrack",false);
    if (alwaysDraw) {
        scrollabilityCache->scrollBar->setAlwaysDrawHorizontalTrack(true);
    }
 
    track = a.getDrawable("scrollbarTrackVertical");
    scrollabilityCache->scrollBar->setVerticalTrackDrawable(track);
 
    thumb = a.getDrawable("scrollbarThumbVertical");
    if (thumb) {
        scrollabilityCache->scrollBar->setVerticalThumbDrawable(thumb);
    }
 
    alwaysDraw = a.getBoolean("scrollbarAlwaysDrawVerticalTrack",false);
    if (alwaysDraw) {
        scrollabilityCache->scrollBar->setAlwaysDrawVerticalTrack(true);
    }
 
    // Apply layout direction to the new Drawables if needed
    const int layoutDirection = getLayoutDirection();
    if (track) {
        track->setLayoutDirection(layoutDirection);
    }
    if (thumb) {
        thumb->setLayoutDirection(layoutDirection);
    }
 
    // Re-apply user/background padding so that scrollbar(s) get added
    resolvePadding(); 
}

void View::initScrollCache(){
    if(mScrollCache == nullptr)
        mScrollCache = new ScrollabilityCache(ViewConfiguration::get(mContext),this);
}

void View::initializeScrollBarDrawable(){
    initScrollCache();

    if (mScrollCache->scrollBar == nullptr) {
        mScrollCache->scrollBar = new ScrollBarDrawable();
        mScrollCache->scrollBar->setState(getDrawableState());
        mScrollCache->scrollBar->setCallback(this);
    }
}

View::ScrollabilityCache*View::getScrollCache(){
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

void View::setScrollBarStyle(int style) {
    if (style != (mViewFlags & SCROLLBARS_STYLE_MASK)) {
        mViewFlags = (mViewFlags & ~SCROLLBARS_STYLE_MASK) | (style & SCROLLBARS_STYLE_MASK);
        computeOpaqueFlags();
        resolvePadding();
    }
}

int View::getScrollBarStyle()const{
    return mViewFlags & SCROLLBARS_STYLE_MASK;
}

bool View::isVerticalScrollBarHidden()const{
    return false;
}

void View::setScrollbarFadingEnabled(bool fadeScrollbars) {
    initScrollCache();
    mScrollCache->fadeScrollBars = fadeScrollbars;
    if (fadeScrollbars) {
        mScrollCache->state = ScrollabilityCache::OFF;
    } else {
        mScrollCache->state = ScrollabilityCache::ON;
    }
}

bool View::isScrollbarFadingEnabled() {
    return mScrollCache  && mScrollCache->fadeScrollBars;
}

int View::getScrollBarDefaultDelayBeforeFade() {
    return mScrollCache == nullptr ? ViewConfiguration::getScrollDefaultDelay() :
            mScrollCache->scrollBarDefaultDelayBeforeFade;
}

void View::setScrollBarDefaultDelayBeforeFade(int scrollBarDefaultDelayBeforeFade) {
    getScrollCache()->scrollBarDefaultDelayBeforeFade = scrollBarDefaultDelayBeforeFade;
}

int View::getScrollBarFadeDuration() {
    return mScrollCache == nullptr ? ViewConfiguration::getScrollBarFadeDuration() :
            mScrollCache->scrollBarFadeDuration;
}

void View::setScrollBarFadeDuration(int scrollBarFadeDuration) {
    getScrollCache()->scrollBarFadeDuration = scrollBarFadeDuration;
}

void View::setVerticalScrollbarThumbDrawable(Drawable* drawable){
    initializeScrollBarDrawable();
    mScrollCache->scrollBar->setVerticalThumbDrawable(drawable);
}

void View::setVerticalScrollbarTrackDrawable(Drawable* drawable){
    initializeScrollBarDrawable();
    mScrollCache->scrollBar->setVerticalTrackDrawable(drawable);
}

void View::setHorizontalScrollbarThumbDrawable(Drawable* drawable){
    initializeScrollBarDrawable();
    mScrollCache->scrollBar->setHorizontalThumbDrawable(drawable);
}

void View::setHorizontalScrollbarTrackDrawable(Drawable* drawable){
    initializeScrollBarDrawable();
    mScrollCache->scrollBar->setHorizontalTrackDrawable(drawable);
}

Drawable* View::getVerticalScrollbarThumbDrawable()const{
    return mScrollCache ? mScrollCache->scrollBar->getVerticalThumbDrawable() : nullptr;
}

Drawable* View::getVerticalScrollbarTrackDrawable()const{
    return mScrollCache  ? mScrollCache->scrollBar->getVerticalTrackDrawable() : nullptr;
}

Drawable* View::getHorizontalScrollbarThumbDrawable()const{
    return mScrollCache ? mScrollCache->scrollBar->getHorizontalThumbDrawable() : nullptr;
}

Drawable* View::getHorizontalScrollbarTrackDrawable()const{
    return mScrollCache ? mScrollCache->scrollBar->getHorizontalTrackDrawable() : nullptr;
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
        computeOpaqueFlags();
        resolvePadding();
        awakenScrollBars(0,false);
    }
    return *this;
}

bool View::isScrollContainer()const{
    return (mPrivateFlags & PFLAG_SCROLL_CONTAINER_ADDED) != 0;
}

void View::setScrollContainer(bool isScrollContainer){
    if (isScrollContainer) {
        if (mAttachInfo  && (mPrivateFlags&PFLAG_SCROLL_CONTAINER_ADDED) == 0) {
            mAttachInfo->mScrollContainers.push_back(this);
            mPrivateFlags |= PFLAG_SCROLL_CONTAINER_ADDED;
        }
        mPrivateFlags |= PFLAG_SCROLL_CONTAINER;
    } else {
        if ((mPrivateFlags&PFLAG_SCROLL_CONTAINER_ADDED) != 0) {
            std::vector<View*>&conts = mAttachInfo->mScrollContainers;
            auto it=std::find(conts.begin(),conts.end(),this);
            if(it != conts.end())
                conts.erase(it);
        }
        mPrivateFlags &= ~(PFLAG_SCROLL_CONTAINER|PFLAG_SCROLL_CONTAINER_ADDED);
    }
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
        computeOpaqueFlags();
        resolvePadding();
        awakenScrollBars(0,false);
    }
    return *this;
}


void View::getVerticalScrollBarBounds(Rect*bounds,Rect*touchBounds){
    if (mRoundScrollbarRenderer == nullptr) {
        getStraightVerticalScrollBarBounds(bounds,touchBounds);
    } else {
        getRoundVerticalScrollBarBounds(bounds != nullptr ? bounds : touchBounds);
    }
}

void View::getHorizontalScrollBarBounds(Rect*drawBounds,Rect*touchBounds){
    Rect* bounds = drawBounds != nullptr ? drawBounds : touchBounds;
    if (bounds == nullptr)return;

    const int inside = (mViewFlags & SCROLLBARS_OUTSIDE_MASK) == 0 ? ~0 : 0;
    const bool drawVerticalScrollBar = isVerticalScrollBarEnabled()&& !isVerticalScrollBarHidden();
    const int size   = getHorizontalScrollbarHeight();
    const int verticalScrollBarGap = drawVerticalScrollBar ? getVerticalScrollbarWidth() : 0;
    const int width  = getWidth();
    const int height = getHeight();
    bounds->top   = mScrollY + height - size - (mUserPaddingBottom & inside);
    bounds->left  = mScrollX + (mPaddingLeft & inside);
    bounds->width = width - (mPaddingLeft & inside) - (mUserPaddingRight & inside) - verticalScrollBarGap;
    bounds->height= size;

    if (touchBounds == nullptr)return;
    if (*touchBounds != *bounds) {
        *touchBounds=*bounds;
    }
    int minTouchTarget = mScrollCache->scrollBarMinTouchTarget;
    if (touchBounds->height < minTouchTarget) {
        const int adjust = (minTouchTarget - touchBounds->height) / 2;
        touchBounds->height = std::min(touchBounds->height + adjust,height);
        touchBounds->top = touchBounds->bottom() - minTouchTarget;
    }
    if (touchBounds->width < minTouchTarget) {
        const int adjust = (minTouchTarget - touchBounds->width) / 2;
        touchBounds->left -= adjust;
        touchBounds->width =  minTouchTarget;
    }
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

void View::getStraightVerticalScrollBarBounds(Rect*drawBounds,Rect*touchBounds){
    Rect*bounds = drawBounds != nullptr ? drawBounds : touchBounds;
    if (bounds == nullptr) return;
    const int inside = (mViewFlags & SCROLLBARS_OUTSIDE_MASK) == 0 ? ~0 : 0;
    const int size = getVerticalScrollbarWidth();
    int verticalScrollbarPosition = mVerticalScrollbarPosition;
    if (verticalScrollbarPosition ==SCROLLBAR_POSITION_DEFAULT) {
        verticalScrollbarPosition = isLayoutRtl() ? SCROLLBAR_POSITION_LEFT : SCROLLBAR_POSITION_RIGHT;
    }
    switch (verticalScrollbarPosition) {
    default:
    case SCROLLBAR_POSITION_RIGHT:
        bounds->left = mScrollX + getWidth() - size - (mUserPaddingRight & inside);
        break;
    case SCROLLBAR_POSITION_LEFT:
        bounds->left = mScrollX + (mUserPaddingLeft & inside);
        break;
    }
    bounds->top   = mScrollY+ (mPaddingTop & inside);
    bounds->width = size;
    bounds->height= getHeight() - (mUserPaddingBottom & inside) -(mPaddingTop & inside);

    if (touchBounds == nullptr) return;
    if (touchBounds != bounds) {
        *touchBounds = *bounds;
    }
    int minTouchTarget = mScrollCache->scrollBarMinTouchTarget;
    if (touchBounds->width < minTouchTarget) {
        const int adjust = (minTouchTarget - touchBounds->width) / 2;
        if (verticalScrollbarPosition == SCROLLBAR_POSITION_RIGHT) {
            touchBounds->width= std::min(touchBounds->width + adjust, getWidth());
            touchBounds->left = touchBounds->width - minTouchTarget;
        } else {
            touchBounds->left = std::max(touchBounds->left+ adjust, mScrollX);
            touchBounds->width= minTouchTarget;
        }
    }
    if (touchBounds->height < minTouchTarget) {
        const int adjust = (minTouchTarget - touchBounds->height) / 2;
        touchBounds->top -= adjust;
        touchBounds->height = minTouchTarget;
    }
}

void View::getRoundVerticalScrollBarBounds(Rect* bounds){
    // Do not take padding into account as we always want the scrollbars
    // to hug the screen for round wearable devices.
    bounds->left = mScrollX;
    bounds->top = mScrollY;
    bounds->width =  mRight-mLeft;
    bounds->height = mBottom -mTop;
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
        computeOpaqueFlags();
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
    const int offset = computeHorizontalScrollOffset();
    const int range = computeHorizontalScrollRange() - computeHorizontalScrollExtent();
    if (range == 0) return false;
    if (direction < 0) {
        return offset > 0;
    } else {
        return offset < range - 1;
    }
}

bool View::canScrollVertically(int direction){
    const int offset = computeVerticalScrollOffset();
    const int range = computeVerticalScrollRange() - computeVerticalScrollExtent();
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
        Rect& touchBounds = mScrollCache->mScrollBarTouchBounds;
        getVerticalScrollBarBounds(nullptr,&touchBounds);
        if (touchBounds.contains(x,y)) {
            return true;
        }
    }
    if (isHorizontalScrollBarEnabled()) {
        Rect& touchBounds = mScrollCache->mScrollBarTouchBounds;
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
        Rect& bounds = mScrollCache->mScrollBarBounds;
        Rect& touchBounds = mScrollCache->mScrollBarTouchBounds;
        getVerticalScrollBarBounds(&bounds, &touchBounds);
        const int range = computeVerticalScrollRange();
        const int offset = computeVerticalScrollOffset();
        const int extent = computeVerticalScrollExtent();
        const int thumbLength = ViewConfiguration::getThumbLength(bounds.height, bounds.width,extent, range);
        const int thumbOffset = ViewConfiguration::getThumbOffset(bounds.height, thumbLength, extent, range, offset);
        const int thumbTop = bounds.top + thumbOffset;
        const int adjust = std::max(mScrollCache->scrollBarMinTouchTarget - thumbLength, 0) / 2;
        if (x >= touchBounds.left && x <= touchBounds.right()
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
        Rect& bounds = mScrollCache->mScrollBarBounds;
        Rect& touchBounds = mScrollCache->mScrollBarTouchBounds;
        getHorizontalScrollBarBounds(&bounds, &touchBounds);
        const int range = computeHorizontalScrollRange();
        const int offset = computeHorizontalScrollOffset();
        const int extent = computeHorizontalScrollExtent();
        const int thumbLength = ViewConfiguration::getThumbLength(bounds.width, bounds.height,extent, range);
        const int thumbOffset = ViewConfiguration::getThumbOffset(bounds.width, thumbLength,extent, range, offset);
        const int thumbLeft = bounds.left + thumbOffset;
        const int adjust = std::max(mScrollCache->scrollBarMinTouchTarget - thumbLength, 0) / 2;
        if (x >= thumbLeft - adjust && x <= thumbLeft + thumbLength + adjust
                && y >= touchBounds.top && y <= touchBounds.bottom()) {
            return true;
        }
    }
    return false;
}

void View::initializeScrollIndicatorsInternal(){
    if (mScrollIndicatorDrawable == nullptr) {
        mScrollIndicatorDrawable = mContext->getDrawable("cdroid:drawable/scroll_indicator_material");
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
        invalidate(true);
    }
}

void View::getScrollIndicatorBounds(Rect&out) {
    out.left     = mScrollX;
    out.width = getWidth();
    out.top     = mScrollY;
    out.height= getHeight();
}

void View::onDrawScrollIndicators(Canvas& canvas){
    if ((mPrivateFlags3 & SCROLL_INDICATORS_PFLAG3_MASK) == 0)
        return;// No scroll indicators enabled.

    Drawable* dr = mScrollIndicatorDrawable;
    if (dr == nullptr)
        return;// Scroll indicators aren't supported here.

    Rect rect ;
    const int h = dr->getIntrinsicHeight();
    const int w = dr->getIntrinsicWidth();

    getScrollIndicatorBounds(rect);
    if ((mPrivateFlags3 & PFLAG3_SCROLL_INDICATOR_TOP) != 0) {
        bool canScrollUp = canScrollVertically(-1);
        if (canScrollUp) {
            dr->setBounds(rect.left, rect.top, rect.width,h);
            dr->draw(canvas);
        }
    }

    if ((mPrivateFlags3 & PFLAG3_SCROLL_INDICATOR_BOTTOM) != 0) {
        bool canScrollDown = canScrollVertically(1);
        if (canScrollDown) {
            dr->setBounds(rect.left, rect.bottom() - h, rect.width, rect.height);
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

    const int leftMask = PFLAG3_SCROLL_INDICATOR_LEFT | leftRtl;
    if ((mPrivateFlags3 & leftMask) != 0) {
        bool canScrollLeft = canScrollHorizontally(-1);
        if (canScrollLeft) {
            dr->setBounds(rect.left, rect.top, w, rect.height);
            dr->draw(canvas);
        }
    }

    const int rightMask = PFLAG3_SCROLL_INDICATOR_RIGHT | rightRtl;
    if ((mPrivateFlags3 & rightMask) != 0) {
        const bool canScrollRight = canScrollHorizontally(1);
        if (canScrollRight) {
            dr->setBounds(rect.right() - w, rect.top,w, rect.height);
            dr->draw(canvas);
        }
    }
    rect=dr->getBounds();
}

void View::onDrawScrollBars(Canvas& canvas){
    ScrollabilityCache* cache = mScrollCache;
    if(cache == nullptr||cache->state==ScrollabilityCache::OFF) 
        return;

    bool bInvalidate = false;
    long now = AnimationUtils::currentAnimationTimeMillis();
    if (cache->state == ScrollabilityCache::FADING){
        // We're fading -- get our fade interpolation

        // Stops the animation if we're done
        if (now-cache->fadeStartTime>cache->scrollBarFadeDuration){
            cache->state = ScrollabilityCache::OFF;
        } else {
            const int alpha=255-255*(now-cache->fadeStartTime)/cache->scrollBarFadeDuration;
            cache->scrollBar->mutate()->setAlpha(alpha);
        }
        // This will make the scroll bars inval themselves after drawing. We only 
        // want this when we're fading so that we prevent excessive redraws
        bInvalidate = true;
    }else {//now < fadeStartTime
        // We're just on -- but we may have been fading before so reset alpha
        cache->scrollBar->mutate()->setAlpha(255);
    }

    const bool drawHorizontalScrollBar = isHorizontalScrollBarEnabled();
    const bool drawVerticalScrollBar = isVerticalScrollBarEnabled() && !isVerticalScrollBarHidden();
    
    // Fork out the scroll bar drawing for round wearable devices.
    if (mRoundScrollbarRenderer != nullptr) {
        if (drawVerticalScrollBar) {
            Rect& bounds = mScrollCache->mScrollBarBounds;
            getVerticalScrollBarBounds(&bounds, nullptr);
            mRoundScrollbarRenderer->drawRoundScrollbars(
                canvas, (float)mScrollCache->scrollBar->getAlpha() / 255.f, bounds);
            if (bInvalidate) invalidate(true);
        }
        // Do not draw horizontal scroll bars for round wearable devices.
    } else if ( drawVerticalScrollBar || drawHorizontalScrollBar) {
        ScrollBarDrawable* scrollBar = mScrollCache->scrollBar;
        if (drawHorizontalScrollBar) {
            scrollBar->setParameters(computeHorizontalScrollRange(),
                computeHorizontalScrollOffset(),computeHorizontalScrollExtent(), false);
            Rect& bounds = mScrollCache->mScrollBarBounds;
            getHorizontalScrollBarBounds(&bounds, nullptr);
            onDrawHorizontalScrollBar(canvas,scrollBar, bounds);
            if (bInvalidate) invalidate(bounds);
        }
        if (drawVerticalScrollBar) {
            scrollBar->setParameters(computeVerticalScrollRange(),
                computeVerticalScrollOffset(),computeVerticalScrollExtent(), true);
            Rect& bounds = mScrollCache->mScrollBarBounds;
            getVerticalScrollBarBounds(&bounds, nullptr);
            onDrawVerticalScrollBar(canvas, scrollBar, bounds);
            if (bInvalidate) invalidate(bounds);
        }
    }
}

void View::onDrawHorizontalScrollBar(Canvas& canvas, Drawable* scrollBar,const Rect&rect){
    scrollBar->setBounds(rect);
    scrollBar->draw(canvas);
}

void View::onDrawVerticalScrollBar (Canvas& canvas , Drawable* scrollBar,const Rect&rect){
    scrollBar->setBounds(rect);
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

View::ListenerInfo*View::getListenerInfo(){
    if(mListenerInfo==nullptr)
        mListenerInfo = new ListenerInfo();
    return mListenerInfo;
}

void View::setOnClickListener(OnClickListener l){
    if(!isClickable())
        setClickable(true);
    getListenerInfo()->mOnClickListener=l;
}

bool View::hasClickListener()const{
    return mListenerInfo&&mListenerInfo->mOnClickListener!=nullptr;
}

void View::setOnLongClickListener(OnLongClickListener l){
    if(!isLongClickable())
        setLongClickable(true);
    getListenerInfo()->mOnLongClickListener=l;
}

void View::setOnFocusChangeListener(OnFocusChangeListener listtener){
    getListenerInfo()->mOnFocusChangeListener = listtener;
}

void View::addOnLayoutChangeListener(OnLayoutChangeListener listener){
    getListenerInfo()->mOnLayoutChangeListeners.push_back(listener);
}

void View::removeOnLayoutChangeListener(OnLayoutChangeListener listener){
    if(mListenerInfo){
        std::vector<View::OnLayoutChangeListener>&ls= mListenerInfo->mOnLayoutChangeListeners;
        auto it= std::find(ls.begin(),ls.end(),listener);
        ls.erase(it);
    }
}

void View::addOnAttachStateChangeListener(OnAttachStateChangeListener listener) {
    ListenerInfo* li = getListenerInfo();
    if (li) {
        std::vector<OnAttachStateChangeListener>&ls = li->mOnAttachStateChangeListeners;
        ls.push_back(listener);
    }
}

static bool operator == (const View::OnAttachStateChangeListener& left, const View::OnAttachStateChangeListener& right){
    return (left.onViewAttachedToWindow==right.onViewAttachedToWindow) &&
           (left.onViewDetachedFromWindow==right.onViewDetachedFromWindow);
}

void View::removeOnAttachStateChangeListener(OnAttachStateChangeListener listener) {
    if (mListenerInfo == nullptr || mListenerInfo->mOnAttachStateChangeListeners.empty()) {
        return;
    }
    std::vector<OnAttachStateChangeListener>&ls = mListenerInfo->mOnAttachStateChangeListeners;
    auto it = std::find(ls.begin(),ls.end(),listener);
    if(it != ls.end()){
        mListenerInfo->mOnAttachStateChangeListeners.erase(it);
    }
}

void View::startActivityForResult(Intent intent, int requestCode){
    mStartActivityRequestWho="cdroid:view";
}

bool View::dispatchActivityResult(const std::string& who, int requestCode, int resultCode, Intent data){
    if (mStartActivityRequestWho.compare(who)==0) {
        onActivityResult(requestCode, resultCode, data);
        mStartActivityRequestWho.erase();
        return true;
    }
    return false;
}

void View::onActivityResult(int requestCode, int resultCode, Intent data){
}

void View::setOnScrollChangeListener(OnScrollChangeListener l){
    getListenerInfo()->mOnScrollChangeListener=l;
}

void View::setOnKeyListener(OnKeyListener l){
    getListenerInfo()->mOnKeyListener = l;
}

void View::setOnTouchListener(OnTouchListener l){
    getListenerInfo()->mOnTouchListener = l;
}

void View::setOnGenericMotionListener(OnGenericMotionListener l){
    getListenerInfo()->mOnGenericMotionListener = l;
}

void View::setOnHoverListener(OnHoverListener l){
    getListenerInfo()->mOnHoverListener = l;
}

/*void View::setOnDragListener(OnDragListener l){
    getListenerInfo()->
}*/

void View::setDrawingCacheEnabled(bool enabled) {
    mCachingFailed = false;
    //setFlags(enabled ? DRAWING_CACHE_ENABLED : 0, DRAWING_CACHE_ENABLED);
}

bool View::isDrawingCacheEnabled()const{
    return (mViewFlags & DRAWING_CACHE_ENABLED) == DRAWING_CACHE_ENABLED;
}


bool View::isPaddingOffsetRequired() {
    return false;
}

/**
 * Amount by which to extend the left fading region. Called only when
 * {@link #isPaddingOffsetRequired()} returns true.
 *
 * @return The left padding offset in pixels.
 *
 * @see #isPaddingOffsetRequired()
 *
 * @since CURRENT
 */
int View::getLeftPaddingOffset() {
    return 0;
}

/**
 * Amount by which to extend the right fading region. Called only when
 * {@link #isPaddingOffsetRequired()} returns true.
 *
 * @return The right padding offset in pixels.
 *
 * @see #isPaddingOffsetRequired()
 *
 * @since CURRENT
 */
int View::getRightPaddingOffset() {
    return 0;
}

/**
 * Amount by which to extend the top fading region. Called only when
 * {@link #isPaddingOffsetRequired()} returns true.
 *
 * @return The top padding offset in pixels.
 *
 * @see #isPaddingOffsetRequired()
 *
 * @since CURRENT
 */
int View::getTopPaddingOffset() {
    return 0;
}

/**
 * Amount by which to extend the bottom fading region. Called only when
 * {@link #isPaddingOffsetRequired()} returns true.
 *
 * @return The bottom padding offset in pixels.
 *
 * @see #isPaddingOffsetRequired()
 *
 * @since CURRENT
 */
int View::getBottomPaddingOffset() {
    return 0;
}

/**
 * @hide
 * @param offsetRequired
 */
int View::getFadeTop(bool offsetRequired) {
    int top = mPaddingTop;
    if (offsetRequired) top += getTopPaddingOffset();
    return top;
}

/**
 * @hide
 * @param offsetRequired
 */
int View::getFadeHeight(bool offsetRequired) {
    int padding = mPaddingTop;
    if (offsetRequired) padding += getTopPaddingOffset();
    return (mBottom-mTop) - mPaddingBottom - padding;
}

bool View::isHardwareAccelerated()const{
    return mAttachInfo && mAttachInfo->mHardwareAccelerated;
}

void View::setClipBounds(const Rect*clipBounds){
    if (clipBounds != nullptr) {
        mClipBounds=*clipBounds;
    } else {
        mClipBounds.setEmpty();
    }
    //mRenderNode->setClipRect(mClipBounds);
    invalidateViewProperty(false, false);
}

bool View::getClipBounds(Rect&outRect){
    if(!mClipBounds.empty()){
	outRect=mClipBounds;
	return true;
    }
    return false;
}

void View::dispatchDraw(Canvas&){
    //for inherited view(container) to draw children...
}

void View::drawBackground(Canvas&canvas){
    if(mBackground==nullptr)
        return ;
    mBackground->setBounds(0, 0,getWidth(),getHeight());
    mBackgroundSizeChanged =false;
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
    onDrawScrollBars(canvas);
    Drawable*foreground=mForegroundInfo != nullptr ? mForegroundInfo->mDrawable : nullptr;
    if(foreground){
        if (mForegroundInfo->mBoundsChanged) {
            mForegroundInfo->mBoundsChanged = false;
            Rect& selfBounds = mForegroundInfo->mSelfBounds;
            Rect& overlayBounds = mForegroundInfo->mOverlayBounds;

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
    const int flags = parent->mGroupFlags;
    const bool initialized = a->isInitialized();
    if (!initialized) {
        a->initialize(mRight-mLeft, mBottom-mTop, parent->getWidth(), parent->getHeight());
        a->initializeInvalidateRegion(0, 0, mRight - mLeft, mBottom - mTop);
        //if (mAttachInfo != nullptr) a->setListenerHandler(mAttachInfo->mHandler);
        onAnimationStart();
    }

    Transformation* t = parent->getChildTransformation();
    bool more = a->getTransformation(drawingTime, *t, 1.f);
    if (scalingRequired && mAttachInfo->mApplicationScale != 1.f) {
        if (parent->mInvalidationTransformation == nullptr) {
            parent->mInvalidationTransformation = new Transformation();
        }
        invalidationTransform = parent->mInvalidationTransformation;
        a->getTransformation(drawingTime, *invalidationTransform, 1.f);
        const Matrix&m=invalidationTransform->getMatrix();
        LOGV("matrix=%f,%f,%f,%f,%f,%f",m.xx,m.yy,m.xy,m.yx,m.x0,m.y0);
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
                parent->invalidate(mLeft, mTop, mRight-mLeft, mBottom-mTop);
            }
        } else {
            //if (parent->mInvalidateRegion == nullptr) {
            //    parent->mInvalidateRegion = new RectF();
            //}
            Rect region ;//= parent->mInvalidateRegion;
            a->getInvalidateRegion(0, 0, mRight-mLeft, mBottom-mTop, region,*invalidationTransform);

            // The child need to draw an animation, potentially offscreen, so
            // make sure we do not cancel invalidate requests
            parent->mPrivateFlags |= PFLAG_DRAW_ANIMATION;
            int left = mLeft + (int) region.left;
            int top = mTop + (int) region.top;
            parent->invalidate(left, top, region.width+1,region.height+1);
       }
    }
    return more;
}

void View::draw(Canvas&canvas){
    const int privateFlags = mPrivateFlags;
    const bool dirtyOpaque = (privateFlags & PFLAG_DIRTY_MASK) == PFLAG_DIRTY_OPAQUE &&
                    (mAttachInfo == nullptr || !mAttachInfo->mIgnoreDirtyState);
    mPrivateFlags = (privateFlags & ~PFLAG_DIRTY_MASK) | PFLAG_DRAWN;

    /*
     * Draw traversal performs several drawing steps which must be executed
     * in the appropriate order:
     *
     *      1. Draw the background
     *      2. If necessary, save the canvas' layers to prepare for fading
     *      3. Draw view's content
     *      4. Draw children
     *      5. If necessary, draw the fading edges and restore layers
     *      6. Draw decorations (scrollbars for instance)
     */

    // Step 1, draw the background, if needed

    if (!dirtyOpaque) {
        drawBackground(canvas);
    }

    // skip step 2 & 5 if possible (common case)
    const bool horizontalEdges = (mViewFlags & FADING_EDGE_HORIZONTAL) != 0;
    const bool verticalEdges = (mViewFlags & FADING_EDGE_VERTICAL) != 0;
    if (!verticalEdges && !horizontalEdges) {
        // Step 3, draw the content
        if (!dirtyOpaque) onDraw(canvas);

        // Step 4, draw the children
        dispatchDraw(canvas);

        drawAutofilledHighlight(canvas);

        // Overlay is part of the content and draws beneath Foreground
        if (mOverlay && !mOverlay->isEmpty()) {
            mOverlay->getOverlayView()->dispatchDraw(canvas);
        }

        // Step 6, draw decorations (foreground, scrollbars)
        onDrawForeground(canvas);

        // Step 7, draw the default focus highlight
        drawDefaultFocusHighlight(canvas);

        if (debugDraw()) debugDrawFocus(canvas);
        // we're done...
        return;
    }

    /* Here we do the full fledged routine...
     * (this is an uncommon case where speed matters less,
     * this is why we repeat some of the tests that have been
     * done above)*/

    bool drawTop = false;
    bool drawBottom = false;
    bool drawLeft = false;
    bool drawRight = false;

    float topFadeStrength = 0.0f;
    float bottomFadeStrength = 0.0f;
    float leftFadeStrength = 0.0f;
    float rightFadeStrength = 0.0f;

    // Step 2, save the canvas' layers
    int paddingLeft = mPaddingLeft;

    bool offsetRequired = isPaddingOffsetRequired();
    if (offsetRequired) {
        paddingLeft += getLeftPaddingOffset();
    }

    int left = mScrollX + paddingLeft;
    int right = left + (mRight - mLeft) - mPaddingRight - paddingLeft;
    int top = mScrollY + getFadeTop(offsetRequired);
    int bottom = top + getFadeHeight(offsetRequired);

    if (offsetRequired) {
        right += getRightPaddingOffset();
        bottom += getBottomPaddingOffset();
    }

    const float fadeHeight = getScrollCache()->fadingEdgeLength;
    int length = (int) fadeHeight;

    // clip the fade length if top and bottom fades overlap
    // overlapping fades produce odd-looking artifacts
    if (verticalEdges && (top + length > bottom - length)) {
        length = (bottom - top) / 2;
    }

    // also clip horizontal fades if necessary
    if (horizontalEdges && (left + length > right - length)) {
        length = (right - left) / 2;
    }

    if (verticalEdges) {
        topFadeStrength = std::max(0.0f, std::min(1.0f, getTopFadingEdgeStrength()));
        drawTop = topFadeStrength * fadeHeight > 1.0f;
        bottomFadeStrength = std::max(0.0f, std::min(1.0f, getBottomFadingEdgeStrength()));
        drawBottom = bottomFadeStrength * fadeHeight > 1.0f;
    }

    if (horizontalEdges) {
        leftFadeStrength = std::max(0.0f, std::min(1.0f, getLeftFadingEdgeStrength()));
        drawLeft = leftFadeStrength * fadeHeight > 1.0f;
        rightFadeStrength = std::max(0.0f, std::min(1.0f, getRightFadingEdgeStrength()));
        drawRight = rightFadeStrength * fadeHeight > 1.0f;
    }   

    // Step 3, draw the content
    if (!dirtyOpaque) onDraw(canvas);

    // Step 4, draw the children
    dispatchDraw(canvas);

    // Step 5, draw the fade effect and restore layers
    Cairo::RefPtr<LinearGradient>fade = getScrollCache()->shader;
    if (drawTop) {
        canvas.save();
        canvas.scale(1,fadeHeight * topFadeStrength);
        canvas.set_source(fade);
        canvas.rectangle(left, top,right-left,length);
        canvas.fill();
        canvas.restore();
    }

    if (drawBottom) {
        canvas.save();
        canvas.translate(right,bottom);
        canvas.rotate_degrees(-180);
        canvas.scale(1,fadeHeight * bottomFadeStrength);
        canvas.set_source(fade);
        canvas.rectangle(0,0,right-left,length);
        canvas.fill();
        canvas.restore();
    }

    if (drawLeft) {
        canvas.save();
        canvas.translate(left, bottom);
        canvas.rotate_degrees(-90);
        canvas.scale(1,fadeHeight * leftFadeStrength);
        canvas.set_source(fade);
        canvas.rectangle(0,0,bottom - top, length);
        canvas.fill();
        canvas.restore();
    }

    if (drawRight) {
        canvas.save();
        canvas.translate(right, top);
        canvas.rotate_degrees(90);
        canvas.scale(1, fadeHeight * rightFadeStrength);
        canvas.set_source(fade);
        canvas.rectangle(0,0,bottom - top,length);
        canvas.fill();
        canvas.restore();
    }
    // Step 6, draw decorations (foreground, scrollbars)
    drawAutofilledHighlight(canvas);
    if(mOverlay && !mOverlay->isEmpty())
        mOverlay->getOverlayView()->dispatchDraw(canvas);
    onDrawForeground(canvas);
    if (debugDraw()) debugDrawFocus(canvas);
}

bool View::draw(Canvas&canvas,ViewGroup*parent,long drawingTime){
    const bool hardwareAcceleratedCanvas=false;
    bool drawingWithRenderNode= mAttachInfo && mAttachInfo->mHardwareAccelerated  && hardwareAcceleratedCanvas;
    bool more = false;
    const bool childHasIdentityMatrix = hasIdentityMatrix();
    const int parentFlags = parent->mGroupFlags;

    if ((parentFlags & ViewGroup::FLAG_CLEAR_TRANSFORMATION) != 0) {
        parent->getChildTransformation()->clear();
        parent->mGroupFlags &= ~ViewGroup::FLAG_CLEAR_TRANSFORMATION;
    }

    Transformation* transformToApply = nullptr;
    bool concatMatrix = false;
    const bool scalingRequired = mAttachInfo && mAttachInfo->mScalingRequired;
    Animation* a = getAnimation();
    if (a != nullptr) {
        more = applyLegacyAnimation(parent, drawingTime, a, scalingRequired);
        concatMatrix = a->willChangeTransformationMatrix();
        if (concatMatrix) {
            mPrivateFlags3 |= PFLAG3_VIEW_IS_ANIMATING_TRANSFORM;
        }
        transformToApply = parent->getChildTransformation();
    } else {
        if ((mPrivateFlags3 & PFLAG3_VIEW_IS_ANIMATING_TRANSFORM) != 0) {
            // No longer animating: clear out old animation matrix
            //mRenderNode.setAnimationMatrix(nullptr);
            mPrivateFlags3 &= ~PFLAG3_VIEW_IS_ANIMATING_TRANSFORM;
        }
        if (!drawingWithRenderNode
                && (parentFlags & ViewGroup::FLAG_SUPPORT_STATIC_TRANSFORMATIONS) != 0) {
            Transformation* t = parent->getChildTransformation();
            const bool hasTransform = parent->getChildStaticTransformation(this, t);
            if (hasTransform) {
                const int transformType = t->getTransformationType();
                transformToApply = transformType != Transformation::TYPE_IDENTITY ? t : nullptr;
                concatMatrix = (transformType & Transformation::TYPE_MATRIX) != 0;
            }
        }
    }

    concatMatrix |= !childHasIdentityMatrix;

    // Sets the flag as early as possible to allow draw() implementations
    // to call invalidate() successfully when doing animations
    mPrivateFlags |= PFLAG_DRAWN;
    double cx1,cy1,cx2,cy2;
    canvas.get_clip_extents(cx1,cy1,cx2,cy2);
    Rect rcc=Rect::MakeLTRB(cx1,cy1,cx2,cy2);
    if (!concatMatrix && (parentFlags & (ViewGroup::FLAG_SUPPORT_STATIC_TRANSFORMATIONS |
                    ViewGroup::FLAG_CLIP_CHILDREN)) == ViewGroup::FLAG_CLIP_CHILDREN &&
            (false ==rcc.intersect(mLeft, mTop, mRight-mLeft, mBottom-mTop)) &&
            //canvas.quickReject(mLeft, mTop, mRight, mBottom, Canvas.EdgeType.BW) &&
            (mPrivateFlags & PFLAG_DRAW_ANIMATION) == 0) {
        mPrivateFlags2 |= PFLAG2_VIEW_QUICK_REJECTED;
        return more;
    }
    mPrivateFlags2 &= ~PFLAG2_VIEW_QUICK_REJECTED;

    if (hardwareAcceleratedCanvas) {
        // Clear INVALIDATED flag to allow invalidation to occur during rendering, but
        // retain the flag's value temporarily in the mRecreateDisplayList flag
        //mRecreateDisplayList = (mPrivateFlags & PFLAG_INVALIDATED) != 0;
        mPrivateFlags &= ~PFLAG_INVALIDATED;
    }

    RefPtr<ImageSurface> cache = nullptr;
    RenderNode* renderNode = nullptr;
    int layerType = getLayerType(); // TODO: signify cache state with just 'cache' local
    if (layerType == LAYER_TYPE_SOFTWARE || !drawingWithRenderNode) {
        if (layerType != LAYER_TYPE_NONE) {
             // If not drawing with RenderNode, treat HW layers as SW
             layerType = LAYER_TYPE_SOFTWARE;
             buildDrawingCache(true);
        }
        cache = getDrawingCache(true);
    }

    if (drawingWithRenderNode) {
        // Delay getting the display list until animation-driven alpha values are
        // set up and possibly passed on to the view
        //renderNode = updateDisplayListIfDirty();
        if (true){//!renderNode.isValid()) {
            // Uncommon, but possible. If a view is removed from the hierarchy during the call
            // to getDisplayList(), the display list will be marked invalid and we should not
            // try to use it again.
            //renderNode = nullptr;
            drawingWithRenderNode = false;
        }
    }
    int sx = 0;
    int sy = 0;
    if (!drawingWithRenderNode) {
        computeScroll();
        sx = mScrollX;
        sy = mScrollY;
    }

    const bool drawingWithDrawingCache = (cache != nullptr) && (false==drawingWithRenderNode);
    const bool offsetForScroll = (cache == nullptr) && (false==drawingWithRenderNode);

    int restoreTo =0 ;
    if (!drawingWithRenderNode || transformToApply != nullptr) {
        canvas.save();restoreTo++;
    }
    if (offsetForScroll) {
        canvas.translate(mLeft - sx, mTop - sy);
    } else {
        if (!drawingWithRenderNode) {
            canvas.translate(mLeft, mTop);
        }
        if (scalingRequired) {
            if (drawingWithRenderNode) {
                // TODO: Might not need this if we put everything inside the DL
                canvas.save();restoreTo++;
            }
            // mAttachInfo cannot be null, otherwise scalingRequired == false
            const float scale = 1.0f / mAttachInfo->mApplicationScale;
            canvas.scale(scale, scale);
        }
    }

    float alpha = drawingWithRenderNode ? 1 : (getAlpha() * getTransitionAlpha());//getAlpha()
    if (transformToApply != nullptr || alpha < 1 || !hasIdentityMatrix()
            || (mPrivateFlags3 & PFLAG3_VIEW_IS_ANIMATING_ALPHA) != 0) {
        if (transformToApply != nullptr || !childHasIdentityMatrix) {
            int transX = 0 , transY = 0;

            if (offsetForScroll) {
                transX = -sx;
                transY = -sy;
            }

            if (transformToApply != nullptr) {
                if (concatMatrix) {
                    if (drawingWithRenderNode) {
                        //renderNode->setAnimationMatrix(transformToApply->getMatrix());
                    } else {
                        // Undo the scroll translation, apply the transformation matrix,
                        // then redo the scroll translate to get the correct result.
                        canvas.translate(-transX, -transY);
                        canvas.transform(transformToApply->getMatrix());
                        canvas.translate(transX, transY);
                    }
                    parent->mGroupFlags |= ViewGroup::FLAG_CLEAR_TRANSFORMATION;
                }

                float transformAlpha = transformToApply->getAlpha();
                if (transformAlpha < 1) {
                    alpha *= transformAlpha;
                    parent->mGroupFlags |= ViewGroup::FLAG_CLEAR_TRANSFORMATION;
                }
            }
            if (!childHasIdentityMatrix && !drawingWithRenderNode) {
                canvas.translate(-transX, -transY);
                canvas.transform(getMatrix());//concat(getMatrix());
                canvas.translate(transX, transY);
            }
        }

        // Deal with alpha if it is or used to be <1
        if (alpha < 1 || (mPrivateFlags3 & PFLAG3_VIEW_IS_ANIMATING_ALPHA) != 0) {
            if (alpha < 1) {
                mPrivateFlags3 |= PFLAG3_VIEW_IS_ANIMATING_ALPHA;
            } else {
                mPrivateFlags3 &= ~PFLAG3_VIEW_IS_ANIMATING_ALPHA;
            }
            parent->mGroupFlags |= ViewGroup::FLAG_CLEAR_TRANSFORMATION;
            if (!drawingWithDrawingCache) {
                int multipliedAlpha = (int) (255 * alpha);
                if (!onSetAlpha(multipliedAlpha)) {
                    /*if (drawingWithRenderNode) {
                        renderNode.setAlpha(alpha * getAlpha() * getTransitionAlpha());
                    } else if (layerType == LAYER_TYPE_NONE) {
                        canvas.saveLayerAlpha(sx, sy, sx + getWidth(), sy + getHeight(),multipliedAlpha);
                    }*/
                } else {//Alpha is handled by the child directly,clobber the layer's alpha
                    mPrivateFlags |= PFLAG_ALPHA_SET;
                }
            }
        }
    } else if ((mPrivateFlags & PFLAG_ALPHA_SET) == PFLAG_ALPHA_SET) {
        onSetAlpha(255);
        mPrivateFlags &= ~PFLAG_ALPHA_SET;
    }

    if (!drawingWithRenderNode) {
        // apply clips directly, since RenderNode won't do it for this draw
        int clips=0;
        if ((parentFlags & ViewGroup::FLAG_CLIP_CHILDREN) != 0 && cache == nullptr) {
            if (offsetForScroll){
                canvas.rectangle(sx,sy,getWidth(),getHeight());
            } else {
                if (!scalingRequired || cache == nullptr) {
                    canvas.rectangle(0,0,getWidth(), getHeight());
                } else {
                    canvas.rectangle(0, 0, cache->get_width(), cache->get_height());
                }
            }
            clips++;
        }

        if (!mClipBounds.empty()) {
            // clip bounds ignore scroll
            canvas.rectangle(mClipBounds.left,mClipBounds.top,mClipBounds.width,mClipBounds.height);
            clips++;
        }
        if(clips)canvas.clip();//cant clip here ,for rotation animator(the view will be cutted)
    }

    if (!drawingWithDrawingCache) {
        if (drawingWithRenderNode) {
            mPrivateFlags &= ~PFLAG_DIRTY_MASK;
            //((DisplayListCanvas) canvas).drawRenderNode(renderNode);
        } else {// Fast path for layouts with no backgrounds
            if ((mPrivateFlags & PFLAG_SKIP_DRAW) == PFLAG_SKIP_DRAW) {
                mPrivateFlags &= ~PFLAG_DIRTY_MASK;
                dispatchDraw(canvas);
            } else {
                draw(canvas);
            }
        }
    } else if (cache != nullptr) {
        mPrivateFlags &= ~PFLAG_DIRTY_MASK;
        cache->flush();
        canvas.save();
        canvas.reset_clip();
        canvas.set_source(cache,0,0);
        if(alpha<1.f)canvas.paint_with_alpha(alpha);
        else canvas.paint();
        canvas.restore();
    }
    while(restoreTo-- >0) {
        canvas.restore();//ToCount(restoreTo);
        LOGD_IF(canvas.get_status()!=0,"%p:%d status=%d",this,mID,canvas.get_status());
    }

    if (a != nullptr && !more) {
        if (!hardwareAcceleratedCanvas && !a->getFillAfter()) {
            onSetAlpha(255);
        }
        parent->finishAnimatingView(this, a);
    }

    if (more && hardwareAcceleratedCanvas) {
        if (a->hasAlpha() && (mPrivateFlags & PFLAG_ALPHA_SET) == PFLAG_ALPHA_SET) {
            // alpha animations should cause the child to recreate its display list
            invalidate(true);
        }
    }

    //mRecreateDisplayList = false;
    return more;
}

void View::onDraw(Canvas&canvas){
}

const Rect View::getBound()const{
    return Rect::MakeLTRB(mLeft,mTop,mRight,mBottom);
}

const Rect View::getDrawingRect()const{
    Rect ret;
    ret.set(mScrollX,mScrollY,mScrollX+getWidth(),mScrollY+getHeight());
    return ret;
}

void View::getFocusedRect(Rect&r){
    r.set(mLeft,mTop,mRight-mLeft,mBottom-mTop);
}

View& View::setId(int id){
    mID = id;
    if((mID==View::NO_ID)){// && (mLabelForId!=View::NO_ID)){
        mID = generateViewId();
    }
    return *this;
}

int View::getId() const{
    return mID;
}

int View::generateViewId(){
    static int sNextGeneratedId = 0x0;
    int newValue = sNextGeneratedId + 1;
    if(newValue>0xFFFFFF)newValue=1;
    sNextGeneratedId = newValue;
    return newValue|0xFF000000;
}

bool View::isViewIdGenerated(int id){
    return (id&0xFF000000)==0xFF000000;
}

int View::getNextFocusLeftId()const{
    return mNextFocusLeftId;
}

View& View::setNextFocusLeftId(int id){
    mNextFocusLeftId=id;
    return *this;
}

int View::getNextFocusRightId()const{
   return mNextFocusRightId;
}

View& View::setNextFocusRightId(int id){
    mNextFocusRightId=id;
    return *this;
}

int View::getNextFocusUpId()const{
    return mNextFocusUpId;
}

View& View::setNextFocusUpId(int id){
    mNextFocusUpId=id;
    return *this;
}

int View::getNextFocusDownId()const{
    return mNextFocusDownId;
}

View& View::setNextFocusDownId(int id){
    mNextFocusDownId=id;
    return *this;
}

int View::getNextFocusForwardId()const{
    return mNextFocusForwardId;
}

View& View::setNextFocusForwardId(int id){
    mNextFocusForwardId=id;
    return *this;
}

static int sNextAccessibilityViewId=0;
int View::getAccessibilityViewId(){
    if (mAccessibilityViewId == NO_ID) {
        mAccessibilityViewId = sNextAccessibilityViewId++;
    }
    return mAccessibilityViewId;
}

int View::getAutoFillViewId(){
    if (mAutofillViewId == NO_ID) {
        mAutofillViewId = mContext->getNextAutofillId();
    }
    return mAutofillViewId;
}

void View::setTag(void*tag){
    mTag = tag;
}

void*View::getTag()const{
    return mTag;
}

void View::setTag(int key,void*tag){
    //FATAL_IF((key>>24)<2,"The key must be an application-specific resource id.";
    setKeyedTag(key,tag);
}

void* View::getTag(int key)const{
    if (mKeyedTags != nullptr)
        return mKeyedTags->get(key);
    return nullptr;
}

void View::setTagInternal(int key, void* tag) {
    //FATAL_IF(((key >> 24) != 0x1,"The key must be a framework-specific resource id.";
    setKeyedTag(key, tag);
}

void View::setKeyedTag(int key,void* tag){
    if(mKeyedTags ==nullptr)
        mKeyedTags = new SparseArray<void*>();
    mKeyedTags->put(key,tag);
}

View& View::setHint(const std::string&hint){
    mHint = hint;
    invalidate(true);
    return *this;
}

const std::string&View::getHint()const{
    return mHint;
}

void View::setContentDescription(const std::string&content){
    mContentDescription = content;
}

std::string View::getContentDescription()const{
    return mContentDescription;
}

bool View::isTemporarilyDetached()const{
    return (mPrivateFlags3 & PFLAG3_TEMPORARY_DETACH) != 0;
}

void View::dispatchFinishTemporaryDetach(){
    mPrivateFlags3 &= ~PFLAG3_TEMPORARY_DETACH;
    onFinishTemporaryDetach();
    if (hasWindowFocus() && hasFocus()) {
        InputMethodManager::getInstance().focusIn(this);
    }
    //notifyEnterOrExitForAutoFillIfNeeded(true);
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

void View::setHasTransientState(bool hasState){
    const bool oldHasTransientState = hasTransientState();
    mTransientStateCount = hasState ? mTransientStateCount + 1 :mTransientStateCount - 1;
    if (mTransientStateCount < 0) {
        mTransientStateCount = 0;
        LOGE("hasTransientState decremented below 0: unmatched pair of setHasTransientState calls");
    } else if ((hasState && mTransientStateCount == 1) ||
            (!hasState && mTransientStateCount == 0)) {
        // update flag if we've just incremented up from 0 or decremented down to 0
        mPrivateFlags2 = (mPrivateFlags2 & ~PFLAG2_HAS_TRANSIENT_STATE) |
                (hasState ? PFLAG2_HAS_TRANSIENT_STATE : 0);
        const bool newHasTransientState = hasTransientState();
        if (mParent && newHasTransientState != oldHasTransientState) {
            mParent->childHasTransientStateChanged(this, newHasTransientState);
        }
    }    
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

cdroid::Context*View::getContext()const{
    return mContext;
}

int View::getWidth()const{
    return mRight - mLeft;
}

int View::getHeight()const{
    return mBottom-mTop;
}

bool View::getGlobalVisibleRect(Rect& r, Point* globalOffset) {
    int width = mRight - mLeft;
    int height = mBottom - mTop;
    if (width > 0 && height > 0) {
        r.set(0, 0, width, height);
        if (globalOffset) {
            globalOffset->set(-mScrollX, -mScrollY);
        }
        return mParent == nullptr || mParent->getChildVisibleRect(this, r, globalOffset);
    }
    return false;
}

bool View::getLocalVisibleRect(Rect& r) {
    Point offset;
    if (getGlobalVisibleRect(r,&offset)) {
        r.offset(-offset.x, -offset.y); // make r local
        return true;
    }
    return false;
}

void View::offsetTopAndBottom(int offset){
    mTop += offset;
    mBottom += offset;
}

void View::offsetLeftAndRight(int offset){
    mLeft += offset;
    mRight += offset;
}

int View::getRawTextDirection()const{
    return (mPrivateFlags2 & PFLAG2_TEXT_DIRECTION_MASK) >> PFLAG2_TEXT_DIRECTION_MASK_SHIFT;
}

void View::setTextDirection(int textDirection){
    if (getRawTextDirection() != textDirection) {
         // Reset the current text direction and the resolved one
         mPrivateFlags2 &= ~PFLAG2_TEXT_DIRECTION_MASK;
         resetResolvedTextDirection();
         // Set the new text direction
         mPrivateFlags2 |= ((textDirection << PFLAG2_TEXT_DIRECTION_MASK_SHIFT) & PFLAG2_TEXT_DIRECTION_MASK);
         // Do resolution
         resolveTextDirection();
         // Notify change
         onRtlPropertiesChanged(getLayoutDirection());
         // Refresh
         requestLayout();
         invalidate(true);
    }
}

int View::getTextDirection()const{
    return (mPrivateFlags2 & PFLAG2_TEXT_DIRECTION_RESOLVED_MASK) >> PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT;
}

bool View::isAttachedToWindow()const{
    return mAttachInfo!=nullptr; 
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

bool View::canResolveTextDirection()const{
   switch (getRawTextDirection()) {
   case TEXT_DIRECTION_INHERIT:
       if (mParent != nullptr)
           return mParent->canResolveTextDirection();
       return false;   
   default: return true;
   }
}

void View::resetResolvedTextDirection() {
    // Reset any previous text direction resolution
    mPrivateFlags2 &= ~(PFLAG2_TEXT_DIRECTION_RESOLVED | PFLAG2_TEXT_DIRECTION_RESOLVED_MASK);
    // Set to default value
    mPrivateFlags2 |= PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT;
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

    if (hasRtlSupport()) {
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
                // Resolved text alignment is the same as the parent resolved text alignment
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
        resetResolvedTextAlignment();
        mPrivateFlags2 |= ((textAlignment << PFLAG2_TEXT_ALIGNMENT_MASK_SHIFT) & PFLAG2_TEXT_ALIGNMENT_MASK);
        resolveTextAlignment();
        onRtlPropertiesChanged(getLayoutDirection());
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
        invalidate(true);
    }
    return *this;
}

int View::getLayoutDirection()const{
    return ((mPrivateFlags2 & PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL) ==
          PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL) ? LAYOUT_DIRECTION_RTL : LAYOUT_DIRECTION_LTR;
}

bool View::isLayoutRtl()const{
    return getLayoutDirection() == LAYOUT_DIRECTION_RTL;
}

bool View::setFrame(int left,int top,int width,int height){
    bool changed = false;
    if (mLeft != left || getWidth() != width || mTop != top || getHeight() != height) {
        changed = true;

        // Remember our drawn bit
        int drawn = mPrivateFlags & PFLAG_DRAWN;

        int oldWidth  = mRight-mLeft;
        int oldHeight = mBottom-mTop;
        int newWidth  = width;
        int newHeight = height;
        bool sizeChanged = (newWidth != oldWidth) || (newHeight != oldHeight);

        // Invalidate our old position
        invalidate(sizeChanged);

        mLeft = left;
        mTop  = top;
        mRight  = left+ width;
        mBottom = top + height;
        mRenderNode->setLeftTopRightBottom(mLeft, mTop, mRight, mBottom);
        LOGV("%p:%d (%d,%d %d,%d)",this,mID,left,top,width,height);
        mPrivateFlags |= PFLAG_HAS_BOUNDS;

        if (sizeChanged)
            sizeChange(newWidth, newHeight, oldWidth, oldHeight);

        if ((mViewFlags & VISIBILITY_MASK) == VISIBLE/*|| mGhostView != null*/) {
            // If we are visible, force the DRAWN bit to on so that
            // this invalidate will go through (at least to our parent).
            // This is because someone may have invalidated this view
            // before this call to setFrame came in, thereby clearing
            // the DRAWN bit.
            mPrivateFlags |= PFLAG_DRAWN;
            invalidate(sizeChanged);
            // parent display list may need to be recreated based on a change in the bounds
            // of any child
            invalidateParentCaches();
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
    setFrame(x,y,getWidth(),getHeight());
    return *this;
}

View& View::setSize(int w,int h){
    if( (getWidth()!=w)|| (getHeight()!=h) ){
        setFrame(mLeft,mTop,w,h);
    }
    return *this;
}

const Rect View::getClientRect()const{
    return Rect::Make(0,0,getWidth(),getHeight());
}

void View::getHitRect(Rect& outRect){
    outRect.set(mLeft,mTop,getWidth(),getHeight());
}

bool View::pointInView(int localX,int localY, int slop) {
    return localX >= -slop && localY >= -slop && localX < (mRight-mLeft + slop) &&
            localY < (mBottom-mTop + slop);
}

void View::onResolveDrawables(int layoutDirection){
}

bool View::areDrawablesResolved()const{
    return (mPrivateFlags2 & PFLAG2_DRAWABLE_RESOLVED) == PFLAG2_DRAWABLE_RESOLVED;
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
    resetResolvedDrawablesInternal();
}

void View::resetResolvedDrawablesInternal() {
    mPrivateFlags2 &= ~PFLAG2_DRAWABLE_RESOLVED;
}

bool View::verifyDrawable(Drawable*who)const{
    return who == mBackground || (mForegroundInfo  && mForegroundInfo->mDrawable == who)|| (mDefaultFocusHighlight == who);
}

void View::jumpDrawablesToCurrentState(){
    if (mBackground) mBackground->jumpToCurrentState();
    if (mStateListAnimator) mStateListAnimator->jumpToCurrentState();
    if (mDefaultFocusHighlight)mDefaultFocusHighlight->jumpToCurrentState();

    if (mForegroundInfo  && mForegroundInfo->mDrawable )mForegroundInfo->mDrawable->jumpToCurrentState();
}

std::vector<int>View::onCreateDrawableState(){
    int viewStateIndex = 0;

    if(isFocused()) viewStateIndex |= StateSet::VIEW_STATE_FOCUSED;
    if(mPrivateFlags & PFLAG_PRESSED)  viewStateIndex = StateSet::VIEW_STATE_PRESSED;
    if(mPrivateFlags & PFLAG_SELECTED) viewStateIndex |= StateSet::VIEW_STATE_SELECTED;
    if(hasWindowFocus() ) viewStateIndex|=StateSet::VIEW_STATE_WINDOW_FOCUSED;
    if((mViewFlags & ENABLED_MASK) == ENABLED) viewStateIndex|=StateSet::VIEW_STATE_ENABLED;
    if((mPrivateFlags & PFLAG_ACTIVATED) != 0) viewStateIndex |= StateSet::VIEW_STATE_ACTIVATED;
    //LOGV("**** %p:%d isFocused=%d enabled=%d pressed=%d activated=%d",this,getId(),isFocused(),isEnabled(),isPressed(),isActivated());
    
    if(mPrivateFlags & PFLAG_HOVERED ) viewStateIndex |= StateSet::VIEW_STATE_HOVERED;

    return StateSet::get(viewStateIndex);
}

std::vector<int>& View::mergeDrawableStates(std::vector<int>&baseState,const std::vector<int>&additionalState) {
    const int N = baseState.size();
    const int M = additionalState.size();
    for(int j=0;j<M;j++)
        baseState.push_back(additionalState[j]);
    return baseState;
}

void View::drawableStateChanged(){
    bool changed = false;
    const std::vector<int>state=getDrawableState(); 

    Drawable*d = mBackground;
    if(d && d->isStateful())
        changed |= d->setState(state);

    d = mDefaultFocusHighlight;
    if(d && d->isStateful()){
        changed|= d->setState(state);
    }

    d=mForegroundInfo?mForegroundInfo->mDrawable:nullptr;
    if(d && d->isStateful())
        changed|= d->setState(state);

    if(mScrollCache){
        d= mScrollCache->scrollBar;
        if(d && d->isStateful()){
            changed |= d->setState(state) && mScrollCache->state!=ScrollabilityCache::OFF;
        } 
    }
    if (mStateListAnimator) mStateListAnimator->setState(state);
    if(changed)   invalidate(true);
}

void View::drawableHotspotChanged(float x, float y){
    if(mBackground)mBackground->setHotspot(x,y);
    if(mDefaultFocusHighlight)mDefaultFocusHighlight->setHotspot(x,y);
    if(mForegroundInfo && mForegroundInfo->mDrawable)
        mForegroundInfo->mDrawable->setHotspot(x,y);
    dispatchDrawableHotspotChanged(x,y);
}

void View::dispatchDrawableHotspotChanged(float x,float y){
     //NOTHING
}

void View::refreshDrawableState(){
    mPrivateFlags |= PFLAG_DRAWABLE_STATE_DIRTY;
    drawableStateChanged();
    if (mParent) {
        mParent->childDrawableStateChanged(this);
    }
}

const std::vector<int>View::getDrawableState(){
    if(!(mPrivateFlags & PFLAG_DRAWABLE_STATE_DIRTY))
        return mDrawableState;
    mDrawableState=onCreateDrawableState();
    mPrivateFlags &= ~PFLAG_DRAWABLE_STATE_DIRTY;
    return mDrawableState;
}

int View::getSolidColor()const{
    return 0;
}

Drawable*View::getBackground()const{
    return mBackground;
}

View& View::setBackgroundResource(const std::string&resid){
    Drawable*d=getContext()->getDrawable(resid);
    return setBackground(d);
}

View& View::setBackground(Drawable*background){
    computeOpaqueFlags();
    if(background==mBackground) return*this;

    bool bRequestLayout = false;
    if(mBackground!=nullptr){
        if(isAttachedToWindow()) mBackground->setVisible(false,false);
        mBackground->setCallback(this);
        unscheduleDrawable(*mBackground);
        delete mBackground;
        mBackground = nullptr;
    }
    if(background){        
        Rect padding;
        resetResolvedDrawablesInternal();
        background->setLayoutDirection(getLayoutDirection());  
       
        if(background->getPadding(padding)){
            resetResolvedPaddingInternal();
            switch (background->getLayoutDirection()) {
            case LAYOUT_DIRECTION_RTL:
                mUserPaddingLeftInitial = padding.width;
                mUserPaddingRightInitial = padding.left;
                internalSetPadding(padding.width, padding.top, padding.left, padding.height);
                break;
            case LAYOUT_DIRECTION_LTR:
            default:
                mUserPaddingLeftInitial = padding.left;
                mUserPaddingRightInitial = padding.width;
                internalSetPadding(padding.left, padding.top, padding.width, padding.height);
            }
            mLeftPaddingDefined = false;
            mRightPaddingDefined = false;
        }
        bRequestLayout= mBackground==nullptr||mBackground->getMinimumWidth()!=background->getMinimumWidth()||
               mBackground->getMinimumHeight()!=background->getMinimumHeight();

        delete mBackground;
        mBackground = background; 
        if(background->isStateful())
            background->setState(getDrawableState());
        if(isAttachedToWindow())
            background->setVisible((getWindowVisibility() == VISIBLE) && isShown(),false);
        applyBackgroundTint();
        background->setCallback(this);
        if( (mPrivateFlags & PFLAG_SKIP_DRAW)!=0 ){
            mPrivateFlags &= ~PFLAG_SKIP_DRAW;
            bRequestLayout=true;
        }
    }else{
        mBackground = nullptr;
        if ((mViewFlags & WILL_NOT_DRAW) != 0  && (mDefaultFocusHighlight == nullptr)
              && (mForegroundInfo == nullptr || mForegroundInfo->mDrawable == nullptr)) {
          mPrivateFlags |= PFLAG_SKIP_DRAW;
        }

        /* When the background is set, we try to apply its padding to this
         * View. When the background is removed, we don't touch this View's
         * padding. This is noted in the Javadocs. Hence, we don't need to
         * requestLayout(), the invalidate() below is sufficient.*/

        // The old background's minimum size could have affected this
        // View's layout, so let's requestLayout
        bRequestLayout = true;
    }
    computeOpaqueFlags();
    if(bRequestLayout) requestLayout();
    mBackgroundSizeChanged = true;
    invalidate(true);
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
        setBackground(new ColorDrawable(color));
    return *this;
}

View& View::setBackgroundTintList(const ColorStateList* tint){
    if (mBackgroundTint == nullptr) {
        mBackgroundTint = new TintInfo();
    }
    if(mBackgroundTint->mTintList!=tint){
        mBackgroundTint->mTintList = tint;
        mBackgroundTint->mHasTintList = (tint!=nullptr);
        applyBackgroundTint();
    }
    return *this;
}

const ColorStateList* View::getBackgroundTintList()const{
    return mBackgroundTint != nullptr ? mBackgroundTint->mTintList : nullptr;
}

View& View::setBackgroundTintMode(int tintMode) {
    if (mBackgroundTint == nullptr) {
        mBackgroundTint = new TintInfo();
    }
    mBackgroundTint->mTintMode = tintMode;
    mBackgroundTint->mHasTintMode = true;

    applyBackgroundTint();
    return *this;
}

int View::getBackgroundTintMode() const{
    return mBackgroundTint ? mBackgroundTint->mTintMode : -1;
}

void View::onFocusChanged(bool gainFocus,int direct,Rect*previouslyFocusedRect){
    if(mListenerInfo&&mListenerInfo->mOnFocusChangeListener)
        mListenerInfo->mOnFocusChangeListener(*this,gainFocus);
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
        if (isAttachedToWindow()) {
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
        if (isAttachedToWindow()) {
            foreground->setVisible((getWindowVisibility() == VISIBLE) && isShown(), false);
        }
            // Set callback last, since the view may still be initializing.
        foreground->setCallback(this);
    } else if ((mViewFlags & WILL_NOT_DRAW) != 0 && mBackground == nullptr && (mDefaultFocusHighlight == nullptr)) {
        mPrivateFlags |= PFLAG_SKIP_DRAW;
    }
    requestLayout();
    invalidate(true);
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

View& View::setForegroundTintList(const ColorStateList* tint){
    if (mForegroundInfo == nullptr) {
        mForegroundInfo = new ForegroundInfo();
    }
    if (mForegroundInfo->mTintInfo == nullptr) {
        mForegroundInfo->mTintInfo = new TintInfo();
    }
    if(mForegroundInfo->mTintInfo->mTintList!=tint){
        mForegroundInfo->mTintInfo->mTintList = tint;
        mForegroundInfo->mTintInfo->mHasTintList = (tint!=nullptr);
        applyForegroundTint();
    }
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

const ColorStateList* View::getForegroundTintList(){
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
        if (isAttachedToWindow()) highlight->setVisible(isShown(), false);
        // Set callback last, since the view may still be initializing.
        highlight->setCallback(this);
    } else if ((mViewFlags & WILL_NOT_DRAW) != 0 && mBackground == nullptr
            && (mForegroundInfo == nullptr || mForegroundInfo->mDrawable == nullptr)) {
        mPrivateFlags |= PFLAG_SKIP_DRAW;
    }
    invalidate();
}

bool View::hasSize()const {
    return mRight-mLeft>0 && mBottom-mTop>0;
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
            if (mParent) {
                //ViewRootImpl viewRootImpl = getViewRootImpl();
                if (//!sAutoFocusableOffUIThreadWontNotifyParents||
		    ((mAttachInfo==nullptr)||(mAttachInfo->mRootView==nullptr))
		    ||focusableChangedByAuto == 0)
                    shouldNotifyFocusableAvailable = canTakeFocus();
            }
        }
    }

    const int newVisibility = flags & VISIBILITY_MASK;
    if (newVisibility == VISIBLE) {
        if ((changed & VISIBILITY_MASK) != 0) {
            /* If this view is becoming visible, invalidate it in case it changed while
             * it was not visible. Marking it drawn ensures that the invalidation will
             * go through.*/
            mPrivateFlags |= PFLAG_DRAWN;
            invalidate(true);

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
            clearAccessibilityFocus();
            destroyDrawingCache();
            // GONE views noop invalidation, so invalidate the parent
            if(mParent)mParent->invalidate();
            // Mark the view drawn to ensure that it gets invalidated properly the next
            // time it is visible and gets invalidated
            mPrivateFlags |= PFLAG_DRAWN;
        }
        if (mAttachInfo) mAttachInfo->mViewVisibilityChanged = true;
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
                clearAccessibilityFocus();
            }
        }
        if(mAttachInfo)mAttachInfo->mViewVisibilityChanged = true;
    }

    if ((changed & VISIBILITY_MASK) != 0) {
        // If the view is invisible, cleanup its display list to free up resources
        if (newVisibility != VISIBLE && mAttachInfo ) {
            cleanupDraw();
        }

        if (mParent) {
            mParent->onChildVisibilityChanged(this,(changed & VISIBILITY_MASK), newVisibility);
            mParent->invalidate();
        }

        if (mAttachInfo != nullptr) {
            dispatchVisibilityChanged(*this, newVisibility);

            // Aggregated visibility changes are dispatched to attached views
            // in visible windows where the parent is currently shown/drawn
            // or the parent is not a ViewGroup (and therefore assumed to be a ViewRoot),
            // discounting clipping or overlapping. This makes it a good place
            // to change animation states.
            if (mParent  && (getWindowVisibility() == VISIBLE) && mParent->isShown()) {
                dispatchVisibilityAggregated(newVisibility == VISIBLE);
            }
            //notifySubtreeAccessibilityStateChangedIfNeeded();
        }
    }

    if ((changed & WILL_NOT_CACHE_DRAWING) != 0) destroyDrawingCache();

    if ((changed & DRAWING_CACHE_ENABLED) != 0) {
        destroyDrawingCache();
        mPrivateFlags &= ~PFLAG_DRAWING_CACHE_VALID;
        invalidateParentCaches();
    }

    if ((changed & DRAWING_CACHE_QUALITY_MASK) != 0) {
        destroyDrawingCache();
        mPrivateFlags &= ~PFLAG_DRAWING_CACHE_VALID;
    }

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
    invalidate(true);
    return *this;
}

View& View::clearFlag(int flag) {
    mViewFlags&=(~flag);
    invalidate(true);
    return *this;
}

bool View::hasFlag(int flag) const {
    return (mViewFlags&flag)==flag;
}

void View::bringToFront() {
    if (mParent != nullptr) {
        mParent->bringChildToFront(this);
    }
}

void View::onAttachedToWindow(){
    mPrivateFlags3 &= ~PFLAG3_IS_LAID_OUT;
    jumpDrawablesToCurrentState();
    if(isFocused()){
        InputMethodManager&imm=InputMethodManager::getInstance();
        imm.focusIn((View*)this);
    } 
}

void View::onDetachedFromWindow(){
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

bool View::isInEditMode()const{
    return false;
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
        sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_ACCESSIBILITY_FOCUSED);
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
    if( enable == isEnabled() ) return *this;
    setFlags(enable?ENABLED:DISABLED,ENABLED_MASK);
    refreshDrawableState();
    invalidate(true);
    if (!enable) {
        cancelPendingInputEvents();
    }
    return *this;
}

bool View::isEnabled() const {
    return (mViewFlags&ENABLED_MASK)==ENABLED;
}

void View::setActivated(bool activated){
     if (((mPrivateFlags & PFLAG_ACTIVATED) != 0) != activated) {
        mPrivateFlags = (mPrivateFlags & ~PFLAG_ACTIVATED) | (activated ? PFLAG_ACTIVATED : 0);
        invalidate(true);
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
    if (needsRefresh)
        refreshDrawableState();
    dispatchSetPressed(pressed);
}

void View::setPressed(bool pressed,float x,float y){
    if(pressed)
        drawableHotspotChanged(x,y);
    setPressed(pressed);
}

bool View::isPressed()const{
    return (mPrivateFlags & PFLAG_PRESSED) == PFLAG_PRESSED;
}

void View::dispatchSetPressed(bool pressed){
}

void View::dispatchWindowFocusChanged(bool hasFocus){
    onWindowFocusChanged(hasFocus);
}

void View::onWindowFocusChanged(bool hasWindowFocus){
    InputMethodManager& imm = InputMethodManager::getInstance();
    if (!hasWindowFocus) {
       if (isPressed()) {
           setPressed(false);
       }
       mPrivateFlags3 &= ~PFLAG3_FINGER_DOWN;
       if ((mPrivateFlags & PFLAG_FOCUSED) != 0) {
           imm.focusOut(this);
       }
       removeLongPressCallback();
       removeTapCallback();
       onFocusLost();
    } else if ((mPrivateFlags & PFLAG_FOCUSED) != 0) {
        imm.focusIn(this);
    }
    refreshDrawableState();

}

bool View::hasWindowFocus()const{
    return mAttachInfo && mAttachInfo->mHasWindowFocus;
}

int View::getWindowVisibility()const{
    return mAttachInfo  ? mAttachInfo->mWindowVisibility : GONE;
}

void View::getWindowVisibleDisplayFrame(Rect& outRect){
    if (mAttachInfo) {
        //mAttachInfo.mSession.getDisplayFrame(mAttachInfo.mWindow, outRect);
        // XXX This is really broken, and probably all needs to be done
        // in the window manager, and we need to know more about whether
        // we want the area behind or in front of the IME.
        const DisplayMetrics& metrics= mContext->getDisplayMetrics();
        outRect.set(0,0,metrics.widthPixels,metrics.heightPixels);
        Rect insets = mAttachInfo->mVisibleInsets;
        outRect.left += insets.left;
        outRect.top += insets.top;
        outRect.width -= (insets.width+insets.left);
        outRect.height-= (insets.height+insets.top);
        return;
    }
    // The view is not attached to a display so we don't have a context.
    // Make a best guess about the display size.
    //Display d = DisplayManagerGlobal.getInstance().getRealDisplay(Display.DEFAULT_DISPLAY);
    //d.getRectSize(outRect);
    Point sz;
    WindowManager::getInstance().getDefaultDisplay().getSize(sz);
    outRect.set(0,0,sz.x,sz.y);
}

void View::getWindowDisplayFrame(Rect& outRect) {
    if (mAttachInfo) {
        //mAttachInfo.mSession.getDisplayFrame(mAttachInfo.mWindow, outRect);
        //return;
    }
    // The view is not attached to a display so we don't have a context.
    // Make a best guess about the display size.
    //Display d = DisplayManagerGlobal.getInstance().getRealDisplay(Display.DEFAULT_DISPLAY);
    //d.getRectSize(outRect);
    Point sz;
    WindowManager::getInstance().getDefaultDisplay().getSize(sz);
    outRect.set(0,0,sz.x,sz.y);
}
void View::dispatchVisibilityChanged(View& changedView,int visibility){
    onVisibilityChanged(changedView, visibility); 
}

void View::onVisibilityChanged(View& changedView,int visibility){
    //nothing
}

bool View::isDirty()const{
    return (mPrivateFlags&PFLAG_DIRTY_MASK)!=0;
}

bool View::skipInvalidate()const{
    return (mViewFlags & VISIBILITY_MASK) != VISIBLE && (mCurrentAnimation == nullptr)
           && mParent && (!mParent->isViewTransitioning((View*)this));
}

RefPtr<ImageSurface>View::getDrawingCache(bool autoScale){
    if ((mViewFlags & WILL_NOT_CACHE_DRAWING) == WILL_NOT_CACHE_DRAWING) {
        return nullptr;
    }
    if ((mViewFlags & DRAWING_CACHE_ENABLED) == DRAWING_CACHE_ENABLED) {
        buildDrawingCache(autoScale);
    }
    return autoScale ? mDrawingCache : mUnscaledDrawingCache;
}

void View::destroyDrawingCache(){
    mDrawingCache =nullptr;
    mUnscaledDrawingCache =nullptr;
}

int View::getDrawingCacheBackgroundColor()const{
    return  mDrawingCacheBackgroundColor;
}

void View::setDrawingCacheBackgroundColor(int color){
    if (color != mDrawingCacheBackgroundColor) {
         mDrawingCacheBackgroundColor = color;
         mPrivateFlags &= ~PFLAG_DRAWING_CACHE_VALID;
    }
}

void View::buildDrawingCache(bool autoScale){
    if ((mPrivateFlags & PFLAG_DRAWING_CACHE_VALID) == 0 || (autoScale ?
        mDrawingCache == nullptr : mUnscaledDrawingCache == nullptr)){ 
        buildDrawingCacheImpl(autoScale);
    }
}

void View::buildDrawingCacheImpl(bool autoScale){
    mCachingFailed = false;

    int width = mRight - mLeft;
    int height = mBottom - mTop;

    const bool scalingRequired = mAttachInfo && mAttachInfo->mScalingRequired;

    if (autoScale && scalingRequired) {
        width = (int) ((width * mAttachInfo->mApplicationScale) + 0.5f);
        height = (int) ((height * mAttachInfo->mApplicationScale) + 0.5f);
    }

    const bool opaque = mDrawingCacheBackgroundColor != 0 || isOpaque();
    const bool use32BitCache = mAttachInfo && mAttachInfo->mUse32BitDrawingCache;

    const long projectedBitmapSize = width * height * (opaque && !use32BitCache ? 2 : 4);
    const long drawingCacheSize =  ViewConfiguration::get(mContext).getScaledMaximumDrawingCacheSize();
    if (width <= 0 || height <= 0 || projectedBitmapSize > drawingCacheSize) {
        if (width > 0 && height > 0) {
            //LOG(WARN)<<this<<":"<<getId()<<" not displayed because it is too large to fit into a software layer (or drawing cache), needs "
            //        << projectedBitmapSize << " bytes, only "<< drawingCacheSize << " available size:"<<width<<"x"<<height;
        }
        destroyDrawingCache();
        mCachingFailed = true;
        return;
    }

    bool clear = true;
    RefPtr<ImageSurface> bitmap = autoScale ? mDrawingCache : mUnscaledDrawingCache;

    if (bitmap == nullptr || bitmap->get_width() != width || bitmap->get_height() != height) {
        Surface::Format format = Surface::Format::RGB16_565;
        if (!opaque) {
            // Never pick ARGB_4444 because it looks awful
            // Keep the DRAWING_CACHE_QUALITY_LOW flag just in case
            switch (mViewFlags & DRAWING_CACHE_QUALITY_MASK) {
            case DRAWING_CACHE_QUALITY_AUTO:
            case DRAWING_CACHE_QUALITY_LOW:
            case DRAWING_CACHE_QUALITY_HIGH:
            default: format = Surface::Format::ARGB32;
                    break;
            }
        } else {
            // Optimization for translucent windows
            // If the window is translucent, use a 32 bits bitmap to benefit from memcpy()
            format = use32BitCache ? Surface::Format::ARGB32 : Surface::Format::RGB16_565;
        }

        // Try to cleanup memory

        try {
            bitmap = ImageSurface::create(format,width, height);
            //bitmap.setDensity(getResources().getDisplayMetrics().densityDpi);
            if (autoScale) {
                mDrawingCache = bitmap;
            } else {
                mUnscaledDrawingCache = bitmap;
            }
            //if (opaque && use32BitCache) bitmap.setHasAlpha(false);
        } catch (std::exception& e) {
            // If there is not enough memory to create the bitmap cache, just
            // ignore the issue as bitmap caches are not required to draw the
            // view hierarchy
            if (autoScale) {
                mDrawingCache = nullptr;
            } else {
                mUnscaledDrawingCache = nullptr;
            }
            mCachingFailed = true;
            return;
        }

        clear = mDrawingCacheBackgroundColor != 0;
    }

    RefPtr<Canvas> oldCanvas=mAttachInfo?mAttachInfo->mCanvas:nullptr;
    RefPtr<Canvas> canvas=std::make_shared<Canvas>(bitmap);
    if (clear) {
        canvas->save();
        canvas->rectangle(0,0,width,height);
        canvas->set_operator(Cairo::Context::Operator::SOURCE);
        canvas->set_color(mDrawingCacheBackgroundColor); 
        canvas->fill();
        canvas->restore();
    }

    computeScroll();
    canvas->save();

    if (autoScale && scalingRequired) {
        float scale = mAttachInfo->mApplicationScale;
        canvas->scale(scale, scale);
    }

    canvas->translate(-mScrollX, -mScrollY);

    mPrivateFlags |= PFLAG_DRAWN;
    if (mAttachInfo == nullptr || !mAttachInfo->mHardwareAccelerated ||mLayerType != LAYER_TYPE_NONE) {
        mPrivateFlags |= PFLAG_DRAWING_CACHE_VALID;
    }

    // Fast path for layouts with no backgrounds
    if ((mPrivateFlags & PFLAG_SKIP_DRAW) == PFLAG_SKIP_DRAW) {
        mPrivateFlags &= ~PFLAG_DIRTY_MASK;
        dispatchDraw(*canvas);
        drawAutofilledHighlight(*canvas);
        if (mOverlay && !mOverlay->isEmpty()) {
            mOverlay->getOverlayView()->draw(*canvas);
        }
    } else {
        draw(*canvas);
    }

    canvas->restore();

    if (mAttachInfo) {
        // Restore the cached Canvas for our siblings
        mAttachInfo->mCanvas = oldCanvas;
    }
}

bool View::isAutofilled()const{
    return mPrivateFlags3&PFLAG3_IS_AUTOFILLED;
}

void View::setAutofilled(bool autofilled) {
    bool wasChanged = autofilled != isAutofilled();
    if (wasChanged) {
        if (autofilled) {
            mPrivateFlags3 |= PFLAG3_IS_AUTOFILLED;
        } else {
            mPrivateFlags3 &= ~PFLAG3_IS_AUTOFILLED;
        }
        invalidate();
    }
}

Drawable* View::getAutofilledDrawable(){
    if (mAttachInfo->mAutofilledDrawable == nullptr) {
        Drawable*dr=getContext()->getDrawable("cdroid:attr/autofilled_highlight");
        mAttachInfo->mAutofilledDrawable = dr;
    }
    return mAttachInfo->mAutofilledDrawable;
}

void View::drawAutofilledHighlight(Canvas& canvas){
    if (isAutofilled()) {
        Drawable* autofilledHighlight = getAutofilledDrawable();
        if (autofilledHighlight) {
            autofilledHighlight->setBounds(0, 0, getWidth(), getHeight());
            autofilledHighlight->draw(canvas);
        }
   }
}

void View::invalidateViewProperty(bool invalidateParent, bool forceRedraw) {
    if (!isHardwareAccelerated()//|| !mRenderNode->isValid()
             || (mPrivateFlags & PFLAG_DRAW_ANIMATION) != 0) {
        if (invalidateParent) {
            invalidateParentCaches();
        }
        if (forceRedraw) {
            mPrivateFlags |= PFLAG_DRAWN; // force another invalidation with the new orientation
        }
        invalidate(false);
    } else {
        damageInParent();
    }
}

void View::damageInParent() {
    if (mParent && mAttachInfo ) {
        mParent->onDescendantInvalidated(this, this);
    }
}

void View::transformRect(Rect&rect){
    if(hasIdentityMatrix()){
        getMatrix().transform_rectangle((Cairo::RectangleInt&)rect);
    }
}
void View::invalidateParentCaches(){
    if(mParent)mParent->mPrivateFlags |= PFLAG_INVALIDATED;
}

void View::invalidateParentIfNeeded(){
    if(mParent)mParent->invalidate(true);
}

void View::invalidateInheritedLayoutMode(int layoutModeOfRoot){
    //nothing
}

void View::invalidateParentIfNeededAndWasQuickRejected() {
    if ((mPrivateFlags2 & PFLAG2_VIEW_QUICK_REJECTED) != 0) {
        // View was rejected last time it was drawn by its parent; this may have changed
        invalidateParentIfNeeded();
    }
}

void View::invalidateInternal(int l, int t, int w, int h, bool invalidateCache,bool fullInvalidate){

    if (skipInvalidate())   return;

    if ((mPrivateFlags & (PFLAG_DRAWN | PFLAG_HAS_BOUNDS)) == (PFLAG_DRAWN | PFLAG_HAS_BOUNDS)
              || (invalidateCache && (mPrivateFlags & PFLAG_DRAWING_CACHE_VALID) == PFLAG_DRAWING_CACHE_VALID)
              || (mPrivateFlags & PFLAG_INVALIDATED) != PFLAG_INVALIDATED
              || (fullInvalidate || isOpaque() != mLastIsOpaque)) {
        if (fullInvalidate) {
            mLastIsOpaque = isOpaque();
            mPrivateFlags &= ~PFLAG_DRAWN;
        }

        mPrivateFlags |= PFLAG_DIRTY;

        if (invalidateCache) {
            mPrivateFlags |= PFLAG_INVALIDATED;
            mPrivateFlags &= ~PFLAG_DRAWING_CACHE_VALID;
        }

        // Propagate the damage rectangle to the parent view.
        if ( mAttachInfo && mParent && w>0 && h>0) {
            Rect damage;
            damage.set(l, t, w, h);
            mParent->invalidateChild(this,damage);
        }
        if( mAttachInfo && getRootView()==this && dynamic_cast<ViewGroup*>(this) &&(w>0)&&(h>0)){
            const RectangleInt damage={l,t,w,h};
            mPrivateFlags|=PFLAG_DIRTY;
            ((ViewGroup*)this)->mInvalidRgn->do_union(damage);
        }
    }
}

/*param:rect is views logical area,maybe it is large than views'bounds,function invalidate must convert it to bound area*/
void View::invalidate(const Rect&dirty){
    invalidateInternal(dirty.left - mScrollX, dirty.top - mScrollY,
            dirty.width, dirty.height, true, false);
}

void View::invalidate(int l,int t,int w,int h){
    invalidateInternal(l - mScrollX, t - mScrollY, w, h, true, false);
}

void View::invalidate(bool invalidateCache){
    invalidateInternal(0, 0, mRight-mLeft, mBottom-mTop, invalidateCache, true);
}
 
void View::postInvalidate(){
    postInvalidateDelayed(0);
}

void View::postInvalidate(int left,int top,int width,int height){
    postInvalidateDelayed(0,left,top,width,height);
}

void View::postInvalidateDelayed(long delayMilliseconds) {
    // We try only with the AttachInfo because there's no point in invalidating
    // if we are not attached to our window
    if (mAttachInfo) {
        mAttachInfo->mRootView->dispatchInvalidateDelayed(this, delayMilliseconds);
    }
}

void View::postInvalidateDelayed(long delayMilliseconds, int left, int top,
            int width, int height) {

    // We try only with the AttachInfo because there's no point in invalidating
    // if we are not attached to our window
    if (mAttachInfo) {
        AttachInfo::InvalidateInfo* info = AttachInfo::InvalidateInfo::obtain();
        info->target= this;
        info->rect.set(left,top,width,height);
        mAttachInfo->mRootView->dispatchInvalidateRectDelayed(info, delayMilliseconds);
    }
}

void View::cleanupDraw(){
    if (mAttachInfo != nullptr) {
        ViewGroup*root=(ViewGroup*)mAttachInfo->mRootView;
        root->cancelInvalidate(this);
    }
}

int View::getWindowAttachCount()const{
    return mWindowAttachCount;
}

void View::postInvalidateOnAnimation(){
    if(mAttachInfo){
	ViewGroup*w=dynamic_cast<ViewGroup*>(mAttachInfo->mRootView);
	w->dispatchInvalidateOnAnimation(this);
    }else{//else is tobe removed
        invalidate(true);
        ViewGroup*root=getRootView();
        if(root)root->dispatchInvalidateOnAnimation(this);
    }
}

void View::postInvalidateOnAnimation(int left, int top, int width, int height) {
    // We try only with the AttachInfo because there's no point in invalidating
    // if we are not attached to our window
    if (mAttachInfo) {
        const Rect rect={left,top,width,height};
        ViewGroup*root=mAttachInfo->mRootView;
        root->dispatchInvalidateRectOnAnimation(this,rect);
    }
}

 
void View::invalidateDrawable(Drawable& who){
    if(verifyDrawable(&who)) invalidate(true);
}

void View::scheduleDrawable(Drawable& who,Runnable what, long when){
    long delay = when - SystemClock::uptimeMillis();
    postDelayed(what,delay);
}

void View::unscheduleDrawable(Drawable& who,Runnable what){
    if(verifyDrawable(&who)&&what!=nullptr){
        Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,&what,&who); 
        removeCallbacks(what);
    }
}

void View::unscheduleDrawable(Drawable& who){
    Choreographer::getInstance().removeCallbacks(Choreographer::CALLBACK_ANIMATION,nullptr,&who);
}

ViewGroup*View::getParent()const{
    return mParent;
}

void View::assignParent(ViewGroup*parent){
    if(mParent ==nullptr)mParent=parent;
    else if(parent==nullptr)mParent=nullptr;
    else{
        LOGE("View %p:%d being added but it already has a parent",this,mID);
    }
}

ViewTreeObserver*View::getViewTreeObserver(){
    if(mAttachInfo)return mAttachInfo->mTreeObserver;
    if(mFloatingTreeObserver)
	mFloatingTreeObserver = new ViewTreeObserver(mContext);
    return mFloatingTreeObserver;
}

ViewGroup*View::getRootView()const{
    if( mAttachInfo && mAttachInfo->mRootView)
        return dynamic_cast<ViewGroup*>(mAttachInfo->mRootView);
    View* parent = (View*)this;
    while (parent->mParent != nullptr) {
        parent =parent->mParent;
    }
    return dynamic_cast<ViewGroup*>(parent);
}

bool View::toGlobalMotionEvent(MotionEvent& ev){
    if (mAttachInfo == nullptr) {
        return false;
    }

    Matrix m = identity_matrix();
    transformMatrixToGlobal(m);
    ev.transform(m);
    return true;
}

bool View::toLocalMotionEvent(MotionEvent& ev){
    if (mAttachInfo == nullptr) {
        return false;
    }

    Matrix m = identity_matrix();
    transformMatrixToLocal(m);
    ev.transform(m);
    return true;
}

void View::transformMatrixToGlobal(Matrix& matrix){
    if (mParent){
        View* vp = (View*) mParent;
        vp->transformMatrixToGlobal(matrix);
        matrix.translate(-vp->mScrollX, -vp->mScrollY);
    }
    matrix.translate(mLeft, mTop);
    if (!hasIdentityMatrix()) {
        Matrix mtx=getMatrix();
        matrix.multiply(matrix,mtx);
    }
}

void View::transformMatrixToLocal(Matrix& matrix){
    if (mParent){
        View* vp = (View*) mParent;
        vp->transformMatrixToLocal(matrix);
        matrix.translate(vp->mScrollX, vp->mScrollY);
    }
    matrix.translate(-mLeft, -mTop);
    if (!hasIdentityMatrix()) {
        Matrix inv=getInverseMatrix();
        matrix.multiply(matrix,inv);
    }
}

View*View::focusSearch(int direction)const{
    if(mParent)
        mParent->focusSearch((View*)this,direction);
    return nullptr;
}

bool View::requestRectangleOnScreen(Rect& rectangle, bool immediate){
    if (mParent == nullptr) {
        return false;
    }

    View* child = this;

    RectF position;
    position.set(rectangle.left,rectangle.top,rectangle.width,rectangle.height);

    ViewGroup* parent = mParent;
    bool scrolled = false;
    while (parent != nullptr) {
        rectangle.set((int) position.left, (int) position.top,
               (int) position.width, (int) position.height);

        scrolled |= parent->requestChildRectangleOnScreen(child, rectangle, immediate);

        //if (!(parent instanceof View)) { break; }
        // move it from child's content coordinate space to parent's content coordinate space
        position.offset(child->mLeft - child->getScrollX(), child->mTop -child->getScrollY());

        child = parent;
        parent = child->getParent();
    }
    return scrolled;
}

View& View::clearAccessibilityFocus(){
    return *this;
}

void View::sendAccessibilityHoverEvent(int eventType){
}

View& View::clearAccessibilityFocusNoCallbacks(int action){
    return *this;
}

View& View::sendAccessibilityEvent(int eventType){
    return *this;
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

bool View::requestFocus(int direction,Rect* previouslyFocusedRect){
    return requestFocusNoSearch(direction, previouslyFocusedRect);
}

void View::clearParentsWantFocus(){
    if(mParent!=nullptr){
        mParent->mPrivateFlags &= ~PFLAG_WANTS_FOCUS;
        mParent->clearParentsWantFocus();
    }
}

bool View::requestFocusNoSearch(int direction,Rect*previouslyFocusedRect) {
    // need to be focusable
    if (!canTakeFocus())  return false;
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
       if(viewRoot)viewRoot->ensureTouchMode(false);
   }
   return requestFocus(View::FOCUS_DOWN);
}

int View::getImportantForAccessibility() {
    return (mPrivateFlags2 & PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_MASK)
            >> PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_SHIFT;
}

void View::setImportantForAccessibility(int mode){
    const int oldMode = getImportantForAccessibility();
    if (mode != oldMode) {
        const bool hideDescendants = mode == IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS;

        // If this node or its descendants are no longer important, try to
        // clear accessibility focus.
        if (mode == IMPORTANT_FOR_ACCESSIBILITY_NO || hideDescendants) {
            View* focusHost = nullptr;//findAccessibilityFocusHost(hideDescendants);
            if (focusHost) {
                focusHost->clearAccessibilityFocus();
            }
        }

        // If we're moving between AUTO and another state, we might not need
        // to send a subtree changed notification. We'll store the computed
        // importance, since we'll need to check it later to make sure.
        const bool maySkipNotify = oldMode == IMPORTANT_FOR_ACCESSIBILITY_AUTO
                || mode == IMPORTANT_FOR_ACCESSIBILITY_AUTO;
        const bool oldIncludeForAccessibility = maySkipNotify ;//&& includeForAccessibility();
        mPrivateFlags2 &= ~PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_MASK;
        mPrivateFlags2 |= (mode << PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_SHIFT)
                & PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_MASK;
        /*if (!maySkipNotify || oldIncludeForAccessibility != includeForAccessibility()) {
            notifySubtreeAccessibilityStateChangedIfNeeded();
        } else {
            notifyViewAccessibilityStateChangedIfNeeded(AccessibilityEvent::CONTENT_CHANGE_TYPE_UNDEFINED);
        }*/
    }
}

bool View::isImportantForAccessibility()const{
    const int mode = (mPrivateFlags2 & PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_MASK) >> PFLAG2_IMPORTANT_FOR_ACCESSIBILITY_SHIFT;
    if ((mode == IMPORTANT_FOR_ACCESSIBILITY_NO) || (mode == IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS)) {
        return false;
    }

    // Check parent mode to ensure we're not hidden.
    ViewGroup* parent = mParent;
    while (parent){// instanceof View) {
        if (((View*) parent)->getImportantForAccessibility()
                == IMPORTANT_FOR_ACCESSIBILITY_NO_HIDE_DESCENDANTS) {
            return false;
        }
        parent = parent->getParent();
    }

    return mode == IMPORTANT_FOR_ACCESSIBILITY_YES;
/*	    || isActionableForAccessibility()
        || hasListenersForAccessibility() || getAccessibilityNodeProvider() != null
        || getAccessibilityLiveRegion() != ACCESSIBILITY_LIVE_REGION_NONE || isAccessibilityPane();*/
}

bool View::hasAncestorThatBlocksDescendantFocus()const{
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
         invalidate(true);
         refreshDrawableState();

         if (propagate && (!refocus || !rootViewRequestFocus())) notifyGlobalFocusCleared(this);        
    }
}

void View::notifyGlobalFocusCleared(View*oldFocus){
    if(oldFocus && mAttachInfo)
	mAttachInfo->mTreeObserver->dispatchOnGlobalFocusChange(oldFocus,nullptr);
}

bool View::rootViewRequestFocus() {
    View* root = getRootView();
    return root && root->requestFocus();
}

void View::handleFocusGainInternal(int direction,Rect*previouslyFocusedRect){
    if ((mPrivateFlags & PFLAG_FOCUSED) == 0) {
        mPrivateFlags |= PFLAG_FOCUSED;
        View* oldFocus =getRootView()?getRootView()->findFocus():nullptr;
        LOGV("%p :%d gained focused oldFocus:%p:%d",this,mID,oldFocus,(oldFocus?oldFocus->mID:-1));
        if (mParent != nullptr) {
            mParent->requestChildFocus(this, this);
            updateFocusedInCluster(oldFocus, direction);
        }

        if (mAttachInfo) mAttachInfo->mTreeObserver->dispatchOnGlobalFocusChange(oldFocus, this);

        onFocusChanged(true, direction, previouslyFocusedRect);
        invalidate(true);
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

void View::addTouchables(std::vector<View*>& views){
    const int viewFlags = mViewFlags;
    if (((viewFlags & CLICKABLE) == CLICKABLE || (viewFlags & LONG_CLICKABLE) == LONG_CLICKABLE
            || (viewFlags & CONTEXT_CLICKABLE) == CONTEXT_CLICKABLE)
            && (viewFlags & ENABLED_MASK) == ENABLED) {
        views.push_back((View*)this);
    }
}

void View::addFocusables(std::vector<View*>& views,int direction){
    addFocusables(views, direction, isInTouchMode() ? FOCUSABLES_TOUCH_MODE : FOCUSABLES_ALL);
}

void View::addFocusables(std::vector<View*>& views,int direction,int focusableMode){
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
    return mAttachInfo && mAttachInfo->mInTouchMode;
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

void View::addKeyboardNavigationClusters(std::vector<View*>&views,int drection){
    if(!isKeyboardNavigationCluster()) return;
    if(!hasFocusable())return;
    views.push_back((View*)this);
}

std::vector<View*>View::getFocusables(int direction){
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
    case FOCUS_BACKWARD: 
         if (mID == NO_ID) return nullptr;
         return root->findViewByPredicateInsideOut((View*)this,[this](const View*v) {
               return v->mNextFocusForwardId == mID;
         });
     }
     return nullptr;
}

View*View::findUserSetNextKeyboardNavigationCluster(View*root,int direction)const{
    switch (direction) {
    case FOCUS_FORWARD:
        if (mNextClusterForwardId == NO_ID) return nullptr;
            return findViewInsideOutShouldExist(root, mNextClusterForwardId);
    case FOCUS_BACKWARD: 
        if (mID == NO_ID) return nullptr;
        return root->findViewByPredicateInsideOut((View*)this,[this](const View*v){ 
            return v->mNextClusterForwardId == mID;
        });
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

bool View::hasParentWantsFocus()const{
    ViewGroup* parent = mParent;
    while (parent){
        ViewGroup* pv = (ViewGroup*) parent;
        if ((pv->mPrivateFlags & PFLAG_WANTS_FOCUS) != 0) {
            return true;
        }
        parent = pv->mParent;
    }
    return false;
}

bool View::isInLayout()const{
    ViewGroup* viewRoot = getRootView();
    return (viewRoot && viewRoot->isInLayout());
}

void View::onLayout(bool change,int l,int t,int w,int h){
}

bool View::shouldDrawRoundScrollbar()const{
    if(!ViewConfiguration::isScreenRound()||(mAttachInfo==nullptr)) return false;
    View* rootView = getRootView();
    const int height = getHeight();
    const int width  = getWidth();
    const int displayHeight = rootView->getHeight();
    const int displayWidth  = rootView->getWidth();
    if((height!=displayHeight)||(width!=displayWidth)) return false;
    return true;
}

void View::setTooltipText(const std::string& tooltipText) {
    if (tooltipText.empty()){//TextUtils::isEmpty(tooltipText)) {
        setFlags(0, TOOLTIP);
        hideTooltip();
        mTooltipInfo = nullptr;
    } else {
        setFlags(TOOLTIP, TOOLTIP);
        if (mTooltipInfo == nullptr) {
            mTooltipInfo = new TooltipInfo();
            mTooltipInfo->mShowTooltipRunnable = std::bind(&View::showHoverTooltip,this);
            mTooltipInfo->mHideTooltipRunnable = std::bind(&View::hideTooltip,this);
            mTooltipInfo->mHoverSlop = ViewConfiguration::get(mContext).getScaledHoverSlop();
            mTooltipInfo->clearAnchorPos();
        }
        mTooltipInfo->mTooltipText = tooltipText;
    }
}

std::string View::getTooltipText()const{
    return mTooltipInfo?mTooltipInfo->mTooltipText:std::string("");
}

void View::hideTooltip(){
    if (mTooltipInfo == nullptr) {
        return;
    }
    removeCallbacks(mTooltipInfo->mShowTooltipRunnable);
    if (mTooltipInfo->mTooltipPopup == nullptr) {
        return;
    }
    //mTooltipInfo->mTooltipPopup->hide();
    //mTooltipInfo->mTooltipPopup = nullptr;
    mTooltipInfo->mTooltipFromLongClick = false;
    mTooltipInfo->clearAnchorPos();
    if (mAttachInfo != nullptr) {
        mAttachInfo->mTooltipHost = nullptr;
    }
    // The available accessibility actions have changed
    //notifyViewAccessibilityStateChangedIfNeeded(CONTENT_CHANGE_TYPE_UNDEFINED);
}

bool View::showLongClickTooltip(int x, int y) {
    removeCallbacks(mTooltipInfo->mShowTooltipRunnable);
    removeCallbacks(mTooltipInfo->mHideTooltipRunnable);
    return showTooltip(x, y, true);
}

bool View::showHoverTooltip() {
    return showTooltip(mTooltipInfo->mAnchorX, mTooltipInfo->mAnchorY, false);
}

bool View::showTooltip(int x, int y, bool fromLongClick){
    if (mAttachInfo == nullptr || mTooltipInfo == nullptr) {
        return false;
    }
    if (fromLongClick && (mViewFlags & ENABLED_MASK) != ENABLED) {
        return false;
    }
    if (true){//TextUtils::isEmpty(mTooltipInfo->mTooltipText)) {
        return false;
    }
    hideTooltip();
    mTooltipInfo->mTooltipFromLongClick = fromLongClick;
    //mTooltipInfo->mTooltipPopup = new TooltipPopup(getContext());
    const bool fromTouch = (mPrivateFlags3 & PFLAG3_FINGER_DOWN) == PFLAG3_FINGER_DOWN;
    //mTooltipInfo->mTooltipPopup->show(this, x, y, fromTouch, mTooltipInfo.mTooltipText);
    mAttachInfo->mTooltipHost = this;
    // The available accessibility actions have changed
    //notifyViewAccessibilityStateChangedIfNeeded(CONTENT_CHANGE_TYPE_UNDEFINED);
    return true;
}

void View::layout(int l, int t, int w, int h){
    if ((mPrivateFlags3 & PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT) != 0) {
        onMeasure(mOldWidthMeasureSpec, mOldHeightMeasureSpec);
        mPrivateFlags3 &= ~PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT;
    }
    int oldL = mLeft;
    int oldT = mTop;
    int oldW = mRight-mLeft;
    int oldH = mBottom-mTop;
    mPrivateFlags &= ~PFLAG_FORCE_LAYOUT;
    mPrivateFlags3 |= PFLAG3_IS_LAID_OUT;
    bool changed = setFrame(l,t,w,h);
    if(changed|| ((mPrivateFlags & PFLAG_LAYOUT_REQUIRED) == PFLAG_LAYOUT_REQUIRED)){
        onLayout(changed, l, t, w, h);
        if (shouldDrawRoundScrollbar()) {
            if(mRoundScrollbarRenderer == nullptr)
                mRoundScrollbarRenderer = new RoundScrollbarRenderer(this);
        } else {
            mRoundScrollbarRenderer = nullptr;
        }
        if(mListenerInfo){
            for(auto ls:mListenerInfo->mOnLayoutChangeListeners){
                ls(*this,l, t, w, h,oldL,oldT,oldW,oldH);
            }
        }
        mPrivateFlags &= (~PFLAG_LAYOUT_REQUIRED);
    }

    const bool wasLayoutValid = isLayoutValid();

    mPrivateFlags &= ~PFLAG_FORCE_LAYOUT;
    mPrivateFlags3 |= PFLAG3_IS_LAID_OUT;

    if (!wasLayoutValid && isFocused()) {
        mPrivateFlags &= ~PFLAG_WANTS_FOCUS;
        if (canTakeFocus()) {
            // We have a robust focus, so parents should no longer be wanting focus.
            clearParentsWantFocus();
        } else if (getRootView() == nullptr || !getRootView()->isInLayout()) {
            // This is a weird case. Most-likely the user, rather than ViewRootImpl, called
            // layout. In this case, there's no guarantee that parent layouts will be evaluated
            // and thus the safest action is to clear focus here.
            clearFocusInternal(nullptr, /* propagate */ true, /* refocus */ false);
            clearParentsWantFocus();
        } else if (!hasParentWantsFocus()) {
            // original requestFocus was likely on this view directly, so just clear focus
            clearFocusInternal(nullptr, /* propagate */ true, /* refocus */ false);
        }
        // otherwise, we let parents handle re-assigning focus during their layout passes.
    } else if ((mPrivateFlags & PFLAG_WANTS_FOCUS) != 0) {
        mPrivateFlags &= ~PFLAG_WANTS_FOCUS;
        View* focused = findFocus();
        if (focused) {
            // Try to restore focus as close as possible to our starting focus.
            if (!restoreDefaultFocus() && !hasParentWantsFocus()) {
                // Give up and clear focus once we've reached the top-most parent which wants
                // focus.
                focused->clearFocusInternal(nullptr, /* propagate */ true, /* refocus */ false);
            }
        }
    }

    /*if ((mPrivateFlags3 & PFLAG3_NOTIFY_AUTOFILL_ENTER_ON_LAYOUT) != 0) {
        mPrivateFlags3 &= ~PFLAG3_NOTIFY_AUTOFILL_ENTER_ON_LAYOUT;
        //notifyEnterOrExitForAutoFillIfNeeded(true);
    }*/
}

void View::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    setMeasuredDimension(getDefaultSize(getSuggestedMinimumWidth(), widthMeasureSpec),
                getDefaultSize(getSuggestedMinimumHeight(), heightMeasureSpec));
}

void View::sizeChange(int newWidth,int newHeight,int oldWidth,int oldHeight){
    onSizeChanged(newWidth,newHeight,oldWidth,oldHeight);
    if(mOverlay)
        mOverlay->getOverlayView()->setFrame(mLeft,mTop,newWidth,newHeight);

    if(isLayoutValid()){
        if(newWidth<=0||newHeight<=0){
            if(hasFocus()){
                clearFocus();
                if(mParent)mParent->clearFocusedInCluster(); 
            }  
        }else if(oldWidth<=0||oldHeight<=0){
            if(mParent && canTakeFocus())
                 mParent->focusableViewAvailable(this);
        }
    } 
}

void View::onSizeChanged(int w,int h,int ow,int oh){
}

void View::onScrollChanged(int l, int t, int oldl, int oldt){
    mBackgroundSizeChanged = true;
    mBoundsChangedmDefaultFocusHighlightSizeChanged = true;
    if (mForegroundInfo != nullptr) {
        mForegroundInfo->mBoundsChanged = true;
    }
    if(mListenerInfo && mListenerInfo->mOnScrollChangeListener)
        mListenerInfo->mOnScrollChangeListener(*this,l,t,oldl,oldt);
}

void View::onFinishInflate(){
}

KeyEvent::DispatcherState* View::getKeyDispatcherState()const{
    return mAttachInfo ? &mAttachInfo->mKeyDispatchState : nullptr;
}

bool View::dispatchKeyEvent(KeyEvent&event){
    if(mInputEventConsistencyVerifier)
	mInputEventConsistencyVerifier->onKeyEvent(event,0);
    if (mListenerInfo && mListenerInfo->mOnKeyListener && (mViewFlags & ENABLED_MASK) == ENABLED
            && mListenerInfo->mOnKeyListener(*this, event.getKeyCode(), event)) {
        return true;
    }
    const bool result = event.dispatch(this,(mAttachInfo? &mAttachInfo->mKeyDispatchState : nullptr),this);
    LOGV("%s.%s=%d",event.getLabel(event.getKeyCode()),KeyEvent::actionToString(event.getAction()).c_str(),result);
    if(mInputEventConsistencyVerifier && (result==false))
        mInputEventConsistencyVerifier->onUnhandledEvent(event, 0);
    return result;
}

bool View::dispatchTooltipHoverEvent(MotionEvent& event){
    if (mTooltipInfo == nullptr) {
            return false;
    }
    switch(event.getAction()) {
    case MotionEvent::ACTION_HOVER_MOVE:
        if ((mViewFlags & TOOLTIP) != TOOLTIP) {
            break;
        }
        if (!mTooltipInfo->mTooltipFromLongClick && mTooltipInfo->updateAnchorPos(event)) {
            if (mTooltipInfo->mTooltipPopup == nullptr) {
                // Schedule showing the tooltip after a timeout.
                removeCallbacks(mTooltipInfo->mShowTooltipRunnable);
                postDelayed(mTooltipInfo->mShowTooltipRunnable,
                    ViewConfiguration::getHoverTooltipShowTimeout());
            }

            // Hide hover-triggered tooltip after a period of inactivity.
            // Match the timeout used by NativeInputManager to hide the mouse pointer
            // (depends on SYSTEM_UI_FLAG_LOW_PROFILE being set).
            const int  timeout = ViewConfiguration::getHoverTooltipHideTimeout();
            removeCallbacks(mTooltipInfo->mHideTooltipRunnable);
            postDelayed(mTooltipInfo->mHideTooltipRunnable, timeout);
        }
        return true;

    case MotionEvent::ACTION_HOVER_EXIT:
        mTooltipInfo->clearAnchorPos();
        if (!mTooltipInfo->mTooltipFromLongClick) {
            hideTooltip();
        }
        break;
    }
    return false;    
}

View* View::dispatchUnhandledKeyEvent(KeyEvent& evt) {
    if (onUnhandledKeyEvent(evt)) {
        return this;
    }
    return nullptr;
}

bool View::onUnhandledKeyEvent(KeyEvent& event) {
    if (mListenerInfo != nullptr){
        for (int i = mListenerInfo->mUnhandledKeyListeners.size() - 1; i >= 0; --i) {
            auto ls=mListenerInfo->mUnhandledKeyListeners.at(i);
            if (ls && ls(*this, event)) {
                return true;
            }
        }
    }
    return false;
}

bool View::hasUnhandledKeyListener()const{
    return mListenerInfo && mListenerInfo->mUnhandledKeyListeners.size();
}

void View::addOnUnhandledKeyEventListener(OnUnhandledKeyEventListener listener) {
    std::vector<OnUnhandledKeyEventListener>& listeners = getListenerInfo()->mUnhandledKeyListeners;
    listeners.push_back(listener);
    if (listeners.size() == 1 && mParent) {
        mParent->incrementChildUnhandledKeyListeners();
    }
}

void View::removeOnUnhandledKeyEventListener(OnUnhandledKeyEventListener listener) {
    if (mListenerInfo && mListenerInfo->mUnhandledKeyListeners.size()) {
        std::vector<OnUnhandledKeyEventListener>& listeners=mListenerInfo->mUnhandledKeyListeners;
        auto it=std::find(listeners.begin(),listeners.end(),listener);
        if(it!=listeners.end())
            listeners.erase(it);
        if (listeners.size()==0) {
            mParent->decrementChildUnhandledKeyListeners();
        }
    }
}

/** This method is the last chance for the focused view and its ancestors to
  * respond to an arrow key. This is called when the focused view did not
  * consume the key internally, nor could the view system find a new view in
  * the requested direction to give focus to.*/
bool View::dispatchUnhandledMove(View* focused,int direction){
    return false;
}

bool View::onKeyDown(int keyCode,KeyEvent& evt){
    int mc=InputMethodManager::getInstance().getCharacter(keyCode,evt.getMetaState());
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
                if (!event.isCanceled())
                    return performClickInternal();
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

bool View::canReceivePointerEvents()const{
    return (mViewFlags & VISIBILITY_MASK) == VISIBLE || getAnimation() != nullptr;
}

bool View::dispatchGenericMotionEventInternal(MotionEvent& event){
    if (mListenerInfo && mListenerInfo->mOnGenericMotionListener
          && (mViewFlags & ENABLED_MASK) == ENABLED
          && mListenerInfo->mOnGenericMotionListener(*this, event)) {
        return true;
    }
    if (onGenericMotionEvent(event)) {
        return true;
    }

    const int actionButton = event.getActionButton();
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
    if(mInputEventConsistencyVerifier)
        mInputEventConsistencyVerifier->onUnhandledEvent(event, 0);
    return false;
}

bool View::dispatchGenericPointerEvent(MotionEvent& event) {
    return false;
}

bool View::dispatchGenericFocusedEvent(MotionEvent& event) {
    return false;
}

bool View::dispatchPointerEvent(MotionEvent& event){
    if (event.isTouchEvent()) {
        return dispatchTouchEvent(event);
    } else {
        return dispatchGenericMotionEvent(event);
    }
}

bool View::dispatchHoverEvent(MotionEvent& event){
    if (mListenerInfo && mListenerInfo->mOnHoverListener
                && (mViewFlags & ENABLED_MASK) == ENABLED
                && mListenerInfo->mOnHoverListener(*this, event)) {
            return true;
    }
    return onHoverEvent(event);
}

bool View::hasHoveredChild() {
    return false;
}

bool View::pointInHoveredChild(MotionEvent& event) {
    return false;
}

bool View::dispatchGenericMotionEvent(MotionEvent&event){
    const int source = event.getSource();
    if (mInputEventConsistencyVerifier)
         mInputEventConsistencyVerifier->onGenericMotionEvent(event, 0);
    if ((source & InputDevice::SOURCE_CLASS_POINTER) != 0) {
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
    if (mInputEventConsistencyVerifier)
        mInputEventConsistencyVerifier->onUnhandledEvent(event, 0);
    return false;
}

bool View::dispatchTouchEvent(MotionEvent&event){
    bool result = false;
    const int actionMasked = event.getActionMasked();
    if (mInputEventConsistencyVerifier)
        mInputEventConsistencyVerifier->onTouchEvent(event, 0);

    if (actionMasked == MotionEvent::ACTION_UP ||
            actionMasked == MotionEvent::ACTION_CANCEL ||
            (actionMasked == MotionEvent::ACTION_DOWN && !result)) {
        stopNestedScroll();
    }

    if ((mViewFlags & ENABLED_MASK) == ENABLED && handleScrollBarDragging(event)) {
        result = true;
    }

    if ( mListenerInfo && mListenerInfo->mOnTouchListener
            && (mViewFlags & ENABLED_MASK) == ENABLED
            && mListenerInfo->mOnTouchListener(*this, event)) {
        result = true;
    }

    if(!result&& onTouchEvent(event)){
        result=true;
    }
    if (!result && mInputEventConsistencyVerifier)
        mInputEventConsistencyVerifier->onUnhandledEvent(event, 0);
    if (actionMasked == MotionEvent::ACTION_UP ||
         actionMasked == MotionEvent::ACTION_CANCEL ||
         (actionMasked == MotionEvent::ACTION_DOWN && !result)) {
        stopNestedScroll();
    }

    return result;
}

bool View::onInterceptTouchEvent(MotionEvent&event){
    return false;
}

bool View::onGenericMotionEvent(MotionEvent& event){
    return false;
}

bool View::onHoverEvent(MotionEvent& event){
    /*if (mTouchDelegate != null && dispatchTouchExplorationHoverEvent(event)) {
         return true;
    }*/

    // The root view may receive hover (or touch) events that are outside the bounds of
    // the window.  This code ensures that we only send accessibility events for
    // hovers that are actually within the bounds of the root view.
    const int action = event.getActionMasked();
    if (!mSendingHoverAccessibilityEvents) {
        if ((action == MotionEvent::ACTION_HOVER_ENTER
                || action == MotionEvent::ACTION_HOVER_MOVE)
                && !hasHoveredChild()
                && pointInView(event.getX(), event.getY(),0)) {
            sendAccessibilityHoverEvent(AccessibilityEvent::TYPE_VIEW_HOVER_ENTER);
            mSendingHoverAccessibilityEvents = true;
        }
    } else {
        if (action == MotionEvent::ACTION_HOVER_EXIT
                || (action == MotionEvent::ACTION_HOVER_MOVE
                        && !pointInView(event.getX(), event.getY(),0))) {
            mSendingHoverAccessibilityEvents = false;
            sendAccessibilityHoverEvent(AccessibilityEvent::TYPE_VIEW_HOVER_EXIT);
        }
    }

    if ((action == MotionEvent::ACTION_HOVER_ENTER || action == MotionEvent::ACTION_HOVER_MOVE)
            && event.isFromSource(InputDevice::SOURCE_MOUSE)
            && isOnScrollbar(event.getX(), event.getY())) {
        awakenScrollBars();
    }

    // If we consider ourself hoverable, or if we we're already hovered,
    // handle changing state in response to ENTER and EXIT events.
    if (isHoverable() || isHovered()) {
        switch (action) {
        case MotionEvent::ACTION_HOVER_ENTER:
            setHovered(true);
            break;
        case MotionEvent::ACTION_HOVER_EXIT:
            setHovered(false);
            break;
        }

        // Dispatch the event to onGenericMotionEvent before returning true.
        // This is to provide compatibility with existing applications that
        // handled HOVER_MOVE events in onGenericMotionEvent and that would
        // break because of the new default handling for hoverable views
        // in onHoverEvent.
        // Note that onGenericMotionEvent will be called by default when
        // onHoverEvent returns false (refer to dispatchGenericMotionEvent).
        dispatchGenericMotionEventInternal(event);
        // The event was already handled by calling setHovered(), so always
        // return true.
        return true;
    }
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
    bool result = false;
    if(mListenerInfo && mListenerInfo->mOnClickListener){
         playSoundEffect(SoundEffectConstants::CLICK);
         mListenerInfo->mOnClickListener(*this);
         result = true;
    }
    return result;
}

bool View::performLongClickInternal(float x, float y){
    bool handled = false;
    if(mListenerInfo && mListenerInfo->mOnLongClickListener)
        handled = mListenerInfo->mOnLongClickListener(*this);
    if((mViewFlags & TOOLTIP)==TOOLTIP){
        if(!handled)handled = showLongClickTooltip((int) x, (int) y);
    }
    return handled;
}

bool View::performLongClick(){
   return performLongClickInternal(mLongClickX,mLongClickY);
}

bool View::performLongClick(float x,float y){
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

bool View::performContextClick(float x, float y) {
    return performContextClick();
}

bool View::performContextClick() {
    sendAccessibilityEvent(AccessibilityEvent::TYPE_VIEW_CONTEXT_CLICKED);
    bool handled = false;
    if (mListenerInfo && mListenerInfo->mOnContextClickListener) {
        handled = mListenerInfo->mOnContextClickListener(*this);
    }
    if (handled) {
        performHapticFeedback(HapticFeedbackConstants::CONTEXT_CLICK);
    }
    return handled;
}

bool View::showContextMenu(){
    return mParent->showContextMenuForChild(this);
}

bool View::showContextMenu(float x, float y){
    return mParent->showContextMenuForChild(this, x, y);
}

bool View::performButtonActionOnTouchDown(MotionEvent& event) {
    if (event.isFromSource(InputDevice::SOURCE_MOUSE) &&
        (event.getButtonState() & MotionEvent::BUTTON_SECONDARY) != 0) {
        showContextMenu(event.getX(), event.getY());
        mPrivateFlags |= PFLAG_CANCEL_NEXT_UP_EVENT;
        return true;
    }
    return false;
}

void View::checkForLongClick(int delayOffset,int x,int y){
    //LOGV("%p:%d checkForLongClick longclickable=%d",this,mID,(mViewFlags & LONG_CLICKABLE) == LONG_CLICKABLE );
    if ((mViewFlags & LONG_CLICKABLE) == LONG_CLICKABLE || (mViewFlags & TOOLTIP) == TOOLTIP) {
        mHasPerformedLongPress = false;
        if (mPendingCheckForLongPress == nullptr) {
            mPendingCheckForLongPress = new CheckForLongPress(this);//new Runnable;
        }
        mPendingCheckForLongPress->setAnchor(x,y);
        mPendingCheckForLongPress->rememberWindowAttachCount();
        mPendingCheckForLongPress->rememberPressedState();
        mPendingCheckForLongPress->postDelayed(ViewConfiguration::getLongPressTimeout()-delayOffset);
    }
}

void View::removeTapCallback() {
    if (mPendingCheckForTap != nullptr) {
        mPrivateFlags &= ~PFLAG_PREPRESSED;
        mPendingCheckForTap->removeCallbacks();
        delete mPendingCheckForTap;
        mPendingCheckForTap=nullptr;
    }
}

void View::cancelLongPress() {
    removeLongPressCallback();
    /* The prepressed state handled by the tap callback is a display
     * construct, but the tap callback will post a long press callback
     * less its own timeout. Remove it here.*/
    removeTapCallback();
}

void View::removeLongPressCallback() {
    if (mPendingCheckForLongPress != nullptr) {
        mPendingCheckForLongPress->removeCallbacks();
    }
}

void View::removePerformClickCallback(){
    if(mPerformClick!=nullptr){
        removeCallbacks(mPerformClick);
    } 
}

void View::removeUnsetPressCallback() {
    //LOGV("removeUnsetPressCallback pressed=%d",mPrivateFlags & PFLAG_PRESSED);
    if ((mPrivateFlags & PFLAG_PRESSED) != 0 && mUnsetPressedState != nullptr) {
        setPressed(false);
        removeCallbacks(mUnsetPressedState);
    }
}

bool View::handleScrollBarDragging(MotionEvent& event) {
    const float x = event.getX();
    const float y = event.getY();
    const int action = event.getAction();
    if (mScrollCache == nullptr) return false;
    if ((mScrollCache->mScrollBarDraggingState == ScrollabilityCache::NOT_DRAGGING
            && action != MotionEvent::ACTION_DOWN)
            || !event.isFromSource(InputDevice::SOURCE_MOUSE)
            || !event.isButtonPressed(MotionEvent::BUTTON_PRIMARY)) {
        mScrollCache->mScrollBarDraggingState = ScrollabilityCache::NOT_DRAGGING;
        return false;
    }

    switch (action) {
    case MotionEvent::ACTION_MOVE:
        if (mScrollCache->mScrollBarDraggingState == ScrollabilityCache::NOT_DRAGGING){
            return false;
        }
        if (mScrollCache->mScrollBarDraggingState == ScrollabilityCache::DRAGGING_VERTICAL_SCROLL_BAR){
            Rect& bounds = mScrollCache->mScrollBarBounds;
            getVerticalScrollBarBounds(&bounds, nullptr);
            const int range = computeVerticalScrollRange();
            const int offset = computeVerticalScrollOffset();
            const int extent = computeVerticalScrollExtent();

            const int thumbLength = ViewConfiguration::getThumbLength(
                    bounds.height, bounds.width, extent, range);
            const int thumbOffset = ViewConfiguration::getThumbOffset(
                            bounds.height, thumbLength, extent, range, offset);

            const float diff = y - mScrollCache->mScrollBarDraggingPos;
            const float maxThumbOffset = bounds.height - thumbLength;
            const float newThumbOffset =std::min(std::max(thumbOffset + diff, 0.0f), maxThumbOffset);
            const int height = getHeight();
            if (std::round(newThumbOffset) != thumbOffset && maxThumbOffset > 0
                    && height > 0 && extent > 0) {
                const int newY = std::round((range - extent)
                        / ((float)extent / height) * (newThumbOffset / maxThumbOffset));
                if (newY != getScrollY()) {
                    mScrollCache->mScrollBarDraggingPos = y;
                    setScrollY(newY);
                }
            }
            return true;
        }
        if (mScrollCache->mScrollBarDraggingState
                == ScrollabilityCache::DRAGGING_HORIZONTAL_SCROLL_BAR) {
            Rect& bounds = mScrollCache->mScrollBarBounds;
            getHorizontalScrollBarBounds(&bounds, nullptr);
            const int range = computeHorizontalScrollRange();
            const int offset = computeHorizontalScrollOffset();
            const int extent = computeHorizontalScrollExtent();

            const int thumbLength = ViewConfiguration::getThumbLength(
                    bounds.width, bounds.height, extent, range);
            const int thumbOffset = ViewConfiguration::getThumbOffset(
                            bounds.width, thumbLength, extent, range, offset);

            const float diff = x - mScrollCache->mScrollBarDraggingPos;
            const float maxThumbOffset = bounds.width - thumbLength;
            const float newThumbOffset = std::min(std::max(thumbOffset + diff, 0.0f), maxThumbOffset);
            const int width = getWidth();
            if (std::round(newThumbOffset) != thumbOffset && maxThumbOffset > 0
                    && width > 0 && extent > 0) {
                const int newX = std::round((range - extent)
                        / ((float)extent / width) * (newThumbOffset / maxThumbOffset));
                if (newX != getScrollX()) {
                    mScrollCache->mScrollBarDraggingPos = x;
                    setScrollX(newX);
                }
            }
            return true;
        }
    case MotionEvent::ACTION_DOWN:
        if (mScrollCache->state == ScrollabilityCache::OFF) {
            return false;
        }
        if (isOnVerticalScrollbarThumb(x, y)) {
            mScrollCache->mScrollBarDraggingState =
                 ScrollabilityCache::DRAGGING_VERTICAL_SCROLL_BAR;
            mScrollCache->mScrollBarDraggingPos = y;
            return true;
        }
        if (isOnHorizontalScrollbarThumb(x, y)) {
            mScrollCache->mScrollBarDraggingState =
                 ScrollabilityCache::DRAGGING_HORIZONTAL_SCROLL_BAR;
            mScrollCache->mScrollBarDraggingPos = x;
            return true;
        }
    }
    mScrollCache->mScrollBarDraggingState = ScrollabilityCache::NOT_DRAGGING;
    return false;
}

bool View::onTouchEvent(MotionEvent& event){
    const int x = event.getX();
    const int y = event.getY();
    const int action = event.getAction();
    const bool clickable = ((mViewFlags&CLICKABLE) == CLICKABLE  || (mViewFlags&LONG_CLICKABLE) == LONG_CLICKABLE);
    bool prepressed;

    if ((mViewFlags & ENABLED_MASK) == DISABLED) {
        if (action == MotionEvent::ACTION_UP && (mPrivateFlags & PFLAG_PRESSED) != 0) {
            setPressed(false);
        }
        mPrivateFlags3 &= ~PFLAG3_FINGER_DOWN;
        // A disabled view that is clickable still consumes the touch
        // events, it just doesn't respond to them.
        return clickable;
    }

    if (!(clickable || (mViewFlags & TOOLTIP) == TOOLTIP))return false; 

    switch(action){
    case MotionEvent::ACTION_UP:
        mPrivateFlags3 &= ~PFLAG3_FINGER_DOWN;
        if(mPrivateFlags&PFLAG_PRESSED);
        if (!clickable){
            removeTapCallback();
            removeLongPressCallback();
            mHasPerformedLongPress = false;
            mIgnoreNextUpEvent = false;
            break;
        }

        prepressed = (mPrivateFlags & PFLAG_PREPRESSED) != 0;
        if ((mPrivateFlags & PFLAG_PRESSED) != 0 || prepressed) {
            bool focusTaken = false;
            LOGV("%p:%d focusable=%d,%d,%d longpress=%d",this,mID,isFocusable(),isFocusableInTouchMode(),isFocused(),mHasPerformedLongPress);
            if (isFocusable() && isFocusableInTouchMode() && !isFocused()) {
                focusTaken = requestFocus();
            }

            if (prepressed)setPressed(true);
            if(!mHasPerformedLongPress && !mIgnoreNextUpEvent){
                removeLongPressCallback();
                if (!focusTaken){
                    if(mPerformClick == nullptr){
                        mPerformClick= [this](){performClickInternal();};
                    }
                    if(!post(mPerformClick))performClickInternal();
                }
            }
            if(isAttachedToWindow()){
                if(mUnsetPressedState == nullptr){
                    mUnsetPressedState=[this]{setPressed(false);};
                }
                postDelayed(mUnsetPressedState,ViewConfiguration::getPressedStateDuration());
                removeTapCallback();
            }
        }
        mIgnoreNextUpEvent=false;
        break; 
    case MotionEvent::ACTION_DOWN:
        mHasPerformedLongPress=false;
        if (!clickable) {
            checkForLongClick(0, x, y);
            break;
        }

        if(performButtonActionOnTouchDown(event))break;

        if(isInScrollingContainer()){
            mPrivateFlags |= PFLAG_PREPRESSED;
            if(mPendingCheckForTap == nullptr){
                mPendingCheckForTap = new CheckForTap(this);//Runnable;
                mPendingCheckForTap->setAnchor(x,y);
            }
            mPendingCheckForTap->postDelayed(ViewConfiguration::getTapTimeout());
        }else{
            setPressed(true,x,y);
            checkForLongClick(0, x, y);
        }
        break;
    case MotionEvent::ACTION_MOVE:
        if (clickable)drawableHotspotChanged(x, y);

        // Be lenient about moving outside of buttons
        if (!pointInView(x, y,ViewConfiguration::get(mContext).getScaledTouchSlop())) {
            // Outside button Remove any future long press/tap checks
            removeTapCallback();
            removeLongPressCallback();
            if ((mPrivateFlags & PFLAG_PRESSED) != 0) {
                setPressed(false);
            }
            mPrivateFlags3 &= ~PFLAG3_FINGER_DOWN;
        }
        break;
    case MotionEvent::ACTION_CANCEL:
        if (clickable) setPressed(false);
        removeTapCallback();
        removeLongPressCallback();
        mInContextButtonPress = false;
        mHasPerformedLongPress = false;
        mIgnoreNextUpEvent = false;
        mPrivateFlags3 &= ~PFLAG3_FINGER_DOWN;
        break;
    }
    return true;
}

void View::postOnAnimation(Runnable& action){
    postDelayed(action,0);
}

void View::postOnAnimationDelayed(Runnable& action, uint32_t delayMillis){
    postDelayed(action,delayMillis);
}

bool View::post(Runnable& what){
    return postDelayed(what,0);
}

bool  View::postDelayed(Runnable& what,uint32_t delay){
    View*root = getRootView();
    if(root&&(root!=this))
        return root->postDelayed(what,delay);
    return false;
}

bool View::post(const std::function<void()>&what){
    Runnable r;
    r = what;
    return post(r);
}

bool View::postDelayed(const std::function<void()>&what,uint32_t delay){
    Runnable r;
    r=what;
    return postDelayed(r,delay);    
}

bool View::removeCallbacks(const Runnable& what){
    View*root = getRootView();
    if( root && (root!=this) )
        return root->removeCallbacks(what);
    return false;
}

ViewOverlay*View::getOverlay(){
    if (mOverlay == nullptr) {
        mOverlay = new ViewOverlay(mContext, this);
        mOverlay->getOverlayView()->setFrame(mLeft,mTop,mRight-mLeft,mBottom-mTop);
    }
    return mOverlay;
}

///////////////////////////////////////////////////////////////////////////////////////
//   For Layout support

void View::requestLayout(){
    if(mAttachInfo && mAttachInfo->mViewRequestingLayout==nullptr){
        ViewGroup*viewRoot= getRootView();
        if(viewRoot && viewRoot->isInLayout()){
            if(!viewRoot->requestLayoutDuringLayout(this))return ;
        }
        mAttachInfo->mViewRequestingLayout = this;
    }
    mPrivateFlags |= PFLAG_FORCE_LAYOUT;
    mPrivateFlags |= PFLAG_INVALIDATED;
    if (mParent != nullptr && !mParent->isLayoutRequested()) {
        mParent->requestLayout();
    }
    if ( mAttachInfo && (mAttachInfo->mViewRequestingLayout == this) ) {
         mAttachInfo->mViewRequestingLayout = nullptr;
    }
}

void View::forceLayout(){
    mMeasureCache.clear();
    mPrivateFlags |= PFLAG_FORCE_LAYOUT;
    mPrivateFlags |= PFLAG_INVALIDATED;
}

/**Returns true if this view has been through at least one layout since it
 * was last attached to or detached from a window. */
bool View::isLaidOut()const{
    return (mPrivateFlags3 & PFLAG3_IS_LAID_OUT) == PFLAG3_IS_LAID_OUT;
}

/** @return {@code true} if laid-out and not about to do another layout.*/
bool View::isLayoutValid()const{
     return isLaidOut() && ((mPrivateFlags & PFLAG_FORCE_LAYOUT) == 0);
}

bool View::isLayoutRequested()const{
    return (mPrivateFlags & PFLAG_FORCE_LAYOUT) == PFLAG_FORCE_LAYOUT;
}

bool View::hasRtlSupport()const{
    return true;//mContext.getApplicationInfo().hasRtlSupport();
}

bool View::isRtlCompatibilityMode()const{
    //const int targetSdkVersion = getContext().getApplicationInfo().targetSdkVersion;
    return !hasRtlSupport();//targetSdkVersion < Build.VERSION_CODES.JELLY_BEAN_MR1 || !hasRtlSupport();
}

bool View::isTextDirectionInherited()const{
    return (getRawTextDirection() == TEXT_DIRECTION_INHERIT);
}

bool View::isTextDirectionResolved()const{
    return (mPrivateFlags2 & PFLAG2_TEXT_DIRECTION_RESOLVED) == PFLAG2_TEXT_DIRECTION_RESOLVED;
}

bool View::resolveTextDirection(){
    // Reset any previous text direction resolution
    mPrivateFlags2 &= ~(PFLAG2_TEXT_DIRECTION_RESOLVED | PFLAG2_TEXT_DIRECTION_RESOLVED_MASK);

    if (hasRtlSupport()) {
        // Set resolved text direction flag depending on text direction flag
        int textDirection = getRawTextDirection();
        int parentResolvedDirection;
        switch(textDirection) {
        case TEXT_DIRECTION_INHERIT:
            if (!canResolveTextDirection()) {
                // We cannot do the resolution if there is no parent, so use the default one
                mPrivateFlags2 |= PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT;
                // Resolution will need to happen again later
                return false;
            }

            // Parent has not yet resolved, so we still return the default
            if (!mParent->isTextDirectionResolved()) {
                mPrivateFlags2 |= PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT;
                // Resolution will need to happen again later
                return false;
            }
            // Set current resolved direction to the same value as the parent's one
            parentResolvedDirection = mParent->getTextDirection();
            switch (parentResolvedDirection) {
            case TEXT_DIRECTION_FIRST_STRONG:
            case TEXT_DIRECTION_ANY_RTL:
            case TEXT_DIRECTION_LTR:
            case TEXT_DIRECTION_RTL:
            case TEXT_DIRECTION_LOCALE:
            case TEXT_DIRECTION_FIRST_STRONG_LTR:
            case TEXT_DIRECTION_FIRST_STRONG_RTL:
                mPrivateFlags2 |= (parentResolvedDirection << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT);
                break;
            default:
                // Default resolved direction is "first strong" heuristic
                mPrivateFlags2 |= PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT;
            }
            break;
        case TEXT_DIRECTION_FIRST_STRONG:
        case TEXT_DIRECTION_ANY_RTL:
        case TEXT_DIRECTION_LTR:
        case TEXT_DIRECTION_RTL:
        case TEXT_DIRECTION_LOCALE:
        case TEXT_DIRECTION_FIRST_STRONG_LTR:
        case TEXT_DIRECTION_FIRST_STRONG_RTL:
            // Resolved direction is the same as text direction
            mPrivateFlags2 |= (textDirection << PFLAG2_TEXT_DIRECTION_RESOLVED_MASK_SHIFT);
            break;
        default:
            // Default resolved direction is "first strong" heuristic
             mPrivateFlags2 |= PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT;
        }
    }else {
        // Default resolved direction is "first strong" heuristic
        mPrivateFlags2 |= PFLAG2_TEXT_DIRECTION_RESOLVED_DEFAULT;
    }

    // Set to resolved
    mPrivateFlags2 |= PFLAG2_TEXT_DIRECTION_RESOLVED;
    return true;
}

bool View::isLayoutModeOptical(View*p){
    if(dynamic_cast<ViewGroup*>(p)){
        return ((ViewGroup*)p)->isLayoutModeOptical();
    }
    return false;
}

bool View::needRtlPropertiesResolution()const{
    return (mPrivateFlags2 & ALL_RTL_PROPERTIES_RESOLVED) != ALL_RTL_PROPERTIES_RESOLVED;
}

void View::onRtlPropertiesChanged(int layoutDirection){
}

bool View::resolveLayoutDirection(){
    // Clear any previous layout direction resolution
    mPrivateFlags2 &= ~PFLAG2_LAYOUT_DIRECTION_RESOLVED_MASK;

    if (hasRtlSupport()) {
        // Set resolved depending on layout direction
        switch ((mPrivateFlags2 & PFLAG2_LAYOUT_DIRECTION_MASK) >> PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT) {
        case LAYOUT_DIRECTION_INHERIT:
            // We cannot resolve yet. LTR is by default and let the resolution happen again
            // later to get the correct resolved value
            if (!canResolveLayoutDirection()) return false;

            // Parent has not yet resolved, LTR is still the default
            if (!mParent->isLayoutDirectionResolved()) return false;
            if (mParent->getLayoutDirection() == LAYOUT_DIRECTION_RTL) {
                mPrivateFlags2 |= PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL;
            }
            break;
        case LAYOUT_DIRECTION_RTL:
            mPrivateFlags2 |= PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL;
            break;
        case LAYOUT_DIRECTION_LOCALE:
            /*if((LAYOUT_DIRECTION_RTL ==
                    TextUtils.getLayoutDirectionFromLocale(Locale.getDefault()))) {
                mPrivateFlags2 |= PFLAG2_LAYOUT_DIRECTION_RESOLVED_RTL;
            }*/
            break;
        default:break;// Nothing to do, LTR by default
        }
    }

    // Set to resolved
    mPrivateFlags2 |= PFLAG2_LAYOUT_DIRECTION_RESOLVED;
    return true;
}

bool View::canResolveLayoutDirection()const{
    switch (getRawLayoutDirection()) {
    case LAYOUT_DIRECTION_INHERIT:
        if (mParent != nullptr) {
            return mParent->canResolveLayoutDirection();
        }
        return false;

    default: return true;
    }
}

void View::resetResolvedPadding(){
    resetResolvedPaddingInternal();
}

void View::resetResolvedPaddingInternal() {
    mPrivateFlags2 &= ~PFLAG2_PADDING_RESOLVED;
}

bool View::resolveRtlPropertiesIfNeeded(){
    if (!needRtlPropertiesResolution()) return false;

    // Order is important here: LayoutDirection MUST be resolved first
    if (!isLayoutDirectionResolved()) {
        resolveLayoutDirection();
        resolveLayoutParams();
    }
    // ... then we can resolve the others properties depending on the resolved LayoutDirection.
    if (!isTextDirectionResolved()) {
        resolveTextDirection();
    }
    if (!isTextAlignmentResolved()) {
        resolveTextAlignment();
    }
    // Should resolve Drawables before Padding because we need the layout direction of the
    // Drawable to correctly resolve Padding.
    if (!areDrawablesResolved()) {
        resolveDrawables();
    }
    if (!isPaddingResolved()) {
        resolvePadding();
    }
    onRtlPropertiesChanged(getLayoutDirection());
    return true;

}

void View::resetRtlProperties(){
    resetResolvedLayoutDirection();
    resetResolvedTextDirection();
    resetResolvedTextAlignment();
    resetResolvedPadding();
    resetResolvedDrawables();
}

int View::getDefaultSize(int size, int measureSpec) {
    int result = size;
    const int specMode = MeasureSpec::getMode(measureSpec);
    const int specSize = MeasureSpec::getSize(measureSpec);

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

void View::setSoundEffectsEnabled(bool soundEffectsEnabled) {
    setFlags(soundEffectsEnabled ? SOUND_EFFECTS_ENABLED: 0, SOUND_EFFECTS_ENABLED);
}

bool View::isSoundEffectsEnabled()const{
    return SOUND_EFFECTS_ENABLED == (mViewFlags & SOUND_EFFECTS_ENABLED);
}

void View::playSoundEffect(int soundConstant){
    if((mAttachInfo==nullptr)||(mAttachInfo->mPlaySoundEffect==nullptr)||(isSoundEffectsEnabled()==false))
        return ;
    mAttachInfo->mPlaySoundEffect(soundConstant);
}

bool View::performHapticFeedback(int feedbackConstant, int flags){
    if (mAttachInfo == nullptr) {
        return false;
    }
    //noinspection SimplifiableIfStatement
    if ((flags & HapticFeedbackConstants::FLAG_IGNORE_VIEW_SETTING) == 0
            && !isHapticFeedbackEnabled()) {
        return false;
    }
    return mAttachInfo->mPerformHapticFeedback(feedbackConstant,
            (flags & HapticFeedbackConstants::FLAG_IGNORE_GLOBAL_SETTING) != 0);
}

void View::setHapticFeedbackEnabled(bool hapticFeedbackEnabled) {
    setFlags(hapticFeedbackEnabled ? HAPTIC_FEEDBACK_ENABLED: 0, HAPTIC_FEEDBACK_ENABLED);
}

bool View::isHapticFeedbackEnabled() const{
    return HAPTIC_FEEDBACK_ENABLED == (mViewFlags & HAPTIC_FEEDBACK_ENABLED);
}

void View::setMeasuredDimensionRaw(int measuredWidth, int measuredHeight) {
    mMeasuredWidth = measuredWidth;
    mMeasuredHeight = measuredHeight;
    mPrivateFlags |= PFLAG_MEASURED_DIMENSION_SET;
}

void View::setMeasuredDimension(int measuredWidth, int measuredHeight){
    const bool optical = isLayoutModeOptical(this);
    if (optical != isLayoutModeOptical(mParent)) {
        Insets insets = getOpticalInsets();
        const int opticalWidth  = insets.left + insets.right;
        const int opticalHeight = insets.top  + insets.bottom;

        measuredWidth  += optical ? opticalWidth  : -opticalWidth;
        measuredHeight += optical ? opticalHeight : -opticalHeight;
    }
    setMeasuredDimensionRaw(measuredWidth, measuredHeight);
}

int View::getBaseline(){
    return -1;
}

void View::getDrawingRect(Rect& outRect) const {
    outRect.left  = mScrollX;
    outRect.top   = mScrollY;
    outRect.width = getWidth();
    outRect.height= getHeight();
}

long View::getDrawingTime() const{
    return mAttachInfo ? mAttachInfo->mDrawingTime : 0;
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

bool View::hasIdentityMatrix()const{
    return mRenderNode->hasIdentityMatrix();
}

Matrix& View::getMatrix() {
    ensureTransformationInfo();
    Matrix& matrix=mTransformationInfo->mMatrix;
    mRenderNode->getMatrix(matrix);
    return matrix;
}

Matrix& View::getInverseMatrix() {
    ensureTransformationInfo();
    Matrix& matrix=mTransformationInfo->mInverseMatrix;
    mRenderNode->getInverseMatrix(matrix);
    return matrix;
}

float View::getX()const{
    return mLeft+getTranslationX();
}

float View::getY()const{
    return mTop+getTranslationY();
}

float View::getZ()const{
    return getElevation() + getTranslationZ();
}

void View::setZ(float z){
    setTranslationZ(z - getElevation());
}

float View::getElevation()const{
    return mRenderNode->getElevation();
}

void View::setElevation(float elevation){
    mRenderNode->setElevation(elevation);
}

void View::setX(float x){
    setTranslationX(x-mLeft);
}

void View::setY(float y){
    setTranslationY(y-mTop);
}

void View::setScaleX(float x){
    if(x != getScaleX()){
        invalidateViewProperty(true,false);
        mRenderNode->setScaleX((x==.0f)?.00001f:x);//scale cant be zero
        invalidateViewProperty(false,true);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

float View::getScaleX()const{
    return mRenderNode->getScaleX();
}

void View::setScaleY(float y){
    if(y!=getScaleY()){
        invalidateViewProperty(true,false);
        mRenderNode->setScaleY((y==.0f)?.00001f:y);//scale cant be zero
        invalidateViewProperty(false,true);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

float View::getScaleY()const{
    return mRenderNode->getScaleY();
}

void View::setTranslationX(float x){
    if(x != getTranslationX()){
        invalidateViewProperty(true,false);
        mRenderNode->setTranslationX(x);
        invalidateViewProperty(false,true);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

float View::getTranslationX()const{
    return mRenderNode->getTranslationX();
}

void View::setTranslationY(float y){
    if(y!=getTranslationY()){
        invalidateViewProperty(true,false);
        mRenderNode->setTranslationY(y); 
        invalidateViewProperty(false,true);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

float View::getTranslationY()const{
    return mRenderNode->getTranslationY();
}

void View::setTranslationZ(float z){
    if(z!=getTranslationZ()){
        invalidateViewProperty(true,false);
        mRenderNode->setTranslationZ(z);
        invalidateViewProperty(false,true);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

float View::getTranslationZ()const{
    return mRenderNode->getTranslationZ();
}

float View::getRotation()const{
    return mRenderNode->getRotation();
}

/* Sets the degrees that the view is rotated around the pivot point. Increasing values
 * result in clockwise rotation.*/
void View::setRotation(float rotation){
    if(rotation != getRotation()){
        invalidateViewProperty(true,false);
        mRenderNode->setRotation(rotation);
        invalidateViewProperty(false,true);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

float View::getRotationX()const{
    return mRenderNode->getRotationX();
}

void View::setRotationX(float x){
    if(x!= getRotationX()){
        mRenderNode->setRotationX(x);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

float View::getRotationY()const{
    return mRenderNode->getRotationY();
}

void View::setRotationY(float y){
     mRenderNode->setRotationY(y); 
}

float View::getPivotX()const{
    return mRenderNode->getPivotX();
}

void View::setPivotX(float x){
    if(x!=getPivotX()){
        invalidateViewProperty(true,false);
        mRenderNode->setPivotX(x);
        invalidateViewProperty(false,true);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

float View::getPivotY()const{
    return mRenderNode->getPivotY();
}

void View::setPivotY(float y){
    if(y!=getPivotY()){
        invalidateViewProperty(true,false);
        mRenderNode->setPivotY(y);
        invalidateViewProperty(false,true);
        invalidateParentIfNeededAndWasQuickRejected();
    }
}

bool View::isPivotSet()const{
    return false;
}

void View::resetPivot(){
    mRenderNode->setPivotX(getWidth()/2.f);
    mRenderNode->setPivotY(getHeight()/2.f);
}

float View::getAlpha()const{
    return mTransformationInfo  ? mTransformationInfo->mAlpha : 1.f;
}

void View::setAlpha(float alpha){
    ensureTransformationInfo();
    if(mTransformationInfo->mAlpha != alpha){
        setAlphaInternal(alpha);
        if(onSetAlpha(int(alpha*255))){
            mPrivateFlags |= PFLAG_ALPHA_SET;
            invalidateParentCaches();
            invalidate(true);
        }else{
            mPrivateFlags &= ~PFLAG_ALPHA_SET;
            invalidateViewProperty(true,false);
            mRenderNode->setAlpha(getFinalAlpha());
        }
        invalidate();
    }
}

bool View::setAlphaNoInvalidation(float alpha){
    ensureTransformationInfo();
    if(mTransformationInfo->mAlpha != alpha){
        setAlphaInternal(alpha);
        const bool subclassHandlesAlpha = onSetAlpha(int(alpha*255));
        if(subclassHandlesAlpha){
            mPrivateFlags |= PFLAG_ALPHA_SET;
            return true;
        }else{
            mPrivateFlags &= ~PFLAG_ALPHA_SET;
            mRenderNode->setAlpha(getFinalAlpha());
        }
    }
    return false;
}

void View::setAlphaInternal(float alpha){
    const float oldAlpha = mTransformationInfo->mAlpha;
    mTransformationInfo->mAlpha = alpha;
    if((alpha==0)^(oldAlpha==0)){
	    //notifySubtreeAccessibilityStateChangedIfNeeded();
    }
}

float View::getFinalAlpha()const{
    if(mTransformationInfo)
        return mTransformationInfo->mAlpha*mTransformationInfo->mTransitionAlpha;
    return 1;
}

float View::getTransitionAlpha()const{
    return mTransformationInfo?mTransformationInfo->mTransitionAlpha:1;
}

void View::setTransitionAlpha(float alpha){
    ensureTransformationInfo();
    if(mTransformationInfo->mTransitionAlpha!=alpha){
        mTransformationInfo->mTransitionAlpha = alpha;
        mPrivateFlags &= ~PFLAG_ALPHA_SET;
        invalidateViewProperty(true,false);
        mRenderNode->setAlpha(getFinalAlpha());
    }
}

StateListAnimator* View::getStateListAnimator()const{
    return mStateListAnimator;
}

void View::setStateListAnimator(StateListAnimator*stateListAnimator){
    if (mStateListAnimator == stateListAnimator) {
        return;
    }
    if (mStateListAnimator != nullptr) {
        mStateListAnimator->setTarget(nullptr);
    }
    mStateListAnimator = stateListAnimator;
    if (stateListAnimator != nullptr) {
        stateListAnimator->setTarget(this);
        if (isAttachedToWindow()) {
            stateListAnimator->setState(getDrawableState());
        }
    }    
}

LayoutParams*View::getLayoutParams(){
    return mLayoutParams;
}

void View::setLayoutParams(LayoutParams*params){
    if( mLayoutParams && (params!=mLayoutParams) )
        delete mLayoutParams;
    mLayoutParams = params;
    resolveLayoutParams();
    if(mParent)((ViewGroup*) mParent)->onSetLayoutParams(this,params);
    requestLayout();
}

int View::getRawLayoutDirection()const{
    return (mPrivateFlags2 & PFLAG2_LAYOUT_DIRECTION_MASK) >> PFLAG2_LAYOUT_DIRECTION_MASK_SHIFT;
}

void View::resetResolvedLayoutDirection() {
    // Reset the current resolved bits
    mPrivateFlags2 &= ~PFLAG2_LAYOUT_DIRECTION_RESOLVED_MASK;
}

bool View::isLayoutDirectionInherited()const{
    return (getRawLayoutDirection() == LAYOUT_DIRECTION_INHERIT);
}

void View::resolveLayoutParams() {
    if(mLayoutParams)
        mLayoutParams->resolveLayoutDirection(getLayoutDirection());
}

static bool operator<(const Size&a,const Size&b){
    if(a.x!=b.x)
       return a.x<b.x;
    else if(a.y!=b.y)
       return a.y<b.y;
    return  false;
}

ViewPropertyAnimator& View::animate() {
   if (mAnimator == nullptr) {
       mAnimator = new ViewPropertyAnimator(this);
   }
   return *mAnimator;
}

void View::measure(int widthMeasureSpec, int heightMeasureSpec){
    bool optical = isLayoutModeOptical(this);
    if (optical != isLayoutModeOptical(mParent)) {
        Insets insets = getOpticalInsets();
        int oWidth  = insets.left + insets.right;
        int oHeight = insets.top  + insets.bottom;
        widthMeasureSpec  = MeasureSpec::adjust(widthMeasureSpec,  optical ? -oWidth  : oWidth);
        heightMeasureSpec = MeasureSpec::adjust(heightMeasureSpec, optical ? -oHeight : oHeight);
    }

    // Suppress sign extension for the low bytes
    Size key ={widthMeasureSpec,heightMeasureSpec};

    const bool forceLayout = (mPrivateFlags & PFLAG_FORCE_LAYOUT) == PFLAG_FORCE_LAYOUT;

    // Optimize layout by avoiding an extra EXACTLY pass when the view is
    // already measured as the correct size. In API 23 and below, this
    // extra pass is required to make LinearLayout re-distribute weight.
    const bool specChanged = (widthMeasureSpec != mOldWidthMeasureSpec)  || (heightMeasureSpec != mOldHeightMeasureSpec);
    const bool isSpecExactly = (MeasureSpec::getMode(widthMeasureSpec) == MeasureSpec::EXACTLY)
                && (MeasureSpec::getMode(heightMeasureSpec) == MeasureSpec::EXACTLY);
    const bool matchesSpecSize = (getMeasuredWidth() == MeasureSpec::getSize(widthMeasureSpec))
                && (getMeasuredHeight() == MeasureSpec::getSize(heightMeasureSpec));
    const bool needsLayout = specChanged  && ( sAlwaysRemeasureExactly || !isSpecExactly || !matchesSpecSize);

    if (forceLayout || needsLayout) {
        // first clears the measured dimension flag
        mPrivateFlags &= ~PFLAG_MEASURED_DIMENSION_SET;

        resolveRtlPropertiesIfNeeded();
        auto itc = mMeasureCache.find(key);
        if ( (itc == mMeasureCache.end()) || forceLayout || sIgnoreMeasureCache ) {
            // measure ourselves, this should set the measured dimension flag back
            onMeasure(widthMeasureSpec, heightMeasureSpec);
            mPrivateFlags3 &= ~PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT;
        } else {
            Size value =itc->second;// mMeasureCache.valueAt(cacheIndex);
            // Casting a long to int drops the high 32 bits, no mask needed
            setMeasuredDimensionRaw(value.x,value.y);
            mPrivateFlags3 |= PFLAG3_MEASURE_NEEDED_BEFORE_LAYOUT;
        }

        // flag not set, setMeasuredDimension() was not invoked, we raise
        // an exception to warn the developer
        if ((mPrivateFlags & PFLAG_MEASURED_DIMENSION_SET) != PFLAG_MEASURED_DIMENSION_SET) {
            LOGE("View with id %d : #onMeasure() did not set the measured dimension by calling setMeasuredDimension()",mID);
        }
        mPrivateFlags |= PFLAG_LAYOUT_REQUIRED;
    }

    mOldWidthMeasureSpec = widthMeasureSpec;
    mOldHeightMeasureSpec = heightMeasureSpec;
    Size szMeasured = { mMeasuredWidth , mMeasuredHeight };
    mMeasureCache[key] = szMeasured;//
    //mMeasureCache.insert(std::pair<Size,Size>(key,szMeasured)); // suppress sign extension
}

View::AttachInfo::InvalidateInfo::InvalidateInfo(){
    target = nullptr;
    rect.set(0,0,0,0);
}
std::vector<View::AttachInfo::InvalidateInfo*>View::AttachInfo::InvalidateInfo::sPool;

View::AttachInfo::InvalidateInfo*View::AttachInfo::InvalidateInfo::obtain(){
    AttachInfo::InvalidateInfo*ret = nullptr;
    if(sPool.empty()){
	ret = new AttachInfo::InvalidateInfo();
    }else{
        ret = sPool.back();
        sPool.pop_back();
    }
    ret->rect.set(0,0,0,0);
    return ret;
}

void View::AttachInfo::InvalidateInfo::recycle(){
    sPool.push_back(this);
}

View::AttachInfo::AttachInfo(Context*ctx){
    mDisplay = nullptr;
    mHardwareAccelerated =false;
    mWindowVisibility = VISIBLE;
    mHasWindowFocus   = true;
    mApplicationScale = 1.0f;
    mScalingRequired  = false;
    mIgnoreDirtyState = false;
    mUse32BitDrawingCache = false;
    mAlwaysConsumeSystemBars= false;
    mDebugLayout  = false;
    mDrawingTime  = 0;
    mInTouchMode  = true;
    mKeepScreenOn = true;
    mRootView     = nullptr;
    mCanvas       = nullptr;
    mDisplayState = true;
    mTooltipHost  = nullptr;
    mViewRequestingLayout = nullptr;
    mAutofilledDrawable = nullptr;
    mTreeObserver = new ViewTreeObserver(ctx);
    mWindowLeft = 0;
    mWindowTop  = 0;
}

View::AttachInfo::~AttachInfo(){
    delete mTreeObserver;
    delete mAutofilledDrawable; 
}

View::CheckForTap::CheckForTap(View*v)
    :mView(v){
    mRunnable = std::bind(&CheckForTap::run,this);
}
void View::CheckForTap::setAnchor(float x,float y){
    mX = x;
    mY = y;
}

void View::CheckForTap::run(){
    mView->mPrivateFlags &= ~PFLAG_PRESSED;
    mView->setPressed(true,mX,mY);
    mView->checkForLongClick(ViewConfiguration::getTapTimeout(),mX, mY);
    LOGD("View::CheckForTap %p:%d",mView,mView->mID);
}

void View::CheckForTap::postDelayed(long ms){
    mView->postDelayed(mRunnable,ms);
}

void View::CheckForTap::removeCallbacks(){
    mView->removeCallbacks(mRunnable);
}

View::CheckForLongPress::CheckForLongPress(View*v)
   :CheckForTap(v){
    mRunnable = std::bind(&CheckForLongPress::run,this);
}

void View::CheckForLongPress::rememberWindowAttachCount(){
    mOriginalWindowAttachCount = mView->mWindowAttachCount;
}

void View::CheckForLongPress::rememberPressedState(){
    mOriginalPressedState = mView->isPressed();
}

void View::CheckForLongPress::run(){
    LOGD("View::CheckForLongPress %p:%d",mView,mView->mID);
    if( (mOriginalPressedState == mView->isPressed()) && mView->mParent
        && (mOriginalWindowAttachCount == mView->mWindowAttachCount)){
        if(mView->performLongClick(mX,mY)){
            mView->mHasPerformedLongPress = true;
        }
    }
}

View::TintInfo::TintInfo(){
    mTintList = nullptr;
    mHasTintList = false;
    mHasTintMode = false;
    mTintMode = PorterDuff::Mode::SRC_IN;
}

View::TintInfo::~TintInfo(){
    delete mTintList;
}

View::ForegroundInfo::ForegroundInfo(){
    mInsidePadding = mBoundsChanged = true;
    mGravity = Gravity::FILL;
    mSelfBounds.set(0,0,0,0);
    mOverlayBounds.set(0,0,0,0);
    mTintInfo = nullptr;
    mDrawable = nullptr;
}

View::ForegroundInfo::~ForegroundInfo(){
    delete mDrawable;
    delete mTintInfo;
}

bool View::TooltipInfo::updateAnchorPos(MotionEvent&event){
    const int newAnchorX = event.getX();
    const int newAnchorY = event.getY();
    if((std::abs(newAnchorX-mAnchorX)<=mHoverSlop)&& (std::abs(newAnchorY-mAnchorY)<=mHoverSlop))
        return false;
    mAnchorX = newAnchorX;
    mAnchorY = newAnchorY;
    return true;
}
void View::TooltipInfo::clearAnchorPos(){
    mAnchorX = INT_MAX;
    mAnchorY = INT_MAX;
}

View::TransformationInfo::TransformationInfo(){
    mMatrix = identity_matrix();
    mInverseMatrix = identity_matrix();
    mAlpha = 1.f;
    mTransitionAlpha = 1.f;
}

View::ScrollabilityCache::ScrollabilityCache(ViewConfiguration&configuration,View*host){//int sz){
    fadingEdgeLength = configuration.getScaledFadingEdgeLength();
    scrollBarSize = configuration.getScaledScrollBarSize();
    scrollBarMinTouchTarget = configuration.getScaledMinScrollbarTouchTarget();
    scrollBarDefaultDelayBeforeFade = ViewConfiguration::getScrollDefaultDelay();
    scrollBarFadeDuration = ViewConfiguration::getScrollBarFadeDuration();
    fadeScrollBars = true;
    scrollBar = nullptr;
    state = OFF;
    mScrollBarDraggingPos = 0;
    mScrollBarBounds.set(0,0,0,0);
    mScrollBarTouchBounds.set(0,0,0,0);
    shader = LinearGradient::create(0,0,0,1);
    shader->add_color_stop_rgba(0.f,0,0,0,1.f);
    shader->add_color_stop_rgba(1.f,0,0,0,0.f);
    this->host = host;
    mRunner = [this](){
       run();
    };
}

View::ScrollabilityCache::~ScrollabilityCache(){
    host->removeCallbacks(mRunner);
    delete scrollBar;
}

void View::ScrollabilityCache::run(){
    long now = AnimationUtils::currentAnimationTimeMillis();
    if (host && (now >= fadeStartTime)) {
        // the animation fades the scrollbars out by changing
        // the opacity (alpha) from fully opaque to fully
        // transparent
        state = FADING;
        // Kick off the fade animation
        host->invalidate(mScrollBarBounds);
    }
}

int View::MeasureSpec::makeMeasureSpec(int size,int mode){
    return (size & ~MODE_MASK) | (mode & MODE_MASK);
}

int View::MeasureSpec::makeSafeMeasureSpec(int size, int mode){
    return makeMeasureSpec(size,mode);
}

int View::MeasureSpec::getMode(int measureSpec){
    //noinspection ResourceType
    return (measureSpec & MODE_MASK);
}

int View::MeasureSpec::getSize(int measureSpec){
    return (measureSpec & ~MODE_MASK);
}

int View::MeasureSpec::adjust(int measureSpec, int delta) {
    int mode = getMode(measureSpec);
    int size = getSize(measureSpec);
    if (mode == UNSPECIFIED) {
        // No need to adjust size for UNSPECIFIED mode.
        return makeMeasureSpec(size, UNSPECIFIED);
    }
    size += delta;
    if (size < 0) {
        LOGE("MeasureSpec.adjust: new size would be negative! (%d) spec:%s  delta:%d",size,
                                toString(measureSpec).c_str(),delta);
        size = 0;
    }
    return makeMeasureSpec(size, mode);
}

const std::string View::MeasureSpec::toString(int measureSpec) {
    int mode = getMode(measureSpec);
    int size = getSize(measureSpec);

        std::ostringstream sb;
    if (mode == UNSPECIFIED)  sb<<"UNSPECIFIED ";
    else if (mode == EXACTLY) sb<<"EXACTLY ";
    else if (mode == AT_MOST) sb<<"AT_MOST ";
    else     sb<<mode<<" ";
    sb<<size;
    return sb.str();
}

View::BaseSavedState::BaseSavedState(Parcel& source)
    :AbsSavedState(source){
    mSavedData = source.readInt();
    mStartActivityRequestWhoSaved = source.readString();
    mIsAutofilled = source.readBoolean();
    mHideHighlight = source.readBoolean();
    mAutofillViewId = source.readInt();
}

View::BaseSavedState::BaseSavedState(Parcelable* superState)
   :AbsSavedState(superState){
}

void View::BaseSavedState::writeToParcel(Parcel& out, int flags) {
    AbsSavedState::writeToParcel(out, flags);
    out.writeInt(mSavedData);
    out.writeString(mStartActivityRequestWhoSaved);
    out.writeBoolean(mIsAutofilled);
    out.writeBoolean(mHideHighlight);
    out.writeInt(mAutofillViewId);
}
}//endof namespace
