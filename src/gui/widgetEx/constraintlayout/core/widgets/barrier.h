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
