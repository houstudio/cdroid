/*
 * Copyright (C) 2018 The Android Open Source Project
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
