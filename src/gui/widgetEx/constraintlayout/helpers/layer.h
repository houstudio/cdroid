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
 *
 * Layer adds the ability to move/rotate/scale a group of referenced views as if they were contained
 * in a ViewGroup. Unlike Barrier/Flow it owns NO core widget (mHelperWidget stays null, like Group):
 * it operates purely at the view layer, applying a post-layout affine transform to each referenced
 * view's translation/scale/rotation about a shared bounding-box center (or an explicit pivot).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_LAYER_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_LAYER_H

#include <limits>
#include <vector>

#include <widgetEx/constraintlayout/helpers/constrainthelper.h>

namespace cdroid {

class Layer : public ConstraintHelper {
  public:
    Layer(Context* ctx, const AttributeSet& attrs);
    explicit Layer(int width, int height);

    // --- group transforms (each recomputes & applies the affine transform) ---
    void setRotation(float angle) override;
    void setScaleX(float scaleX) override;
    void setScaleY(float scaleY) override;
    void setPivotX(float pivotX) override;
    void setPivotY(float pivotY) override;
    void setTranslationX(float dx) override;
    void setTranslationY(float dy) override;
    void setVisibility(int visibility) override;
    void setElevation(float elevation) override;

  protected:
    void init(const AttributeSet& attrs) override;
    void onAttachedToWindow() override;
    void updatePreDraw(ConstraintLayout* container) override;
    void updatePostLayout(ConstraintLayout* container) override;

    void transform();
    void calcCenters();
    void reCacheViews();

    float mRotationCenterX = std::numeric_limits<float>::quiet_NaN();
    float mRotationCenterY = std::numeric_limits<float>::quiet_NaN();
    float mGroupRotateAngle = std::numeric_limits<float>::quiet_NaN();
    float mScaleX = 1;
    float mScaleY = 1;
    float mComputedCenterX = std::numeric_limits<float>::quiet_NaN();
    float mComputedCenterY = std::numeric_limits<float>::quiet_NaN();
    float mComputedMaxX = std::numeric_limits<float>::quiet_NaN();
    float mComputedMaxY = std::numeric_limits<float>::quiet_NaN();
    float mComputedMinX = std::numeric_limits<float>::quiet_NaN();
    float mComputedMinY = std::numeric_limits<float>::quiet_NaN();
    bool mNeedBounds = true;
    std::vector<View*> mViews;   // cache to avoid repeated findViewById() cost
    float mShiftX = 0;
    float mShiftY = 0;

  private:
    ConstraintLayout* mContainer = nullptr;
    bool mApplyVisibilityOnAttach = false;
    bool mApplyElevationOnAttach = false;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_LAYER_H
