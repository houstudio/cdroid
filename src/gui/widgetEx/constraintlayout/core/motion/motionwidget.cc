/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.motion.MotionWidget.
 */
#include <widgetEx/constraintlayout/core/motion/motionwidget.h>

namespace cdroid {

MotionWidget::MotionWidget() : mWidgetFrame(new WidgetFrame()) {}
MotionWidget::MotionWidget(WidgetFrame* f) : mWidgetFrame(f ? f : new WidgetFrame()) {}

// --- geometry ---
int MotionWidget::getTop() const    { return mWidgetFrame->top; }
int MotionWidget::getLeft() const   { return mWidgetFrame->left; }
int MotionWidget::getBottom() const { return mWidgetFrame->bottom; }
int MotionWidget::getRight() const  { return mWidgetFrame->right; }
int MotionWidget::getX() const      { return mWidgetFrame->left; }
int MotionWidget::getY() const      { return mWidgetFrame->top; }
int MotionWidget::getWidth() const  { return mWidgetFrame->width(); }
int MotionWidget::getHeight() const { return mWidgetFrame->height(); }
int MotionWidget::getVisibility() const { return mWidgetFrame->visibility; }
void MotionWidget::setVisibility(int visibility) { mWidgetFrame->visibility = visibility; }
void MotionWidget::setBounds(int left, int top, int right, int bottom) {
    mWidgetFrame->left = left; mWidgetFrame->top = top;
    mWidgetFrame->right = right; mWidgetFrame->bottom = bottom;
}

// --- transforms ---
float MotionWidget::getRotationX() const    { return mWidgetFrame->rotationX; }
float MotionWidget::getRotationY() const    { return mWidgetFrame->rotationY; }
float MotionWidget::getRotationZ() const    { return mWidgetFrame->rotationZ; }
float MotionWidget::getTranslationX() const { return mWidgetFrame->translationX; }
float MotionWidget::getTranslationY() const { return mWidgetFrame->translationY; }
float MotionWidget::getTranslationZ() const { return mWidgetFrame->translationZ; }
float MotionWidget::getScaleX() const       { return mWidgetFrame->scaleX; }
float MotionWidget::getScaleY() const       { return mWidgetFrame->scaleY; }
float MotionWidget::getPivotX() const       { return mWidgetFrame->pivotX; }
float MotionWidget::getPivotY() const       { return mWidgetFrame->pivotY; }
float MotionWidget::getAlpha() const        { return mWidgetFrame->alpha; }

void MotionWidget::setRotationX(float v)    { mWidgetFrame->rotationX = v; }
void MotionWidget::setRotationY(float v)    { mWidgetFrame->rotationY = v; }
void MotionWidget::setRotationZ(float v)    { mWidgetFrame->rotationZ = v; }
void MotionWidget::setTranslationX(float v) { mWidgetFrame->translationX = v; }
void MotionWidget::setTranslationY(float v) { mWidgetFrame->translationY = v; }
void MotionWidget::setTranslationZ(float v) { mWidgetFrame->translationZ = v; }
void MotionWidget::setScaleX(float v)       { mWidgetFrame->scaleX = v; }
void MotionWidget::setScaleY(float v)       { mWidgetFrame->scaleY = v; }
void MotionWidget::setPivotX(float v)       { mWidgetFrame->pivotX = v; }
void MotionWidget::setPivotY(float v)       { mWidgetFrame->pivotY = v; }
void MotionWidget::setAlpha(float v)        { mWidgetFrame->alpha = v; }

// --- TypedValues dispatch ---
bool MotionWidget::setValue(int id, int value) {
    return setValueAttributes(id, (float) value) || setValueMotion(id, value);
}
bool MotionWidget::setValue(int id, float value) {
    return setValueAttributes(id, value) || setValueMotion(id, value);
}
bool MotionWidget::setValue(int id, const std::string& value) {
    if (id == MotionType::TYPE_ANIMATE_RELATIVE_TO) { mMotion.mAnimateRelativeTo = value; return true; }
    return setValueMotion(id, value);
}
bool MotionWidget::setValue(int /*id*/, bool /*value*/) { return false; }

int MotionWidget::getId(const std::string& name) {
    int ret = AttributesType::getId(name);
    if (ret != -1) return ret;
    ret = MotionType::getId(name);
    return ret;
}

bool MotionWidget::setValueMotion(int id, int value) {
    switch (id) {
        case MotionType::TYPE_ANIMATE_CIRCLEANGLE_TO:     mMotion.mAnimateCircleAngleTo = value; break;
        case MotionType::TYPE_PATHMOTION_ARC:             mMotion.mPathMotionArc = value; break;
        case MotionType::TYPE_DRAW_PATH:                  mMotion.mDrawPath = value; break;
        case MotionType::TYPE_POLAR_RELATIVETO:           mMotion.mPolarRelativeTo = value; break;
        case MotionType::TYPE_QUANTIZE_MOTIONSTEPS:       mMotion.mQuantizeMotionSteps = value; break;
        case MotionType::TYPE_QUANTIZE_INTERPOLATOR_TYPE: mMotion.mQuantizeInterpolatorType = value; break;
        case MotionType::TYPE_QUANTIZE_INTERPOLATOR_ID:   mMotion.mQuantizeInterpolatorID = value; break;
        default: return false;
    }
    return true;
}

bool MotionWidget::setValueMotion(int id, const std::string& value) {
    switch (id) {
        case MotionType::TYPE_EASING:                mMotion.mTransitionEasing = value; break;
        case MotionType::TYPE_QUANTIZE_INTERPOLATOR: mMotion.mQuantizeInterpolatorString = value; break;
        default: return false;
    }
    return true;
}

bool MotionWidget::setValueMotion(int id, float value) {
    switch (id) {
        case MotionType::TYPE_STAGGER:               mMotion.mMotionStagger = value; break;
        case MotionType::TYPE_PATH_ROTATE:           mMotion.mPathRotate = value; break;
        case MotionType::TYPE_QUANTIZE_MOTION_PHASE: mMotion.mQuantizeMotionPhase = value; break;
        default: return false;
    }
    return true;
}

bool MotionWidget::setValueAttributes(int id, float value) {
    switch (id) {
        case AttributesType::TYPE_ALPHA:         mWidgetFrame->alpha = value; break;
        case AttributesType::TYPE_TRANSLATION_X: mWidgetFrame->translationX = value; break;
        case AttributesType::TYPE_TRANSLATION_Y: mWidgetFrame->translationY = value; break;
        case AttributesType::TYPE_TRANSLATION_Z: mWidgetFrame->translationZ = value; break;
        case AttributesType::TYPE_ROTATION_X:    mWidgetFrame->rotationX = value; break;
        case AttributesType::TYPE_ROTATION_Y:    mWidgetFrame->rotationY = value; break;
        case AttributesType::TYPE_ROTATION_Z:    mWidgetFrame->rotationZ = value; break;
        case AttributesType::TYPE_SCALE_X:       mWidgetFrame->scaleX = value; break;
        case AttributesType::TYPE_SCALE_Y:       mWidgetFrame->scaleY = value; break;
        case AttributesType::TYPE_PIVOT_X:       mWidgetFrame->pivotX = value; break;
        case AttributesType::TYPE_PIVOT_Y:       mWidgetFrame->pivotY = value; break;
        case AttributesType::TYPE_PROGRESS:      mProgress = value; break;
        case AttributesType::TYPE_PATH_ROTATE:   mTransitionPathRotate = value; break;
        default: return false;
    }
    return true;
}

float MotionWidget::getValueAttributes(int id) const {
    switch (id) {
        case AttributesType::TYPE_ALPHA:         return mWidgetFrame->alpha;
        case AttributesType::TYPE_TRANSLATION_X: return mWidgetFrame->translationX;
        case AttributesType::TYPE_TRANSLATION_Y: return mWidgetFrame->translationY;
        case AttributesType::TYPE_TRANSLATION_Z: return mWidgetFrame->translationZ;
        case AttributesType::TYPE_ROTATION_X:    return mWidgetFrame->rotationX;
        case AttributesType::TYPE_ROTATION_Y:    return mWidgetFrame->rotationY;
        case AttributesType::TYPE_ROTATION_Z:    return mWidgetFrame->rotationZ;
        case AttributesType::TYPE_SCALE_X:       return mWidgetFrame->scaleX;
        case AttributesType::TYPE_SCALE_Y:       return mWidgetFrame->scaleY;
        case AttributesType::TYPE_PIVOT_X:       return mWidgetFrame->pivotX;
        case AttributesType::TYPE_PIVOT_Y:       return mWidgetFrame->pivotY;
        case AttributesType::TYPE_PROGRESS:      return mProgress;
        case AttributesType::TYPE_PATH_ROTATE:   return mTransitionPathRotate;
        default: return NAN;
    }
}

void MotionWidget::updateMotion(TypedValues& toUpdate) {
    if (mWidgetFrame->getMotionProperties() != nullptr) {
        mWidgetFrame->getMotionProperties()->applyDelta(toUpdate);
    }
}

// --- custom attributes ---
std::unordered_set<std::string> MotionWidget::getCustomAttributeNames() const {
    return mWidgetFrame->getCustomAttributeNames();
}
void MotionWidget::setCustomAttribute(const std::string& name, int type, float value) {
    mWidgetFrame->setCustomAttribute(name, type, value);
}
void MotionWidget::setCustomAttribute(const std::string& name, int type, int value) {
    mWidgetFrame->setCustomAttribute(name, type, value);
}
void MotionWidget::setCustomAttribute(const std::string& name, int type, bool value) {
    mWidgetFrame->setCustomAttribute(name, type, value);
}
void MotionWidget::setCustomAttribute(const std::string& name, int type, const std::string& value) {
    mWidgetFrame->setCustomAttribute(name, type, value);
}
CustomVariable* MotionWidget::getCustomAttribute(const std::string& name) {
    return mWidgetFrame->getCustomAttribute(name);
}
void MotionWidget::setInterpolatedValue(CustomAttribute& attribute, std::vector<float>& mCache) {
    attribute.setValue(mCache);
}

} // namespace cdroid
