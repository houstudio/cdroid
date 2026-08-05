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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Optimizer.
 */
#include <widgetEx/constraintlayout/core/widgets/optimizer.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidgetcontainer.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>

namespace cdroid {

// Verbatim port of androidx.constraintlayout.core.widgets.Optimizer.checkMatchParent.
// A MATCH_PARENT widget is pinned to the container edges ([leftMargin, containerW-rightMargin])
// via solver equalities and marked DIRECT, so ConstraintWidget::addToSolver skips applyConstraints
// (the mHorizontalResolution==DIRECT guard at constraintwidget.cc) and the solver resolves the
// widget to the full container dimension regardless of any explicit anchors on that axis.
// Called per-child from ConstraintWidgetContainer::layout() before addToSolver.
void Optimizer::checkMatchParent(ConstraintWidgetContainer* container, LinearSystem* system,
                                 ConstraintWidget* widget) {
    widget->mHorizontalResolution = ConstraintWidget::UNKNOWN;
    widget->mVerticalResolution   = ConstraintWidget::UNKNOWN;

    if (container->mListDimensionBehaviors[ConstraintWidget::HORIZONTAL]
            != ConstraintWidget::DimensionBehaviour::WRAP_CONTENT
        && widget->mListDimensionBehaviors[ConstraintWidget::HORIZONTAL]
            == ConstraintWidget::DimensionBehaviour::MATCH_PARENT) {
        int left  = widget->mLeft.mMargin;
        int right = container->getWidth() - widget->mRight.mMargin;
        widget->mLeft.mSolverVariable  = system->createObjectVariable(&widget->mLeft);
        widget->mRight.mSolverVariable = system->createObjectVariable(&widget->mRight);
        system->addEquality(widget->mLeft.mSolverVariable, left);
        system->addEquality(widget->mRight.mSolverVariable, right);
        widget->mHorizontalResolution = ConstraintWidget::DIRECT;
        widget->setHorizontalDimension(left, right);
    }

    if (container->mListDimensionBehaviors[ConstraintWidget::VERTICAL]
            != ConstraintWidget::DimensionBehaviour::WRAP_CONTENT
        && widget->mListDimensionBehaviors[ConstraintWidget::VERTICAL]
            == ConstraintWidget::DimensionBehaviour::MATCH_PARENT) {
        int top    = widget->mTop.mMargin;
        int bottom = container->getHeight() - widget->mBottom.mMargin;
        widget->mTop.mSolverVariable    = system->createObjectVariable(&widget->mTop);
        widget->mBottom.mSolverVariable = system->createObjectVariable(&widget->mBottom);
        system->addEquality(widget->mTop.mSolverVariable, top);
        system->addEquality(widget->mBottom.mSolverVariable, bottom);
        if (widget->mBaselineDistance > 0 || widget->getVisibility() == ConstraintWidget::GONE) {
            widget->mBaseline.mSolverVariable = system->createObjectVariable(&widget->mBaseline);
            system->addEquality(widget->mBaseline.mSolverVariable, top + widget->mBaselineDistance);
        }
        widget->mVerticalResolution = ConstraintWidget::DIRECT;
        widget->setVerticalDimension(top, bottom);
    }
}

} // namespace cdroid
