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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Barrier.
 */
#include <widgetEx/constraintlayout/core/widgets/barrier.h>

#include <algorithm>

#include <porting/cdlog.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

namespace cdroid::clcore {

Barrier::Barrier() = default;

Barrier::Barrier(const std::string& debugName) {
    setDebugName(debugName);
}

bool Barrier::allowedInBarrier() const {
    return true;
}
bool Barrier::isBarrier() const {
    return true;
}
bool Barrier::isResolvedHorizontally() const {
    return mResolved;
}
bool Barrier::isResolvedVertically() const {
    return mResolved;
}

int  Barrier::getBarrierType() const {
    return mBarrierType;
}
void Barrier::setBarrierType(int barrierType) {
    mBarrierType = barrierType;
}
bool Barrier::getAllowsGoneWidget() const {
    return mAllowsGoneWidget;
}
void Barrier::setAllowsGoneWidget(bool allowsGoneWidget) {
    mAllowsGoneWidget = allowsGoneWidget;
}
int  Barrier::getMargin() const {
    return mMargin;
}
void Barrier::setMargin(int margin) {
    mMargin = margin;
}

int Barrier::getOrientation() const {
    switch (mBarrierType) {
    case LEFT:
    case RIGHT:
        return HORIZONTAL;
    case TOP:
    case BOTTOM:
        return VERTICAL;
    default:
        return UNKNOWN;
    }
}

void Barrier::copy(ConstraintWidget* src,
                   std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) {
    HelperWidget::copy(src, map);
    auto* srcBarrier = static_cast<Barrier*>(src);
    mBarrierType = srcBarrier->mBarrierType;
    mAllowsGoneWidget = srcBarrier->mAllowsGoneWidget;
    mMargin = srcBarrier->mMargin;
}

std::string Barrier::getType() const {
    return "Barrier";
}

void Barrier::markWidgets() {
    for (ConstraintWidget* widget : mWidgets) {
        if (!mAllowsGoneWidget && !widget->allowedInBarrier()) {
            continue;
        }
        if (mBarrierType == LEFT || mBarrierType == RIGHT) {
            widget->setIsInBarrier(HORIZONTAL, true);
        } else if (mBarrierType == TOP || mBarrierType == BOTTOM) {
            widget->setIsInBarrier(VERTICAL, true);
        }
    }
}

// Faithful port of androidx.constraintlayout.core.widgets.Barrier#addToSolver.
void Barrier::addToSolver(LinearSystem* system, bool /*optimize*/) {
    // Re-anchor the list (Guideline swaps these; for Barrier they are the plain side anchors).
    mListAnchors[LEFT]   = &mLeft;
    mListAnchors[TOP]    = &mTop;
    mListAnchors[RIGHT]  = &mRight;
    mListAnchors[BOTTOM] = &mBottom;
    for (int i = 0; i < 6; i++) {
        mListAnchors[i]->mSolverVariable = system->createObjectVariable(mListAnchors[i]);
    }

    ConstraintAnchor* position;
    if (mBarrierType >= 0 && mBarrierType < 4) {
        position = mListAnchors[mBarrierType];
    } else {
        return;
    }

    constexpr bool USE_RESOLUTION = true;
    if (USE_RESOLUTION) {
        if (!mResolved) {
            allSolved();
        }
        if (mResolved) {
            mResolved = false;
            if (mBarrierType == LEFT || mBarrierType == RIGHT) {
                system->addEquality(mLeft.getSolverVariable(), mX);
                system->addEquality(mRight.getSolverVariable(), mX);
            } else { // TOP / BOTTOM
                system->addEquality(mTop.getSolverVariable(), mY);
                system->addEquality(mBottom.getSolverVariable(), mY);
            }
            return;
        }
    }

    // If any referenced widget is match_constraint on the barrier's axis (and fully anchored),
    // the barrier must use weaker constraints so the solver can size those widgets.
    bool hasMatchConstraintWidgets = false;
    for (ConstraintWidget* widget : mWidgets) {
        if (!mAllowsGoneWidget && !widget->allowedInBarrier()) {
            continue;
        }
        if ((mBarrierType == LEFT || mBarrierType == RIGHT)
                && widget->getHorizontalDimensionBehaviour()
                == DimensionBehaviour::MATCH_CONSTRAINT
                && widget->mLeft.getTarget() != nullptr
                && widget->mRight.getTarget() != nullptr) {
            hasMatchConstraintWidgets = true;
            break;
        } else if ((mBarrierType == TOP || mBarrierType == BOTTOM)
                   && widget->getVerticalDimensionBehaviour()
                   == DimensionBehaviour::MATCH_CONSTRAINT
                   && widget->mTop.getTarget() != nullptr
                   && widget->mBottom.getTarget() != nullptr) {
            hasMatchConstraintWidgets = true;
            break;
        }
    }

    bool mHasHorizontalCenteredDependents =
        mLeft.hasCenteredDependents() || mRight.hasCenteredDependents();
    bool mHasVerticalCenteredDependents =
        mTop.hasCenteredDependents() || mBottom.hasCenteredDependents();
    bool applyEqualityOnReferences = !hasMatchConstraintWidgets
                                     && ((mBarrierType == LEFT   && mHasHorizontalCenteredDependents)
                                         || (mBarrierType == TOP    && mHasVerticalCenteredDependents)
                                         || (mBarrierType == RIGHT  && mHasHorizontalCenteredDependents)
                                         || (mBarrierType == BOTTOM && mHasVerticalCenteredDependents));

    int equalityOnReferencesStrength = SolverVariable::STRENGTH_EQUALITY;
    if (!applyEqualityOnReferences) {
        equalityOnReferencesStrength = SolverVariable::STRENGTH_HIGHEST;
    }
    for (ConstraintWidget* widget : mWidgets) {
        if (!mAllowsGoneWidget && !widget->allowedInBarrier()) {
            continue;
        }
        ConstraintAnchor* widgetAnchor = widget->mListAnchors[mBarrierType];
        SolverVariable* target = system->createObjectVariable(widgetAnchor);
        widgetAnchor->mSolverVariable = target;
        int margin = 0;
        if (widgetAnchor->getTarget() != nullptr && widgetAnchor->getTarget()->getOwner() == this) {
            margin += widgetAnchor->getMargin();
        }
        if (mBarrierType == LEFT || mBarrierType == TOP) {
            system->addLowerBarrier(position->getSolverVariable(), target,
                                    mMargin - margin, hasMatchConstraintWidgets);
        } else {
            system->addGreaterBarrier(position->getSolverVariable(), target,
                                      mMargin + margin, hasMatchConstraintWidgets);
        }
        // USE_RELAX_GONE is false in the Java original — always emit the equality.
        system->addEquality(position->getSolverVariable(), target,
                            mMargin + margin, equalityOnReferencesStrength);
    }

    int barrierParentStrength = SolverVariable::STRENGTH_HIGHEST;
    int barrierParentStrengthOpposite = SolverVariable::STRENGTH_NONE;

    if (mBarrierType == LEFT) {
        system->addEquality(mRight.getSolverVariable(), mLeft.getSolverVariable(),
                            0, SolverVariable::STRENGTH_FIXED);
        system->addEquality(mLeft.getSolverVariable(), mParent->mRight.getSolverVariable(),
                            0, barrierParentStrength);
        system->addEquality(mLeft.getSolverVariable(), mParent->mLeft.getSolverVariable(),
                            0, barrierParentStrengthOpposite);
    } else if (mBarrierType == RIGHT) {
        system->addEquality(mLeft.getSolverVariable(), mRight.getSolverVariable(),
                            0, SolverVariable::STRENGTH_FIXED);
        system->addEquality(mLeft.getSolverVariable(), mParent->mLeft.getSolverVariable(),
                            0, barrierParentStrength);
        system->addEquality(mLeft.getSolverVariable(), mParent->mRight.getSolverVariable(),
                            0, barrierParentStrengthOpposite);
    } else if (mBarrierType == TOP) {
        system->addEquality(mBottom.getSolverVariable(), mTop.getSolverVariable(),
                            0, SolverVariable::STRENGTH_FIXED);
        system->addEquality(mTop.getSolverVariable(), mParent->mBottom.getSolverVariable(),
                            0, barrierParentStrength);
        system->addEquality(mTop.getSolverVariable(), mParent->mTop.getSolverVariable(),
                            0, barrierParentStrengthOpposite);
    } else if (mBarrierType == BOTTOM) {
        system->addEquality(mTop.getSolverVariable(), mBottom.getSolverVariable(),
                            0, SolverVariable::STRENGTH_FIXED);
        system->addEquality(mTop.getSolverVariable(), mParent->mTop.getSolverVariable(),
                            0, barrierParentStrength);
        system->addEquality(mTop.getSolverVariable(), mParent->mBottom.getSolverVariable(),
                            0, barrierParentStrengthOpposite);
    }
}

bool Barrier::allSolved() {
    constexpr bool USE_RESOLUTION = true;
    if (!USE_RESOLUTION) {
        return false;
    }
    bool hasAllWidgetsResolved = true;
    for (ConstraintWidget* widget : mWidgets) {
        if (!mAllowsGoneWidget && !widget->allowedInBarrier()) {
            continue;
        }
        if ((mBarrierType == LEFT || mBarrierType == RIGHT)
                && !widget->isResolvedHorizontally()) {
            hasAllWidgetsResolved = false;
        } else if ((mBarrierType == TOP || mBarrierType == BOTTOM)
                   && !widget->isResolvedVertically()) {
            hasAllWidgetsResolved = false;
        }
    }

    if (hasAllWidgetsResolved && !mWidgets.empty()) {
        // we're done!
        int barrierPosition = 0;
        bool initialized = false;
        for (ConstraintWidget* widget : mWidgets) {
            if (!mAllowsGoneWidget && !widget->allowedInBarrier()) {
                continue;
            }
            if (!initialized) {
                if (mBarrierType == LEFT) {
                    barrierPosition = widget->getAnchor(ConstraintAnchor::Type::LEFT)->getFinalValue();
                } else if (mBarrierType == RIGHT) {
                    barrierPosition = widget->getAnchor(ConstraintAnchor::Type::RIGHT)->getFinalValue();
                } else if (mBarrierType == TOP) {
                    barrierPosition = widget->getAnchor(ConstraintAnchor::Type::TOP)->getFinalValue();
                } else if (mBarrierType == BOTTOM) {
                    barrierPosition = widget->getAnchor(ConstraintAnchor::Type::BOTTOM)->getFinalValue();
                }
                initialized = true;
            }
            if (mBarrierType == LEFT) {
                barrierPosition = std::min(barrierPosition,
                                           widget->getAnchor(ConstraintAnchor::Type::LEFT)->getFinalValue());
            } else if (mBarrierType == RIGHT) {
                barrierPosition = std::max(barrierPosition,
                                           widget->getAnchor(ConstraintAnchor::Type::RIGHT)->getFinalValue());
            } else if (mBarrierType == TOP) {
                barrierPosition = std::min(barrierPosition,
                                           widget->getAnchor(ConstraintAnchor::Type::TOP)->getFinalValue());
            } else if (mBarrierType == BOTTOM) {
                barrierPosition = std::max(barrierPosition,
                                           widget->getAnchor(ConstraintAnchor::Type::BOTTOM)->getFinalValue());
            }
        }
        barrierPosition += mMargin;
        if (mBarrierType == LEFT || mBarrierType == RIGHT) {
            setFinalHorizontal(barrierPosition, barrierPosition);
        } else {
            setFinalVertical(barrierPosition, barrierPosition);
        }
        mResolved = true;
        return true;
    }
    return false;
}

} // namespace cdroid::clcore
