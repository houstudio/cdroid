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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.ConstraintAnchor.
 */
#include <widgetEx/constraintlayout/core/widgets/constraintanchor.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/guideline.h>
#include <widgetEx/constraintlayout/core/cache.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

namespace cdroid {

ConstraintAnchor::ConstraintAnchor(ConstraintWidget* owner, Type type)
    : mOwner(owner)
    , mType(type) {
}

ConstraintAnchor::~ConstraintAnchor() {
    delete mSolverVariable;
    mSolverVariable = nullptr;
    delete mDependents;
    mDependents = nullptr;
}

SolverVariable* ConstraintAnchor::getSolverVariable() const {
    return mSolverVariable;
}

void ConstraintAnchor::resetSolverVariable(Cache* /*cache*/) {
    if (mSolverVariable == nullptr) {
        // Java: new SolverVariable(SolverVariable.Type.UNRESTRICTED, null)
        mSolverVariable = new SolverVariable(SolverVariable::Type::UNRESTRICTED, std::string());
    } else {
        mSolverVariable->reset();
    }
}

ConstraintWidget* ConstraintAnchor::getOwner() const {
    return mOwner;
}

ConstraintAnchor::Type ConstraintAnchor::getType() const {
    return mType;
}

int ConstraintAnchor::getMargin() const {
    if (mOwner->getVisibility() == ConstraintWidget::GONE) {
        return 0;
    }
    if (mGoneMargin != UNSET_GONE_MARGIN && mTarget != nullptr
            && mTarget->mOwner->getVisibility() == ConstraintWidget::GONE) {
        return mGoneMargin;
    }
    return mMargin;
}

ConstraintAnchor* ConstraintAnchor::getTarget() const {
    return mTarget;
}

void ConstraintAnchor::reset() {
    if (mTarget != nullptr && mTarget->mDependents != nullptr) {
        mTarget->mDependents->erase(this);
        if (mTarget->mDependents->empty()) {
            delete mTarget->mDependents;
            mTarget->mDependents = nullptr;
        }
    }
    if (mDependents != nullptr) {
        delete mDependents;
        mDependents = nullptr;
    }
    mTarget = nullptr;
    mMargin = 0;
    mGoneMargin = UNSET_GONE_MARGIN;
    mHasFinalValue = false;
    mFinalValue = 0;
}

bool ConstraintAnchor::connect(ConstraintAnchor* toAnchor, int margin, int goneMargin,
                               bool forceConnection) {
    if (toAnchor == nullptr) {
        reset();
        return true;
    }
    if (!forceConnection && !isValidConnection(toAnchor)) {
        return false;
    }
    mTarget = toAnchor;
    if (mTarget->mDependents == nullptr) {
        mTarget->mDependents = new std::unordered_set<ConstraintAnchor*>();
    }
    if (mTarget->mDependents != nullptr) {
        mTarget->mDependents->insert(this);
    }
    mMargin = margin;
    mGoneMargin = goneMargin;
    return true;
}

bool ConstraintAnchor::connect(ConstraintAnchor* toAnchor, int margin) {
    return connect(toAnchor, margin, UNSET_GONE_MARGIN, false);
}

bool ConstraintAnchor::connect(ConstraintAnchor& toAnchor, int margin) {
    return connect(&toAnchor, margin);
}

bool ConstraintAnchor::isConnected() const {
    return mTarget != nullptr;
}

bool ConstraintAnchor::isValidConnection(ConstraintAnchor* anchor) const {
    if (anchor == nullptr) {
        return false;
    }
    Type target = anchor->getType();
    if (target == mType) {
        if (mType == Type::BASELINE
                && (!anchor->getOwner()->hasBaseline() || !getOwner()->hasBaseline())) {
            return false;
        }
        return true;
    }
    switch (mType) {
    case Type::CENTER: {
        // allow everything but baseline and center_x/center_y
        return target != Type::BASELINE && target != Type::CENTER_X
               && target != Type::CENTER_Y;
    }
    case Type::LEFT:
    case Type::RIGHT: {
        bool isCompatible = target == Type::LEFT || target == Type::RIGHT;
        if (dynamic_cast<clcore::Guideline*>(anchor->getOwner()) != nullptr) {
            isCompatible = isCompatible || target == Type::CENTER_X;
        }
        return isCompatible;
    }
    case Type::TOP:
    case Type::BOTTOM: {
        bool isCompatible = target == Type::TOP || target == Type::BOTTOM;
        if (dynamic_cast<clcore::Guideline*>(anchor->getOwner()) != nullptr) {
            isCompatible = isCompatible || target == Type::CENTER_Y;
        }
        return isCompatible;
    }
    case Type::BASELINE: {
        if (target == Type::LEFT || target == Type::RIGHT) {
            return false;
        }
        return true;
    }
    case Type::CENTER_X:
    case Type::CENTER_Y:
    case Type::NONE:
        return false;
    }
    return false; // unreachable (Java throws AssertionError(mType.name()))
}

bool ConstraintAnchor::isSideAnchor() const {
    switch (mType) {
    case Type::LEFT:
    case Type::RIGHT:
    case Type::TOP:
    case Type::BOTTOM:
        return true;
    case Type::BASELINE:
    case Type::CENTER:
    case Type::CENTER_X:
    case Type::CENTER_Y:
    case Type::NONE:
        return false;
    }
    return false; // unreachable
}

bool ConstraintAnchor::isSimilarDimensionConnection(const ConstraintAnchor* anchor) const {
    Type target = anchor->getType();
    if (target == mType) {
        return true;
    }
    switch (mType) {
    case Type::CENTER: {
        return target != Type::BASELINE;
    }
    case Type::LEFT:
    case Type::RIGHT:
    case Type::CENTER_X: {
        return target == Type::LEFT || target == Type::RIGHT || target == Type::CENTER_X;
    }
    case Type::TOP:
    case Type::BOTTOM:
    case Type::CENTER_Y:
    case Type::BASELINE: {
        return target == Type::TOP || target == Type::BOTTOM
               || target == Type::CENTER_Y || target == Type::BASELINE;
    }
    case Type::NONE:
        return false;
    }
    return false; // unreachable
}

void ConstraintAnchor::setMargin(int margin) {
    if (isConnected()) {
        mMargin = margin;
    }
}

void ConstraintAnchor::setGoneMargin(int margin) {
    if (isConnected()) {
        mGoneMargin = margin;
    }
}

bool ConstraintAnchor::isVerticalAnchor() const {
    switch (mType) {
    case Type::LEFT:
    case Type::RIGHT:
    case Type::CENTER:
    case Type::CENTER_X:
        return false;
    case Type::CENTER_Y:
    case Type::TOP:
    case Type::BOTTOM:
    case Type::BASELINE:
    case Type::NONE:
        return true;
    }
    return false; // unreachable
}

std::string ConstraintAnchor::toString() const {
    return mOwner->getDebugName() + ":" /* + mType */; // mType name omitted (no Java enum.name() equiv here)
}

bool ConstraintAnchor::isConnectionAllowed(ConstraintWidget* target, ConstraintAnchor* anchor) const {
    if (ALLOW_BINARY) {
        if (anchor != nullptr && anchor->getTarget() == this) {
            return true;
        }
    }
    return isConnectionAllowed(target);
}

bool ConstraintAnchor::isConnectionAllowed(ConstraintWidget* target) const {
    std::unordered_set<ConstraintWidget*> checked;
    if (isConnectionToMe(target, checked)) {
        return false;
    }
    ConstraintWidget* parent = getOwner()->getParent();
    if (parent == target) { // allow connections to parent
        return true;
    }
    if (target->getParent() == parent) { // allow if we share the same parent
        return true;
    }
    return false;
}

bool ConstraintAnchor::isConnectionToMe(ConstraintWidget* target,
                                        std::unordered_set<ConstraintWidget*>& checked) const {
    if (checked.find(target) != checked.end()) {
        return false;
    }
    checked.insert(target);

    if (target == getOwner()) {
        return true;
    }
    std::vector<ConstraintAnchor*> targetAnchors = target->getAnchors();
    for (size_t i = 0, targetAnchorsSize = targetAnchors.size(); i < targetAnchorsSize; i++) {
        ConstraintAnchor* anchor = targetAnchors[i];
        if (anchor->isSimilarDimensionConnection(this) && anchor->isConnected()) {
            if (isConnectionToMe(anchor->getTarget()->getOwner(), checked)) {
                return true;
            }
        }
    }
    return false;
}

ConstraintAnchor* ConstraintAnchor::getOpposite() const {
    switch (mType) {
    case Type::LEFT: {
        return &mOwner->mRight;
    }
    case Type::RIGHT: {
        return &mOwner->mLeft;
    }
    case Type::TOP: {
        return &mOwner->mBottom;
    }
    case Type::BOTTOM: {
        return &mOwner->mTop;
    }
    case Type::BASELINE:
    case Type::CENTER:
    case Type::CENTER_X:
    case Type::CENTER_Y:
    case Type::NONE:
        return nullptr;
    }
    return nullptr; // unreachable
}

std::unordered_set<ConstraintAnchor*>* ConstraintAnchor::getDependents() const {
    return mDependents;
}

bool ConstraintAnchor::hasDependents() const {
    if (mDependents == nullptr) {
        return false;
    }
    return !mDependents->empty();
}

bool ConstraintAnchor::hasCenteredDependents() const {
    if (mDependents == nullptr) {
        return false;
    }
    for (ConstraintAnchor* anchor : *mDependents) {
        ConstraintAnchor* opposite = anchor->getOpposite();
        if (opposite != nullptr && opposite->isConnected()) {
            return true;
        }
    }
    return false;
}

void ConstraintAnchor::findDependents(int /*orientation*/, std::vector<WidgetGroup*>& /*list*/,
                                      WidgetGroup* /*group*/) {
    // DEFERRED(analyzer): depends on Grouping::findDependents + WidgetGroup (Stage 3).
    // Java body:
    //   if (mDependents != null) {
    //       for (ConstraintAnchor anchor : mDependents) {
    //           Grouping::findDependents(anchor->mOwner, orientation, list, group);
    //       }
    //   }
    (void)mDependents;
}

void ConstraintAnchor::setFinalValue(int finalValue) {
    mFinalValue = finalValue;
    mHasFinalValue = true;
}

int ConstraintAnchor::getFinalValue() const {
    if (!mHasFinalValue) {
        return 0;
    }
    return mFinalValue;
}

void ConstraintAnchor::resetFinalResolution() {
    mHasFinalValue = false;
    mFinalValue = 0;
}

bool ConstraintAnchor::hasFinalValue() const {
    return mHasFinalValue;
}

void ConstraintAnchor::copyFrom(ConstraintAnchor* source,
                                std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) {
    if (mTarget != nullptr) {
        if (mTarget->mDependents != nullptr) {
            mTarget->mDependents->erase(this);
        }
    }
    if (source->mTarget != nullptr) {
        Type type = source->mTarget->getType();
        ConstraintWidget* owner = map[source->mTarget->mOwner];
        mTarget = owner->getAnchor(type);
    } else {
        mTarget = nullptr;
    }
    if (mTarget != nullptr) {
        if (mTarget->mDependents == nullptr) {
            mTarget->mDependents = new std::unordered_set<ConstraintAnchor*>();
        }
        mTarget->mDependents->insert(this);
    }
    mMargin = source->mMargin;
    mGoneMargin = source->mGoneMargin;
}

} // namespace cdroid
