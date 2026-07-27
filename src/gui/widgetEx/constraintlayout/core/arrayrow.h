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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.ArrayRow.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_ARRAY_ROW_H__
#define __CONSTRAINTLAYOUT_CORE_ARRAY_ROW_H__

#include <widgetEx/constraintlayout/core/linearsystem.h>
#include <vector>
#include <string>

namespace cdroid {

class SolverVariable; // forward
class Cache;

/**
 * Ported verbatim from androidx.constraintlayout.core.ArrayRow.
 * Implements LinearSystem::Row. Owns an ArrayRowVariables instance
 * (ArrayLinkedVariables by default, created in the Cache constructor overload).
 */
class ArrayRow : public LinearSystem::Row {
  public:
    /**
     * Interface for the variable store of a row (Java: ArrayRow.ArrayRowVariables).
     * Implemented by ArrayLinkedVariables (and SolverVariableValues).
     */
    class ArrayRowVariables {
      public:
        virtual ~ArrayRowVariables() = default;
        virtual int getCurrentSize() = 0;
        virtual SolverVariable* getVariable(int index) = 0;
        virtual float getVariableValue(int index) = 0;
        virtual float get(SolverVariable* variable) = 0;
        virtual int indexOf(SolverVariable* variable) = 0;
        virtual void display() = 0;
        virtual void clear() = 0;
        virtual bool contains(SolverVariable* variable) = 0;
        virtual void put(SolverVariable* variable, float value) = 0;
        virtual int sizeInBytes() = 0;
        virtual void invert() = 0;
        virtual float remove(SolverVariable* v, bool removeFromDefinition) = 0;
        virtual void divideByAmount(float amount) = 0;
        virtual void add(SolverVariable* v, float value, bool removeFromDefinition) = 0;
        virtual float use(ArrayRow* definition, bool removeFromDefinition) = 0;
    };

    // --- fields (Java package-private -> public) ---
    SolverVariable* mVariable = nullptr;
    float  mConstantValue = 0;
    bool   mUsed = false;
    ArrayRowVariables* variables = nullptr; // owned (new ArrayLinkedVariables)
    bool   mIsSimpleDefinition = false;
    std::vector<SolverVariable*> mVariablesToUpdate;

    ArrayRow();
    explicit ArrayRow(Cache* cache);
    virtual ~ArrayRow();

    bool hasKeyVariable();
    std::string toReadableString();
    virtual std::string toString(); // returns toReadableString()
    void reset();
    bool hasVariable(SolverVariable* v);

    ArrayRow* createRowDefinition(SolverVariable* variable, int value);
    ArrayRow* createRowEquals(SolverVariable* variable, int value);
    ArrayRow* createRowEquals(SolverVariable* variableA, SolverVariable* variableB, int margin);
    ArrayRow* addSingleError(SolverVariable* error, int sign);
    ArrayRow* createRowGreaterThan(SolverVariable* variableA, SolverVariable* variableB,
                                   SolverVariable* slack, int margin);
    ArrayRow* createRowGreaterThan(SolverVariable* a, int b, SolverVariable* slack);
    ArrayRow* createRowLowerThan(SolverVariable* variableA, SolverVariable* variableB,
                                 SolverVariable* slack, int margin);
    ArrayRow* createRowEqualMatchDimensions(float currentWeight, float totalWeights, float nextWeight,
                                            SolverVariable* variableStartA, SolverVariable* variableEndA,
                                            SolverVariable* variableStartB, SolverVariable* variableEndB);
    ArrayRow* createRowEqualDimension(float currentWeight, float totalWeights, float nextWeight,
                                      SolverVariable* variableStartA, int marginStartA,
                                      SolverVariable* variableEndA, int marginEndA,
                                      SolverVariable* variableStartB, int marginStartB,
                                      SolverVariable* variableEndB, int marginEndB);
    ArrayRow* createRowCentering(SolverVariable* variableA, SolverVariable* variableB,
                                 int marginA, float bias, SolverVariable* variableC,
                                 SolverVariable* variableD, int marginB);
    ArrayRow* addError(LinearSystem* system, int strength);
    ArrayRow* createRowDimensionPercent(SolverVariable* variableA, SolverVariable* variableC, float percent);
    ArrayRow* createRowDimensionRatio(SolverVariable* variableA, SolverVariable* variableB,
                                      SolverVariable* variableC, SolverVariable* variableD, float ratio);
    ArrayRow* createRowWithAngle(SolverVariable* at, SolverVariable* ab,
                                 SolverVariable* bt, SolverVariable* bb, float angleComponent);

    int  sizeInBytes();
    void ensurePositiveConstant();

    bool chooseSubject(LinearSystem* system);
    SolverVariable* chooseSubjectInVariables(LinearSystem* system);
    void pivot(SolverVariable* v);

    // --- LinearSystem::Row overrides ---
    bool isEmpty() override;
    void updateFromRow(LinearSystem* system, ArrayRow* definition, bool b) override;
    void updateFromFinalVariable(LinearSystem* system, SolverVariable* variable,
                                 bool removeFromDefinition) override;
    void updateFromSynonymVariable(LinearSystem* system, SolverVariable* variable,
                                   bool removeFromDefinition);
    SolverVariable* pickPivot(SolverVariable* exclude);
    SolverVariable* getPivotCandidate(LinearSystem* system, bool* avoid) override;
    void clear() override;
    void initFromRow(Row* row) override;
    void addError(SolverVariable* error) override; // Row overload (weight by strength)
    SolverVariable* getKey() override;
    void updateFromSystem(LinearSystem* system) override;

  private:
    SolverVariable* pickPivotInVariables(bool* avoid, SolverVariable* exclude);
    bool isNew(SolverVariable* variable, LinearSystem* system);
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_ARRAY_ROW_H__
