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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Guideline.
 *
 * Lives in the `clcore` sub-namespace because the widget layer (Stage 5) will define a
 * `cdroid::Guideline` View subclass of the same name; the core solver model Guideline must not
 * collide with it. Extends cdroid::ConstraintWidget (cross-namespace inheritance is fine).
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_GUIDELINE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_GUIDELINE_H

#include <string>
#include <unordered_map>

#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

class LinearSystem;

namespace clcore {

class Guideline : public ConstraintWidget {
  public:
    static const int HORIZONTAL = 0;
    static const int VERTICAL   = 1;

    static const int RELATIVE_PERCENT = 0;
    static const int RELATIVE_BEGIN   = 1;
    static const int RELATIVE_END     = 2;
    static const int RELATIVE_UNKNOWN = -1;

    Guideline();

    void copy(ConstraintWidget* src,
              std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) override;
    bool allowedInBarrier() const override;
    bool isGuideline() const override { return true; }

    int getRelativeBehaviour() const;
    void setOrientation(int orientation);
    ConstraintAnchor* getAnchor() const;        // no-arg: returns the active guideline anchor
    std::string getType() const override;
    int getOrientation() const;
    void setMinimumPosition(int minimum);
    int getMinimumPosition() const;
    ConstraintAnchor* getAnchor(ConstraintAnchor::Type anchorType) override;

    void setGuidePercent(int value);
    void setGuidePercent(float value);
    void setGuideBegin(int value);
    void setGuideEnd(int value);
    float getRelativePercent() const;
    int getRelativeBegin() const;
    int getRelativeEnd() const;

    void setFinalValue(int position);
    bool isResolvedHorizontally() const override;
    bool isResolvedVertically() const override;

    void addToSolver(LinearSystem* system, bool optimize) override;
    void updateFromSolver(LinearSystem* system, bool optimize) override;

    void inferRelativePercentPosition();
    void inferRelativeBeginPosition();
    void inferRelativeEndPosition();
    void cyclePosition();
    bool isPercent() const;

  protected:
    float mRelativePercent = -1;
    int   mRelativeBegin   = -1;
    int   mRelativeEnd     = -1;
    bool  mGuidelineUseRtl = true;

  private:
    ConstraintAnchor* mAnchor = nullptr; // points at mTop (horizontal) or mLeft (vertical)
    int   mOrientation    = HORIZONTAL;
    int   mMinimumPosition = 0;
    bool  mResolved       = false;
};

} // namespace clcore
} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_GUIDELINE_H
