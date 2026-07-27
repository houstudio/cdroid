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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.SolverVariableValues.
 *
 * ArrayRowVariables backed by an array-based linked list coupled with a custom
 * hashmap. Used by ValuesRow when OPTIMIZED_ENGINE is on.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_SOLVER_VARIABLE_VALUES_H__
#define __CONSTRAINTLAYOUT_CORE_SOLVER_VARIABLE_VALUES_H__

#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <vector>
#include <string>

namespace cdroid {

class SolverVariable;
class Cache;

class SolverVariableValues : public ArrayRow::ArrayRowVariables {
  private:
    static float sEpsilon;
    const int mNone = -1;
    int mSize = 16;
    int mHashSize = 16;

    std::vector<int>   mKeys;
    std::vector<int>   mNextKeys;
    std::vector<int>   mVariables;
    std::vector<float> mValues;
    std::vector<int>   mPrevious;
    std::vector<int>   mNext;
    int   mCount = 0;
    int   mHead = -1;

    ArrayRow* mRow;  // owner (not owned)
    Cache*    mCache;

  public:
    SolverVariableValues(ArrayRow* row, Cache* cache);

    int getCurrentSize() override;
    SolverVariable* getVariable(int index) override;
    float getVariableValue(int index) override;
    bool contains(SolverVariable* variable) override;
    int indexOf(SolverVariable* variable) override;
    float get(SolverVariable* variable) override;
    void display() override;
    std::string toString();
    void clear() override;
    void put(SolverVariable* variable, float value) override;
    int sizeInBytes() override;
    float remove(SolverVariable* v, bool removeFromDefinition) override;
    void add(SolverVariable* v, float value, bool removeFromDefinition) override;
    float use(ArrayRow* definition, bool removeFromDefinition) override;
    void invert() override;
    void divideByAmount(float amount) override;

  private:
    void increaseSize();
    void addToHashMap(SolverVariable* variable, int index);
    void removeFromHashMap(SolverVariable* variable);
    void addVariable(int index, SolverVariable* variable, float value);
    int  findEmptySlot();
    void insertVariable(int index, SolverVariable* variable, float value);
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_SOLVER_VARIABLE_VALUES_H__
