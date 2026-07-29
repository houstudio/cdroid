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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.ChainHead.
 *
 * Represents a chain by its main elements (first/last/head + match-constraint bookkeeping),
 * computed by walking the anchor chain. Data fields are public: Chain.applyChainConstraints
 * (same package) reads them directly, mirroring Java's package-private access.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CHAIN_HEAD_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CHAIN_HEAD_H

#include <vector>

#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

class ChainHead {
  public:
    ChainHead(ConstraintWidget* first, int orientation, bool isRtl);

    ConstraintWidget* getFirst() const;
    ConstraintWidget* getFirstVisibleWidget() const;
    ConstraintWidget* getLast() const;
    ConstraintWidget* getLastVisibleWidget() const;
    ConstraintWidget* getHead() const;
    ConstraintWidget* getFirstMatchConstraintWidget() const;
    ConstraintWidget* getLastMatchConstraintWidget() const;
    float getTotalWeight() const;

    // Lazily compute the chain properties (idempotent).
    void define();

    // --- data fields (read by Chain.applyChainConstraints) ---
    ConstraintWidget* mFirst = nullptr;
    ConstraintWidget* mFirstVisibleWidget = nullptr;
    ConstraintWidget* mLast = nullptr;
    ConstraintWidget* mLastVisibleWidget = nullptr;
    ConstraintWidget* mHead = nullptr;
    ConstraintWidget* mFirstMatchConstraintWidget = nullptr;
    ConstraintWidget* mLastMatchConstraintWidget = nullptr;
    std::vector<ConstraintWidget*> mWeightedMatchConstraintsWidgets;
    int   mWidgetsCount = 0;
    int   mWidgetsMatchCount = 0;
    float mTotalWeight = 0.0f;
    int   mVisibleWidgets = 0;
    int   mTotalSize = 0;
    int   mTotalMargins = 0;
    bool  mOptimizable = false;
    bool  mHasUndefinedWeights = false;
    bool  mHasDefinedWeights = false;
    bool  mHasComplexMatchWeights = false;
    bool  mHasRatio = false;

  private:
    static bool isMatchConstraintEqualityCandidate(ConstraintWidget* widget, int orientation);
    void defineChainProperties();

    int  mOrientation = 0;
    bool mIsRtl = false;
    bool mDefined = false;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CHAIN_HEAD_H
