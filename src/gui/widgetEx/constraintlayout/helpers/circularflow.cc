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
 *********************************************************************************/

/*
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
    // The ConstraintHelper base ctor calls init(attrs), but during base construction that virtual
    // call statically binds to ConstraintHelper::init — so only constraint_referenced_ids is parsed
    // and every circularflow_* attribute stays at its default: mAngles/mRadius empty, mDefaultRadius
    // 0. anchorReferences then assigns every referenced view angle=0 radius=0, so all of them collapse
    // onto the center point (only the topmost is visible). Re-invoke init now that *this is fully
    // constructed so it dispatches to CircularFlow::init — same pattern as Carousel/MotionEffect/Placeholder.
    // ConstraintHelper::init is idempotent on re-run (mIds cleared then refilled).
    init(attrs);
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
