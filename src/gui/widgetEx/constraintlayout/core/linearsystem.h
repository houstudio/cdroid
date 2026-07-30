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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.LinearSystem.
 *
 * createObjectVariable / getObjectVariableValue / addCenterPoint bridge to the
 * widgets model (ConstraintAnchor/ConstraintWidget) and are stubbed until that
 * layer lands; the solver core (minimize/optimize/enforceBFS/addConstraint/
 * addEquality/...) is fully ported and usable standalone.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_LINEAR_SYSTEM_H__
#define __CONSTRAINTLAYOUT_CORE_LINEAR_SYSTEM_H__

#include <cstdint>
#include <widgetEx/constraintlayout/core/metrics.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace cdroid {

class ArrayRow;
class Cache;
class ConstraintAnchor;  // forward (widget bridge)
class ConstraintWidget;  // forward (widget bridge)

/**
 * Represents and solves a system of linear equations.
 * Ported verbatim from androidx.constraintlayout.core.LinearSystem.
 */
class LinearSystem {
  public:
    // --- public static toggles (Java public static boolean; DEBUG field name
    //     avoided — it clashes with a system DEBUG macro) ---
    static const bool FULL_DEBUG;
    static const bool DEBUG_CONSTRAINTS;        // == FULL_DEBUG
    static bool USE_DEPENDENCY_ORDERING;
    static bool USE_BASIC_SYNONYMS;
    static bool SIMPLIFY_SYNONYMS;
    static bool USE_SYNONYMS;
    static bool SKIP_COLUMNS;
    static bool OPTIMIZED_ENGINE;
    static int  sPoolSize;
    static int64_t ARRAY_ROW_CREATION;
    static int64_t OPTIMIZED_ARRAY_ROW_CREATION;
    static Metrics* sMetrics;

    // --- public/package fields (Java package-private -> public) ---
    bool hasSimpleDefinition = false;
    int  mVariablesID = 0;
    Cache* mCache = nullptr;
    std::vector<ArrayRow*>      mRows;            // Java: ArrayRow[]
    int  mNumRows = 0;
    int  mNumColumns = 1;
    int  mTableSize = 32;
    int  mMaxColumns = 32;
    int  mMaxRows = 32;
    bool graphOptimizer = false;
    bool newgraphOptimizer = false;
    std::vector<SolverVariable*> mPoolVariables;  // Java: SolverVariable[sPoolSize]
    int  mPoolVariablesCount = 0;

    /**
     * Nested Row interface (Java: interface LinearSystem.Row). ArrayRow implements it.
     */
    class Row {
      public:
        virtual ~Row() = default;
        virtual SolverVariable* getPivotCandidate(LinearSystem* system, bool* avoid) = 0;
        virtual void clear() = 0;
        virtual void initFromRow(Row* row) = 0;
        virtual void addError(SolverVariable* variable) = 0;
        virtual void updateFromSystem(LinearSystem* system) = 0;
        virtual SolverVariable* getKey() = 0;
        virtual bool isEmpty() = 0;
        virtual void updateFromRow(LinearSystem* system, ArrayRow* definition, bool b) = 0;
        virtual void updateFromFinalVariable(LinearSystem* system, SolverVariable* variable,
                                             bool removeFromDefinition) = 0;
    };

    LinearSystem();
    virtual ~LinearSystem();

    void fillMetrics(Metrics* metrics);
    static Metrics* getMetrics();

    // --- creation of rows / variables / errors ---
    ArrayRow* createRow();
    SolverVariable* createSlackVariable();
    SolverVariable* createExtraVariable();
    SolverVariable* createErrorVariable(int strength, const std::string& prefix);
    void addSingleError(ArrayRow* row, int sign, int strength);

    // --- widget bridge (stubbed until widgets layer lands) ---
    SolverVariable* createObjectVariable(ConstraintAnchor* anchor);
    int  getObjectVariableValue(ConstraintAnchor* anchor);
    void addCenterPoint(ConstraintWidget* widget, ConstraintWidget* target,
                        float angle, int radius);

    // --- accessors ---
    Row* getGoal();
    ArrayRow* getRow(int n);
    float getValueFor(const std::string& name);
    SolverVariable* getVariable(const std::string& name, SolverVariable::Type type);
    Cache* getCache();
    int  getMemoryUsed();
    int  getNumEquations();
    int  getNumVariables();

    // --- system resolution ---
    void minimize();
    void minimizeGoal(Row* goal);
    void cleanupRows();
    void addConstraint(ArrayRow* row);
    void removeRow(ArrayRow* row);

    // --- equations ---
    void addGreaterThan(SolverVariable* a, SolverVariable* b, int margin, int strength);
    void addGreaterBarrier(SolverVariable* a, SolverVariable* b, int margin, bool hasMatchConstraintWidgets);
    void addLowerThan(SolverVariable* a, SolverVariable* b, int margin, int strength);
    void addLowerBarrier(SolverVariable* a, SolverVariable* b, int margin, bool hasMatchConstraintWidgets);
    void addCentering(SolverVariable* a, SolverVariable* b, int m1, float bias,
                      SolverVariable* c, SolverVariable* d, int m2, int strength);
    void addRatio(SolverVariable* a, SolverVariable* b, SolverVariable* c, SolverVariable* d,
                  float ratio, int strength);
    void addSynonym(SolverVariable* a, SolverVariable* b, int margin);
    ArrayRow* addEquality(SolverVariable* a, SolverVariable* b, int margin, int strength);
    void addEquality(SolverVariable* a, int value);
    static ArrayRow* createRowDimensionPercent(LinearSystem* linearSystem,
            SolverVariable* variableA,
            SolverVariable* variableC, float percent);

    // --- display (debug; best-effort, may be no-op) ---
    void displayReadableRows();
    void displayRows();
    void displayVariablesReadableRows();
    void displaySystemInformation();

    void reset();

  private:
    Row*  mGoal = nullptr;
    Row*  mTempGoal = nullptr;
    bool* mAlreadyTestedCandidates = nullptr;     // Java: boolean[mTableSize] (raw bool* to match Row::getPivotCandidate)
    std::unordered_map<std::string, SolverVariable*> mVariables; // Java: lazy HashMap

    SolverVariable* createVariable(const std::string& name, SolverVariable::Type type);
    SolverVariable* acquireSolverVariable(SolverVariable::Type type, const std::string& prefix);
    void addRow(ArrayRow* row);
    int  optimize(Row* goal, bool b);
    int  enforceBFS(Row* goal);
    void computeValues();
    void increaseTableSize();
    void releaseRows();
    void displaySolverVariables();
    std::string getDisplaySize(int n);
    std::string getDisplayStrength(int strength);
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_LINEAR_SYSTEM_H__
