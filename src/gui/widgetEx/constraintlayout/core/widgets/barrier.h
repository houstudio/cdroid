/*
 * Copyright (C) 2017 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Barrier.
 *
 * A Barrier references multiple widgets and resolves to the most extreme side among them
 * (a LEFT barrier sits at min(referenced.left), a RIGHT barrier at max(referenced.right), etc.).
 * Other widgets can then constrain to the barrier. Lives in clcore:: to avoid clashing with the
 * widget-layer cdroid::Barrier View subclass.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_BARRIER_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_BARRIER_H

#include <widgetEx/constraintlayout/core/widgets/helperwidget.h>

namespace cdroid::clcore {

class Barrier : public HelperWidget {
public:
    // Barrier direction constants (also index into mListAnchors, whose order is
    // {LEFT, RIGHT, TOP, BOTTOM, BASELINE, CENTER}).
    static const int LEFT   = 0;
    static const int RIGHT  = 1;
    static const int TOP    = 2;
    static const int BOTTOM = 3;

    Barrier();
    explicit Barrier(const std::string& debugName);

    bool allowedInBarrier() const override;  // true
    bool isBarrier() const override;          // true
    bool isResolvedHorizontally() const override;
    bool isResolvedVertically() const override;

    int  getBarrierType() const;
    void setBarrierType(int barrierType);
    bool getAllowsGoneWidget() const;
    void setAllowsGoneWidget(bool allowsGoneWidget);
    int  getMargin() const;
    void setMargin(int margin);
    int  getOrientation() const; // HORIZONTAL / VERTICAL / UNKNOWN

    // Solver lifecycle.
    void addToSolver(LinearSystem* system, bool optimize) override;
    void copy(ConstraintWidget* src,
              std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map) override;

    // Pre-resolution fast path: if every referenced widget is already resolved, pin this barrier
    // to the extreme edge and return true (so addToSolver emits only equalities).
    bool allSolved();

    std::string getType() const override; // "Barrier"

private:
    void markWidgets();

    int  mBarrierType = LEFT;
    bool mAllowsGoneWidget = true;
    int  mMargin = 0;
    bool mResolved = false;
};

} // namespace cdroid::clcore

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_BARRIER_H
