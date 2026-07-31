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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.PriorityGoalRow.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_PRIORITY_GOAL_ROW_H__
#define __CONSTRAINTLAYOUT_CORE_PRIORITY_GOAL_ROW_H__

#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <vector>
#include <string>

namespace cdroid {

class SolverVariable;
class Cache;

/**
 * Ported verbatim from androidx.constraintlayout.core.PriorityGoalRow.
 * Implements a row containing goals taking into account priorities.
 */
class PriorityGoalRow : public ArrayRow {
  public:
    /**
     * Accessor that manipulates a SolverVariable's goal strength vector.
     * (Java inner class GoalVariableAccessor; nested here, friended so it can
     * call the outer removeGoal().)
     */
    class GoalVariableAccessor {
      public:
        SolverVariable* mVariable = nullptr;
        PriorityGoalRow* mRow;

        explicit GoalVariableAccessor(PriorityGoalRow* row) : mRow(row) {}

        void init(SolverVariable* variable);
        bool addToGoal(SolverVariable* other, float value);
        void add(SolverVariable* other);
        bool isNegative();
        bool isSmallerThan(SolverVariable* other);
        bool isNull();
        void reset();
        std::string toString();
    };

    friend class GoalVariableAccessor;

  private:
    static constexpr float EPSILON = 0.0001f;
    static const int NOT_FOUND = -1;

    int mTableSize = 128;
    std::vector<SolverVariable*> mArrayGoals;
    std::vector<SolverVariable*> mSortArray;
    int mNumGoals = 0;
    GoalVariableAccessor mAccessor;
    Cache* mCache;

  public:
    explicit PriorityGoalRow(Cache* cache);

    void clear() override;
    bool isEmpty() override;
    SolverVariable* getPivotCandidate(LinearSystem* system, bool* avoid) override;
    void addError(SolverVariable* error) override;
    void updateFromRow(LinearSystem* system, ArrayRow* definition, bool removeFromDefinition) override;
    std::string toString() override;

  private:
    void addToGoal(SolverVariable* variable);
    void removeGoal(SolverVariable* variable);
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_PRIORITY_GOAL_ROW_H__
