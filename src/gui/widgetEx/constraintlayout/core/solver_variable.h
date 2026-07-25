/*
 * Copyright (C) 2015 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.SolverVariable.
 *
 * Java package-private fields are exposed as public here: the solver classes
 * (ArrayRow, LinearSystem, ArrayLinkedVariables) cooperate tightly and access
 * them directly. C++ has no package-private access, so public is the faithful
 * equivalent.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_SOLVER_VARIABLE_H__
#define __CONSTRAINTLAYOUT_CORE_SOLVER_VARIABLE_H__

#include <string>
#include <vector>

namespace cdroid {

class ArrayRow;      // forward
class LinearSystem;  // forward

/**
 * Represents a given variable used in the LinearSystem linear expression solver.
 * Ported verbatim from androidx.constraintlayout.core.SolverVariable.
 * Implements Comparable<SolverVariable> via operator< / compareTo (by id).
 */
class SolverVariable {
public:
    enum class Type {
        UNRESTRICTED, // can take negative or positive values
        CONSTANT,     // actually a constant number, not a variable
        SLACK,        // restricted to positive values, represents a slack
        ERROR,        // restricted to positive values, represents an error
        UNKNOWN       // unknown (invalid) type
    };

    static const int STRENGTH_NONE      = 0;
    static const int STRENGTH_LOW       = 1;
    static const int STRENGTH_MEDIUM    = 2;
    static const int STRENGTH_HIGH      = 3;
    static const int STRENGTH_HIGHEST   = 4;
    static const int STRENGTH_EQUALITY  = 5;
    static const int STRENGTH_BARRIER   = 6;
    static const int STRENGTH_CENTERING = 7;
    static const int STRENGTH_FIXED     = 8;

    static const int MAX_STRENGTH = 9;

    // --- fields (Java package-private -> public; Java private kept private) ---
    bool   inGoal = false;
    int    id = -1;
    int    mDefinitionId = -1;
    int    strength = 0;
    float  computedValue = 0;
    bool   isFinalValue = false;
    Type   mType = Type::UNKNOWN;

    float  mStrengthVector[MAX_STRENGTH]      = {};
    float  mGoalStrengthVector[MAX_STRENGTH]  = {};

    // VAR_USE_HASH == false path: array-backed client equations list.
    // (Java: ArrayRow[16] + mClientEquationsCount; C++ uses std::vector.)
    std::vector<ArrayRow*> mClientEquations;
    int    usageInRowCount = 0;

    bool   mIsSynonym = false;
    int    mSynonym = -1;
    float  mSynonymDelta = 0;

    // --- constructors ---
    SolverVariable(const std::string& name, Type type);
    SolverVariable(Type type, const std::string& prefix);

    // --- behavior ---
    void clearStrengths();
    std::string strengthsToString();

    void addToRow(ArrayRow* row);
    void removeFromRow(ArrayRow* row);
    void updateReferencesWithNewDefinition(LinearSystem* system, ArrayRow* definition);

    void setFinalValue(LinearSystem* system, float value);
    void setSynonym(LinearSystem* system, SolverVariable* synonymVariable, float value);

    void reset();

    std::string getName();
    void setName(const std::string& name);
    void setType(Type type, const std::string& prefix);

    // Comparable<SolverVariable>
    int compareTo(const SolverVariable& v) const { return id - v.id; }
    bool operator<(const SolverVariable& v) const { return id < v.id; }

    std::string toString();

    static void increaseErrorId();

private:
    static bool kInternalDebug; // == LinearSystem::FULL_DEBUG (false)
    static bool kVarUseHash;    // Java VAR_USE_HASH (false)
    static bool kDoNotUse;      // Java DO_NOT_USE (false)

    std::string mName;

    static int sUniqueSlackId;
    static int sUniqueErrorId;
    static int sUniqueUnrestrictedId;
    static int sUniqueConstantId;
    static int sUniqueId;

    static std::string getUniqueName(Type type, const std::string& prefix);
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_SOLVER_VARIABLE_H__
