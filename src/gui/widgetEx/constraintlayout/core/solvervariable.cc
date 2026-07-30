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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.SolverVariable.
 */
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>

#include <sstream>

namespace cdroid {

// static field definitions
bool SolverVariable::kInternalDebug = false; // == LinearSystem::FULL_DEBUG
bool SolverVariable::kVarUseHash    = false; // Java VAR_USE_HASH
bool SolverVariable::kDoNotUse      = false; // Java DO_NOT_USE

int SolverVariable::sUniqueSlackId      = 1;
int SolverVariable::sUniqueErrorId      = 1;
int SolverVariable::sUniqueUnrestrictedId = 1;
int SolverVariable::sUniqueConstantId   = 1;
int SolverVariable::sUniqueId           = 1;

SolverVariable::SolverVariable(const std::string& name, Type type)
    : mName(name), mType(type) {
}

SolverVariable::SolverVariable(Type type, const std::string& prefix)
    : mType(type) {
    (void)prefix;
    // Java: if (INTERNAL_DEBUG) mName = getUniqueName(type, prefix);
}

void SolverVariable::increaseErrorId() {
    sUniqueErrorId++;
}

std::string SolverVariable::getUniqueName(Type type, const std::string& prefix) {
    if (!prefix.empty()) {
        return prefix + std::to_string(sUniqueErrorId);
    }
    switch (type) {
    case Type::UNRESTRICTED:
        return "U" + std::to_string(++sUniqueUnrestrictedId);
    case Type::CONSTANT:
        return "C" + std::to_string(++sUniqueConstantId);
    case Type::SLACK:
        return "S" + std::to_string(++sUniqueSlackId);
    case Type::ERROR:
        return "e" + std::to_string(++sUniqueErrorId);
    case Type::UNKNOWN:
        return "V" + std::to_string(++sUniqueId);
    }
    return "V" + std::to_string(++sUniqueId); // unreachable (Java throws AssertionError)
}

void SolverVariable::clearStrengths() {
    for (int i = 0; i < MAX_STRENGTH; i++) {
        mStrengthVector[i] = 0;
    }
}

std::string SolverVariable::strengthsToString() {
    std::string representation = toString() + "[";
    bool negative = false;
    bool empty = true;
    for (int j = 0; j < MAX_STRENGTH; j++) {
        representation += std::to_string(mStrengthVector[j]);
        if (mStrengthVector[j] > 0) {
            negative = false;
        } else if (mStrengthVector[j] < 0) {
            negative = true;
        }
        if (mStrengthVector[j] != 0) {
            empty = false;
        }
        if (j < MAX_STRENGTH - 1) {
            representation += ", ";
        } else {
            representation += "] ";
        }
    }
    if (negative) {
        representation += " (-)";
    }
    if (empty) {
        representation += " (*)";
    }
    return representation;
}

void SolverVariable::addToRow(ArrayRow* row) {
    // VAR_USE_HASH == false path
    for (auto* r : mClientEquations) {
        if (r == row) {
            return;
        }
    }
    mClientEquations.push_back(row);
}

void SolverVariable::removeFromRow(ArrayRow* row) {
    // VAR_USE_HASH == false path
    for (auto it = mClientEquations.begin(); it != mClientEquations.end(); ++it) {
        if (*it == row) {
            mClientEquations.erase(it);
            return;
        }
    }
}

void SolverVariable::updateReferencesWithNewDefinition(LinearSystem* system, ArrayRow* definition) {
    // VAR_USE_HASH == false path. Snapshot first: updateFromRow may touch the list.
    auto equations = mClientEquations;
    for (auto* row : equations) {
        row->updateFromRow(system, definition, false);
    }
    mClientEquations.clear();
}

void SolverVariable::setFinalValue(LinearSystem* system, float value) {
    computedValue = value;
    isFinalValue = true;
    mIsSynonym = false;
    mSynonym = -1;
    mSynonymDelta = 0;
    mDefinitionId = -1;
    auto equations = mClientEquations; // snapshot (Java uses count)
    for (auto* row : equations) {
        row->updateFromFinalVariable(system, this, false);
    }
    mClientEquations.clear();
}

void SolverVariable::setSynonym(LinearSystem* system, SolverVariable* synonymVariable, float value) {
    mIsSynonym = true;
    mSynonym = synonymVariable->id;
    mSynonymDelta = value;
    mDefinitionId = -1;
    auto equations = mClientEquations; // snapshot
    for (auto* row : equations) {
        row->updateFromSynonymVariable(system, this, false);
    }
    mClientEquations.clear();
    system->displayReadableRows();
}

void SolverVariable::reset() {
    mName.clear();
    mType = Type::UNKNOWN;
    strength = STRENGTH_NONE;
    id = -1;
    mDefinitionId = -1;
    computedValue = 0;
    isFinalValue = false;
    mIsSynonym = false;
    mSynonym = -1;
    mSynonymDelta = 0;
    // VAR_USE_HASH == false path
    mClientEquations.clear();
    usageInRowCount = 0;
    inGoal = false;
    for (int i = 0; i < MAX_STRENGTH; i++) {
        mGoalStrengthVector[i] = 0;
    }
}

std::string SolverVariable::getName() {
    return mName;
}

void SolverVariable::setName(const std::string& name) {
    mName = name;
}

void SolverVariable::setType(Type type, const std::string& prefix) {
    mType = type;
    if (kInternalDebug && mName.empty()) {
        mName = getUniqueName(type, prefix);
    }
}

std::string SolverVariable::toString() {
    std::string result;
    if (kInternalDebug) {
        result += mName + "(" + std::to_string(id) + "):" + std::to_string(strength);
        if (mIsSynonym) {
            result += ":S(" + std::to_string(mSynonym) + ")";
        }
        if (isFinalValue) {
            result += ":F(" + std::to_string(computedValue) + ")";
        }
    } else {
        if (!mName.empty()) {
            result += mName;
        } else {
            result += std::to_string(id);
        }
    }
    return result;
}

} // namespace cdroid
