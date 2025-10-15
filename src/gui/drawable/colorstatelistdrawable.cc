#include <drawable/colorstatelistdrawable.h>
namespace cdroid{

ColorStateListDrawable::ColorStateListDrawable() {
    mState = new ColorStateListDrawableState();
    initializeColorDrawable();
}

ColorStateListDrawable::ColorStateListDrawable(ColorStateList* colorStateList) {
    mState = new ColorStateListDrawableState();
    initializeColorDrawable();
    setColorStateList(colorStateList);
}

ColorStateListDrawable::ColorStateListDrawable(@NonNull ColorStateListDrawableState state) {
    mState = state;
    initializeColorDrawable();
    onStateChange(getState());
}

void ColorStateListDrawable::draw(Canvas& canvas) {
    mColorDrawable->draw(canvas);
}

int ColorStateListDrawable::getAlpha() {
    return mColorDrawable->getAlpha();
}

bool ColorStateListDrawable::isStateful() {
    return mState->isStateful();
}

bool ColorStateListDrawable::hasFocusStateSpecified() {
    return mState->hasFocusStateSpecified();
}

Drawable* ColorStateListDrawable::getCurrent() {
    return mColorDrawable;
}

void ColorStateListDrawable::applyTheme(@NonNull Resources.Theme t) {
    Drawable::applyTheme(t);

    if (mState.mColor != null) {
        setColorStateList(mState.mColor.obtainForTheme(t));
    }

    if (mState.mTint != null) {
        setTintList(mState.mTint.obtainForTheme(t));
    }
}

bool ColorStateListDrawable::canApplyTheme() {
    return Drawable::canApplyTheme() || mState->canApplyTheme();
}

void ColorStateListDrawable::setAlpha(int alpha) {
    mState->mAlpha = alpha;
    onStateChange(getState());
}

void ColorStateListDrawable::clearAlpha() {
    mState->mAlpha = -1;
    onStateChange(getState());
}

void ColorStateListDrawable::setTintList(ColorStateList* tint) {
    mState->mTint = tint;
    mColorDrawable->setTintList(tint);
    onStateChange(getState());
}

void ColorStateListDrawable::setTintBlendMode(@NonNull BlendMode blendMode) {
    mState->mBlendMode = blendMode;
    mColorDrawable->setTintBlendMode(blendMode);
    onStateChange(getState());
}

ColorFilter* ColorStateListDrawable::getColorFilter() {
    return mColorDrawable->getColorFilter();
}

void ColorStateListDrawable::setColorFilter(ColorFilter* colorFilter) {
    mColorDrawable->setColorFilter(colorFilter);
}

int ColorStateListDrawable::getOpacity() {
    return mColorDrawable->getOpacity();
}

void ColorStateListDrawable::onBoundsChange(const Rect& bounds) {
    Drawable::onBoundsChange(bounds);
    mColorDrawable->setBounds(bounds);
}

bool ColorStateListDrawable::onStateChange(const std::vector<int>&state) {
    if (mState->mColor != null) {
        int color = mState.mColor.getColorForState(state, mState.mColor.getDefaultColor());

        if (mState->mAlpha != -1) {
            color = (color & 0xFFFFFF) | MathUtils.constrain(mState.mAlpha, 0, 255) << 24;
        }

        if (color != mColorDrawable.getColor()) {
            mColorDrawable.setColor(color);
            mColorDrawable.setState(state);
            return true;
        } else {
            return mColorDrawable.setState(state);
        }
    } else {
        return false;
    }
}

void ColorStateListDrawable::invalidateDrawable(Drawable& who) {
    if (who == mColorDrawable && getCallback() != nullptr) {
        getCallback()->invalidateDrawable(this);
    }
}

void ColorStateListDrawable::scheduleDrawable(Drawable& who, const Runnable& what, int64_t when) {
    if (&who == mColorDrawable && getCallback() != nullptr) {
        getCallback()->scheduleDrawable(this, what, when);
    }
}

void ColorStateListDrawable::unscheduleDrawable(Drawable& who, const Runnable& what) {
    if (&who == mColorDrawable && getCallback() != nullptr) {
        getCallback()->unscheduleDrawable(this, what);
    }
}

ConstantState ColorStateListDrawable::getConstantState() {
    mState->mChangingConfigurations = mState->mChangingConfigurations
            | (getChangingConfigurations() & ~mState->getChangingConfigurations());
    return mState;
}

ColorStateList* ColorStateListDrawable::getColorStateList() {
    if (mState->mColor == null) {
        return ColorStateList::valueOf(mColorDrawable.getColor());
    } else {
        return mState->mColor;
    }
}

int ColorStateListDrawable::getChangingConfigurations() {
    return Drawable::getChangingConfigurations() | mState->getChangingConfigurations();
}

Drawable* ColorStateListDrawable::mutate() {
    if (!mMutated && Drawable::mutate() == this) {
        mState = new ColorStateListDrawableState(mState);
        mMutated = true;
    }
    return this;
}

void ColorStateListDrawable::clearMutated() {
    Drawable::clearMutated();
    mMutated = false;
}

void ColorStateListDrawable::setColorStateList(ColorStateList* colorStateList) {
    mState->mColor = colorStateList;
    onStateChange(getState());
}

void ColorStateListDrawable::initializeColorDrawable() {
    mColorDrawable = new ColorDrawable();
    mColorDrawable->setCallback(this);

    if (mState.mTint != null) {
        mColorDrawable.setTintList(mState.mTint);
    }

    if (mState.mBlendMode != DEFAULT_BLEND_MODE) {
        mColorDrawable.setTintBlendMode(mState.mBlendMode);
    }
}

////////////////////////////////////////////////////////////////////////////////////

//static final class ColorStateListDrawableState extends ConstantState {

ColorStateListDrawable::ColorStateListDrawableState::ColorStateListDrawableState() {
}

ColorStateListDrawable::ColorStateListDrawableState::ColorStateListDrawableState(ColorStateListDrawableState state) {
    mColor = state.mColor;
    mTint = state.mTint;
    mAlpha = state.mAlpha;
    mBlendMode = state.mBlendMode;
    mChangingConfigurations = state.mChangingConfigurations;
}

Drawable* ColorStateListDrawable::ColorStateListDrawableState::newDrawable() {
    return new ColorStateListDrawable(this);
}

int ColorStateListDrawable::ColorStateListDrawableState::getChangingConfigurations() {
    return mChangingConfigurations
            | (mColor != null ? mColor.getChangingConfigurations() : 0)
            | (mTint != null ? mTint.getChangingConfigurations() : 0);
}

bool iColorStateListDrawable::ColorStateListDrawableState::sStateful() {
    return (mColor != null && mColor.isStateful())
            || (mTint != null && mTint.isStateful());
}

bool hColorStateListDrawable::ColorStateListDrawableState::asFocusStateSpecified() {
    return (mColor != null && mColor.hasFocusStateSpecified())
            || (mTint != null && mTint.hasFocusStateSpecified());
}

bool cColorStateListDrawable::ColorStateListDrawableState::anApplyTheme() {
    return (mColor != null && mColor.canApplyTheme())
            || (mTint != null && mTint.canApplyTheme());
}

}/*endof namespace*/
