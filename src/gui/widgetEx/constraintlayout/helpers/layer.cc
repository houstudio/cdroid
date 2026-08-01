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
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.Layer.
 */
#include <widgetEx/constraintlayout/helpers/layer.h>

#include <algorithm>
#include <cmath>

#include <widgetEx/constraintlayout/constraintlayout.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

DECLARE_WIDGET(Layer)

namespace cdroid {

Layer::Layer(Context* ctx, const AttributeSet& attrs)
    : ConstraintHelper(ctx, attrs) {
    // The ConstraintHelper base ctor calls init(attrs), but during base construction that virtual
    // call statically binds to ConstraintHelper::init — so only constraint_referenced_ids is parsed
    // and the visibility/elevation XML attributes are never scanned. Re-invoke init now that *this
    // is fully constructed so it dispatches to Layer::init — same pattern as
    // Carousel/MotionEffect/Placeholder/CircularFlow/Grid. ConstraintHelper::init is idempotent.
    init(attrs);
}

Layer::Layer(int width, int height)
    : ConstraintHelper(width, height) {
}

void Layer::init(const AttributeSet& attrs) {
    ConstraintHelper::init(attrs);
    mUseViewMeasure = false;
    if (attrs.hasAttribute("visibility")) {
        mApplyVisibilityOnAttach = true;
    }
    if (attrs.hasAttribute("elevation")) {
        mApplyElevationOnAttach = true;
    }
}

void Layer::onAttachedToWindow() {
    ConstraintHelper::onAttachedToWindow();
    mContainer = dynamic_cast<ConstraintLayout*>(getParent());
    if (mContainer != nullptr && (mApplyVisibilityOnAttach || mApplyElevationOnAttach)) {
        int visibility = getVisibility();
        for (int id : mIds) {
            View* view = mContainer->findViewById(id);
            if (view != nullptr) {
                if (mApplyVisibilityOnAttach) {
                    view->setVisibility(visibility);
                }
                // Elevation/translationZ propagation is deferred (CDROID View setElevation is limited);
                // visibility propagation matches AndroidX on-attach behavior.
            }
        }
    }
}

void Layer::updatePreDraw(ConstraintLayout* container) {
    mContainer = container;
    float rotate = getRotation();
    if (rotate == 0) {
        if (!std::isnan(mGroupRotateAngle)) {
            mGroupRotateAngle = rotate;
        }
    } else {
        mGroupRotateAngle = rotate;
    }
}

void Layer::setRotation(float angle) {
    mGroupRotateAngle = angle;
    transform();
}

void Layer::setScaleX(float scaleX) {
    mScaleX = scaleX;
    transform();
}

void Layer::setScaleY(float scaleY) {
    mScaleY = scaleY;
    transform();
}

void Layer::setPivotX(float pivotX) {
    mRotationCenterX = pivotX;
    transform();
}

void Layer::setPivotY(float pivotY) {
    mRotationCenterY = pivotY;
    transform();
}

void Layer::setTranslationX(float dx) {
    mShiftX = dx;
    transform();
}

void Layer::setTranslationY(float dy) {
    mShiftY = dy;
    transform();
}

void Layer::setVisibility(int visibility) {
    ConstraintHelper::setVisibility(visibility);
    applyLayoutFeatures();
}

void Layer::setElevation(float elevation) {
    ConstraintHelper::setElevation(elevation);
    applyLayoutFeatures();
}

void Layer::updatePostLayout(ConstraintLayout* container) {
    mContainer = container;
    reCacheViews();

    mComputedCenterX = std::numeric_limits<float>::quiet_NaN();
    mComputedCenterY = std::numeric_limits<float>::quiet_NaN();

    // The Layer itself occupies no space in the solver (its own ConstraintWidget is zeroed), like Group.
    auto* lp = dynamic_cast<ConstraintLayout::LayoutParams*>(getLayoutParams());
    if (lp != nullptr && lp->mWidget) {
        lp->mWidget->setWidth(0);
        lp->mWidget->setHeight(0);
    }
    calcCenters();
    int left   = (int) mComputedMinX - getPaddingLeft();
    int top    = (int) mComputedMinY - getPaddingTop();
    int right  = (int) mComputedMaxX + getPaddingRight();
    int bottom = (int) mComputedMaxY + getPaddingBottom();
    // CDROID View::layout takes (left, top, width, height), not (left, top, right, bottom).
    layout(left, top, right - left, bottom - top);
    transform();
}

void Layer::reCacheViews() {
    if (mContainer == nullptr) {
        return;
    }
    if (mIds.empty()) {
        return;
    }
    mViews.resize(mIds.size());
    for (size_t i = 0; i < mIds.size(); i++) {
        mViews[i] = mContainer->findViewById(mIds[i]);
    }
}

void Layer::calcCenters() {
    if (mContainer == nullptr) {
        return;
    }
    if (!mNeedBounds) {
        if (!(std::isnan(mComputedCenterX) || std::isnan(mComputedCenterY))) {
            return;
        }
    }
    if (std::isnan(mRotationCenterX) || std::isnan(mRotationCenterY)) {
        reCacheViews();
        if (mViews.empty()) {
            return;
        }
        int minx = mViews[0]->getLeft();
        int miny = mViews[0]->getTop();
        int maxx = mViews[0]->getRight();
        int maxy = mViews[0]->getBottom();
        for (View* view : mViews) {
            if (view == nullptr) continue;
            minx = std::min(minx, view->getLeft());
            miny = std::min(miny, view->getTop());
            maxx = std::max(maxx, view->getRight());
            maxy = std::max(maxy, view->getBottom());
        }
        mComputedMaxX = maxx;
        mComputedMaxY = maxy;
        mComputedMinX = minx;
        mComputedMinY = miny;

        if (std::isnan(mRotationCenterX)) {
            mComputedCenterX = (minx + maxx) / 2.0f;
        } else {
            mComputedCenterX = mRotationCenterX;
        }
        if (std::isnan(mRotationCenterY)) {
            mComputedCenterY = (miny + maxy) / 2.0f;
        } else {
            mComputedCenterY = mRotationCenterY;
        }
    } else {
        mComputedCenterY = mRotationCenterY;
        mComputedCenterX = mRotationCenterX;
    }
}

void Layer::transform() {
    if (mContainer == nullptr) {
        return;
    }
    if (mViews.empty()) {
        reCacheViews();
    }
    calcCenters();

    double rad = std::isnan(mGroupRotateAngle) ? 0.0 : (mGroupRotateAngle * M_PI / 180.0);
    float sin = (float) std::sin(rad);
    float cos = (float) std::cos(rad);
    float m11 = mScaleX * cos;
    float m12 = -mScaleY * sin;
    float m21 = mScaleX * sin;
    float m22 = mScaleY * cos;

    for (View* view : mViews) {
        if (view == nullptr) continue;
        int x = (view->getLeft() + view->getRight()) / 2;
        int y = (view->getTop() + view->getBottom()) / 2;
        float dx = x - mComputedCenterX;
        float dy = y - mComputedCenterY;
        float shiftx = m11 * dx + m12 * dy - dx + mShiftX;
        float shifty = m21 * dx + m22 * dy - dy + mShiftY;

        view->setTranslationX(shiftx);
        view->setTranslationY(shifty);
        view->setScaleY(mScaleY);
        view->setScaleX(mScaleX);
        if (!std::isnan(mGroupRotateAngle)) {
            view->setRotation(mGroupRotateAngle);
        }
    }
}

} // namespace cdroid
