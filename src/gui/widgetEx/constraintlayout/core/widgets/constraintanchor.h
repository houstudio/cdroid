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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.ConstraintAnchor.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_ANCHOR_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_ANCHOR_H

#include <climits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cdroid {

class ConstraintWidget;
class Cache;
class SolverVariable;
// Analyzer types (Stage 3) — referenced only by findDependents(); forward-declared here.
class WidgetGroup;
class Grouping;

/**
 * Model a constraint relation. Widgets contains anchors, and a constraint relation between
 * two widgets is made by connecting one anchor to another. The anchor will contains a pointer
 * to the target anchor if it is connected.
 *
 * NOTE: ConstraintWidget is only forward-declared here (the anchor holds its owner by pointer);
 * the .cc includes constraintwidget.h to access the owner's members (e.g. getOpposite/getMargin).
 */
class ConstraintAnchor {
  public:
    /** Define the type of anchor */
    enum class Type { NONE, LEFT, TOP, RIGHT, BOTTOM, BASELINE, CENTER, CENTER_X, CENTER_Y };

    static constexpr int UNSET_GONE_MARGIN = INT_MIN;

    // Java: public final ConstraintWidget mOwner / public final Type mType.
    // Kept non-const members (set once at construction, never reassigned) — `final` is a
    // Java compile-time check; relaxing it avoids deleting the implicit copy/assignment.
    ConstraintWidget*   mOwner;
    Type                mType;
    ConstraintAnchor*   mTarget = nullptr;
    int                 mMargin = 0;
    int                 mGoneMargin = UNSET_GONE_MARGIN;

    SolverVariable*     mSolverVariable = nullptr;

    ConstraintAnchor(ConstraintWidget* owner, Type type);
    // The anchor owns its lazily-created SolverVariable (resetSolverVariable) and its
    // dependents set. LinearSystem::reset() only reset()s/index-clears them, never frees —
    // so the anchor is the sole owner and frees them here.
    ~ConstraintAnchor();

    SolverVariable* getSolverVariable() const;
    void resetSolverVariable(Cache* cache);

    ConstraintWidget* getOwner() const;
    Type getType() const;
    int getMargin() const;
    ConstraintAnchor* getTarget() const;

    void reset();
    bool connect(ConstraintAnchor* toAnchor, int margin, int goneMargin, bool forceConnection);
    bool connect(ConstraintAnchor* toAnchor, int margin);
    // Convenience overload taking the target by reference (Java anchors are reference types;
    // lets callers write `a.connect(b, m)` without `&b`).
    bool connect(ConstraintAnchor& toAnchor, int margin);
    bool isConnected() const;
    bool isValidConnection(ConstraintAnchor* anchor) const;
    bool isSideAnchor() const;
    bool isSimilarDimensionConnection(const ConstraintAnchor* anchor) const;
    void setMargin(int margin);
    void setGoneMargin(int margin);
    bool isVerticalAnchor() const;
    std::string toString() const;

    bool isConnectionAllowed(ConstraintWidget* target, ConstraintAnchor* anchor) const;
    bool isConnectionAllowed(ConstraintWidget* target) const;
    ConstraintAnchor* getOpposite() const;

    // --- dependents tracking ---
    std::unordered_set<ConstraintAnchor*>* getDependents() const;
    bool hasDependents() const;
    bool hasCenteredDependents() const;
    void findDependents(int orientation, std::vector<WidgetGroup*>& list, WidgetGroup* group);

    // --- final resolution (Chain fast-path bookkeeping) ---
    void setFinalValue(int finalValue);
    int  getFinalValue() const;
    void resetFinalResolution();
    bool hasFinalValue() const;

    void copyFrom(ConstraintAnchor* source, std::unordered_map<ConstraintWidget*, ConstraintWidget*>& map);

  private:
    static constexpr bool ALLOW_BINARY = false;

    std::unordered_set<ConstraintAnchor*>* mDependents = nullptr;
    int  mFinalValue = 0;
    bool mHasFinalValue = false;

    bool isConnectionToMe(ConstraintWidget* target, std::unordered_set<ConstraintWidget*>& checked) const;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CONSTRAINT_ANCHOR_H
