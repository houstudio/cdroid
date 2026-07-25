/*
 * Copyright (C) 2020 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.PriorityGoalRow.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_PRIORITY_GOAL_ROW_H__
#define __CONSTRAINTLAYOUT_CORE_PRIORITY_GOAL_ROW_H__

#include <widgetEx/constraintlayout/core/array_row.h>
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
