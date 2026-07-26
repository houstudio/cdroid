/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.state.WidgetFrame.
 */
#include <widgetEx/constraintlayout/core/state/widgetframe.h>

#include <algorithm>
#include <cmath>

#include <widgetEx/constraintlayout/core/motion/customattribute.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

float WidgetFrame::phone_orientation = NAN;

WidgetFrame::WidgetFrame() = default;

WidgetFrame::WidgetFrame(ConstraintWidget* w) : widget(w) {}

WidgetFrame::WidgetFrame(const WidgetFrame& frame) {
    widget = frame.widget;
    left = frame.left; top = frame.top; right = frame.right; bottom = frame.bottom;
    updateAttributes(&frame);
}

void WidgetFrame::updateAttributes(const WidgetFrame* frame) {
    if (frame == nullptr) return;
    pivotX = frame->pivotX; pivotY = frame->pivotY;
    rotationX = frame->rotationX; rotationY = frame->rotationY; rotationZ = frame->rotationZ;
    translationX = frame->translationX; translationY = frame->translationY; translationZ = frame->translationZ;
    scaleX = frame->scaleX; scaleY = frame->scaleY;
    alpha = frame->alpha; visibility = frame->visibility;
    mMotionProperties = frame->mMotionProperties;
    mCustom.clear();
    for (const auto& kv : frame->mCustom) {
        mCustom[kv.first] = kv.second.copy();
    }
}

bool WidgetFrame::isDefaultTransform() const {
    return std::isnan(rotationX) && std::isnan(rotationY) && std::isnan(rotationZ)
        && std::isnan(translationX) && std::isnan(translationY) && std::isnan(translationZ)
        && std::isnan(scaleX) && std::isnan(scaleY) && std::isnan(alpha);
}

WidgetFrame& WidgetFrame::update() {
    if (widget != nullptr) {
        left = widget->getX();
        top = widget->getY();
        right = left + widget->getWidth();
        bottom = top + widget->getHeight();
        // Java also syncs transforms from widget->frame; CDROID ConstraintWidget has no frame field,
        // so the transform sync is deferred (transforms are set via the motion state directly).
    }
    return *this;
}

WidgetFrame& WidgetFrame::update(ConstraintWidget* w) {
    if (w == nullptr) return *this;
    widget = w;
    update();
    return *this;
}

bool WidgetFrame::containsCustom(const std::string& name) const {
    return mCustom.find(name) != mCustom.end();
}

void WidgetFrame::addCustomColor(const std::string& name, int color) {
    setCustomAttribute(name, TypedValues::Custom::TYPE_COLOR, color);
}

int WidgetFrame::getCustomColor(const std::string& name) const {
    auto it = mCustom.find(name);
    if (it != mCustom.end()) return it->second.getColorValue();
    return 0xFFFFAA88;
}

void WidgetFrame::addCustomFloat(const std::string& name, float value) {
    setCustomAttribute(name, TypedValues::Custom::TYPE_FLOAT, value);
}

float WidgetFrame::getCustomFloat(const std::string& name) const {
    auto it = mCustom.find(name);
    if (it != mCustom.end()) return it->second.getFloatValue();
    return NAN;
}

void WidgetFrame::setCustomAttribute(const std::string& name, int type, float value) {
    auto it = mCustom.find(name);
    if (it != mCustom.end()) it->second.setFloatValue(value);
    else mCustom.emplace(name, CustomVariable(name, type, value));
}

void WidgetFrame::setCustomAttribute(const std::string& name, int type, int value) {
    auto it = mCustom.find(name);
    if (it != mCustom.end()) it->second.setIntValue(value);
    else mCustom.emplace(name, CustomVariable(name, type, value));
}

void WidgetFrame::setCustomAttribute(const std::string& name, int type, bool value) {
    auto it = mCustom.find(name);
    if (it != mCustom.end()) it->second.setBooleanValue(value);
    else mCustom.emplace(name, CustomVariable(name, type, value));
}

void WidgetFrame::setCustomAttribute(const std::string& name, int type, const std::string& value) {
    auto it = mCustom.find(name);
    if (it != mCustom.end()) it->second.setStringValue(value);
    else mCustom.emplace(name, CustomVariable(name, type, value));
}

CustomVariable* WidgetFrame::getCustomAttribute(const std::string& name) {
    auto it = mCustom.find(name);
    return (it != mCustom.end()) ? &it->second : nullptr;
}

std::unordered_set<std::string> WidgetFrame::getCustomAttributeNames() const {
    std::unordered_set<std::string> keys;
    for (const auto& kv : mCustom) keys.insert(kv.first);
    return keys;
}

void WidgetFrame::setCustomValue(CustomAttribute& /*valueAt*/, std::vector<float>& /*mTempValues*/) {
    // no-op (matches Java).
}

} // namespace cdroid
