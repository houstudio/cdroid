#include <widget/expandablelayout.h>
#include <widget/linearlayout.h>

//Referenced from 
//https://github.com/cachapa/ExpandableLayout/blob/master/lib/src/main/java/net/cachapa/expandablelayout/ExpandableLayout.java

namespace cdroid{

ExpandableLayout::ExpandableLayout(int w,int h):FrameLayout(w,h){
    initView();
}

ExpandableLayout::ExpandableLayout(Context* context, const AttributeSet& attrs)
	:FrameLayout(context, attrs){
    mDuration = attrs.getInt("duration", DEFAULT_DURATION);
    mExpansion = attrs.getBoolean("expanded", false) ? 1 : 0;
    mOrientation = attrs.getInt("orientation", VERTICAL);
    mParallax = attrs.getFloat("parallax", 1);

    mState = mExpansion == 0 ? COLLAPSED : EXPANDED;
    setParallax(mParallax);
}

void ExpandableLayout::initView(){
    mDuration = DEFAULT_DURATION;
    mExpansion =0;
    mOrientation=VERTICAL;
    mParallax=1;
    mState =COLLAPSED;
    mAnimator = nullptr;
    mInterpolator = new FastOutSlowInInterpolator();
}

ExpandableLayout::~ExpandableLayout(){
    delete mInterpolator;
}

void ExpandableLayout::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    FrameLayout::onMeasure(widthMeasureSpec, heightMeasureSpec);

    const int width = getMeasuredWidth();
    const int height = getMeasuredHeight();

    const int size = mOrientation == LinearLayout::HORIZONTAL ? width : height;

    setVisibility(mExpansion == 0 && size == 0 ? GONE : VISIBLE);

    int expansionDelta = size - std::round(size * mExpansion);
    if (mParallax > 0) {
        float parallaxDelta = expansionDelta * mParallax;
        for (int i = 0; i < getChildCount(); i++) {
            View* child = getChildAt(i);
            if (mOrientation == HORIZONTAL) {
                int direction = -1;
                if (getLayoutDirection() == LAYOUT_DIRECTION_RTL) {
                    direction = 1;
                }
                child->setTranslationX(direction * parallaxDelta);
            } else {
                child->setTranslationY(-parallaxDelta);
            }
        }
    }

    if (mOrientation == HORIZONTAL) {
        setMeasuredDimension(width - expansionDelta, height);
    } else {
        setMeasuredDimension(width, height - expansionDelta);
    }
}

/*void ExpandableLayout::onConfigurationChanged(Configuration newConfig) {
    if (mAnimator != null) {
         mAnimator->cancel();
    }
    FrameLayout::onConfigurationChanged(newConfig);
}*/

int ExpandableLayout::getState()const{
    return mState;
}

bool ExpandableLayout::isExpanded()const{
    return mState == EXPANDING || mState == EXPANDED;
}

void ExpandableLayout::toggle() {
    toggle(true);
}

void ExpandableLayout::toggle(bool animate) {
    if (isExpanded()) {
        collapse(animate);
    } else {
        expand(animate);
    }
}

void ExpandableLayout::expand() {
    expand(true);
}

void ExpandableLayout::expand(bool animate) {
    setExpanded(true, animate);
}

void ExpandableLayout::collapse() {
    collapse(true);
}

void ExpandableLayout::collapse(bool animate) {
    setExpanded(false, animate);
}

/**
 * Convenience method - same as calling setExpanded(expanded, true)
 */
void ExpandableLayout::setExpanded(bool expand) {
    setExpanded(expand, true);
}

void ExpandableLayout::setExpanded(bool expand, bool animate) {
    if (expand == isExpanded()) {
        return;
    }

    int targetExpansion = expand ? 1 : 0;
    if (animate) {
        animateSize(targetExpansion);
    } else {
        setExpansion(targetExpansion);
    }
}

int ExpandableLayout::getDuration()const{
    return mDuration;
}

void ExpandableLayout::setInterpolator(Interpolator* interpolator) {
    this->mInterpolator = interpolator;
}

void ExpandableLayout::setDuration(int duration) {
    this->mDuration = duration;
}

float ExpandableLayout::getExpansion()const{
    return mExpansion;
}

void ExpandableLayout::setExpansion(float expansion) {
    if (this->mExpansion == expansion) {
        return;
    }

    // Infer state from previous value
    float delta = expansion - this->mExpansion;
    if (expansion == 0) {
        mState = COLLAPSED;
    } else if (expansion == 1) {
        mState = EXPANDED;
    } else if (delta < 0) {
        mState = COLLAPSING;
    } else if (delta > 0) {
        mState = EXPANDING;
    }

    setVisibility(mState == COLLAPSED ? GONE : VISIBLE);
    this->mExpansion = expansion;
    requestLayout();

    if (mListener != nullptr) {
        mListener(expansion, mState);
    }
}

float ExpandableLayout::getParallax()const{
    return mParallax;
}

void ExpandableLayout::setParallax(float parallax) {
    // Make sure parallax is between 0 and 1
    parallax = std::min(1.f, std::max(0.f, parallax));
    this->mParallax = parallax;
}

int ExpandableLayout::getOrientation()const{
    return mOrientation;
}

void ExpandableLayout::setOrientation(int orientation) {
    if (orientation < 0 || orientation > 1) {
        FATAL("Orientation(%d) must be either 0 (horizontal) or 1 (vertical)",orientation);
    }
    this->mOrientation = orientation;
}

void ExpandableLayout::setOnExpansionUpdateListener(OnExpansionUpdateListener listener) {
    this->mListener = listener;
}

void ExpandableLayout::animateSize(int targetExpansion) {
    mTargetExpansion = targetExpansion;
    mCanceled = false;
    if (mAnimator != nullptr) {
        mAnimator->cancel();
        //delete mAnimator;
        //mAnimator = nullptr;
        mAnimator->setFloatValues({mExpansion, (float)targetExpansion});
    }else{
        mAnimator = ValueAnimator::ofFloat({mExpansion, (float)targetExpansion});
        mAnimator->setInterpolator(mInterpolator);
        mAnimator->setDuration(mDuration);

        mAnimator->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim) {
            setExpansion((float) anim.getAnimatedValue().get<float>());
        }));

        Animator::AnimatorListener ls;
        ls.onAnimationStart=[this](Animator&anim,bool isReverse){
	    mState = mTargetExpansion ==0 ? COLLAPSING : EXPANDING;
        };
        ls.onAnimationEnd=[this](Animator&anim,bool isReverse){
            if (!mCanceled) {
                mState = mTargetExpansion == 0 ? COLLAPSED : EXPANDED;
                setExpansion(mTargetExpansion);
            }
        };
        ls.onAnimationCancel=[this](Animator&anim){
	    mCanceled = true;
        };
        mAnimator->addListener(ls);
    }
    mAnimator->start();
}

}

