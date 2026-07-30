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
