/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.CircularFlow.
 */
#include <widgetEx/constraintlayout/helpers/circularflow.h>

#include <cctype>
#include <sstream>

#include <core/displaymetrics.h>
#include <widgetEx/constraintlayout/constraintlayout.h>

DECLARE_WIDGET(CircularFlow)

namespace cdroid {

CircularFlow::CircularFlow(Context* ctx, const AttributeSet& attrs)
    : ConstraintHelper(ctx, attrs) {
}

CircularFlow::CircularFlow(int width, int height)
    : ConstraintHelper(width, height) {
}

void CircularFlow::init(const AttributeSet& attrs) {
    ConstraintHelper::init(attrs);
    mViewCenter = attrs.getResourceId("circularflow_viewCenter", 0);
    mReferenceAngles = attrs.getString("circularflow_angles", "");
    mReferenceRadius = attrs.getString("circularflow_radiusInDP", "");
    if (!mReferenceAngles.empty()) setAngles(mReferenceAngles);
    if (!mReferenceRadius.empty()) setRadius(mReferenceRadius);
    setDefaultAngle(attrs.getFloat("circularflow_defaultAngle", 0));
    setDefaultRadius(attrs.getDimensionPixelSize("circularflow_defaultRadius", 0));
}

std::vector<float> CircularFlow::getAngles() const {
    return mAngles;
}

std::vector<int> CircularFlow::getRadius() const {
    return mRadius;
}

void CircularFlow::setAngles(const std::vector<float>& angles) {
    mAngles = angles;
}

void CircularFlow::setAngles(const std::string& angleList) {
    mAngles.clear();
    std::stringstream ss(angleList);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // trim whitespace
        size_t s = 0; while (s < token.size() && std::isspace((unsigned char) token[s])) s++;
        size_t e = token.size(); while (e > s && std::isspace((unsigned char) token[e - 1])) e--;
        if (s == e) continue;
        mAngles.push_back((float) std::atof(token.substr(s, e - s).c_str()));
    }
}

void CircularFlow::setRadius(const std::vector<int>& radius) {
    mRadius = radius;
}

void CircularFlow::setRadius(const std::string& radiusList) {
    mRadius.clear();
    float density = 1.0f;
    if (getContext() != nullptr) {
        density = getContext()->getDisplayMetrics().density;
        if (density <= 0) density = 1.0f;
    }
    std::stringstream ss(radiusList);
    std::string token;
    while (std::getline(ss, token, ',')) {
        size_t s = 0; while (s < token.size() && std::isspace((unsigned char) token[s])) s++;
        size_t e = token.size(); while (e > s && std::isspace((unsigned char) token[e - 1])) e--;
        if (s == e) continue;
        // radiusInDP values are in dp → px (AndroidX applies display density).
        mRadius.push_back((int) (std::atoi(token.substr(s, e - s).c_str()) * density));
    }
}

void CircularFlow::setDefaultAngle(float angle) {
    mDefaultAngle = angle;
}

void CircularFlow::setDefaultRadius(int radius) {
    mDefaultRadius = radius;
}

void CircularFlow::addViewToCircularFlow(View* view, int radius, float angle) {
    if (view == nullptr || containsId(view->getId())) {
        return;
    }
    addView(view);
    mAngles.push_back(angle);
    float density = 1.0f;
    if (getContext() != nullptr) {
        density = getContext()->getDisplayMetrics().density;
        if (density <= 0) density = 1.0f;
    }
    mRadius.push_back((int) (radius * density));
}

void CircularFlow::updatePreLayout(ConstraintLayout* container) {
    mContainer = container;
    anchorReferences();
}

void CircularFlow::anchorReferences() {
    if (mContainer == nullptr) {
        return;
    }
    for (size_t i = 0; i < mIds.size(); i++) {
        View* view = mContainer->findViewById(mIds[i]);
        if (view == nullptr) {
            continue;
        }
        float angle  = (i < mAngles.size()) ? mAngles[i] : mDefaultAngle;
        int   radius = (i < mRadius.size())  ? mRadius[i] : mDefaultRadius;

        auto* params = dynamic_cast<ConstraintLayout::LayoutParams*>(view->getLayoutParams());
        if (params != nullptr) {
            params->circleAngle      = angle;
            params->circleConstraint = mViewCenter;
            params->circleRadius     = radius;
        }
    }
    applyLayoutFeatures();
}

} // namespace cdroid
