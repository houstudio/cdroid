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
 */
#include <widgetEx/constraintlayout/core/widgets/guideline.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

namespace cdroid {
namespace clcore {

Guideline::Guideline() {
    // Java field init: mAnchor = mTop (default HORIZONTAL orientation).
    mAnchor = &mTop;
    mAnchors.clear();
    mAnchors.push_back(mAnchor);
    for (auto& slot : mListAnchors) {
        slot = mAnchor;
    }
}

void Guideline::copy(ConstraintWidget* src,
                     std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) {
    ConstraintWidget::copy(src, map);
    Guideline* srcGuideline = static_cast<Guideline*>(src);
    mRelativePercent = srcGuideline->mRelativePercent;
    mRelativeBegin = srcGuideline->mRelativeBegin;
    mRelativeEnd = srcGuideline->mRelativeEnd;
    mGuidelineUseRtl = srcGuideline->mGuidelineUseRtl;
    setOrientation(srcGuideline->mOrientation);
}

bool Guideline::allowedInBarrier() const {
    return true;
}

int Guideline::getRelativeBehaviour() const {
    if (mRelativePercent != -1) {
        return RELATIVE_PERCENT;
    }
    if (mRelativeBegin != -1) {
        return RELATIVE_BEGIN;
    }
    if (mRelativeEnd != -1) {
        return RELATIVE_END;
    }
    return RELATIVE_UNKNOWN;
}

void Guideline::setOrientation(int orientation) {
    if (mOrientation == orientation) {
        return;
    }
    mOrientation = orientation;
    mAnchors.clear();
    if (mOrientation == VERTICAL) {
        mAnchor = &mLeft;
    } else {
        mAnchor = &mTop;
    }
    mAnchors.push_back(mAnchor);
    for (auto& slot : mListAnchors) {
        slot = mAnchor;
    }
}

ConstraintAnchor* Guideline::getAnchor() const {
    return mAnchor;
}

std::string Guideline::getType() const {
    return "Guideline";
}

int Guideline::getOrientation() const {
    return mOrientation;
}

void Guideline::setMinimumPosition(int minimum) {
    mMinimumPosition = minimum;
}

int Guideline::getMinimumPosition() const {
    return mMinimumPosition;
}

ConstraintAnchor* Guideline::getAnchor(ConstraintAnchor::Type anchorType) {
    switch (anchorType) {
        case ConstraintAnchor::Type::LEFT:
        case ConstraintAnchor::Type::RIGHT: {
            if (mOrientation == VERTICAL) {
                return mAnchor;
            }
        } break;
        case ConstraintAnchor::Type::TOP:
        case ConstraintAnchor::Type::BOTTOM: {
            if (mOrientation == HORIZONTAL) {
                return mAnchor;
            }
        } break;
        case ConstraintAnchor::Type::BASELINE:
        case ConstraintAnchor::Type::CENTER:
        case ConstraintAnchor::Type::CENTER_X:
        case ConstraintAnchor::Type::CENTER_Y:
        case ConstraintAnchor::Type::NONE:
            return nullptr;
    }
    return nullptr;
}

void Guideline::setGuidePercent(int value) {
    setGuidePercent(value / 100.0f);
}

void Guideline::setGuidePercent(float value) {
    if (value > -1) {
        mRelativePercent = value;
        mRelativeBegin = -1;
        mRelativeEnd = -1;
    }
}

void Guideline::setGuideBegin(int value) {
    if (value > -1) {
        mRelativePercent = -1;
        mRelativeBegin = value;
        mRelativeEnd = -1;
    }
}

void Guideline::setGuideEnd(int value) {
    if (value > -1) {
        mRelativePercent = -1;
        mRelativeBegin = -1;
        mRelativeEnd = value;
    }
}

float Guideline::getRelativePercent() const {
    return mRelativePercent;
}

int Guideline::getRelativeBegin() const {
    return mRelativeBegin;
}

int Guideline::getRelativeEnd() const {
    return mRelativeEnd;
}

void Guideline::setFinalValue(int position) {
    mAnchor->setFinalValue(position);
    mResolved = true;
}

bool Guideline::isResolvedHorizontally() const {
    return mResolved;
}

bool Guideline::isResolvedVertically() const {
    return mResolved;
}

void Guideline::addToSolver(LinearSystem* system, bool /*optimize*/) {
    // Java casts getParent() to ConstraintWidgetContainer, but only ConstraintWidget-inherited
    // members are used (getAnchor / mListDimensionBehaviors), so the cast is cosmetic.
    ConstraintWidget* parent = getParent();
    if (parent == nullptr) {
        return;
    }
    ConstraintAnchor* begin = parent->getAnchor(ConstraintAnchor::Type::LEFT);
    ConstraintAnchor* end   = parent->getAnchor(ConstraintAnchor::Type::RIGHT);
    bool parentWrapContent = mParent != nullptr
            ? mParent->mListDimensionBehaviors[DIMENSION_HORIZONTAL]
                    == DimensionBehaviour::WRAP_CONTENT
            : false;
    if (mOrientation == HORIZONTAL) {
        begin = parent->getAnchor(ConstraintAnchor::Type::TOP);
        end   = parent->getAnchor(ConstraintAnchor::Type::BOTTOM);
        parentWrapContent = mParent != nullptr
                ? mParent->mListDimensionBehaviors[DIMENSION_VERTICAL]
                        == DimensionBehaviour::WRAP_CONTENT
                : false;
    }
    if (mResolved && mAnchor->hasFinalValue()) {
        SolverVariable* guide = system->createObjectVariable(mAnchor);
        system->addEquality(guide, mAnchor->getFinalValue());
        if (mRelativeBegin != -1) {
            if (parentWrapContent) {
                system->addGreaterThan(system->createObjectVariable(end), guide,
                        0, SolverVariable::STRENGTH_EQUALITY);
            }
        } else if (mRelativeEnd != -1) {
            if (parentWrapContent) {
                SolverVariable* parentRight = system->createObjectVariable(end);
                system->addGreaterThan(guide, system->createObjectVariable(begin),
                        0, SolverVariable::STRENGTH_EQUALITY);
                system->addGreaterThan(parentRight, guide, 0, SolverVariable::STRENGTH_EQUALITY);
            }
        }
        mResolved = false;
        return;
    }
    if (mRelativeBegin != -1) {
        SolverVariable* guide = system->createObjectVariable(mAnchor);
        SolverVariable* parentLeft = system->createObjectVariable(begin);
        system->addEquality(guide, parentLeft, mRelativeBegin, SolverVariable::STRENGTH_FIXED);
        if (parentWrapContent) {
            system->addGreaterThan(system->createObjectVariable(end),
                    guide, 0, SolverVariable::STRENGTH_EQUALITY);
        }
    } else if (mRelativeEnd != -1) {
        SolverVariable* guide = system->createObjectVariable(mAnchor);
        SolverVariable* parentRight = system->createObjectVariable(end);
        system->addEquality(guide, parentRight, -mRelativeEnd, SolverVariable::STRENGTH_FIXED);
        if (parentWrapContent) {
            system->addGreaterThan(guide, system->createObjectVariable(begin),
                    0, SolverVariable::STRENGTH_EQUALITY);
            system->addGreaterThan(parentRight, guide, 0, SolverVariable::STRENGTH_EQUALITY);
        }
    } else if (mRelativePercent != -1) {
        SolverVariable* guide = system->createObjectVariable(mAnchor);
        SolverVariable* parentRight = system->createObjectVariable(end);
        system->addConstraint(LinearSystem::
                createRowDimensionPercent(system, guide, parentRight, mRelativePercent));
    }
}

void Guideline::updateFromSolver(LinearSystem* system, bool /*optimize*/) {
    if (getParent() == nullptr) {
        return;
    }
    int value = system->getObjectVariableValue(mAnchor);
    if (mOrientation == VERTICAL) {
        setX(value);
        setY(0);
        setHeight(getParent()->getHeight());
        setWidth(0);
    } else {
        setX(0);
        setY(value);
        setWidth(getParent()->getWidth());
        setHeight(0);
    }
}

void Guideline::inferRelativePercentPosition() {
    float percent = (getX() / (float) getParent()->getWidth());
    if (mOrientation == HORIZONTAL) {
        percent = (getY() / (float) getParent()->getHeight());
    }
    setGuidePercent(percent);
}

void Guideline::inferRelativeBeginPosition() {
    int position = getX();
    if (mOrientation == HORIZONTAL) {
        position = getY();
    }
    setGuideBegin(position);
}

void Guideline::inferRelativeEndPosition() {
    int position = getParent()->getWidth() - getX();
    if (mOrientation == HORIZONTAL) {
        position = getParent()->getHeight() - getY();
    }
    setGuideEnd(position);
}

void Guideline::cyclePosition() {
    if (mRelativeBegin != -1) {
        // cycle to percent-based position
        inferRelativePercentPosition();
    } else if (mRelativePercent != -1) {
        // cycle to end-based position
        inferRelativeEndPosition();
    } else if (mRelativeEnd != -1) {
        // cycle to begin-based position
        inferRelativeBeginPosition();
    }
}

bool Guideline::isPercent() const {
    return mRelativePercent != -1 && mRelativeBegin == -1 && mRelativeEnd == -1;
}

} // namespace clcore
} // namespace cdroid
