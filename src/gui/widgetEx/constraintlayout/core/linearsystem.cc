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
 * Debug print branches (Java if (DEBUG)/if (DEBUG_CONSTRAINTS)) are omitted
 * (both flags are false).
 */
#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <widgetEx/constraintlayout/core/cache.h>
#include <widgetEx/constraintlayout/core/prioritygoalrow.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <widgetEx/constraintlayout/core/widgets/constraintanchor.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

namespace cdroid {

// --- static definitions ---
const bool LinearSystem::FULL_DEBUG = false;
const bool LinearSystem::DEBUG_CONSTRAINTS = false;
bool LinearSystem::USE_DEPENDENCY_ORDERING = false;
bool LinearSystem::USE_BASIC_SYNONYMS = true;
bool LinearSystem::SIMPLIFY_SYNONYMS = true;
bool LinearSystem::USE_SYNONYMS = true;
bool LinearSystem::SKIP_COLUMNS = true;
bool LinearSystem::OPTIMIZED_ENGINE = false;
int  LinearSystem::sPoolSize = 1000;
int64_t LinearSystem::ARRAY_ROW_CREATION = 0;
int64_t LinearSystem::OPTIMIZED_ARRAY_ROW_CREATION = 0;
Metrics* LinearSystem::sMetrics = nullptr;

LinearSystem::LinearSystem() {
    mRows.assign(mTableSize, nullptr);                  // Java: new ArrayRow[mTableSize]
    releaseRows();
    mCache = new Cache();
    mGoal = new PriorityGoalRow(mCache);                // Java: mGoal = new PriorityGoalRow(mCache)
    // OPTIMIZED_ENGINE == false -> mTempGoal = new ArrayRow(mCache)
    mTempGoal = new ArrayRow(mCache);
    mPoolVariables.assign(sPoolSize, nullptr);          // Java: new SolverVariable[sPoolSize]
    mAlreadyTestedCandidates = new bool[mTableSize]();
}

LinearSystem::~LinearSystem() {
    // Return the last solve's rows/variables to the (owning) pools so the Cache destructor frees
    // them. mPoolVariables holds only the acquireSolverVariable() vars (NOT the anchor-owned ones
    // createObjectVariable() creates — those are freed by ConstraintAnchor, so they must NOT be
    // pooled here, or their dtor would double-free).
    releaseRows();
    for (int i = 0; i < mPoolVariablesCount; i++) {
        if (mPoolVariables[i] != nullptr) mCache->mSolverVariablePool.release(mPoolVariables[i]);
    }
    mPoolVariablesCount = 0;
    delete mCache;   // owning pools (Cache dtor) delete every pooled ArrayRow / SolverVariable
    delete mGoal;
    delete mTempGoal;
    delete[] mAlreadyTestedCandidates;
}

void LinearSystem::fillMetrics(Metrics* metrics) {
    sMetrics = metrics;
}

Metrics* LinearSystem::getMetrics() {
    return sMetrics;
}

//-------------------------------- creation of rows/variables/errors --------------------------------

ArrayRow* LinearSystem::createRow() {
    ArrayRow* row;
    if (OPTIMIZED_ENGINE) {
        row = mCache->mOptimizedArrayRowPool.acquire();
        if (row == nullptr) {
            // ValuesRow (OPTIMIZED_ENGINE only) — not used in default build
            OPTIMIZED_ARRAY_ROW_CREATION++;
        } else {
            row->reset();
        }
    } else {
        row = mCache->mArrayRowPool.acquire();
        if (row == nullptr) {
            row = new ArrayRow(mCache);
            ARRAY_ROW_CREATION++;
        } else {
            row->reset();
        }
    }
    SolverVariable::increaseErrorId();
    return row;
}

SolverVariable* LinearSystem::createSlackVariable() {
    if (sMetrics != nullptr) sMetrics->slackvariables++;
    if (mNumColumns + 1 >= mMaxColumns) increaseTableSize();
    SolverVariable* variable = acquireSolverVariable(SolverVariable::Type::SLACK, "");
    mVariablesID++;
    mNumColumns++;
    variable->id = mVariablesID;
    mCache->mIndexedVariables[mVariablesID] = variable;
    return variable;
}

SolverVariable* LinearSystem::createExtraVariable() {
    if (sMetrics != nullptr) sMetrics->extravariables++;
    if (mNumColumns + 1 >= mMaxColumns) increaseTableSize();
    SolverVariable* variable = acquireSolverVariable(SolverVariable::Type::SLACK, "");
    mVariablesID++;
    mNumColumns++;
    variable->id = mVariablesID;
    mCache->mIndexedVariables[mVariablesID] = variable;
    return variable;
}

SolverVariable* LinearSystem::createVariable(const std::string& name, SolverVariable::Type type) {
    if (sMetrics != nullptr) sMetrics->variables++;
    if (mNumColumns + 1 >= mMaxColumns) increaseTableSize();
    SolverVariable* variable = acquireSolverVariable(type, "");
    variable->setName(name);
    mVariablesID++;
    mNumColumns++;
    variable->id = mVariablesID;
    mVariables[name] = variable;
    mCache->mIndexedVariables[mVariablesID] = variable;
    return variable;
}

SolverVariable* LinearSystem::createErrorVariable(int strength, const std::string& prefix) {
    if (sMetrics != nullptr) sMetrics->errors++;
    if (mNumColumns + 1 >= mMaxColumns) increaseTableSize();
    SolverVariable* variable = acquireSolverVariable(SolverVariable::Type::ERROR, prefix);
    mVariablesID++;
    mNumColumns++;
    variable->id = mVariablesID;
    variable->strength = strength;
    mCache->mIndexedVariables[mVariablesID] = variable;
    mGoal->addError(variable);
    return variable;
}

SolverVariable* LinearSystem::acquireSolverVariable(SolverVariable::Type type, const std::string& prefix) {
    SolverVariable* variable = mCache->mSolverVariablePool.acquire();
    if (variable == nullptr) {
        variable = new SolverVariable(type, prefix);
        variable->setType(type, prefix);
    } else {
        variable->reset();
        variable->setType(type, prefix);
    }
    if (mPoolVariablesCount >= sPoolSize) {
        sPoolSize *= 2;
        mPoolVariables.resize(sPoolSize, nullptr);
    }
    mPoolVariables[mPoolVariablesCount++] = variable;
    return variable;
}

void LinearSystem::addSingleError(ArrayRow* row, int sign, int strength) {
    std::string prefix; // Java null (DEBUG-only prefix omitted)
    SolverVariable* error = createErrorVariable(strength, prefix);
    row->addSingleError(error, sign);
}

//--------------------------------- widget bridge ---------------------------------

SolverVariable* LinearSystem::createObjectVariable(ConstraintAnchor* anchor) {
    if (anchor == nullptr) {
        return nullptr;
    }
    if (mNumColumns + 1 >= mMaxColumns) {
        increaseTableSize();
    }
    SolverVariable* variable = anchor->getSolverVariable();
    if (variable == nullptr) {
        anchor->resetSolverVariable(mCache);
        variable = anchor->getSolverVariable();
    }
    if (variable->id == -1
            || variable->id > mVariablesID
            || mCache->mIndexedVariables[variable->id] == nullptr) {
        if (variable->id != -1) {
            variable->reset();
        }
        mVariablesID++;
        mNumColumns++;
        variable->id = mVariablesID;
        variable->mType = SolverVariable::Type::UNRESTRICTED;
        mCache->mIndexedVariables[mVariablesID] = variable;
    }
    return variable;
}

int LinearSystem::getObjectVariableValue(ConstraintAnchor* anchor) {
    // Java: if (Chain.USE_CHAIN_OPTIMIZATION) { if (anchor.hasFinalValue()) return anchor.getFinalValue(); }
    // Chain.USE_CHAIN_OPTIMIZATION is a static final == false, so that branch is dead and omitted.
    SolverVariable* variable = anchor->getSolverVariable();
    if (variable != nullptr) {
        return (int) (variable->computedValue + 0.5f);
    }
    return 0;
}

void LinearSystem::addCenterPoint(ConstraintWidget* widget, ConstraintWidget* target,
                                  float angle, int radius) {
    SolverVariable* Al = createObjectVariable(widget->getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* At = createObjectVariable(widget->getAnchor(ConstraintAnchor::Type::TOP));
    SolverVariable* Ar = createObjectVariable(widget->getAnchor(ConstraintAnchor::Type::RIGHT));
    SolverVariable* Ab = createObjectVariable(widget->getAnchor(ConstraintAnchor::Type::BOTTOM));

    SolverVariable* Bl = createObjectVariable(target->getAnchor(ConstraintAnchor::Type::LEFT));
    SolverVariable* Bt = createObjectVariable(target->getAnchor(ConstraintAnchor::Type::TOP));
    SolverVariable* Br = createObjectVariable(target->getAnchor(ConstraintAnchor::Type::RIGHT));
    SolverVariable* Bb = createObjectVariable(target->getAnchor(ConstraintAnchor::Type::BOTTOM));

    ArrayRow* row = createRow();
    float angleComponent = (float) (std::sin(angle) * radius);
    row->createRowWithAngle(At, Ab, Bt, Bb, angleComponent);
    addConstraint(row);
    row = createRow();
    angleComponent = (float) (std::cos(angle) * radius);
    row->createRowWithAngle(Al, Ar, Bl, Br, angleComponent);
    addConstraint(row);
}

//--------------------------------------- accessors -----------------------------------------

LinearSystem::Row* LinearSystem::getGoal() {
    return mGoal;
}

ArrayRow* LinearSystem::getRow(int n) {
    return mRows[n];
}

float LinearSystem::getValueFor(const std::string& name) {
    SolverVariable* v = getVariable(name, SolverVariable::Type::UNRESTRICTED);
    if (v == nullptr) return 0;
    return v->computedValue;
}

SolverVariable* LinearSystem::getVariable(const std::string& name, SolverVariable::Type type) {
    auto it = mVariables.find(name);
    SolverVariable* variable = (it != mVariables.end()) ? it->second : nullptr;
    if (variable == nullptr) {
        variable = createVariable(name, type);
    }
    return variable;
}

Cache* LinearSystem::getCache() {
    return mCache;
}

int LinearSystem::getMemoryUsed() {
    int actualRowSize = 0;
    for (int i = 0; i < mNumRows; i++) {
        if (mRows[i] != nullptr) actualRowSize += mRows[i]->sizeInBytes();
    }
    return actualRowSize;
}

int LinearSystem::getNumEquations() {
    return mNumRows;
}

int LinearSystem::getNumVariables() {
    return mVariablesID;
}

//------------------------------------- system resolution -----------------------------------

void LinearSystem::minimize() {
    if (sMetrics != nullptr) sMetrics->minimize++;
    if (mGoal->isEmpty()) {
        computeValues();
        return;
    }
    if (graphOptimizer || newgraphOptimizer) {
        if (sMetrics != nullptr) sMetrics->graphOptimizer++;
        bool fullySolved = true;
        for (int i = 0; i < mNumRows; i++) {
            ArrayRow* r = mRows[i];
            if (!r->mIsSimpleDefinition) {
                fullySolved = false;
                break;
            }
        }
        if (!fullySolved) {
            minimizeGoal(mGoal);
        } else {
            if (sMetrics != nullptr) sMetrics->fullySolved++;
            computeValues();
        }
    } else {
        minimizeGoal(mGoal);
    }
}

void LinearSystem::minimizeGoal(Row* goal) {
    if (sMetrics != nullptr) {
        sMetrics->minimizeGoal++;
        sMetrics->maxVariables = std::max(sMetrics->maxVariables, (int64_t)mNumColumns);
        sMetrics->maxRows = std::max(sMetrics->maxRows, (int64_t)mNumRows);
    }
    enforceBFS(goal);
    optimize(goal, false);
    computeValues();
}

void LinearSystem::cleanupRows() {
    int i = 0;
    while (i < mNumRows) {
        ArrayRow* current = mRows[i];
        if (current->variables->getCurrentSize() == 0) {
            current->mIsSimpleDefinition = true;
        }
        if (current->mIsSimpleDefinition) {
            current->mVariable->computedValue = current->mConstantValue;
            current->mVariable->removeFromRow(current);
            for (int j = i; j < mNumRows - 1; j++) {
                mRows[j] = mRows[j + 1];
            }
            mRows[mNumRows - 1] = nullptr;
            mNumRows--;
            i--;
            if (OPTIMIZED_ENGINE) mCache->mOptimizedArrayRowPool.release(current);
            else mCache->mArrayRowPool.release(current);
        }
        i++;
    }
}

void LinearSystem::addConstraint(ArrayRow* row) {
    if (row == nullptr) return;
    if (sMetrics != nullptr) {
        sMetrics->constraints++;
        if (row->mIsSimpleDefinition) sMetrics->simpleconstraints++;
    }
    if (mNumRows + 1 >= mMaxRows || mNumColumns + 1 >= mMaxColumns) {
        increaseTableSize();
    }

    bool added = false;
    if (!row->mIsSimpleDefinition) {
        row->updateFromSystem(this);
        if (row->isEmpty()) {
            if (OPTIMIZED_ENGINE) mCache->mOptimizedArrayRowPool.release(row);
            else mCache->mArrayRowPool.release(row);
            return;
        }
        row->ensurePositiveConstant();

        if (row->chooseSubject(this)) {
            SolverVariable* extra = createExtraVariable();
            row->mVariable = extra;
            int numRows = mNumRows;
            addRow(row);
            if (mNumRows == numRows + 1) {
                added = true;
                mTempGoal->initFromRow(row);
                optimize(mTempGoal, true);
                if (extra->mDefinitionId == -1) {
                    if (row->mVariable == extra) {
                        SolverVariable* pivotCandidate = row->pickPivot(extra);
                        if (pivotCandidate != nullptr) {
                            if (sMetrics != nullptr) sMetrics->pivots++;
                            row->pivot(pivotCandidate);
                        }
                    }
                    if (!row->mIsSimpleDefinition) {
                        row->mVariable->updateReferencesWithNewDefinition(this, row);
                    }
                    if (OPTIMIZED_ENGINE) mCache->mOptimizedArrayRowPool.release(row);
                    else mCache->mArrayRowPool.release(row);
                    mNumRows--;
                    return; // row already released above; bail before the !hasKeyVariable() re-release
                }
            }
        }

        if (!row->hasKeyVariable()) {
            if (OPTIMIZED_ENGINE) mCache->mOptimizedArrayRowPool.release(row);
            else mCache->mArrayRowPool.release(row);
            return;
        }
    }
    if (!added) {
        addRow(row);
    }
}

void LinearSystem::addRow(ArrayRow* row) {
    if (SIMPLIFY_SYNONYMS && row->mIsSimpleDefinition) {
        row->mVariable->setFinalValue(this, row->mConstantValue);
        if (OPTIMIZED_ENGINE) mCache->mOptimizedArrayRowPool.release(row);
        else mCache->mArrayRowPool.release(row);
    } else {
        mRows[mNumRows] = row;
        row->mVariable->mDefinitionId = mNumRows;
        mNumRows++;
        row->mVariable->updateReferencesWithNewDefinition(this, row);
    }
    if (SIMPLIFY_SYNONYMS && hasSimpleDefinition) {
        for (int i = 0; i < mNumRows; i++) {
            if (mRows[i] != nullptr && mRows[i]->mIsSimpleDefinition) {
                ArrayRow* removedRow = mRows[i];
                removedRow->mVariable->setFinalValue(this, removedRow->mConstantValue);
                if (OPTIMIZED_ENGINE) mCache->mOptimizedArrayRowPool.release(removedRow);
                else mCache->mArrayRowPool.release(removedRow);
                mRows[i] = nullptr;
                int lastRow = i + 1;
                for (int j = i + 1; j < mNumRows; j++) {
                    mRows[j - 1] = mRows[j];
                    if (mRows[j - 1]->mVariable->mDefinitionId == j) {
                        mRows[j - 1]->mVariable->mDefinitionId = j - 1;
                    }
                    lastRow = j;
                }
                if (lastRow < mNumRows) {
                    mRows[lastRow] = nullptr;
                }
                mNumRows--;
                i--;
            }
        }
        hasSimpleDefinition = false;
    }
}

void LinearSystem::removeRow(ArrayRow* row) {
    if (row->mIsSimpleDefinition && row->mVariable != nullptr) {
        if (row->mVariable->mDefinitionId != -1) {
            for (int i = row->mVariable->mDefinitionId; i < mNumRows - 1; i++) {
                SolverVariable* rowVariable = mRows[i + 1]->mVariable;
                if (rowVariable->mDefinitionId == i + 1) {
                    rowVariable->mDefinitionId = i;
                }
                mRows[i] = mRows[i + 1];
            }
            mNumRows--;
        }
        if (!row->mVariable->isFinalValue) {
            row->mVariable->setFinalValue(this, row->mConstantValue);
        }
        if (OPTIMIZED_ENGINE) mCache->mOptimizedArrayRowPool.release(row);
        else mCache->mArrayRowPool.release(row);
    }
}

int LinearSystem::optimize(Row* goal, bool /*b*/) {
    if (sMetrics != nullptr) sMetrics->optimize++;
    bool done = false;
    int tries = 0;
    for (int i = 0; i < mNumColumns; i++) {
        mAlreadyTestedCandidates[i] = false;
    }
    while (!done) {
        if (sMetrics != nullptr) sMetrics->iterations++;
        tries++;
        if (tries >= 2 * mNumColumns) {
            return tries;
        }
        if (goal->getKey() != nullptr) {
            mAlreadyTestedCandidates[goal->getKey()->id] = true;
        }
        SolverVariable* pivotCandidate = goal->getPivotCandidate(this, mAlreadyTestedCandidates);
        if (pivotCandidate != nullptr) {
            if (mAlreadyTestedCandidates[pivotCandidate->id]) {
                return tries;
            } else {
                mAlreadyTestedCandidates[pivotCandidate->id] = true;
            }
        }
        if (pivotCandidate != nullptr) {
            float min = std::numeric_limits<float>::max();
            int pivotRowIndex = -1;
            for (int i = 0; i < mNumRows; i++) {
                ArrayRow* current = mRows[i];
                SolverVariable* variable = current->mVariable;
                if (variable->mType == SolverVariable::Type::UNRESTRICTED) continue;
                if (current->mIsSimpleDefinition) continue;
                if (current->hasVariable(pivotCandidate)) {
                    float a_j = current->variables->get(pivotCandidate);
                    if (a_j < 0) {
                        float value = -current->mConstantValue / a_j;
                        if (value < min) {
                            min = value;
                            pivotRowIndex = i;
                        }
                    }
                }
            }
            if (pivotRowIndex > -1) {
                ArrayRow* pivotEquation = mRows[pivotRowIndex];
                pivotEquation->mVariable->mDefinitionId = -1;
                if (sMetrics != nullptr) sMetrics->pivots++;
                pivotEquation->pivot(pivotCandidate);
                pivotEquation->mVariable->mDefinitionId = pivotRowIndex;
                pivotEquation->mVariable->updateReferencesWithNewDefinition(this, pivotEquation);
            }
        } else {
            done = true;
        }
    }
    return tries;
}

int LinearSystem::enforceBFS(Row* /*goal*/) {
    int tries = 0;
    bool infeasibleSystem = false;
    for (int i = 0; i < mNumRows; i++) {
        SolverVariable* variable = mRows[i]->mVariable;
        if (variable->mType == SolverVariable::Type::UNRESTRICTED) continue;
        if (mRows[i]->mConstantValue < 0) {
            infeasibleSystem = true;
            break;
        }
    }
    if (infeasibleSystem) {
        bool done = false;
        tries = 0;
        while (!done) {
            if (sMetrics != nullptr) sMetrics->bfs++;
            tries++;
            float min = std::numeric_limits<float>::max();
            int strength = 0;
            int pivotRowIndex = -1;
            int pivotColumnIndex = -1;
            for (int i = 0; i < mNumRows; i++) {
                ArrayRow* current = mRows[i];
                SolverVariable* variable = current->mVariable;
                if (variable->mType == SolverVariable::Type::UNRESTRICTED) continue;
                if (current->mIsSimpleDefinition) continue;
                if (current->mConstantValue < 0) {
                    if (SKIP_COLUMNS) {
                        const int size = current->variables->getCurrentSize();
                        for (int j = 0; j < size; j++) {
                            SolverVariable* candidate = current->variables->getVariable(j);
                            float a_j = current->variables->get(candidate);
                            if (a_j <= 0) continue;
                            for (int k = 0; k < SolverVariable::MAX_STRENGTH; k++) {
                                float value = candidate->mStrengthVector[k] / a_j;
                                if ((value < min && k == strength) || k > strength) {
                                    min = value;
                                    pivotRowIndex = i;
                                    pivotColumnIndex = candidate->id;
                                    strength = k;
                                }
                            }
                        }
                    } else {
                        for (int j = 1; j < mNumColumns; j++) {
                            SolverVariable* candidate = mCache->mIndexedVariables[j];
                            float a_j = current->variables->get(candidate);
                            if (a_j <= 0) continue;
                            for (int k = 0; k < SolverVariable::MAX_STRENGTH; k++) {
                                float value = candidate->mStrengthVector[k] / a_j;
                                if ((value < min && k == strength) || k > strength) {
                                    min = value;
                                    pivotRowIndex = i;
                                    pivotColumnIndex = j;
                                    strength = k;
                                }
                            }
                        }
                    }
                }
            }
            if (pivotRowIndex != -1) {
                ArrayRow* pivotEquation = mRows[pivotRowIndex];
                pivotEquation->mVariable->mDefinitionId = -1;
                if (sMetrics != nullptr) sMetrics->pivots++;
                pivotEquation->pivot(mCache->mIndexedVariables[pivotColumnIndex]);
                pivotEquation->mVariable->mDefinitionId = pivotRowIndex;
                pivotEquation->mVariable->updateReferencesWithNewDefinition(this, pivotEquation);
            } else {
                done = true;
            }
            if (tries > mNumColumns / 2) {
                done = true;
            }
        }
    }
    return tries;
}

void LinearSystem::computeValues() {
    for (int i = 0; i < mNumRows; i++) {
        ArrayRow* row = mRows[i];
        row->mVariable->computedValue = row->mConstantValue;
    }
}

void LinearSystem::increaseTableSize() {
    mTableSize *= 2;
    mRows.resize(mTableSize, nullptr);
    mCache->mIndexedVariables.resize(mTableSize, nullptr);
    delete[] mAlreadyTestedCandidates;
    mAlreadyTestedCandidates = new bool[mTableSize]();
    mMaxColumns = mTableSize;
    mMaxRows = mTableSize;
    if (sMetrics != nullptr) {
        sMetrics->tableSizeIncrease++;
        sMetrics->maxTableSize = std::max(sMetrics->maxTableSize, (int64_t)mTableSize);
        sMetrics->lastTableSize = sMetrics->maxTableSize;
    }
}

void LinearSystem::releaseRows() {
    if (OPTIMIZED_ENGINE) {
        for (int i = 0; i < mNumRows; i++) {
            ArrayRow* row = mRows[i];
            if (row != nullptr) mCache->mOptimizedArrayRowPool.release(row);
            mRows[i] = nullptr;
        }
    } else {
        for (int i = 0; i < mNumRows; i++) {
            ArrayRow* row = mRows[i];
            if (row != nullptr) mCache->mArrayRowPool.release(row);
            mRows[i] = nullptr;
        }
    }
}

void LinearSystem::reset() {
    for (int i = 0; i < (int)mCache->mIndexedVariables.size(); i++) {
        SolverVariable* variable = mCache->mIndexedVariables[i];
        if (variable != nullptr) variable->reset();
    }
    for (int i = 0; i < mPoolVariablesCount; i++) {
        if (mPoolVariables[i] != nullptr) mCache->mSolverVariablePool.release(mPoolVariables[i]);
    }
    mPoolVariablesCount = 0;
    for (int i = 0; i < (int)mCache->mIndexedVariables.size(); i++) {
        mCache->mIndexedVariables[i] = nullptr;
    }
    mVariables.clear();
    mVariablesID = 0;
    mGoal->clear();
    mNumColumns = 1;
    for (int i = 0; i < mNumRows; i++) {
        if (mRows[i] != nullptr) mRows[i]->mUsed = false;
    }
    releaseRows();
    mNumRows = 0;
    delete mTempGoal;
    mTempGoal = new ArrayRow(mCache); // OPTIMIZED_ENGINE == false
}

//----------------------------------------- display -----------------------------------------

void LinearSystem::displayReadableRows() { /* debug dump omitted */ }
void LinearSystem::displayRows() { /* debug dump omitted */ }
void LinearSystem::displayVariablesReadableRows() { /* debug dump omitted */ }
void LinearSystem::displaySystemInformation() { /* debug dump omitted */ }
void LinearSystem::displaySolverVariables() { /* debug dump omitted */ }

std::string LinearSystem::getDisplaySize(int n) {
    int64_t mb = ((int64_t)n * 4) / 1024 / 1024;
    if (mb > 0) return std::to_string(mb) + " Mb";
    int64_t kb = ((int64_t)n * 4) / 1024;
    if (kb > 0) return std::to_string(kb) + " Kb";
    return std::to_string((int64_t)n * 4) + " bytes";
}

std::string LinearSystem::getDisplayStrength(int strength) {
    if (strength == SolverVariable::STRENGTH_LOW) return "LOW";
    if (strength == SolverVariable::STRENGTH_MEDIUM) return "MEDIUM";
    if (strength == SolverVariable::STRENGTH_HIGH) return "HIGH";
    if (strength == SolverVariable::STRENGTH_HIGHEST) return "HIGHEST";
    if (strength == SolverVariable::STRENGTH_EQUALITY) return "EQUALITY";
    if (strength == SolverVariable::STRENGTH_FIXED) return "FIXED";
    if (strength == SolverVariable::STRENGTH_BARRIER) return "BARRIER";
    return "NONE";
}

//----------------------------------------- equations ---------------------------------------

void LinearSystem::addGreaterThan(SolverVariable* a, SolverVariable* b, int margin, int strength) {
    ArrayRow* row = createRow();
    SolverVariable* slack = createSlackVariable();
    slack->strength = 0;
    row->createRowGreaterThan(a, b, slack, margin);
    if (strength != SolverVariable::STRENGTH_FIXED) {
        float slackValue = row->variables->get(slack);
        addSingleError(row, (int)(-1 * slackValue), strength);
    }
    addConstraint(row);
}

void LinearSystem::addGreaterBarrier(SolverVariable* a, SolverVariable* b, int margin, bool /*hasMatchConstraintWidgets*/) {
    ArrayRow* row = createRow();
    SolverVariable* slack = createSlackVariable();
    slack->strength = 0;
    row->createRowGreaterThan(a, b, slack, margin);
    addConstraint(row);
}

void LinearSystem::addLowerThan(SolverVariable* a, SolverVariable* b, int margin, int strength) {
    ArrayRow* row = createRow();
    SolverVariable* slack = createSlackVariable();
    slack->strength = 0;
    row->createRowLowerThan(a, b, slack, margin);
    if (strength != SolverVariable::STRENGTH_FIXED) {
        float slackValue = row->variables->get(slack);
        addSingleError(row, (int)(-1 * slackValue), strength);
    }
    addConstraint(row);
}

void LinearSystem::addLowerBarrier(SolverVariable* a, SolverVariable* b, int margin, bool /*hasMatchConstraintWidgets*/) {
    ArrayRow* row = createRow();
    SolverVariable* slack = createSlackVariable();
    slack->strength = 0;
    row->createRowLowerThan(a, b, slack, margin);
    addConstraint(row);
}

void LinearSystem::addCentering(SolverVariable* a, SolverVariable* b, int m1, float bias,
                                SolverVariable* c, SolverVariable* d, int m2, int strength) {
    ArrayRow* row = createRow();
    row->createRowCentering(a, b, m1, bias, c, d, m2);
    if (strength != SolverVariable::STRENGTH_FIXED) {
        row->addError(this, strength);
    }
    addConstraint(row);
}

void LinearSystem::addRatio(SolverVariable* a, SolverVariable* b, SolverVariable* c, SolverVariable* d,
                            float ratio, int strength) {
    ArrayRow* row = createRow();
    row->createRowDimensionRatio(a, b, c, d, ratio);
    if (strength != SolverVariable::STRENGTH_FIXED) {
        row->addError(this, strength);
    }
    addConstraint(row);
}

void LinearSystem::addSynonym(SolverVariable* a, SolverVariable* b, int margin) {
    if (a->mDefinitionId == -1 && margin == 0) {
        if (b->mIsSynonym) {
            margin += (int)b->mSynonymDelta;
            b = mCache->mIndexedVariables[b->mSynonym];
        }
        if (a->mIsSynonym) {
            margin -= (int)a->mSynonymDelta;
            a = mCache->mIndexedVariables[a->mSynonym];
        } else {
            a->setSynonym(this, b, 0);
        }
    } else {
        addEquality(a, b, margin, SolverVariable::STRENGTH_FIXED);
    }
}

ArrayRow* LinearSystem::addEquality(SolverVariable* a, SolverVariable* b, int margin, int strength) {
    if (sMetrics != nullptr) sMetrics->mSimpleEquations++;
    if (USE_BASIC_SYNONYMS && strength == SolverVariable::STRENGTH_FIXED
            && b->isFinalValue && a->mDefinitionId == -1) {
        a->setFinalValue(this, b->computedValue + margin);
        return nullptr;
    }
    ArrayRow* row = createRow();
    row->createRowEquals(a, b, margin);
    if (strength != SolverVariable::STRENGTH_FIXED) {
        row->addError(this, strength);
    }
    addConstraint(row);
    return row;
}

void LinearSystem::addEquality(SolverVariable* a, int value) {
    if (sMetrics != nullptr) sMetrics->mSimpleEquations++;
    if (USE_BASIC_SYNONYMS && a->mDefinitionId == -1) {
        a->setFinalValue(this, value);
        for (int i = 0; i < mVariablesID + 1; i++) {
            SolverVariable* variable = mCache->mIndexedVariables[i];
            if (variable != nullptr && variable->mIsSynonym && variable->mSynonym == a->id) {
                variable->setFinalValue(this, value + variable->mSynonymDelta);
            }
        }
        return;
    }
    int idx = a->mDefinitionId;
    if (a->mDefinitionId != -1) {
        ArrayRow* row = mRows[idx];
        if (row->mIsSimpleDefinition) {
            row->mConstantValue = value;
        } else {
            if (row->variables->getCurrentSize() == 0) {
                row->mIsSimpleDefinition = true;
                row->mConstantValue = value;
            } else {
                ArrayRow* newRow = createRow();
                newRow->createRowEquals(a, value);
                addConstraint(newRow);
            }
        }
    } else {
        ArrayRow* row = createRow();
        row->createRowDefinition(a, value);
        addConstraint(row);
    }
}

ArrayRow* LinearSystem::createRowDimensionPercent(LinearSystem* linearSystem,
        SolverVariable* variableA,
        SolverVariable* variableC, float percent) {
    ArrayRow* row = linearSystem->createRow();
    return row->createRowDimensionPercent(variableA, variableC, percent);
}

} // namespace cdroid
