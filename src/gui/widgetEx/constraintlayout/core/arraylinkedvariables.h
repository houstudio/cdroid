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
