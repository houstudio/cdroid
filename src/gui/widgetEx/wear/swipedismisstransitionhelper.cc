/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <widgetEx/wear/swipedismisstransitionhelper.h>

namespace cdroid{

SwipeDismissTransitionHelper::SwipeDismissTransitionHelper(Context* context,DismissibleFrameLayout* layout) {
    mLayout = layout;
    mVelocityTracker = nullptr;
    mIsScreenRound = true;//layout.getResources().getConfiguration().isScreenRound();
    mScreenWidth = context->getDisplayMetrics().widthPixels;
    mScrimBackground = generateScrimBackgroundDrawable(mScreenWidth,
            context->getDisplayMetrics().heightPixels);
}

void SwipeDismissTransitionHelper::clipOutline(View* view, bool useRoundShape) {
    view->setOutlineProvider([useRoundShape](View& view, Outline& outline) {
        if (useRoundShape) {
            outline.setOval(0, 0, view.getWidth(), view.getHeight());
        } else {
            outline.setRect(0, 0, view.getWidth(), view.getHeight());
        }
        outline.setAlpha(0);
    });
    view->setClipToOutline(true);
}


static float lerp(float minV, float maxV, float value) {
    return minV + (maxV - minV) * value;
}

static float clamp(float minV, float maxV, float value) {
    return std::max(minV, std::min(maxV, value));
}

static float lerpInv(float minV, float maxV, float value) {
    return minV != maxV ? ((value - minV) / (maxV - minV)) : 0.0f;
}

ColorFilter* SwipeDismissTransitionHelper::createDimmingColorFilter(float level) {
    level = clamp(0, 1, level);
    int alpha = (int) (0xFF * level);
    int color = Color::toArgb(0 , 0, 0, (uint8_t)alpha);
    ColorFilter* colorFilter = mDimmingColorFilterCache.get(alpha);
    if (colorFilter != nullptr) {
        return colorFilter;
    }
    colorFilter = new PorterDuffColorFilter(color, PorterDuff::Mode::SRC_ATOP);
    mDimmingColorFilterCache.put(alpha, colorFilter);
    return colorFilter;
}

SpringAnimation* SwipeDismissTransitionHelper::createSpringAnimation(float startValue, float finalValue, float startVelocity,
        const DynamicAnimation::OnAnimationUpdateListener& onUpdateListener,
        const DynamicAnimation::OnAnimationEndListener& onEndListener) {
    SpringAnimation* animation = new SpringAnimation(new FloatValueHolder());
    animation->setStartValue(startValue);
    animation->setMinimumVisibleChange(SPRING_MIN_VISIBLE_CHANGE);
    SpringForce* spring = new SpringForce();
    spring->setFinalPosition(finalValue);
    spring->setDampingRatio(SPRING_DAMPING_RATIO);
    spring->setStiffness(SPRING_STIFFNESS);
    animation->setMinValue(0.0f);
    animation->setMaxValue(mScreenWidth);
    animation->setStartVelocity(startVelocity);
    animation->setSpring(spring);
    animation->addUpdateListener(onUpdateListener);
    animation->addEndListener(onEndListener);
    animation->start();
    return animation;
}

void SwipeDismissTransitionHelper::onSwipeProgressChanged(float deltaX, MotionEvent& ev) {
    if (!mStarted) {
        initializeTransition();
    }

    mVelocityTracker->addMovement(ev);
    mOriginalViewWidth = mLayout->getWidth();
    // For swiping, mProgress is directly manipulated
    // mProgress = 0 (no swipe) - 0.5 (swiped to mid screen) - 1 (swipe to right of screen)
    mProgress = deltaX / mOriginalViewWidth;
    // Solve for other variables
    // Scale = lerp 100% -> 70% when swiping from left edge to right edge
    mScale = lerp((float)SCALE_MAX, (float)SCALE_MIN, mProgress);
    // Translation: make sure the right edge of mOriginalView touches right edge of screen
    mTranslationX = std::max(0.0f, 1.0f - mScale) * mLayout->getWidth() / 2.0f;
    mDimming = std::min((float)DIM_FOREGROUND_MIN, mProgress / DIM_FOREGROUND_PROGRESS_FACTOR);

    updateView();
}

void SwipeDismissTransitionHelper::onDismissalRecoveryAnimationProgressChanged(float translationX) {
    mOriginalViewWidth = mLayout->getWidth();
    mTranslationX = translationX;

    mScale = 1 - mTranslationX * 2 / mOriginalViewWidth;
    // Clamp mScale so that we can solve for mProgress
    mScale = std::max((float)SCALE_MIN, std::min(mScale, (float)SCALE_MAX));
    float nextProgress = lerpInv((float)SCALE_MAX, (float)SCALE_MIN, mScale);
    if (nextProgress > mProgress) {
        mProgress = nextProgress;
    }
    mDimming = std::min((float)DIM_FOREGROUND_MIN, mProgress / DIM_FOREGROUND_PROGRESS_FACTOR);
    updateView();
}

void SwipeDismissTransitionHelper::updateView() {
    mLayout->setScaleX(mScale);
    mLayout->setScaleY(mScale);
    mLayout->setTranslationX(mTranslationX);
    updateDim();
    updateScrim();
}

void SwipeDismissTransitionHelper::updateDim() {
    //mCompositingPaint->setColorFilter(createDimmingColorFilter(mDimming));
    //mLayout->setLayerPaint(mCompositingPaint);
}

void SwipeDismissTransitionHelper::updateScrim() {
    float alpha = SCRIM_BACKGROUND_MAX * (1.0f - mProgress);
    // Scaling alpha between 0 to 255, as Drawable.setAlpha expects it in range [0,255].
    mScrimBackground->setAlpha((int) (alpha * 255));
}

void SwipeDismissTransitionHelper::initializeTransition() {
    mStarted = true;
    ViewGroup* originalParentView = getOriginalParentView();

    if (originalParentView == nullptr) {
        return;
    }

    if (mPrevParentBackground == nullptr) {
        mPrevParentBackground = originalParentView->getBackground();
    }

    // Adding scrim over parent background if it exists.
    Drawable* parentBackgroundLayers;
    if (mPrevParentBackground != nullptr) {
        std::vector<Drawable*> drawables = {mPrevParentBackground,mScrimBackground};
        parentBackgroundLayers = new LayerDrawable(drawables);
    } else {
        parentBackgroundLayers = mScrimBackground;
    }
    originalParentView->setBackground(parentBackgroundLayers);

    //mCompositingPaint->setColorFilter(nullptr);
    mLayout->setLayerType(View::LAYER_TYPE_HARDWARE);//, mCompositingPaint);
    clipOutline(mLayout, mIsScreenRound);
}

void SwipeDismissTransitionHelper::resetTranslationAndAlpha() {
    // resetting variables
    mStarted = false;
    mTranslationX = 0;
    mProgress = 0;
    mScale = 1;
    // resetting layout params
    mLayout->setTranslationX(0);
    mLayout->setScaleX(1);
    mLayout->setScaleY(1);
    mLayout->setAlpha(1);
    mScrimBackground->setAlpha(0);

    //mCompositingPaint->setColorFilter(nullptr);
    mLayout->setLayerType(View::LAYER_TYPE_NONE);//, nullptr);
    mLayout->setClipToOutline(false);

    // Restoring previous background
    ViewGroup* originalParentView = getOriginalParentView();
    if (originalParentView != nullptr) {
        originalParentView->setBackground(mPrevParentBackground);
    }
    mPrevParentBackground = nullptr;
}

Drawable* SwipeDismissTransitionHelper::generateScrimBackgroundDrawable(int width, int height) {
    ShapeDrawable* shape = new ShapeDrawable();
    shape->setShape(new RectShape());
    shape->setBounds(0, 0, width, height);
    //shape.getPaint().setColor(Color.BLACK);
    return shape;
}

bool SwipeDismissTransitionHelper::isAnimating() const{
    return (mDismissalSpring != nullptr && mDismissalSpring->isRunning()) || (
            mRecoverySpring != nullptr && mRecoverySpring->isRunning());
}

void SwipeDismissTransitionHelper::animateRecovery(const DismissController::OnDismissListener& dismissListener) {
    mVelocityTracker->computeCurrentVelocity(VELOCITY_UNIT);
    const DynamicAnimation::OnAnimationUpdateListener update([this](DynamicAnimation&animation, float value, float velocity){
                const float distanceRemaining = std::max(0.f, (value - 0.f));
                if (distanceRemaining <= SPRING_ANIMATION_PROGRESS_FINISH_THRESHOLD_PX
                        && mRecoverySpring != nullptr) {
                    // Skip last 2% of animation.
                    mRecoverySpring->skipToEnd();
                }
                onDismissalRecoveryAnimationProgressChanged(value);
    
            });
    const DynamicAnimation::OnAnimationEndListener endls(
            [this,&dismissListener](DynamicAnimation&animation,bool canceled,float value,float velocity){
                resetTranslationAndAlpha();
                if (dismissListener.onDismissCanceled != nullptr) {
                    dismissListener.onDismissCanceled();
                }
            });
    mRecoverySpring = createSpringAnimation(mTranslationX, 0, 
            mVelocityTracker->getXVelocity(),update,endls);
}

void SwipeDismissTransitionHelper::animateDismissal(const DismissController::OnDismissListener& dismissListener) {
    if (mVelocityTracker == nullptr) {
        mVelocityTracker = VelocityTracker::obtain();
    }
    mVelocityTracker->computeCurrentVelocity(VELOCITY_UNIT);
    // Dismissal has started
    if (dismissListener.onDismissStarted != nullptr) {
        dismissListener.onDismissStarted();
    }
    const DynamicAnimation::OnAnimationUpdateListener update([this](DynamicAnimation&animation,float value,float velocity){
                const float distanceRemaining = std::max(0.f, float(mScreenWidth - value));
                if (distanceRemaining <= SPRING_ANIMATION_PROGRESS_FINISH_THRESHOLD_PX
                        && mDismissalSpring != nullptr) {
                    // Skip last 2% of animation.
                    mDismissalSpring->skipToEnd();
                }
                onDismissalRecoveryAnimationProgressChanged(value);
            });
    const DynamicAnimation::OnAnimationEndListener endls(
            [this,&dismissListener](DynamicAnimation&animation,bool canceled,float value,float velocity){
                resetTranslationAndAlpha();
                if (dismissListener.onDismissed != nullptr) {
                    dismissListener.onDismissed();
                }
            });
    mDismissalSpring = createSpringAnimation(mTranslationX, mScreenWidth, mVelocityTracker->getXVelocity(), update, endls);
}

ViewGroup* SwipeDismissTransitionHelper::getOriginalParentView() {
    if (dynamic_cast<ViewGroup*>(mLayout->getParent())){// instanceof ViewGroup) {
        return (ViewGroup*) mLayout->getParent();
    }
    return nullptr;
}

VelocityTracker* SwipeDismissTransitionHelper::getVelocityTracker() const{
    return mVelocityTracker;
}

void SwipeDismissTransitionHelper::obtainVelocityTracker() {
    mVelocityTracker = VelocityTracker::obtain();
}

void SwipeDismissTransitionHelper::resetVelocityTracker() {
    mVelocityTracker = nullptr;
}
}/*endof namespace*/
