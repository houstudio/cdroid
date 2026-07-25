/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include <widgetEx/constraintlayout/core/widgets/constraint_widget.h>

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
