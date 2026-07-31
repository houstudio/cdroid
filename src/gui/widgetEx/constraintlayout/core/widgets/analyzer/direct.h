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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1300  USA
 *********************************************************************************/

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.Direct.
 *
 * The chain fast-path: solveChain() resolves a chain positionally via setFinalValue, bypassing the
 * Cassowary solver; horizontalSolvingPass()/verticalSolvingPass() then resolve the chain members'
 * anchored dependents. Invoked by Chain::applyChainConstraints when USE_CHAIN_OPTIMIZATION is on;
 * on false it falls back to the pure-solver path. Only the chain-path methods are ported — the
 * top-level solvingPass()/solveBarrier() (container-wide direct resolution) are out of scope.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DIRECT_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DIRECT_H

#include <widgetEx/constraintlayout/core/widgets/analyzer/basicmeasure.h>

namespace cdroid {

class ConstraintWidget;
class ConstraintWidgetContainer;
class ChainHead;
class LinearSystem;

class Direct {
  public:
    // Resolve a whole chain positionally; returns true if fully solved (caller returns), false to
    // fall back to the Cassowary solver path. Java Direct.solveChain (869).
    static bool solveChain(ConstraintWidgetContainer* container, LinearSystem* system,
                           int orientation, int offset, ChainHead* chainHead,
                           bool isChainSpread, bool isChainSpreadInside, bool isChainPacked);

  private:
    static bool canMeasure(int level, ConstraintWidget* layout);
    static void horizontalSolvingPass(int level, ConstraintWidget* layout,
                                      BasicMeasure::Measurer* measurer, bool isRtl);
    static void verticalSolvingPass(int level, ConstraintWidget* layout,
                                    BasicMeasure::Measurer* measurer);
    static void solveHorizontalCenterConstraints(int level, BasicMeasure::Measurer* measurer,
                                                 ConstraintWidget* widget, bool isRtl);
    static void solveVerticalCenterConstraints(int level, BasicMeasure::Measurer* measurer,
                                               ConstraintWidget* widget);
    static void solveHorizontalMatchConstraint(int level, ConstraintWidget* layout,
                                               BasicMeasure::Measurer* measurer,
                                               ConstraintWidget* widget, bool isRtl);
    static void solveVerticalMatchConstraint(int level, ConstraintWidget* layout,
                                             BasicMeasure::Measurer* measurer,
                                             ConstraintWidget* widget);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DIRECT_H
