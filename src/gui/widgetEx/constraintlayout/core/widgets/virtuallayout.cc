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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.VirtualLayout.
 */
#include <widgetEx/constraintlayout/core/widgets/virtuallayout.h>

#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <widgetEx/constraintlayout/core/widgets/guideline.h>

namespace cdroid {

VirtualLayout::VirtualLayout() {
    mMeasure = BasicMeasure::Measure();
}

VirtualLayout::~VirtualLayout() = default;

// --- padding ---
void VirtualLayout::setPadding(int value) {
    mPaddingLeft = value;
    mPaddingTop = value;
    mPaddingRight = value;
    mPaddingBottom = value;
    mPaddingStart = value;
    mPaddingEnd = value;
}
void VirtualLayout::setPaddingStart(int value) {
    mPaddingStart = value;
    mResolvedPaddingLeft = value;
    mResolvedPaddingRight = value;
}
void VirtualLayout::setPaddingEnd(int value) {
    mPaddingEnd = value;
}
void VirtualLayout::setPaddingLeft(int value) {
    mPaddingLeft = value;
    mResolvedPaddingLeft = value;
}
void VirtualLayout::setPaddingRight(int value) {
    mPaddingRight = value;
    mResolvedPaddingRight = value;
}
void VirtualLayout::setPaddingTop(int value) {
    mPaddingTop = value;
}
void VirtualLayout::setPaddingBottom(int value) {
    mPaddingBottom = value;
}

void VirtualLayout::applyRtl(bool isRtl) {
    if (mPaddingStart > 0 || mPaddingEnd > 0) {
        if (isRtl) {
            mResolvedPaddingLeft = mPaddingEnd;
            mResolvedPaddingRight = mPaddingStart;
        } else {
            mResolvedPaddingLeft = mPaddingStart;
            mResolvedPaddingRight = mPaddingEnd;
        }
    }
}

int VirtualLayout::getPaddingTop() const {
    return mPaddingTop;
}
int VirtualLayout::getPaddingBottom() const {
    return mPaddingBottom;
}
int VirtualLayout::getPaddingLeft() const {
    return mResolvedPaddingLeft;
}
int VirtualLayout::getPaddingRight() const {
    return mResolvedPaddingRight;
}

// --- solver callback ---
void VirtualLayout::needsCallbackFromSolver(bool value) {
    mNeedsCallFromSolver = value;
}
bool VirtualLayout::needSolverPass() const {
    return mNeedsCallFromSolver;
}

// --- measure ---
void VirtualLayout::measure(int /*widthMode*/, int /*widthSize*/, int /*heightMode*/, int /*heightSize*/) {
    // base: nothing (Flow overrides)
}

void VirtualLayout::updateConstraints(ConstraintWidgetContainer* /*container*/) {
    captureWidgets();
}

void VirtualLayout::captureWidgets() {
    for (ConstraintWidget* widget : mWidgets) {
        if (widget != nullptr) {
            widget->setInVirtualLayout(true);
        }
    }
}

int VirtualLayout::getMeasuredWidth() const {
    return mMeasuredWidth;
}
int VirtualLayout::getMeasuredHeight() const {
    return mMeasuredHeight;
}
void VirtualLayout::setMeasure(int width, int height) {
    mMeasuredWidth = width;
    mMeasuredHeight = height;
}

bool VirtualLayout::measureChildren() {
    BasicMeasure::Measurer* measurer = nullptr;
    if (mParent != nullptr) {
        auto* container = dynamic_cast<ConstraintWidgetContainer*>(mParent);
        if (container != nullptr) {
            measurer = container->getMeasurer();
        }
    }
    if (measurer == nullptr) {
        return false;
    }
    for (ConstraintWidget* widget : mWidgets) {
        if (widget == nullptr) continue;
        if (dynamic_cast<clcore::Guideline*>(widget) != nullptr) continue;

        ConstraintWidget::DimensionBehaviour widthBehavior  = widget->getDimensionBehaviour(HORIZONTAL);
        ConstraintWidget::DimensionBehaviour heightBehavior = widget->getDimensionBehaviour(VERTICAL);

        bool skip = widthBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && widget->mMatchConstraintDefaultWidth != MATCH_CONSTRAINT_WRAP
                    && heightBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT
                    && widget->mMatchConstraintDefaultHeight != MATCH_CONSTRAINT_WRAP;
        if (skip) {
            continue; // dimension fully computed by the solver
        }
        if (widthBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT) {
            widthBehavior = ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
        }
        if (heightBehavior == ConstraintWidget::DimensionBehaviour::MATCH_CONSTRAINT) {
            heightBehavior = ConstraintWidget::DimensionBehaviour::WRAP_CONTENT;
        }
        mMeasure.horizontalBehavior = widthBehavior;
        mMeasure.verticalBehavior = heightBehavior;
        mMeasure.horizontalDimension = widget->getWidth();
        mMeasure.verticalDimension = widget->getHeight();
        measurer->measure(widget, &mMeasure);
        widget->setWidth(mMeasure.measuredWidth);
        widget->setHeight(mMeasure.measuredHeight);
        widget->setBaselineDistance(mMeasure.measuredBaseline);
    }
    return true;
}

void VirtualLayout::measure(ConstraintWidget* widget,
                            ConstraintWidget::DimensionBehaviour horizontalBehavior, int horizontalDimension,
                            ConstraintWidget::DimensionBehaviour verticalBehavior, int verticalDimension) {
    while (mMeasurer == nullptr && getParent() != nullptr) {
        auto* parent = dynamic_cast<ConstraintWidgetContainer*>(getParent());
        if (parent != nullptr) {
            mMeasurer = parent->getMeasurer();
        }
        break;
    }
    if (mMeasurer == nullptr) return;
    mMeasure.horizontalBehavior = horizontalBehavior;
    mMeasure.verticalBehavior = verticalBehavior;
    mMeasure.horizontalDimension = horizontalDimension;
    mMeasure.verticalDimension = verticalDimension;
    mMeasurer->measure(widget, &mMeasure);
    widget->setWidth(mMeasure.measuredWidth);
    widget->setHeight(mMeasure.measuredHeight);
    widget->setHasBaseline(mMeasure.measuredHasBaseline);
    widget->setBaselineDistance(mMeasure.measuredBaseline);
}

std::string VirtualLayout::getType() const {
    return "VirtualLayout";
}

} // namespace cdroid
