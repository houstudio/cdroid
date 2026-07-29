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
 * Ported to C++ for CDROID from androidx.constraintlayout.helper.widget.CircularFlow.
 *
 * Positions referenced views on a circle around a center view by angle/radius. Owns NO core widget
 * (like Group/Layer): anchorReferences() writes circleConstraint/circleAngle/circleRadius onto each
 * referenced view's LayoutParams, and the ConstraintLayout bridge wires those to
 * ConstraintWidget::connectCircularConstraint (CENTER→CENTER + angle), which the solver's
 * addCenterPoint consumes.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_CIRCULARFLOW_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_CIRCULARFLOW_H

#include <string>
#include <vector>

#include <widgetEx/constraintlayout/helpers/constrainthelper.h>

namespace cdroid {

class CircularFlow : public ConstraintHelper {
  public:
    CircularFlow(Context* ctx, const AttributeSet& attrs);
    explicit CircularFlow(int width, int height);

    std::vector<float> getAngles() const;
    std::vector<int>   getRadius() const;

    void setAngles(const std::string& angleList);
    void setAngles(const std::vector<float>& angles);
    void setRadius(const std::string& radiusList);
    void setRadius(const std::vector<int>& radius);
    void setDefaultAngle(float angle);
    void setDefaultRadius(int radius);
    void setViewCenter(int id) { mViewCenter = id; }
    void addViewToCircularFlow(View* view, int radius, float angle);

  protected:
    void init(const AttributeSet& attrs) override;
    void updatePreLayout(ConstraintLayout* container) override;  // = AndroidX anchorReferences

  private:
    void anchorReferences();

    ConstraintLayout* mContainer = nullptr;
    int mViewCenter = 0;
    std::vector<float> mAngles;
    std::vector<int>   mRadius;
    float mDefaultAngle = 0;
    int   mDefaultRadius = 0;
    std::string mReferenceAngles;
    std::string mReferenceRadius;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_CIRCULARFLOW_H
