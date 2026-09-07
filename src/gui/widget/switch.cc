#include <widget/internal_R.h>
#include <core/context.h>
#include <widget/switch.h>
#include <widget/framework_styleable.h>
#include <widget/internal_R.h>
#include <utils/mathutils.h>
#include <text/textutils.h>
#include <view/viewgroup.h>
#include <text/method/allcapstransformationmethod.h>

namespace cdroid{
using namespace cdroid::internal;

DECLARE_WIDGET2(Switch, "android.widget.Switch");

Switch::Switch(Context*ctx)
    :Switch(ctx,nullptr){}

Switch::Switch(Context* context,const AttributeSet* a):Switch(context,a,cdroid::internal::R::attr::switchStyle){}

Switch::Switch(Context* context,const AttributeSet* pAttrs,int defStyleAttr)
  :CompoundButton(context,pAttrs, defStyleAttr){
    init();
    // Phase 2: TypedArray (binary AXML typed resolution). ta=null → text XML fallback.
    auto ta = context->obtainStyledAttributes(pAttrs, R::styleable::Switch, defStyleAttr);
    

    mThumbDrawable = ta->getDrawable(R::styleable::Switch_thumb);
    if (mThumbDrawable) {
        mThumbDrawable->setCallback(this);
    }
    mTrackDrawable = ta->getDrawable(R::styleable::Switch_track);
    if (mTrackDrawable) {
        mTrackDrawable->setCallback(this);
    }
    mTextOn = ta->getString(R::styleable::Switch_textOn);
    mTextOff = ta->getString(R::styleable::Switch_textOff);
    mShowText = ta->getBoolean(R::styleable::Switch_showText, true);
    mThumbTextPadding = ta->getDimensionPixelSize(R::styleable::Switch_thumbTextPadding, 0);
    mSwitchMinWidth = ta->getDimensionPixelSize(R::styleable::Switch_switchMinWidth, 0);
    mSwitchPadding = ta->getDimensionPixelSize(R::styleable::Switch_switchPadding, 0);
    mSplitTrack = ta->getBoolean(R::styleable::Switch_splitTrack, false);

    mUseFallbackLineSpacing = true;//context.getApplicationInfo().targetSdkVersion >= VERSION_CODES.P;

    mThumbTintList = ta->getColorStateList(R::styleable::Switch_thumbTint);
    mHasThumbTint = (mThumbTintList!=nullptr);

    const int thumbTintMode = ta->getInt(R::styleable::Switch_thumbTintMode, -1);
    if (mThumbBlendMode != thumbTintMode) {
        mThumbBlendMode = thumbTintMode;
        mHasThumbTintMode = true;
    }
    if (mHasThumbTint || mHasThumbTintMode) {
        applyThumbTint();
    }

    mTrackTintList = ta->getColorStateList(R::styleable::Switch_trackTint);
    mHasTrackTint = (mTrackTintList!=nullptr);

    const int trackTintMode = ta->getInt(R::styleable::Switch_trackTintMode, -1);
    if (mTrackBlendMode != trackTintMode) {
        mTrackBlendMode = trackTintMode;
        mHasTrackTintMode = true;
    }
    if (mHasTrackTint || mHasTrackTintMode) {
        applyTrackTint();
    }

    // AOSP reads the @StyleRes id (TextView_appearance is a style reference).
    const int textAppearance = ta->getResourceId(R::styleable::Switch_switchTextAppearance, 0);
    if (textAppearance != 0){
        setSwitchTextAppearance(context, textAppearance);
    }
    ViewConfiguration& config = ViewConfiguration::get(context);
    mTouchSlop = config.getScaledTouchSlop();
    mMinFlingVelocity = config.getScaledMinimumFlingVelocity();

    // Refresh display with current params
    refreshDrawableState();
    // Default state is derived from on/off-text, so state has to be updated when on/off-text
    // are updated.
    //setDefaultStateDescription();
    setChecked(isChecked());

}

class THUMB_POS:public FloatProperty{
public:
    THUMB_POS():FloatProperty("thumbPos"){}
    void set(void*object,const AnimateValue&value)const override {
        float fv = GET_VARIANT(value,float);
        ((Switch*)object)->setThumbPosition(fv);
    }
    AnimateValue get(void*object)const override{
        return ((Switch*)object)->mThumbPosition;
    }
};
static class THUMB_POS THUMB_POS;

void Switch::init(){
    mTouchMode =TOUCH_MODE_IDLE;
    mTextColors    = nullptr;
    mThumbDrawable = nullptr;
    mThumbTintList = nullptr;
    mTrackDrawable = nullptr;
    mTrackTintList = nullptr;
    mSwitchMinWidth= 0;
    mSwitchWidth = 0;
    mSwitchHeight= 0;
    mThumbWidth = 0;
    mPositionAnimator = nullptr;
    // AOSP leaves mOnLayout/mOffLayout null until the first onMeasure;
    // pre-creating empty layouts here would defeat that caching.
    mOnLayout  = nullptr;
    mOffLayout = nullptr;
    mOnText    = nullptr;
    mOffText   = nullptr;
    mOnTransformed  = nullptr;
    mOffTransformed = nullptr;
    mSwitchLeft= mSwitchRight  =0;
    mSwitchTop = mSwitchBottom =0;
    mSwitchTransformationMethod = nullptr;
    mVelocityTracker  = VelocityTracker::obtain();
}

Switch::~Switch(){
    delete mThumbDrawable;
    delete mTrackDrawable;
    delete mPositionAnimator;
    // Delete the layouts before their text: a Layout dtor may deref its text.
    delete mOnLayout;
    delete mOffLayout;
    // getTransformation may hand back the source itself instead of a new
    // object; same pointer-compare trick as TextView's mText/mTransformed.
    if (mOnTransformed == mOnText)   mOnTransformed  = nullptr;
    if (mOffTransformed == mOffText) mOffTransformed = nullptr;
    delete mOnText;
    delete mOffText;
    delete mOnTransformed;
    delete mOffTransformed;
    delete mSwitchTransformationMethod;
    mVelocityTracker->recycle();
}

void Switch::setSwitchTextAppearance(Context* context,int resid){
    // AOSP: obtainStyledAttributes(resid, R.styleable.TextAppearance) directly.
    auto ta = context->obtainStyledAttributes(resid, R::styleable::TextAppearance);

    auto colors = ta->getColorStateList(R::styleable::TextAppearance_textColor);
    if (colors) {
        mTextColors = colors;
    } else {
        // If no color set in TextAppearance, default to the view's textColor
        mTextColors = getTextColors();
    }

    int ts = ta->getDimensionPixelSize(R::styleable::TextAppearance_textSize, 0);
    if (ts != 0) {
        if (ts != mTextPaint.getTextSize()) {
            mTextPaint.setTextSize(ts);
            requestLayout();
        }
    }

    int typefaceIndex = ta->getInt(R::styleable::TextAppearance_typeface, -1);
    int styleIndex    = ta->getInt(R::styleable::TextAppearance_textStyle, -1);

    setSwitchTypefaceByIndex(typefaceIndex, styleIndex);

    const bool allCaps = ta->getBoolean(R::styleable::TextAppearance_textAllCaps, false);
    // AOSP nulls the method out when allCaps is off; delete the old one so a
    // second call with allCaps on does not leak the previous method either.
    delete mSwitchTransformationMethod;
    if (allCaps) {
        mSwitchTransformationMethod = new AllCapsTransformationMethod(getContext());
        mSwitchTransformationMethod->setLengthChangesAllowed(true);
    } else {
        mSwitchTransformationMethod = nullptr;
    }
}

void Switch::setSwitchTypefaceByIndex(int typefaceIndex, int styleIndex){
    Typeface* tf = nullptr;
    switch (typefaceIndex) {
    case SANS:
        tf = Typeface::SANS_SERIF;
        break;

    case SERIF:
        tf = Typeface::SERIF;
        break;

    case MONOSPACE:
        tf = Typeface::MONOSPACE;
        break;
    }
    setSwitchTypeface(tf, styleIndex);
}

void Switch::setSwitchTypeface(Typeface* tf, int style){
    if (style > 0) {
        if (tf == nullptr) {
            tf = Typeface::defaultFromStyle(style);
        } else {
            tf = Typeface::create(tf, style);
        }

        setSwitchTypeface(tf);
        // now compute what (if any) algorithmic styling is needed
        int typefaceStyle = tf ? tf->getStyle() : 0;
        int need = style & ~typefaceStyle;
        mTextPaint.setFakeBoldText((need & Typeface::BOLD) != 0);
        mTextPaint.setTextSkewX((need & Typeface::ITALIC) != 0 ? -0.25f : 0);
    } else {
        if (tf == nullptr) {
            tf = Typeface::defaultFromStyle(style);
        }
        mTextPaint.setFakeBoldText(false);
        mTextPaint.setTextSkewX(0);
        setSwitchTypeface(tf);
    }
}

void Switch::setSwitchTypeface(Typeface* tf){
    if (mTextPaint.getTypeface() != tf) {
        mTextPaint.setTypeface(tf);
        requestLayout();
        invalidate();
    }
}

void Switch::setSwitchPadding(int pixels) {
    mSwitchPadding = pixels;
    requestLayout();
}

int Switch::getSwitchPadding()const{
    return mSwitchPadding;
}

void Switch::setSwitchMinWidth(int pixels) {
    mSwitchMinWidth = pixels;
    requestLayout();
}

int Switch::getSwitchMinWidth()const{
    return mSwitchMinWidth;
}

void Switch::setThumbTextPadding(int pixels) {
    mThumbTextPadding = pixels;
    requestLayout();
}

int Switch::getThumbTextPadding()const{
    return mThumbTextPadding;
}

void Switch::setTrackDrawable(Drawable* track) {
    if (mTrackDrawable)
        mTrackDrawable->setCallback(nullptr);
    mTrackDrawable = track;
    if (track)
        track->setCallback(this);
    requestLayout();
}

void Switch::setTrackResource(int resId){
    setTrackDrawable(getContext()->getDrawable(resId));
}

Drawable* Switch::getTrackDrawable() {
    return mTrackDrawable;
}

void Switch::setTrackTintList(const cdroid::RefPtr<ColorStateList>& tint){
    if(mTrackTintList!=tint){
        mTrackTintList = tint;
        mHasTrackTint = (tint!=nullptr);
        applyTrackTint();
    }
}

const cdroid::RefPtr<ColorStateList> Switch::getTrackTintList() {
    return mTrackTintList;
}

void Switch::setTrackTintMode(PorterDuffMode tintMode){
    mTrackBlendMode = tintMode;
    mHasTrackTintMode = true;
    applyTrackTint();
}

void Switch::applyTrackTint(){
    if (mTrackDrawable && (mHasTrackTint || mHasTrackTintMode)) {
        mTrackDrawable = mTrackDrawable->mutate();

        if (mHasTrackTint) {
            mTrackDrawable->setTintList(mTrackTintList);
        }

        if (mHasTrackTintMode) {
            mTrackDrawable->setTintBlendMode(mTrackBlendMode);
        }

        // The drawable (or one of its children) may not have been
        // stateful before applying the tint, so let's try again.
        if (mTrackDrawable->isStateful()) {
            mTrackDrawable->setState(getDrawableState());
        }
    }
}

PorterDuffMode Switch::getTrackTintMode()const{
    return (PorterDuffMode)mTrackBlendMode;
}

void Switch::setThumbDrawable(Drawable* thumb){
    if (mThumbDrawable) {
        mThumbDrawable->setCallback(nullptr);
    }
    mThumbDrawable = thumb;
    if (thumb) {
        thumb->setCallback(this);
    }
    requestLayout();   
}

void Switch::setThumbResource(int resId){
    setThumbDrawable(getContext()->getDrawable(resId));
}

Drawable* Switch::getThumbDrawable(){
    return mThumbDrawable;
}

void Switch::setThumbTintList(const cdroid::RefPtr<ColorStateList> tint){
    if(mThumbTintList!=tint){
        mThumbTintList = tint;
        mHasThumbTint = (tint!=nullptr);
        applyThumbTint();
    }
}

const cdroid::RefPtr<ColorStateList> Switch::getThumbTintList()const{
    return mThumbTintList;
}

void Switch::setThumbTintMode(PorterDuffMode tintMode){
    mThumbBlendMode = tintMode;
    mHasThumbTintMode = true;
    applyThumbTint();
}

PorterDuffMode Switch::getThumbTintMode()const{
    return (PorterDuffMode)mThumbBlendMode;
}

void Switch::applyThumbTint() {
    if (mThumbDrawable && (mHasThumbTint || mHasThumbTintMode)) {
        mThumbDrawable = mThumbDrawable->mutate();

        if (mHasThumbTint) {
            mThumbDrawable->setTintList(mThumbTintList);
        }

        if (mHasThumbTintMode) mThumbDrawable->setTintBlendMode(mThumbBlendMode);

        // The drawable (or one of its children) may not have been
        // stateful before applying the tint, so let's try again.
        if (mThumbDrawable->isStateful()) {
            mThumbDrawable->setState(getDrawableState());
        }
    }
}

void Switch::setSplitTrack(bool splitTrack){
    mSplitTrack = splitTrack;
    invalidate();
}

bool Switch::getSplitTrack()const{
    return mSplitTrack;
}

std::string Switch::getTextOn()const{
    return mTextOn;
}

void Switch::setTextOn(const std::string&text){
    mTextOn = text;
    requestLayout();
}

std::string Switch::getTextOff()const{
    return mTextOff;
}

void Switch::setTextOff(const std::string&text){
    mTextOff = text;
    requestLayout();
}

bool Switch::getShowText()const{
    return mShowText;
}

void Switch::setShowText(bool showText) {
    if (mShowText != showText) {
        mShowText = showText;
        requestLayout();
    }
}
std::string Switch::getAccessibilityClassName()const{
    return "Switch";
}

void Switch::onPopulateAccessibilityEventInternal(AccessibilityEvent& event){
    CompoundButton::onPopulateAccessibilityEventInternal(event);
    const std::string text = isChecked() ? mTextOn : mTextOff;
    if (!text.empty()) {
         event.getText().push_back(text);
    }
}

void Switch::onInitializeAccessibilityNodeInfoInternal(AccessibilityNodeInfo& info){
    CompoundButton::onInitializeAccessibilityNodeInfoInternal(info);
    const std::string switchText = isChecked() ? mTextOn : mTextOff;
    if (!TextUtils::isEmpty(switchText)) {
        const std::string  oldText = info.getText();
        if (TextUtils::isEmpty(oldText)) {
            info.setText(switchText);
        } else {
            std::ostringstream newText;
            newText<<oldText<<' '<<switchText;
            info.setText(newText.str());
        }
    }
}

void Switch::onMeasure(int widthMeasureSpec, int heightMeasureSpec){
    if (mShowText) {
        if (mOnLayout == nullptr) {
            mOnText = new SpannedString(TextUtils::utf8_utf16(mTextOn));
            mOnTransformed = (mSwitchTransformationMethod != nullptr)
                    ? mSwitchTransformationMethod->getTransformation(*mOnText, *this)
                    : mOnText;
            mOnLayout = makeLayout(mOnTransformed);
        }
        if (mOffLayout == nullptr) {
            mOffText = new SpannedString(TextUtils::utf8_utf16(mTextOff));
            mOffTransformed = (mSwitchTransformationMethod != nullptr)
                    ? mSwitchTransformationMethod->getTransformation(*mOffText, *this)
                    : mOffText;
            mOffLayout = makeLayout(mOffTransformed);
        }
    }

    Rect padding;
    int thumbWidth;
    int thumbHeight;
    if (mThumbDrawable) {
        // Cached thumb width does not include padding.
        mThumbDrawable->getPadding(padding);
        thumbWidth = mThumbDrawable->getIntrinsicWidth() - padding.left - padding.width;
        thumbHeight= mThumbDrawable->getIntrinsicHeight();
    } else {
        thumbWidth = 0;
        thumbHeight= 0;
    }

    int maxTextWidth;
    if (mShowText) {
        maxTextWidth = std::max(mOnLayout->getWidth(), mOffLayout->getWidth())
                + mThumbTextPadding * 2;
    } else {
        maxTextWidth = 0;
    }

    mThumbWidth = std::max(maxTextWidth, thumbWidth);

    int trackHeight;
    if (mTrackDrawable) {
        mTrackDrawable->getPadding(padding);
        trackHeight = mTrackDrawable->getIntrinsicHeight();
    } else {
        padding.setEmpty();
        trackHeight = 0;
    }

    // Adjust left and right padding to ensure there's enough room for the
    // thumb's padding (when present).
    int paddingLeft  = padding.left;
    int paddingRight = padding.width;
    if (mThumbDrawable) {
        Insets inset = mThumbDrawable->getOpticalInsets();
        paddingLeft  = std::max(paddingLeft, inset.left);
        paddingRight = std::max(paddingRight, inset.right);
    }

    const int switchWidth = std::max(mSwitchMinWidth,2 * mThumbWidth + paddingLeft + paddingRight);
    const int switchHeight = std::max(trackHeight, thumbHeight);
    mSwitchWidth  = switchWidth;
    mSwitchHeight = switchHeight;
    CompoundButton::onMeasure(widthMeasureSpec, heightMeasureSpec);

    int measuredHeight = getMeasuredHeight();
    if (measuredHeight < switchHeight) {
        setMeasuredDimension(getMeasuredWidthAndState(), switchHeight);
    }
}

Layout* Switch::makeLayout(CharSequence* text){
    const int width = (int) std::ceil(Layout::getDesiredWidth(text, 0,
                text->length(), mTextPaint, getTextDirectionHeuristic()));
    auto builder= StaticLayout::Builder::obtain(text, 0, text->length(), &mTextPaint, width);
    return builder->setUseLineSpacingFromFallbacks(mUseFallbackLineSpacing).build();
}

bool Switch::hitThumb(float x, float y) {
    if (mThumbDrawable == nullptr) {
        return false;
    }

    // Relies on mTempRect, MUST be called first!
    const int thumbOffset = getThumbOffset();
    Rect mTempRect;
    mThumbDrawable->getPadding(mTempRect);
    const int thumbTop = mSwitchTop - mTouchSlop;
    const int thumbLeft = mSwitchLeft + thumbOffset - mTouchSlop;
    const int thumbRight = thumbLeft + mThumbWidth + mTempRect.left + mTempRect.width + mTouchSlop;
    const int thumbBottom = mSwitchBottom + mTouchSlop;
    return (x > thumbLeft) && (x < thumbRight) && (y > thumbTop) && (y < thumbBottom);
}

void Switch::onDetachedFromWindow(){
    CompoundButton::onDetachedFromWindow();
    cancelPositionAnimator();
    /*ValueAnimator.cancel()/end() will call endAnimation,and destroy it in it's endCallback*/
    if(mThumbDrawable)unscheduleDrawable(*mThumbDrawable);
    if(mTrackDrawable)unscheduleDrawable(*mTrackDrawable);
}

bool Switch::onTouchEvent(MotionEvent& ev){
    mVelocityTracker->addMovement(ev);
    const int action = ev.getActionMasked();
    const float x = ev.getX();
    const float y = ev.getY();
    switch (action) {
    case MotionEvent::ACTION_DOWN:
        if (isEnabled() && hitThumb(x, y)) {
            mTouchMode = TOUCH_MODE_DOWN;
            mTouchX = x;
            mTouchY = y;
        }
        break;

    case MotionEvent::ACTION_MOVE:
        switch (mTouchMode) {
        case TOUCH_MODE_IDLE:
            // Didn't target the thumb, treat normally.
            break;

        case TOUCH_MODE_DOWN:
            if (abs(x - mTouchX) > mTouchSlop ||
                abs(y - mTouchY) > mTouchSlop) {
                mTouchMode = TOUCH_MODE_DRAGGING;
                getParent()->requestDisallowInterceptTouchEvent(true);
                mTouchX = x;
                mTouchY = y;
                return true;
            }
            break;

        case TOUCH_MODE_DRAGGING: {
            const int thumbScrollRange = getThumbScrollRange();
            const float thumbScrollOffset = x - mTouchX;
            float dPos;
            if (thumbScrollRange != 0) {
                dPos = thumbScrollOffset / thumbScrollRange;
            } else {
                // If the thumb scroll range is empty, just use the
                // movement direction to snap on or off.
                dPos = thumbScrollOffset > 0 ? 1 : -1;
            }
            if (isLayoutRtl()) {
                dPos = -dPos;
            }
            const float newPos = MathUtils::constrain(mThumbPosition + dPos, .0f, 1.f);
            if (newPos != mThumbPosition) {
                 mTouchX = x;
                 setThumbPosition(newPos);
            }
            return true;
            }
       }
       break;

    case MotionEvent::ACTION_UP:
    case MotionEvent::ACTION_CANCEL:
        if (mTouchMode == TOUCH_MODE_DRAGGING) {
            stopDrag(ev);
            // Allow super class to handle pressed state, etc.
            CompoundButton::onTouchEvent(ev);
            return true;
        }
        mTouchMode = TOUCH_MODE_IDLE;
        mVelocityTracker->clear();
        break;
    }

    return CompoundButton::onTouchEvent(ev);
}

void Switch::cancelSuperTouch(MotionEvent& ev){
    MotionEvent* cancel = MotionEvent::obtain(ev);
    cancel->setAction(MotionEvent::ACTION_CANCEL);
    CompoundButton::onTouchEvent(*cancel);
    cancel->recycle();
}

void Switch::stopDrag(MotionEvent& ev){
    mTouchMode = TOUCH_MODE_IDLE;

    // Commit the change if the event is up and not canceled and the switch
    // has not been disabled during the drag.
    const bool commitChange = ev.getAction() == MotionEvent::ACTION_UP && isEnabled();
    const bool oldState = isChecked();
    bool newState;
    if (commitChange) {
        mVelocityTracker->computeCurrentVelocity(1000);
        const float xvel = mVelocityTracker->getXVelocity();
        if (abs(xvel) > mMinFlingVelocity) {
            newState = isLayoutRtl() ? (xvel < 0) : (xvel > 0);
        } else {
            newState = getTargetCheckedState();
        }
    } else {
        newState = oldState;
    }

    if (newState != oldState) {
        playSoundEffect(SoundEffectConstants::CLICK);
    }
    // Always call setChecked so that the thumb is moved back to the correct edge
    setChecked(newState);
    cancelSuperTouch(ev);
}

void Switch::animateThumbToCheckedState(bool newCheckedState){
    const float targetPosition = newCheckedState ? 1 : 0;
    AnimatorListenerAdapter animtorListener;
    ObjectAnimator* animator = ObjectAnimator::ofFloat(this,&THUMB_POS,{targetPosition});
    animator->setDuration(THUMB_ANIMATION_DURATION);
    animator->setAutoCancel(true);
    animtorListener.onAnimationEnd=[this](Animator&anim,bool){
        delete mPositionAnimator;
        mPositionAnimator = nullptr;
    };
    animator->addListener(animtorListener);
    mPositionAnimator = animator;
    animator->start();
}

void Switch::cancelPositionAnimator(){
    if (mPositionAnimator) {
        mPositionAnimator->cancel();
    }
}

bool Switch::getTargetCheckedState()const{
    return mThumbPosition > 0.5f;
}

void Switch::setThumbPosition(float position){
    mThumbPosition = position;
    invalidate();
}

std::string Switch::getButtonStateDescription(){
    if (isChecked()) {
        return mTextOn;// == null ? getResources().getString(R.string.capital_on) : mTextOn;
    } else {
        return mTextOff;// == null ? getResources().getString(R.string.capital_off) : mTextOff;
    }
}

void Switch::toggle(){
    setChecked(!isChecked());
}

void Switch::setChecked(bool checked){
    CompoundButton::setChecked(checked);

    // Calling the super method may result in setChecked() getting called
    // recursively with a different value, so load the REAL value...
    checked = isChecked();

    if (isAttachedToWindow() && isLaidOut()) {
        animateThumbToCheckedState(checked);
    } else {
        // Immediately move the thumb to the new position.
        cancelPositionAnimator();
        setThumbPosition(checked ? 1 : 0);
    }
}

void Switch::onLayout(bool changed, int left, int top, int width, int height){
    CompoundButton::onLayout(changed, left, top, width,height);

    int opticalInsetLeft = 0;
    int opticalInsetRight = 0;
    if (mThumbDrawable) {
        Rect trackPadding;
        if (mTrackDrawable) {
            mTrackDrawable->getPadding(trackPadding);
        } else {
            trackPadding.set(0,0,0,0);
        }

        Insets insets = mThumbDrawable->getOpticalInsets();
        opticalInsetLeft = std::max(0, insets.left - trackPadding.left);
        opticalInsetRight= std::max(0, insets.right - trackPadding.width);
    }

    int switchRight;
    int switchLeft;
    if (isLayoutRtl()) {
        switchLeft = getPaddingLeft() + opticalInsetLeft;
        switchRight = switchLeft + mSwitchWidth - opticalInsetLeft - opticalInsetRight;
    } else {
        switchRight = getWidth() - getPaddingRight() - opticalInsetRight;
        switchLeft = switchRight - mSwitchWidth + opticalInsetLeft + opticalInsetRight;
    }

    int switchTop;
    int switchBottom;
    switch (getGravity() & Gravity::VERTICAL_GRAVITY_MASK) {
    default:
    case Gravity::TOP:
        switchTop = getPaddingTop();
        switchBottom = switchTop + mSwitchHeight;
        break;

    case Gravity::CENTER_VERTICAL:
        switchTop = (getPaddingTop() + getHeight() - getPaddingBottom()) / 2 -
                mSwitchHeight / 2;
        switchBottom = switchTop + mSwitchHeight;
        break;

    case Gravity::BOTTOM:
        switchBottom = getHeight() - getPaddingBottom();
        switchTop = switchBottom - mSwitchHeight;
        break;
    }

    mSwitchLeft = switchLeft;
    mSwitchTop = switchTop;
    mSwitchBottom = switchBottom;
    mSwitchRight = switchRight;
}

void Switch::draw(Canvas& c) {
    Rect padding;
    const int switchLeft = mSwitchLeft;
    const int switchTop = mSwitchTop;
    const int switchRight = mSwitchRight;
    const int switchBottom = mSwitchBottom;

    int thumbInitialLeft = switchLeft + getThumbOffset();

    Insets thumbInsets;
    if (mThumbDrawable) {
        thumbInsets = mThumbDrawable->getOpticalInsets();
    } else {
        thumbInsets = Insets::NONE;
    }

    // Layout the track.
    if (mTrackDrawable) {
        mTrackDrawable->getPadding(padding);

        // Adjust thumb position for track padding.
        thumbInitialLeft += padding.left;
        // If necessary, offset by the optical insets of the thumb asset.
        int trackLeft = switchLeft;
        int trackTop = switchTop;
        int trackRight = switchRight;
        int trackBottom = switchBottom;
        if (thumbInsets != Insets::NONE) {
            if (thumbInsets.left > padding.left) {
                trackLeft += thumbInsets.left - padding.left;
            }
            if (thumbInsets.top > padding.top) {
                trackTop += thumbInsets.top - padding.top;
            }
            if (thumbInsets.right > padding.width) {
                trackRight -= thumbInsets.right - padding.width;
            }
            if (thumbInsets.bottom > padding.height) {
                trackBottom -= thumbInsets.bottom - padding.height;
            }
        }
        /* CDROID setBounds is (x, y, w, h) — the old AOSP (l, t, r, b) form
           passed right/bottom as the size and the capsule's right end was
           clipped into a square corner. */
        mTrackDrawable->setBounds(trackLeft, trackTop, trackRight - trackLeft, trackBottom - trackTop);
    }

    // Layout the thumb.
    if (mThumbDrawable) {
        mThumbDrawable->getPadding(padding);
        const int thumbLeft  = thumbInitialLeft - padding.left;
        const int thumbRight = thumbInitialLeft + mThumbWidth + padding.width;
        const int thumbWidth = thumbRight - thumbLeft; 
        mThumbDrawable->setBounds(thumbLeft, switchTop, thumbWidth, switchBottom-switchTop);
        Drawable* background = getBackground();
        if (background) {
            background->setHotspotBounds(thumbLeft, switchTop, thumbWidth, switchBottom-switchTop);
        }
    }

    // Draw the background.
    CompoundButton::draw(c);
}

void Switch::onDraw(Canvas& canvas) {
    CompoundButton::onDraw(canvas);

    Rect padding;
    if (mTrackDrawable) {
        mTrackDrawable->getPadding(padding);
    } else {
        padding.setEmpty();
    }

    const int switchTop = mSwitchTop;
    const int switchBottom = mSwitchBottom;
    const int switchInnerTop = switchTop + padding.top;
    const int switchInnerBottom = switchBottom - padding.height;

    if (mTrackDrawable) {
        if (mSplitTrack && mThumbDrawable) {
            Insets insets = mThumbDrawable->getOpticalInsets();
            padding = mThumbDrawable->getBounds();
            // AOSP: padding.left += insets.left; padding.right -= insets.right
            // → x += il and width shrinks by il + ir to keep both edges moving in.
            padding.left += insets.left;
            padding.width -= insets.left + insets.right;
            canvas.save();
            /* AOSP: clipRect(padding, Op.DIFFERENCE) — draw the track everywhere
               except the thumb's optical bounds. cairo clips by intersection only,
               so clip to the union of the two side strips instead. */
            canvas.rectangle(0, 0, padding.left, getHeight());
            canvas.rectangle(padding.right(), 0, getWidth() - padding.right(), getHeight());
            canvas.clip();
            mTrackDrawable->draw(canvas);
            canvas.restore();
        } else {
            mTrackDrawable->draw(canvas);
        }
    }
    canvas.save();

    if (mThumbDrawable) {
        mThumbDrawable->draw(canvas);
    }

    Layout* switchText = getTargetCheckedState() ? mOnLayout : mOffLayout;
    if (switchText) {
        std::vector<int> drawableState = getDrawableState();
        if (mTextColors) {
            const int stateColor = mTextColors->getColorForState(drawableState, 0);
            mTextPaint.setColor(stateColor);
        }
        //mTextPaint.drawableState = drawableState;
        int cX;
        if (mThumbDrawable) {
            Rect bounds = mThumbDrawable->getBounds();
            /* AOSP: cX = bounds.left + bounds.right (both coordinates, twice the
               thumb center). CDROID Rect stores width, so right() restores the
               AOSP coordinate — reading bounds.width here would left-shift the
               text center by half the thumb position. */
            cX = bounds.left + bounds.right();
        } else {
            cX = getWidth();
        }
        const int left = cX / 2 - switchText->getWidth() / 2;
        const int top = (switchInnerTop + switchInnerBottom) / 2 - switchText->getHeight() / 2;
        canvas.translate(left, top);
        switchText->draw(canvas);
    }
    canvas.restore();
}

int Switch::getCompoundPaddingLeft() const{
    if (!isLayoutRtl()) {
        return CompoundButton::getCompoundPaddingLeft();
    }
    int padding = CompoundButton::getCompoundPaddingLeft() + mSwitchWidth;
    if (!TextUtils::isEmpty(this->mText)) {
        padding += mSwitchPadding;
    }
    return padding;
}

int Switch::getCompoundPaddingRight() const{
    if (isLayoutRtl()) {
        return CompoundButton::getCompoundPaddingRight();
    }
    int padding = CompoundButton::getCompoundPaddingRight() + mSwitchWidth;
    if(!TextUtils::isEmpty(this->mText)) {
        padding += mSwitchPadding;
    }
    return padding;
}

/**
 * Translates thumb position to offset according to current RTL setting and
 * thumb scroll range. Accounts for both track and thumb padding.
 *
 * @return thumb offset
 */
int Switch::getThumbOffset() {
    float thumbPosition;
    if (isLayoutRtl()) {
        thumbPosition = 1 - mThumbPosition;
    } else {
        thumbPosition = mThumbPosition;
    }
    return (int) (thumbPosition * getThumbScrollRange() + 0.5f);
}

int Switch::getThumbScrollRange() {
    if (mTrackDrawable) {
        Rect padding;
        mTrackDrawable->getPadding(padding);

        Insets insets;
        if (mThumbDrawable) {
            insets = mThumbDrawable->getOpticalInsets();
        } else {
            insets = Insets::NONE;
        }

        return mSwitchWidth - mThumbWidth - padding.left - padding.width
                - insets.left - insets.right;
    } else {
        return 0;
    }
}

std::vector<int> Switch::onCreateDrawableState(int extraSpace){
    std::vector<int> drawableState = CompoundButton::onCreateDrawableState(extraSpace + 1);
    if (isChecked()) {
        mergeDrawableStates(drawableState,{cdroid::internal::R::attr::state_checked});
    }
    return drawableState;
}

void Switch::drawableStateChanged() {
    CompoundButton::drawableStateChanged();

    std::vector<int> state = getDrawableState();
    bool changed = false;

    if (mThumbDrawable && mThumbDrawable->isStateful()) {
        changed |= mThumbDrawable->setState(state);
    }

    if (mTrackDrawable && mTrackDrawable->isStateful()) {
        changed |= mTrackDrawable->setState(state);
    }

    if (changed) {
        invalidate();
    }
}

void Switch::drawableHotspotChanged(float x, float y) {
    CompoundButton::drawableHotspotChanged(x, y);

    if (mThumbDrawable) {
        mThumbDrawable->setHotspot(x, y);
    }

    if (mTrackDrawable) {
        mTrackDrawable->setHotspot(x, y);
    }
}

bool Switch::verifyDrawable(Drawable* who)const{
    return CompoundButton::verifyDrawable(who) || who == mThumbDrawable || who == mTrackDrawable;
}

void Switch::jumpDrawablesToCurrentState() {
    CompoundButton::jumpDrawablesToCurrentState();

    if (mThumbDrawable) {
        mThumbDrawable->jumpToCurrentState();
    }

    if (mTrackDrawable) {
        mTrackDrawable->jumpToCurrentState();
    }

    if (mPositionAnimator && mPositionAnimator->isStarted()) {
        mPositionAnimator->end();
        delete mPositionAnimator;
        mPositionAnimator = nullptr;
    }
}

}//endof namespace
