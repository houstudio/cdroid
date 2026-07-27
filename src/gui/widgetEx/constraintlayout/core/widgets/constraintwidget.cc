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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.ConstraintWidget.
 * SKELETON (Stage 2) — see header.
 */
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <widgetEx/constraintlayout/core/widgets/guideline.h>
#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <widgetEx/constraintlayout/core/cache.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

#include <climits>

#include <algorithm>

namespace cdroid {

// odr-use definitions for static const ints referenced by address (e.g. in unordered_map)
const int ConstraintWidget::CHAIN_SPREAD;
const int ConstraintWidget::CHAIN_SPREAD_INSIDE;
const int ConstraintWidget::CHAIN_PACKED;
const int ConstraintWidget::MATCH_CONSTRAINT_SPREAD;
const int ConstraintWidget::MATCH_CONSTRAINT_WRAP;
const int ConstraintWidget::MATCH_CONSTRAINT_PERCENT;

ConstraintWidget::ConstraintWidget() {
    addAnchors();
}

ConstraintWidget::ConstraintWidget(const std::string& debugName) {
    addAnchors();
    setDebugName(debugName);
}

ConstraintWidget::ConstraintWidget(int x, int y, int width, int height) {
    mX = x;
    mY = y;
    mWidth = width;
    mHeight = height;
    addAnchors();
}

ConstraintWidget::ConstraintWidget(const std::string& debugName, int x, int y, int width, int height)
    : ConstraintWidget(x, y, width, height) {
    setDebugName(debugName);
}

ConstraintWidget::ConstraintWidget(int width, int height)
    : ConstraintWidget(0, 0, width, height) {
}

ConstraintWidget::ConstraintWidget(const std::string& debugName, int width, int height)
    : ConstraintWidget(width, height) {
    setDebugName(debugName);
}

ConstraintWidget::~ConstraintWidget() = default;

ConstraintAnchor* ConstraintWidget::getAnchor(ConstraintAnchor::Type anchorType) {
    switch (anchorType) {
    case ConstraintAnchor::Type::LEFT:
        return &mLeft;
    case ConstraintAnchor::Type::TOP:
        return &mTop;
    case ConstraintAnchor::Type::RIGHT:
        return &mRight;
    case ConstraintAnchor::Type::BOTTOM:
        return &mBottom;
    case ConstraintAnchor::Type::BASELINE:
        return &mBaseline;
    case ConstraintAnchor::Type::CENTER_X:
        return &mCenterX;
    case ConstraintAnchor::Type::CENTER_Y:
        return &mCenterY;
    case ConstraintAnchor::Type::CENTER:
        return &mCenter;
    case ConstraintAnchor::Type::NONE:
        return nullptr;
    }
    return nullptr; // unreachable (Java throws AssertionError)
}

const std::vector<ConstraintAnchor*>& ConstraintWidget::getAnchors() const {
    return mAnchors;
}

void ConstraintWidget::resetSolverVariables(Cache* cache) {
    mLeft.resetSolverVariable(cache);
    mTop.resetSolverVariable(cache);
    mRight.resetSolverVariable(cache);
    mBottom.resetSolverVariable(cache);
    mBaseline.resetSolverVariable(cache);
    mCenter.resetSolverVariable(cache);
    mCenterX.resetSolverVariable(cache);
    mCenterY.resetSolverVariable(cache);
}

int ConstraintWidget::getVisibility() const {
    return mVisibility;
}

void ConstraintWidget::setVisibility(int visibility) {
    mVisibility = visibility;
}

ConstraintWidget* ConstraintWidget::getParent() const {
    return mParent;
}

void ConstraintWidget::setParent(ConstraintWidget* widget) {
    mParent = widget;
}

bool ConstraintWidget::isRoot() const {
    return mParent == nullptr;
}

bool ConstraintWidget::hasBaseline() const {
    return mHasBaseline;
}

void ConstraintWidget::setHasBaseline(bool hasBaseline) {
    mHasBaseline = hasBaseline;
}

std::string ConstraintWidget::getDebugName() const {
    return mDebugName;
}

void ConstraintWidget::setDebugName(const std::string& name) {
    mDebugName = name;
}

int ConstraintWidget::getX() const {
    return mX;
}

int ConstraintWidget::getY() const {
    return mY;
}

int ConstraintWidget::getWidth() const {
    return mWidth;
}

int ConstraintWidget::getHeight() const {
    return mHeight;
}

int ConstraintWidget::getLength(int orientation) const {
    if (orientation == HORIZONTAL) {
        return getWidth();
    } else if (orientation == VERTICAL) {
        return getHeight();
    }
    return 0;
}

int ConstraintWidget::getMinWidth() const {
    return mMinWidth;
}
int ConstraintWidget::getMinHeight() const {
    return mMinHeight;
}
void ConstraintWidget::setMinWidth(int minWidth) {
    mMinWidth = minWidth;
}
void ConstraintWidget::setMinHeight(int minHeight) {
    mMinHeight = minHeight;
}
void ConstraintWidget::setMaxWidth(int maxWidth) {
    mMaxDimension[DIMENSION_HORIZONTAL] = maxWidth;
}
void ConstraintWidget::setMaxHeight(int maxHeight) {
    mMaxDimension[DIMENSION_VERTICAL] = maxHeight;
}
void ConstraintWidget::setBaselineDistance(int baselineDistance) {
    mBaselineDistance = baselineDistance;
}
float ConstraintWidget::getDimensionRatio() const {
    return mDimensionRatio;
}
bool ConstraintWidget::isInVirtualLayout() const {
    return mIsInVirtualLayout;
}
void ConstraintWidget::setInVirtualLayout(bool inVirtualLayout) {
    mIsInVirtualLayout = inVirtualLayout;
}

void ConstraintWidget::connect(ConstraintAnchor& from, ConstraintAnchor* to, int margin) {
    if (from.getOwner() == this && to != nullptr) {
        from.connect(to, margin);
    }
}

void ConstraintWidget::resetAnchors() {
    // The Java original short-circuits when the parent container handles internal constraints;
    // CDROID containers never do, so always reset every anchor.
    for (ConstraintAnchor* anchor : mAnchors) {
        anchor->reset();
    }
}

void ConstraintWidget::setMeasureRequested(bool measureRequested) {
    mMeasureRequested = measureRequested;
}
bool ConstraintWidget::isMeasureRequested() const {
    return mMeasureRequested && mVisibility != GONE;
}

void ConstraintWidget::setX(int x) {
    mX = x;
}

void ConstraintWidget::setY(int y) {
    mY = y;
}

void ConstraintWidget::setWidth(int width) {
    mWidth = width;
}

void ConstraintWidget::setHeight(int height) {
    mHeight = height;
}

void ConstraintWidget::setOffset(int x, int y) {
    mOffsetX = x;
    mOffsetY = y;
}

void ConstraintWidget::setHorizontalDimension(int left, int right) {
    mX = left;
    mWidth = right - left;
    if (mWidth < mMinWidth) {
        mWidth = mMinWidth;
    }
}

void ConstraintWidget::setVerticalDimension(int top, int bottom) {
    mY = top;
    mHeight = bottom - top;
    if (mHeight < mMinHeight) {
        mHeight = mMinHeight;
    }
}

void ConstraintWidget::setFrame(int left, int top, int right, int bottom) {
    int w = right - left;
    int h = bottom - top;

    mX = left;
    mY = top;

    if (mVisibility == GONE) {
        mWidth = 0;
        mHeight = 0;
        return;
    }

    // correct dimensional instability caused by rounding errors
    if (mListDimensionBehaviors[DIMENSION_HORIZONTAL] == DimensionBehaviour::FIXED && w < mWidth) {
        w = mWidth;
    }
    if (mListDimensionBehaviors[DIMENSION_VERTICAL] == DimensionBehaviour::FIXED && h < mHeight) {
        h = mHeight;
    }

    mWidth = w;
    mHeight = h;

    if (mHeight < mMinHeight) {
        mHeight = mMinHeight;
    }
    if (mWidth < mMinWidth) {
        mWidth = mMinWidth;
    }
    if (mMatchConstraintMaxWidth > 0
            && mListDimensionBehaviors[HORIZONTAL] == DimensionBehaviour::MATCH_CONSTRAINT) {
        mWidth = std::min(mWidth, mMatchConstraintMaxWidth);
    }
    if (mMatchConstraintMaxHeight > 0
            && mListDimensionBehaviors[VERTICAL] == DimensionBehaviour::MATCH_CONSTRAINT) {
        mHeight = std::min(mHeight, mMatchConstraintMaxHeight);
    }
}

ConstraintWidget::DimensionBehaviour ConstraintWidget::getHorizontalDimensionBehaviour() const {
    return mListDimensionBehaviors[DIMENSION_HORIZONTAL];
}

ConstraintWidget::DimensionBehaviour ConstraintWidget::getVerticalDimensionBehaviour() const {
    return mListDimensionBehaviors[DIMENSION_VERTICAL];
}

ConstraintWidget::DimensionBehaviour ConstraintWidget::getDimensionBehaviour(int orientation) const {
    if (orientation == HORIZONTAL) {
        return getHorizontalDimensionBehaviour();
    } else if (orientation == VERTICAL) {
        return getVerticalDimensionBehaviour();
    }
    return DimensionBehaviour::FIXED; // Java returns null; FIXED is a safe non-null default
}

void ConstraintWidget::setHorizontalDimensionBehaviour(DimensionBehaviour behaviour) {
    mListDimensionBehaviors[DIMENSION_HORIZONTAL] = behaviour;
}

void ConstraintWidget::setVerticalDimensionBehaviour(DimensionBehaviour behaviour) {
    mListDimensionBehaviors[DIMENSION_VERTICAL] = behaviour;
}

void ConstraintWidget::setDimensionBehaviour(int orientation, DimensionBehaviour behaviour) {
    if (orientation == HORIZONTAL) {
        mListDimensionBehaviors[DIMENSION_HORIZONTAL] = behaviour;
    } else if (orientation == VERTICAL) {
        mListDimensionBehaviors[DIMENSION_VERTICAL] = behaviour;
    }
}

std::string ConstraintWidget::getType() const {
    return mType;
}

void ConstraintWidget::setType(const std::string& type) {
    mType = type;
}

bool ConstraintWidget::isVirtualLayout() const {
    return false;
}
bool ConstraintWidget::allowedInBarrier() const {
    return mVisibility != GONE;
}

bool ConstraintWidget::isBarrier() const {
    return false; // Barrier (Stage 5) overrides to true
}

bool ConstraintWidget::isInBarrier(int orientation) const {
    return mIsInBarrier[orientation];
}

void ConstraintWidget::setIsInBarrier(int orientation, bool value) {
    mIsInBarrier[orientation] = value;
}

bool ConstraintWidget::isInPlaceholder() const {
    return mInPlaceholder;
}

void ConstraintWidget::setInPlaceholder(bool inPlaceholder) {
    mInPlaceholder = inPlaceholder;
}

bool ConstraintWidget::hasDependencies() const {
    for (ConstraintAnchor* anchor : mAnchors) {
        if (anchor->hasDependents()) {
            return true;
        }
    }
    return false;
}

bool ConstraintWidget::isInHorizontalChain() const {
    if ((mLeft.mTarget != nullptr && mLeft.mTarget->mTarget == &mLeft)
            || (mRight.mTarget != nullptr && mRight.mTarget->mTarget == &mRight)) {
        return true;
    }
    return false;
}

bool ConstraintWidget::isInVerticalChain() const {
    if ((mTop.mTarget != nullptr && mTop.mTarget->mTarget == &mTop)
            || (mBottom.mTarget != nullptr && mBottom.mTarget->mTarget == &mBottom)) {
        return true;
    }
    return false;
}

bool ConstraintWidget::isChainHead(int orientation) const {
    int offset = orientation * 2;
    return (mListAnchors[offset]->mTarget != nullptr
            && mListAnchors[offset]->mTarget->mTarget != mListAnchors[offset])
           && (mListAnchors[offset + 1]->mTarget != nullptr
               && mListAnchors[offset + 1]->mTarget->mTarget == mListAnchors[offset + 1]);
}

void ConstraintWidget::setupDimensionRatio(bool /*hParentWrapContent*/,
        bool /*vParentWrapContent*/,
        bool horizontalDimensionFixed,
        bool verticalDimensionFixed) {
    if (mResolvedDimensionRatioSide == UNKNOWN) {
        if (horizontalDimensionFixed && !verticalDimensionFixed) {
            mResolvedDimensionRatioSide = HORIZONTAL;
        } else if (!horizontalDimensionFixed && verticalDimensionFixed) {
            mResolvedDimensionRatioSide = VERTICAL;
            if (mDimensionRatioSide == UNKNOWN) {
                // need to reverse the ratio as the parsing is done in horizontal mode
                mResolvedDimensionRatio = 1 / mResolvedDimensionRatio;
            }
        }
    }

    if (mResolvedDimensionRatioSide == HORIZONTAL
            && !(mTop.isConnected() && mBottom.isConnected())) {
        mResolvedDimensionRatioSide = VERTICAL;
    } else if (mResolvedDimensionRatioSide == VERTICAL
               && !(mLeft.isConnected() && mRight.isConnected())) {
        mResolvedDimensionRatioSide = HORIZONTAL;
    }

    // if dimension is still unknown... check parentWrap
    if (mResolvedDimensionRatioSide == UNKNOWN) {
        if (!(mTop.isConnected() && mBottom.isConnected()
                && mLeft.isConnected() && mRight.isConnected())) {
            // only do that if not all connections are set
            if (mTop.isConnected() && mBottom.isConnected()) {
                mResolvedDimensionRatioSide = HORIZONTAL;
            } else if (mLeft.isConnected() && mRight.isConnected()) {
                mResolvedDimensionRatio = 1 / mResolvedDimensionRatio;
                mResolvedDimensionRatioSide = VERTICAL;
            }
        }
    }

    // DO_NOT_USE branches omitted (flag is false).

    if (mResolvedDimensionRatioSide == UNKNOWN) {
        if (mMatchConstraintMinWidth > 0 && mMatchConstraintMinHeight == 0) {
            mResolvedDimensionRatioSide = HORIZONTAL;
        } else if (mMatchConstraintMinWidth == 0 && mMatchConstraintMinHeight > 0) {
            mResolvedDimensionRatio = 1 / mResolvedDimensionRatio;
            mResolvedDimensionRatioSide = VERTICAL;
        }
    }
}

bool ConstraintWidget::isResolvedHorizontally() const {
    return mResolvedHorizontal || (mLeft.hasFinalValue() && mRight.hasFinalValue());
}

bool ConstraintWidget::isResolvedVertically() const {
    return mResolvedVertical || (mTop.hasFinalValue() && mBottom.hasFinalValue());
}

void ConstraintWidget::setFinalHorizontal(int x1, int x2) {
    if (mResolvedHorizontal) {
        return;
    }
    mLeft.setFinalValue(x1);
    mRight.setFinalValue(x2);
    mX = x1;
    mWidth = x2 - x1;
    mResolvedHorizontal = true;
}

void ConstraintWidget::setFinalVertical(int y1, int y2) {
    if (mResolvedVertical) {
        return;
    }
    mTop.setFinalValue(y1);
    mBottom.setFinalValue(y2);
    mY = y1;
    mHeight = y2 - y1;
    if (mHasBaseline) {
        mBaseline.setFinalValue(y1 + mBaselineDistance);
    }
    mResolvedVertical = true;
}

void ConstraintWidget::setFinalBaseline(int baselineValue) {
    if (!mHasBaseline) {
        return;
    }
    int y1 = baselineValue - mBaselineDistance;
    int y2 = y1 + mHeight;
    mY = y1;
    mTop.setFinalValue(y1);
    mBottom.setFinalValue(y2);
    mBaseline.setFinalValue(baselineValue);
    mResolvedVertical = true;
}

void ConstraintWidget::resetFinalResolution() {
    mResolvedHorizontal = false;
    mResolvedVertical = false;
    for (ConstraintAnchor* anchor : mAnchors) {
        anchor->resetFinalResolution();
    }
}

void ConstraintWidget::addToSolver(LinearSystem* system, bool /*optimize*/) {
    SolverVariable* left = system->createObjectVariable(&mLeft);
    SolverVariable* right = system->createObjectVariable(&mRight);
    SolverVariable* top = system->createObjectVariable(&mTop);
    SolverVariable* bottom = system->createObjectVariable(&mBottom);
    SolverVariable* baseline = system->createObjectVariable(&mBaseline);

    bool horizontalParentWrapContent = false;
    bool verticalParentWrapContent = false;
    if (mParent != nullptr) {
        horizontalParentWrapContent = mParent->mListDimensionBehaviors[DIMENSION_HORIZONTAL]
                                      == DimensionBehaviour::WRAP_CONTENT;
        verticalParentWrapContent = mParent->mListDimensionBehaviors[DIMENSION_VERTICAL]
                                    == DimensionBehaviour::WRAP_CONTENT;
        switch (mWrapBehaviorInParent) {
        case WRAP_BEHAVIOR_SKIPPED:
            horizontalParentWrapContent = false;
            verticalParentWrapContent = false;
            break;
        case WRAP_BEHAVIOR_HORIZONTAL_ONLY:
            verticalParentWrapContent = false;
            break;
        case WRAP_BEHAVIOR_VERTICAL_ONLY:
            horizontalParentWrapContent = false;
            break;
        }
    }

    if (!(mVisibility != GONE || mAnimated || hasDependencies()
            || mIsInBarrier[HORIZONTAL] || mIsInBarrier[VERTICAL])) {
        return;
    }

    if (mResolvedHorizontal || mResolvedVertical) {
        // For now apply all, but that won't work for wrap/wrap layouts.
        if (mResolvedHorizontal) {
            system->addEquality(left, mX);
            system->addEquality(right, mX + mWidth);
            if (horizontalParentWrapContent && mParent != nullptr) {
                if (mOptimizeWrapOnResolved) {
                    ConstraintWidgetContainer* container = dynamic_cast<ConstraintWidgetContainer*>(mParent);
                    if (container != nullptr) {
                        container->addHorizontalWrapMinVariable(&mLeft);
                        container->addHorizontalWrapMaxVariable(&mRight);
                    }
                } else {
                    system->addGreaterThan(system->createObjectVariable(&mParent->mRight),
                                           right, 0, SolverVariable::STRENGTH_EQUALITY);
                }
            }
        }
        if (mResolvedVertical) {
            system->addEquality(top, mY);
            system->addEquality(bottom, mY + mHeight);
            if (mBaseline.hasDependents()) {
                system->addEquality(baseline, mY + mBaselineDistance);
            }
            if (verticalParentWrapContent && mParent != nullptr) {
                if (mOptimizeWrapOnResolved) {
                    ConstraintWidgetContainer* container = dynamic_cast<ConstraintWidgetContainer*>(mParent);
                    if (container != nullptr) {
                        container->addVerticalWrapMinVariable(&mTop);
                        container->addVerticalWrapMaxVariable(&mBottom);
                    }
                } else {
                    system->addGreaterThan(system->createObjectVariable(&mParent->mBottom),
                                           bottom, 0, SolverVariable::STRENGTH_EQUALITY);
                }
            }
        }
        if (mResolvedHorizontal && mResolvedVertical) {
            mResolvedHorizontal = false;
            mResolvedVertical = false;
            return;
        }
    }

    if (LinearSystem::getMetrics() != nullptr) {
        LinearSystem::getMetrics()->widgets++;
    }
    // analyzer-run branch (optimize && mHorizontalRun/mVerticalRun resolved) omitted — Stage 3.

    bool inHorizontalChain = false;
    bool inVerticalChain = false;

    if (mParent != nullptr) {
        // Add this widget to a horizontal chain if it is the Head of it.
        if (isChainHead(HORIZONTAL)) {
            ConstraintWidgetContainer* container = dynamic_cast<ConstraintWidgetContainer*>(mParent);
            if (container != nullptr) container->addChain(this, HORIZONTAL);
            inHorizontalChain = true;
        } else {
            inHorizontalChain = isInHorizontalChain();
        }

        // Add this widget to a vertical chain if it is the Head of it.
        if (isChainHead(VERTICAL)) {
            ConstraintWidgetContainer* container = dynamic_cast<ConstraintWidgetContainer*>(mParent);
            if (container != nullptr) container->addChain(this, VERTICAL);
            inVerticalChain = true;
        } else {
            inVerticalChain = isInVerticalChain();
        }

        if (!inHorizontalChain && horizontalParentWrapContent && mVisibility != GONE
                && mLeft.mTarget == nullptr && mRight.mTarget == nullptr) {
            SolverVariable* parentRight = system->createObjectVariable(&mParent->mRight);
            system->addGreaterThan(parentRight, right, 0, SolverVariable::STRENGTH_LOW);
        }

        if (!inVerticalChain && verticalParentWrapContent && mVisibility != GONE
                && mTop.mTarget == nullptr && mBottom.mTarget == nullptr) {
            // Java also checks `mBaseline == null`; in C++ mBaseline is always a value member,
            // so that sub-check is dropped (no-op in practice).
            SolverVariable* parentBottom = system->createObjectVariable(&mParent->mBottom);
            system->addGreaterThan(parentBottom, bottom, 0, SolverVariable::STRENGTH_LOW);
        }
    }

    int width = mWidth;
    if (width < mMinWidth) {
        width = mMinWidth;
    }
    int height = mHeight;
    if (height < mMinHeight) {
        height = mMinHeight;
    }

    // Dimensions can be either fixed (a given value) or dependent on the solver (MATCH_CONSTRAINT).
    bool horizontalDimensionFixed =
        mListDimensionBehaviors[DIMENSION_HORIZONTAL] != DimensionBehaviour::MATCH_CONSTRAINT;
    bool verticalDimensionFixed =
        mListDimensionBehaviors[DIMENSION_VERTICAL] != DimensionBehaviour::MATCH_CONSTRAINT;

    // Evaluate the dimension ratio here as the connections can change.
    bool useRatio = false;
    mResolvedDimensionRatioSide = mDimensionRatioSide;
    mResolvedDimensionRatio = mDimensionRatio;

    int matchConstraintDefaultWidth = mMatchConstraintDefaultWidth;
    int matchConstraintDefaultHeight = mMatchConstraintDefaultHeight;

    if (mDimensionRatio > 0 && mVisibility != GONE) {
        useRatio = true;
        if (mListDimensionBehaviors[DIMENSION_HORIZONTAL] == DimensionBehaviour::MATCH_CONSTRAINT
                && matchConstraintDefaultWidth == MATCH_CONSTRAINT_SPREAD) {
            matchConstraintDefaultWidth = MATCH_CONSTRAINT_RATIO;
        }
        if (mListDimensionBehaviors[DIMENSION_VERTICAL] == DimensionBehaviour::MATCH_CONSTRAINT
                && matchConstraintDefaultHeight == MATCH_CONSTRAINT_SPREAD) {
            matchConstraintDefaultHeight = MATCH_CONSTRAINT_RATIO;
        }

        if (mListDimensionBehaviors[DIMENSION_HORIZONTAL] == DimensionBehaviour::MATCH_CONSTRAINT
                && mListDimensionBehaviors[DIMENSION_VERTICAL] == DimensionBehaviour::MATCH_CONSTRAINT
                && matchConstraintDefaultWidth == MATCH_CONSTRAINT_RATIO
                && matchConstraintDefaultHeight == MATCH_CONSTRAINT_RATIO) {
            setupDimensionRatio(horizontalParentWrapContent, verticalParentWrapContent,
                                horizontalDimensionFixed, verticalDimensionFixed);
        } else if (mListDimensionBehaviors[DIMENSION_HORIZONTAL] == DimensionBehaviour::MATCH_CONSTRAINT
                   && matchConstraintDefaultWidth == MATCH_CONSTRAINT_RATIO) {
            mResolvedDimensionRatioSide = HORIZONTAL;
            width = (int) (mResolvedDimensionRatio * mHeight);
            if (mListDimensionBehaviors[DIMENSION_VERTICAL] != DimensionBehaviour::MATCH_CONSTRAINT) {
                matchConstraintDefaultWidth = MATCH_CONSTRAINT_RATIO_RESOLVED;
                useRatio = false;
            }
        } else if (mListDimensionBehaviors[DIMENSION_VERTICAL] == DimensionBehaviour::MATCH_CONSTRAINT
                   && matchConstraintDefaultHeight == MATCH_CONSTRAINT_RATIO) {
            mResolvedDimensionRatioSide = VERTICAL;
            if (mDimensionRatioSide == UNKNOWN) {
                mResolvedDimensionRatio = 1 / mResolvedDimensionRatio;
            }
            height = (int) (mResolvedDimensionRatio * mWidth);
            if (mListDimensionBehaviors[DIMENSION_HORIZONTAL] != DimensionBehaviour::MATCH_CONSTRAINT) {
                matchConstraintDefaultHeight = MATCH_CONSTRAINT_RATIO_RESOLVED;
                useRatio = false;
            }
        }
    }

    mResolvedMatchConstraintDefault[HORIZONTAL] = matchConstraintDefaultWidth;
    mResolvedMatchConstraintDefault[VERTICAL] = matchConstraintDefaultHeight;
    mResolvedHasRatio = useRatio;

    bool useHorizontalRatio = useRatio && (mResolvedDimensionRatioSide == HORIZONTAL
                                           || mResolvedDimensionRatioSide == UNKNOWN);
    bool useVerticalRatio = useRatio && (mResolvedDimensionRatioSide == VERTICAL
                                         || mResolvedDimensionRatioSide == UNKNOWN);

    // Horizontal resolution
    bool wrapContent = (mListDimensionBehaviors[DIMENSION_HORIZONTAL] == DimensionBehaviour::WRAP_CONTENT)
                       && (dynamic_cast<ConstraintWidgetContainer*>(this) != nullptr);
    if (wrapContent) {
        width = 0;
    }

    bool applyPosition = true;
    if (mCenter.isConnected()) {
        applyPosition = false;
    }

    bool isInHorizontalBarrier = mIsInBarrier[HORIZONTAL];
    bool isInVerticalBarrier = mIsInBarrier[VERTICAL];

    if (mHorizontalResolution != DIRECT && !mResolvedHorizontal) {
        // analyzer-run branch omitted (no mHorizontalRun) — always applyConstraints.
        SolverVariable* parentMax = mParent != nullptr
                                    ? system->createObjectVariable(&mParent->mRight) : nullptr;
        SolverVariable* parentMin = mParent != nullptr
                                    ? system->createObjectVariable(&mParent->mLeft) : nullptr;
        applyConstraints(system, true, horizontalParentWrapContent,
                         verticalParentWrapContent, isTerminalWidget[HORIZONTAL], parentMin,
                         parentMax, mListDimensionBehaviors[DIMENSION_HORIZONTAL], wrapContent,
                         &mLeft, &mRight, mX, width,
                         mMinWidth, mMaxDimension[HORIZONTAL],
                         mHorizontalBiasPercent, useHorizontalRatio,
                         mListDimensionBehaviors[DIMENSION_VERTICAL] == DimensionBehaviour::MATCH_CONSTRAINT,
                         inHorizontalChain, inVerticalChain, isInHorizontalBarrier,
                         matchConstraintDefaultWidth, matchConstraintDefaultHeight,
                         mMatchConstraintMinWidth, mMatchConstraintMaxWidth,
                         mMatchConstraintPercentWidth, applyPosition);
    }

    bool applyVerticalConstraints = true;
    // analyzer-run vertical branch omitted (no mVerticalRun).
    if (mVerticalResolution == DIRECT) {
        applyVerticalConstraints = false;
    }
    if (applyVerticalConstraints && !mResolvedVertical) {
        // Vertical Resolution
        wrapContent = (mListDimensionBehaviors[DIMENSION_VERTICAL] == DimensionBehaviour::WRAP_CONTENT)
                      && (dynamic_cast<ConstraintWidgetContainer*>(this) != nullptr);
        if (wrapContent) {
            height = 0;
        }

        SolverVariable* parentMax = mParent != nullptr
                                    ? system->createObjectVariable(&mParent->mBottom) : nullptr;
        SolverVariable* parentMin = mParent != nullptr
                                    ? system->createObjectVariable(&mParent->mTop) : nullptr;

        if (mBaselineDistance > 0 || mVisibility == GONE) {
            if (mBaseline.mTarget != nullptr) {
                system->addEquality(baseline, top, mBaselineDistance,
                                    SolverVariable::STRENGTH_FIXED);
                SolverVariable* baselineTarget = system->createObjectVariable(mBaseline.mTarget);
                int baselineMargin = mBaseline.getMargin();
                system->addEquality(baseline, baselineTarget, baselineMargin,
                                    SolverVariable::STRENGTH_FIXED);
                applyPosition = false;
                if (verticalParentWrapContent) {
                    SolverVariable* end = system->createObjectVariable(&mBottom);
                    system->addGreaterThan(parentMax, end, 0, SolverVariable::STRENGTH_EQUALITY);
                }
            } else if (mVisibility == GONE) {
                system->addEquality(baseline, top, mBaseline.getMargin(),
                                    SolverVariable::STRENGTH_FIXED);
            } else {
                system->addEquality(baseline, top, mBaselineDistance,
                                    SolverVariable::STRENGTH_FIXED);
            }
        }

        applyConstraints(system, false, verticalParentWrapContent,
                         horizontalParentWrapContent, isTerminalWidget[VERTICAL], parentMin,
                         parentMax, mListDimensionBehaviors[DIMENSION_VERTICAL],
                         wrapContent, &mTop, &mBottom, mY, height,
                         mMinHeight, mMaxDimension[VERTICAL], mVerticalBiasPercent, useVerticalRatio,
                         mListDimensionBehaviors[DIMENSION_HORIZONTAL] == DimensionBehaviour::MATCH_CONSTRAINT,
                         inVerticalChain, inHorizontalChain, isInVerticalBarrier,
                         matchConstraintDefaultHeight, matchConstraintDefaultWidth,
                         mMatchConstraintMinHeight, mMatchConstraintMaxHeight,
                         mMatchConstraintPercentHeight, applyPosition);
    }

    if (useRatio) {
        int strength = SolverVariable::STRENGTH_FIXED;
        if (mResolvedDimensionRatioSide == VERTICAL) {
            system->addRatio(bottom, top, right, left, mResolvedDimensionRatio, strength);
        } else {
            system->addRatio(right, left, bottom, top, mResolvedDimensionRatio, strength);
        }
    }

    if (mCenter.isConnected()) {
        system->addCenterPoint(this, mCenter.mTarget->mOwner,
                               (mCircleConstraintAngle + 90.0f) * (3.14159265358979323846f / 180.0f),
                               mCenter.getMargin());
    }

    mResolvedHorizontal = false;
    mResolvedVertical = false;
    if (LinearSystem::getMetrics() != nullptr) {
        LinearSystem::getMetrics()->mEquations = system->getNumEquations();
        LinearSystem::getMetrics()->mVariables = system->getNumVariables();
    }
}

void ConstraintWidget::updateFromSolver(LinearSystem* system, bool optimize) {
    int left = system->getObjectVariableValue(&mLeft);
    int top = system->getObjectVariableValue(&mTop);
    int right = system->getObjectVariableValue(&mRight);
    int bottom = system->getObjectVariableValue(&mBottom);

    // DEFERRED(analyzer): the optimize branch reads positions from the WidgetRuns
    // (mHorizontalRun/mVerticalRun, Stage 3) when the graph solved them directly.
    //   if (optimize && mHorizontalRun != null && mHorizontalRun.start.resolved
    //           && mHorizontalRun.end.resolved) { left = mHorizontalRun.start.value;
    //           right = mHorizontalRun.end.value; }   (and vertical)
    (void)optimize;

    int w = right - left;
    int h = bottom - top;
    if (w < 0 || h < 0
            || left == INT_MIN || left == INT_MAX
            || top == INT_MIN || top == INT_MAX
            || right == INT_MIN || right == INT_MAX
            || bottom == INT_MIN || bottom == INT_MAX) {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
    }
    setFrame(left, top, right, bottom);
}

void ConstraintWidget::applyConstraints(LinearSystem* system, bool isHorizontal,
                                        bool parentWrapContent, bool oppositeParentWrapContent,
                                        bool isTerminal, SolverVariable* parentMin, SolverVariable* parentMax,
                                        DimensionBehaviour dimensionBehaviour, bool wrapContent,
                                        ConstraintAnchor* beginAnchor, ConstraintAnchor* endAnchor,
                                        int beginPosition, int dimension, int minDimension,
                                        int maxDimension, float bias, bool useRatio,
                                        bool oppositeVariable, bool inChain,
                                        bool oppositeInChain, bool inBarrier,
                                        int matchConstraintDefault, int oppositeMatchConstraintDefault,
                                        int matchMinDimension, int matchMaxDimension,
                                        float matchPercentDimension, bool applyPosition) {
    SolverVariable* begin = system->createObjectVariable(beginAnchor);
    SolverVariable* end = system->createObjectVariable(endAnchor);
    SolverVariable* beginTarget = system->createObjectVariable(beginAnchor->getTarget());
    SolverVariable* endTarget = system->createObjectVariable(endAnchor->getTarget());

    if (LinearSystem::getMetrics() != nullptr) {
        LinearSystem::getMetrics()->nonresolvedWidgets++;
    }

    bool isBeginConnected = beginAnchor->isConnected();
    bool isEndConnected = endAnchor->isConnected();
    bool isCenterConnected = mCenter.isConnected();

    bool variableSize = false;

    int numConnections = 0;
    if (isBeginConnected) numConnections++;
    if (isEndConnected) numConnections++;
    if (isCenterConnected) numConnections++;

    if (useRatio) {
        matchConstraintDefault = MATCH_CONSTRAINT_RATIO;
    }
    switch (dimensionBehaviour) {
    case DimensionBehaviour::FIXED:
    case DimensionBehaviour::WRAP_CONTENT:
    case DimensionBehaviour::MATCH_PARENT:
        variableSize = false;
        break;
    case DimensionBehaviour::MATCH_CONSTRAINT:
        variableSize = matchConstraintDefault != MATCH_CONSTRAINT_RATIO_RESOLVED;
        break;
    }

    if (mWidthOverride != -1 && isHorizontal) {
        variableSize = false;
        dimension = mWidthOverride;
        mWidthOverride = -1;
    }
    if (mHeightOverride != -1 && !isHorizontal) {
        variableSize = false;
        dimension = mHeightOverride;
        mHeightOverride = -1;
    }

    if (mVisibility == GONE) {
        dimension = 0;
        variableSize = false;
    }

    // First apply starting direct connections (more solver-friendly)
    if (applyPosition) {
        if (!isBeginConnected && !isEndConnected && !isCenterConnected) {
            system->addEquality(begin, beginPosition);
        } else if (isBeginConnected && !isEndConnected) {
            system->addEquality(begin, beginTarget,
                                beginAnchor->getMargin(), SolverVariable::STRENGTH_FIXED);
        }
    }

    // Then apply the dimension
    if (!variableSize) {
        if (wrapContent) {
            system->addEquality(end, begin, 0, SolverVariable::STRENGTH_HIGH);
            if (minDimension > 0) {
                system->addGreaterThan(end, begin, minDimension, SolverVariable::STRENGTH_FIXED);
            }
            if (maxDimension < INT_MAX) {
                system->addLowerThan(end, begin, maxDimension, SolverVariable::STRENGTH_FIXED);
            }
        } else {
            system->addEquality(end, begin, dimension, SolverVariable::STRENGTH_FIXED);
        }
    } else {
        if (numConnections != 2 && !useRatio
                && ((matchConstraintDefault == MATCH_CONSTRAINT_WRAP)
                    || (matchConstraintDefault == MATCH_CONSTRAINT_SPREAD))) {
            variableSize = false;
            int d = std::max(matchMinDimension, dimension);
            if (matchMaxDimension > 0) {
                d = std::min(matchMaxDimension, d);
            }
            system->addEquality(end, begin, d, SolverVariable::STRENGTH_FIXED);
        } else {
            if (matchMinDimension == WRAP) {
                matchMinDimension = dimension;
            }
            if (matchMaxDimension == WRAP) {
                matchMaxDimension = dimension;
            }
            if (dimension > 0 && matchConstraintDefault != MATCH_CONSTRAINT_WRAP) {
                // USE_WRAP_DIMENSION_FOR_SPREAD branch omitted (flag is false)
                dimension = 0;
            }

            if (matchMinDimension > 0) {
                system->addGreaterThan(end, begin, matchMinDimension, SolverVariable::STRENGTH_FIXED);
                dimension = std::max(dimension, matchMinDimension);
            }
            if (matchMaxDimension > 0) {
                bool applyLimit = true;
                if (parentWrapContent && matchConstraintDefault == MATCH_CONSTRAINT_WRAP) {
                    applyLimit = false;
                }
                if (applyLimit) {
                    system->addLowerThan(end, begin, matchMaxDimension, SolverVariable::STRENGTH_FIXED);
                }
                dimension = std::min(dimension, matchMaxDimension);
            }
            if (matchConstraintDefault == MATCH_CONSTRAINT_WRAP) {
                if (parentWrapContent) {
                    system->addEquality(end, begin, dimension, SolverVariable::STRENGTH_FIXED);
                } else if (inChain) {
                    system->addEquality(end, begin, dimension, SolverVariable::STRENGTH_EQUALITY);
                    system->addLowerThan(end, begin, dimension, SolverVariable::STRENGTH_FIXED);
                } else {
                    system->addEquality(end, begin, dimension, SolverVariable::STRENGTH_EQUALITY);
                    system->addLowerThan(end, begin, dimension, SolverVariable::STRENGTH_FIXED);
                }
            } else if (matchConstraintDefault == MATCH_CONSTRAINT_PERCENT) {
                SolverVariable* percentBegin = nullptr;
                SolverVariable* percentEnd = nullptr;
                if (beginAnchor->getType() == ConstraintAnchor::Type::TOP
                        || beginAnchor->getType() == ConstraintAnchor::Type::BOTTOM) {
                    percentBegin = system->createObjectVariable(mParent->getAnchor(ConstraintAnchor::Type::TOP));
                    percentEnd = system->createObjectVariable(mParent->getAnchor(ConstraintAnchor::Type::BOTTOM));
                } else {
                    percentBegin = system->createObjectVariable(mParent->getAnchor(ConstraintAnchor::Type::LEFT));
                    percentEnd = system->createObjectVariable(mParent->getAnchor(ConstraintAnchor::Type::RIGHT));
                }
                system->addConstraint(system->createRow()->createRowDimensionRatio(
                                          end, begin, percentEnd, percentBegin, matchPercentDimension));
                if (parentWrapContent) {
                    variableSize = false;
                }
            } else {
                isTerminal = true;
            }
        }
    }

    if (!applyPosition || inChain) {
        // only deal with dimension, not positioning
        if (numConnections < 2 && parentWrapContent && isTerminal) {
            system->addGreaterThan(begin, parentMin, 0, SolverVariable::STRENGTH_FIXED);
            bool applyEnd = isHorizontal || (mBaseline.mTarget == nullptr);
            if (!isHorizontal && mBaseline.mTarget != nullptr) {
                ConstraintWidget* target = mBaseline.mTarget->mOwner;
                if (target->mDimensionRatio != 0
                        && target->mListDimensionBehaviors[0] == DimensionBehaviour::MATCH_CONSTRAINT
                        && target->mListDimensionBehaviors[1] == DimensionBehaviour::MATCH_CONSTRAINT) {
                    applyEnd = true;
                } else {
                    applyEnd = false;
                }
            }
            if (applyEnd) {
                system->addGreaterThan(parentMax, end, 0, SolverVariable::STRENGTH_FIXED);
            }
        }
        return;
    }

    // single or centered constraints
    int wrapStrength = SolverVariable::STRENGTH_EQUALITY;

    if (!isBeginConnected && !isEndConnected && !isCenterConnected) {
        // already applied the start position before
    } else if (isBeginConnected && !isEndConnected) {
        ConstraintWidget* beginWidget = beginAnchor->mTarget->mOwner;
        if (parentWrapContent && beginWidget->isBarrier()) {
            wrapStrength = SolverVariable::STRENGTH_FIXED;
        }
    } else if (!isBeginConnected && isEndConnected) {
        system->addEquality(end, endTarget, -endAnchor->getMargin(), SolverVariable::STRENGTH_FIXED);
        if (parentWrapContent) {
            // mOptimizeWrapO fast-path omitted (flag is false)
            system->addGreaterThan(begin, parentMin, 0, SolverVariable::STRENGTH_EQUALITY);
        }
    } else if (isBeginConnected && isEndConnected) {
        bool applyBoundsCheck = true;
        bool applyCentering = false;
        bool applyStrongChecks = false;
        bool applyRangeCheck = false;
        int rangeCheckStrength = SolverVariable::STRENGTH_EQUALITY;
        int boundsCheckStrength = SolverVariable::STRENGTH_HIGHEST;
        int centeringStrength = SolverVariable::STRENGTH_BARRIER;

        if (parentWrapContent) {
            rangeCheckStrength = SolverVariable::STRENGTH_EQUALITY;
        }
        ConstraintWidget* beginWidget = beginAnchor->mTarget->mOwner;
        ConstraintWidget* endWidget = endAnchor->mTarget->mOwner;
        ConstraintWidget* parent = getParent();

        if (variableSize) {
            if (matchConstraintDefault == MATCH_CONSTRAINT_SPREAD) {
                if (matchMaxDimension == 0 && matchMinDimension == 0) {
                    applyStrongChecks = true;
                    rangeCheckStrength = SolverVariable::STRENGTH_FIXED;
                    boundsCheckStrength = SolverVariable::STRENGTH_FIXED;
                    if (beginTarget->isFinalValue && endTarget->isFinalValue) {
                        system->addEquality(begin, beginTarget,
                                            beginAnchor->getMargin(), SolverVariable::STRENGTH_FIXED);
                        system->addEquality(end, endTarget,
                                            -endAnchor->getMargin(), SolverVariable::STRENGTH_FIXED);
                        return;
                    }
                } else {
                    applyCentering = true;
                    rangeCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                    boundsCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                    applyBoundsCheck = true;
                    applyRangeCheck = true;
                }
                if (beginWidget->isBarrier() || endWidget->isBarrier()) {
                    boundsCheckStrength = SolverVariable::STRENGTH_HIGHEST;
                }
            } else if (matchConstraintDefault == MATCH_CONSTRAINT_PERCENT) {
                applyCentering = true;
                rangeCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                boundsCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                applyBoundsCheck = true;
                applyRangeCheck = true;
                if (beginWidget->isBarrier() || endWidget->isBarrier()) {
                    boundsCheckStrength = SolverVariable::STRENGTH_HIGHEST;
                }
            } else if (matchConstraintDefault == MATCH_CONSTRAINT_WRAP) {
                applyCentering = true;
                applyRangeCheck = true;
                rangeCheckStrength = SolverVariable::STRENGTH_FIXED;
            } else if (matchConstraintDefault == MATCH_CONSTRAINT_RATIO) {
                if (mResolvedDimensionRatioSide == UNKNOWN) {
                    applyCentering = true;
                    applyRangeCheck = true;
                    applyStrongChecks = true;
                    rangeCheckStrength = SolverVariable::STRENGTH_FIXED;
                    boundsCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                    if (oppositeInChain) {
                        boundsCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                        centeringStrength = SolverVariable::STRENGTH_HIGHEST;
                        if (parentWrapContent) {
                            centeringStrength = SolverVariable::STRENGTH_EQUALITY;
                        }
                    } else {
                        centeringStrength = SolverVariable::STRENGTH_FIXED;
                    }
                } else {
                    applyCentering = true;
                    applyRangeCheck = true;
                    applyStrongChecks = true;
                    if (useRatio) {
                        bool otherSideInvariable =
                            oppositeMatchConstraintDefault == MATCH_CONSTRAINT_PERCENT
                            || oppositeMatchConstraintDefault == MATCH_CONSTRAINT_WRAP;
                        if (!otherSideInvariable) {
                            rangeCheckStrength = SolverVariable::STRENGTH_FIXED;
                            boundsCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                        }
                    } else {
                        rangeCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                        if (matchMaxDimension > 0) {
                            boundsCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                        } else if (matchMaxDimension == 0 && matchMinDimension == 0) {
                            if (!oppositeInChain) {
                                boundsCheckStrength = SolverVariable::STRENGTH_FIXED;
                            } else {
                                if (beginWidget != parent && endWidget != parent) {
                                    rangeCheckStrength = SolverVariable::STRENGTH_HIGHEST;
                                } else {
                                    rangeCheckStrength = SolverVariable::STRENGTH_EQUALITY;
                                }
                                boundsCheckStrength = SolverVariable::STRENGTH_HIGHEST;
                            }
                        }
                    }
                }
            }
        } else {
            applyCentering = true;
            applyRangeCheck = true;
            if (beginTarget->isFinalValue && endTarget->isFinalValue) {
                system->addCentering(begin, beginTarget, beginAnchor->getMargin(),
                                     bias, endTarget, end, endAnchor->getMargin(), SolverVariable::STRENGTH_FIXED);
                if (parentWrapContent && isTerminal) {
                    int margin = 0;
                    if (endAnchor->mTarget != nullptr) {
                        margin = endAnchor->getMargin();
                    }
                    if (endTarget != parentMax) {
                        system->addGreaterThan(parentMax, end, margin, wrapStrength);
                    }
                }
                return;
            }
        }

        if (applyRangeCheck && beginTarget == endTarget && beginWidget != parent) {
            applyRangeCheck = false;
            applyBoundsCheck = false;
        }

        if (applyCentering) {
            if (!variableSize && !oppositeVariable && !oppositeInChain
                    && beginTarget == parentMin && endTarget == parentMax) {
                centeringStrength = SolverVariable::STRENGTH_FIXED;
                rangeCheckStrength = SolverVariable::STRENGTH_FIXED;
                applyBoundsCheck = false;
                parentWrapContent = false;
            }
            system->addCentering(begin, beginTarget, beginAnchor->getMargin(),
                                 bias, endTarget, end, endAnchor->getMargin(), centeringStrength);
        }

        if (mVisibility == GONE && !endAnchor->hasDependents()) {
            return;
        }

        if (applyRangeCheck) {
            if (parentWrapContent && beginTarget != endTarget && !variableSize) {
                if (beginWidget->isBarrier() || endWidget->isBarrier()) {
                    rangeCheckStrength = SolverVariable::STRENGTH_BARRIER;
                }
            }
            system->addGreaterThan(begin, beginTarget,
                                   beginAnchor->getMargin(), rangeCheckStrength);
            system->addLowerThan(end, endTarget, -endAnchor->getMargin(), rangeCheckStrength);
        }

        if (parentWrapContent && inBarrier
                && !(beginWidget->isBarrier() || endWidget->isBarrier())
                && !(endWidget == parent)) {
            boundsCheckStrength = SolverVariable::STRENGTH_BARRIER;
            rangeCheckStrength = SolverVariable::STRENGTH_BARRIER;
            applyBoundsCheck = true;
        }

        if (applyBoundsCheck) {
            if (applyStrongChecks && (!oppositeInChain || oppositeParentWrapContent)) {
                int strength = boundsCheckStrength;
                if (beginWidget == parent || endWidget == parent) {
                    strength = SolverVariable::STRENGTH_BARRIER;
                }
                if (dynamic_cast<clcore::Guideline*>(beginWidget) != nullptr
                        || dynamic_cast<clcore::Guideline*>(endWidget) != nullptr) {
                    strength = SolverVariable::STRENGTH_EQUALITY;
                }
                if (beginWidget->isBarrier() || endWidget->isBarrier()) {
                    strength = SolverVariable::STRENGTH_EQUALITY;
                }
                if (oppositeInChain) {
                    strength = SolverVariable::STRENGTH_EQUALITY;
                }
                boundsCheckStrength = std::max(strength, boundsCheckStrength);
            }

            if (parentWrapContent) {
                boundsCheckStrength = std::min(rangeCheckStrength, boundsCheckStrength);
                if (useRatio && !oppositeInChain
                        && (beginWidget == parent || endWidget == parent)) {
                    boundsCheckStrength = SolverVariable::STRENGTH_HIGHEST;
                }
            }
            system->addEquality(begin, beginTarget,
                                beginAnchor->getMargin(), boundsCheckStrength);
            system->addEquality(end, endTarget, -endAnchor->getMargin(), boundsCheckStrength);
        }

        if (parentWrapContent) {
            int margin = 0;
            if (parentMin == beginTarget) {
                margin = beginAnchor->getMargin();
            }
            if (beginTarget != parentMin) {
                system->addGreaterThan(begin, parentMin, margin, wrapStrength);
            }
        }

        if (parentWrapContent && variableSize && minDimension == 0 && matchMinDimension == 0) {
            if (variableSize && matchConstraintDefault == MATCH_CONSTRAINT_RATIO) {
                system->addGreaterThan(end, begin, 0, SolverVariable::STRENGTH_FIXED);
            } else {
                system->addGreaterThan(end, begin, 0, wrapStrength);
            }
        }
    }

    if (parentWrapContent && isTerminal) {
        int margin = 0;
        if (endAnchor->mTarget != nullptr) {
            margin = endAnchor->getMargin();
        }
        if (endTarget != parentMax) {
            // mOptimizeWrapO fast-path omitted (flag is false)
            system->addGreaterThan(parentMax, end, margin, wrapStrength);
        }
    }
}

void ConstraintWidget::copy(ConstraintWidget* /*src*/,
                            std::unordered_map<ConstraintWidget*, ConstraintWidget*>& /*map*/) {
    // DEFERRED: deep-copies geometry/dimension/connection state for ConstraintSet clone
    // (Stage 7). Restore when ConstraintSet lands.
    // TODO(constraintset): see androidx.constraintlayout.core.widgets.ConstraintWidget.copy
}

void ConstraintWidget::reset() {
    mLeft.reset();
    mTop.reset();
    mRight.reset();
    mBottom.reset();
    mBaseline.reset();
    mCenterX.reset();
    mCenterY.reset();
    mCenter.reset();
    mParent = nullptr;
    // NOTE(SKELETON): the Java original also resets many more fields here (chain state,
    // run/analyzer caches, etc.) — restored as those subsystems are ported.
    mOffsetX = 0;
    mOffsetY = 0;
    mBaselineDistance = 0;
    mVisibility = VISIBLE;
    mMinWidth = 0;
    mMinHeight = 0;
    mResolvedHorizontal = false;
    mResolvedVertical = false;
    mType.clear();
    mListDimensionBehaviors[DIMENSION_HORIZONTAL] = DimensionBehaviour::FIXED;
    mListDimensionBehaviors[DIMENSION_VERTICAL] = DimensionBehaviour::FIXED;
    // chain state (matches Java ConstraintWidget.reset)
    mWeight[DIMENSION_HORIZONTAL] = UNKNOWN;
    mWeight[DIMENSION_VERTICAL] = UNKNOWN;
    mResolvedMatchConstraintDefault[DIMENSION_HORIZONTAL] = 0;
    mResolvedMatchConstraintDefault[DIMENSION_VERTICAL] = 0;
    mMatchConstraintMaxWidth  = INT_MAX;
    mMatchConstraintMaxHeight = INT_MAX;
    mMatchConstraintMinWidth  = 0;
    mMatchConstraintMinHeight = 0;
    mDimensionRatio = 0;
    mHorizontalChainStyle  = CHAIN_SPREAD;
    mVerticalChainStyle    = CHAIN_SPREAD;
    mHorizontalBiasPercent = DEFAULT_BIAS;
    mVerticalBiasPercent   = DEFAULT_BIAS;
    mIsInBarrier[DIMENSION_HORIZONTAL] = false;
    mIsInBarrier[DIMENSION_VERTICAL]   = false;
    mMatchConstraintPercentWidth  = 1;
    mMatchConstraintPercentHeight = 1;
    mMaxDimension[DIMENSION_HORIZONTAL] = INT_MAX;
    mMaxDimension[DIMENSION_VERTICAL]   = INT_MAX;
    mResolvedHasRatio          = false;
    mResolvedDimensionRatioSide = UNKNOWN;
    mResolvedDimensionRatio    = 1.0f;
    isTerminalWidget[DIMENSION_HORIZONTAL] = true;
    isTerminalWidget[DIMENSION_VERTICAL]   = true;
}

void ConstraintWidget::addAnchors() {
    mAnchors.push_back(&mLeft);
    mAnchors.push_back(&mTop);
    mAnchors.push_back(&mRight);
    mAnchors.push_back(&mBottom);
    mAnchors.push_back(&mCenterX);
    mAnchors.push_back(&mCenterY);
    mAnchors.push_back(&mCenter);
    mAnchors.push_back(&mBaseline);
}

} // namespace cdroid
