/*
 * Copyright (C) 2016 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.ArrayLinkedVariables.
 *
 * Stores a set of variables and their values in an array-based linked list.
 * Java int[]/float[] + Arrays.copyOf are mapped to std::vector + resize.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_ARRAY_LINKED_VARIABLES_H__
#define __CONSTRAINTLAYOUT_CORE_ARRAY_LINKED_VARIABLES_H__

#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <vector>

namespace cdroid {

class Cache;
class SolverVariable;

/**
 * Ported verbatim from androidx.constraintlayout.core.ArrayLinkedVariables.
 * Implements ArrayRow::ArrayRowVariables.
 */
class ArrayLinkedVariables : public ArrayRow::ArrayRowVariables {
private:
    static const int NONE = -1;
    static float sEpsilon;

    int   mCurrentSize = 0;   // accessed by ArrayRow/LinearSystem -> public below
    ArrayRow* mRow;           // owner (not owned)
    Cache*  mCache;           // system-wide cache (not owned)
    int     mRowSize = 8;
    SolverVariable* mCandidate = nullptr;
    std::vector<int>   mArrayIndices;     // indexes into mCache->mIndexedVariables
    std::vector<int>   mArrayNextIndices; // indexes into mArrayIndices
    std::vector<float> mArrayValues;      // value associated with mArrayIndices
    int     mHead = NONE;
    int     mLast = NONE;
    bool    mDidFillOnce = false;

public:
    ArrayLinkedVariables(ArrayRow* arrayRow, Cache* cache);

    void put(SolverVariable* variable, float value) override;
    void add(SolverVariable* variable, float value, bool removeFromDefinition) override;
    float use(ArrayRow* definition, bool removeFromDefinition) override;
    float remove(SolverVariable* variable, bool removeFromDefinition) override;
    void clear() override;
    bool contains(SolverVariable* variable) override;
    int indexOf(SolverVariable* variable) override;
    void invert() override;
    void divideByAmount(float amount) override;
    int getCurrentSize() override;
    SolverVariable* getVariable(int index) override;
    float getVariableValue(int index) override;
    float get(SolverVariable* v) override;
    int sizeInBytes() override;
    void display() override;

    // package-private extras (used by LinearSystem)
    bool hasAtLeastOnePositiveVariable();
    int getHead();
    int getId(int index);
    float getValue(int index);
    int getNextIndice(int index);
    SolverVariable* getPivotCandidate();

    std::string toString();
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_ARRAY_LINKED_VARIABLES_H__
