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
#include <widgetEx/constraintlayout/core/prioritygoalrow.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <widgetEx/constraintlayout/core/cache.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace cdroid {

// ----------------------------- GoalVariableAccessor -----------------------------

void PriorityGoalRow::GoalVariableAccessor::init(SolverVariable* variable) {
    this->mVariable = variable;
}

bool PriorityGoalRow::GoalVariableAccessor::addToGoal(SolverVariable* other, float value) {
    if (mVariable->inGoal) {
        bool empty = true;
        for (int i = 0; i < SolverVariable::MAX_STRENGTH; i++) {
            mVariable->mGoalStrengthVector[i] += other->mGoalStrengthVector[i] * value;
            float v = mVariable->mGoalStrengthVector[i];
            if (std::abs(v) < PriorityGoalRow::EPSILON) {
                mVariable->mGoalStrengthVector[i] = 0;
            } else {
                empty = false;
            }
        }
        if (empty) {
            mRow->removeGoal(mVariable);
        }
    } else {
        for (int i = 0; i < SolverVariable::MAX_STRENGTH; i++) {
            float strength = other->mGoalStrengthVector[i];
            if (strength != 0) {
                float v = value * strength;
                if (std::abs(v) < PriorityGoalRow::EPSILON) {
                    v = 0;
                }
                mVariable->mGoalStrengthVector[i] = v;
            } else {
                mVariable->mGoalStrengthVector[i] = 0;
            }
        }
        return true;
    }
    return false;
}

void PriorityGoalRow::GoalVariableAccessor::add(SolverVariable* other) {
    for (int i = 0; i < SolverVariable::MAX_STRENGTH; i++) {
        mVariable->mGoalStrengthVector[i] += other->mGoalStrengthVector[i];
        float value = mVariable->mGoalStrengthVector[i];
        if (std::abs(value) < PriorityGoalRow::EPSILON) {
            mVariable->mGoalStrengthVector[i] = 0;
        }
    }
}

bool PriorityGoalRow::GoalVariableAccessor::isNegative() {
    for (int i = SolverVariable::MAX_STRENGTH - 1; i >= 0; i--) {
        float value = mVariable->mGoalStrengthVector[i];
        if (value > 0) {
            return false;
        }
        if (value < 0) {
            return true;
        }
    }
    return false;
}

bool PriorityGoalRow::GoalVariableAccessor::isSmallerThan(SolverVariable* other) {
    for (int i = SolverVariable::MAX_STRENGTH - 1; i >= 0; i--) {
        float comparedValue = other->mGoalStrengthVector[i];
        float value = mVariable->mGoalStrengthVector[i];
        if (value == comparedValue) {
            continue;
        }
        return value < comparedValue;
    }
    return false;
}

bool PriorityGoalRow::GoalVariableAccessor::isNull() {
    for (int i = 0; i < SolverVariable::MAX_STRENGTH; i++) {
        if (mVariable->mGoalStrengthVector[i] != 0) {
            return false;
        }
    }
    return true;
}

void PriorityGoalRow::GoalVariableAccessor::reset() {
    for (int i = 0; i < SolverVariable::MAX_STRENGTH; i++) {
        mVariable->mGoalStrengthVector[i] = 0;
    }
}

std::string PriorityGoalRow::GoalVariableAccessor::toString() {
    std::string result = "[ ";
    if (mVariable != nullptr) {
        for (int i = 0; i < SolverVariable::MAX_STRENGTH; i++) {
            result += std::to_string(mVariable->mGoalStrengthVector[i]) + " ";
        }
    }
    result += "] " + (mVariable != nullptr ? mVariable->toString() : std::string("null"));
    return result;
}

// ------------------------------- PriorityGoalRow --------------------------------

PriorityGoalRow::PriorityGoalRow(Cache* cache)
    : ArrayRow(cache)
    , mArrayGoals(mTableSize, nullptr)
    , mSortArray(mTableSize, nullptr)
    , mAccessor(this)
    , mCache(cache) {
}

void PriorityGoalRow::clear() {
    mNumGoals = 0;
    mConstantValue = 0;
}

bool PriorityGoalRow::isEmpty() {
    return mNumGoals == 0;
}

SolverVariable* PriorityGoalRow::getPivotCandidate(LinearSystem* /*system*/, bool* avoid) {
    int pivot = NOT_FOUND;
    for (int i = 0; i < mNumGoals; i++) {
        SolverVariable* variable = mArrayGoals[i];
        if (avoid != nullptr && avoid[variable->id]) {
            continue;
        }
        mAccessor.init(variable);
        if (pivot == NOT_FOUND) {
            if (mAccessor.isNegative()) {
                pivot = i;
            }
        } else if (mAccessor.isSmallerThan(mArrayGoals[pivot])) {
            pivot = i;
        }
    }
    if (pivot == NOT_FOUND) {
        return nullptr;
    }
    return mArrayGoals[pivot];
}

void PriorityGoalRow::addError(SolverVariable* error) {
    mAccessor.init(error);
    mAccessor.reset();
    error->mGoalStrengthVector[error->strength] = 1;
    addToGoal(error);
}

void PriorityGoalRow::addToGoal(SolverVariable* variable) {
    if (mNumGoals + 1 > (int)mArrayGoals.size()) {
        mArrayGoals.resize(mArrayGoals.size() * 2, nullptr);
        mSortArray.resize(mArrayGoals.size(), nullptr);
    }
    mArrayGoals[mNumGoals] = variable;
    mNumGoals++;

    if (mNumGoals > 1 && mArrayGoals[mNumGoals - 1]->id > variable->id) {
        for (int i = 0; i < mNumGoals; i++) {
            mSortArray[i] = mArrayGoals[i];
        }
        std::sort(mSortArray.begin(), mSortArray.begin() + mNumGoals,
        [](SolverVariable* a, SolverVariable* b) {
            return a->id < b->id;
        });
        for (int i = 0; i < mNumGoals; i++) {
            mArrayGoals[i] = mSortArray[i];
        }
    }

    variable->inGoal = true;
    variable->addToRow(this);
}

void PriorityGoalRow::removeGoal(SolverVariable* variable) {
    for (int i = 0; i < mNumGoals; i++) {
        if (mArrayGoals[i] == variable) {
            for (int j = i; j < mNumGoals - 1; j++) {
                mArrayGoals[j] = mArrayGoals[j + 1];
            }
            mNumGoals--;
            variable->inGoal = false;
            return;
        }
    }
}

void PriorityGoalRow::updateFromRow(LinearSystem* /*system*/, ArrayRow* definition, bool /*removeFromDefinition*/) {
    SolverVariable* goalVariable = definition->mVariable;
    if (goalVariable == nullptr) {
        return;
    }
    ArrayRowVariables* rowVariables = definition->variables;
    int currentSize = rowVariables->getCurrentSize();
    for (int i = 0; i < currentSize; i++) {
        SolverVariable* solverVariable = rowVariables->getVariable(i);
        float value = rowVariables->getVariableValue(i);
        mAccessor.init(solverVariable);
        if (mAccessor.addToGoal(goalVariable, value)) {
            addToGoal(solverVariable);
        }
        mConstantValue += definition->mConstantValue * value;
    }
    removeGoal(goalVariable);
}

std::string PriorityGoalRow::toString() {
    std::string result;
    result += " goal -> (" + std::to_string(mConstantValue) + ") : ";
    for (int i = 0; i < mNumGoals; i++) {
        SolverVariable* v = mArrayGoals[i];
        mAccessor.init(v);
        result += mAccessor.toString() + " ";
    }
    return result;
}

} // namespace cdroid
