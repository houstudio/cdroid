/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
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
