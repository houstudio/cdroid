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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.SolverVariableValues.
 */
#include <widgetEx/constraintlayout/core/solvervariablevalues.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <widgetEx/constraintlayout/core/cache.h>

#include <string>

namespace cdroid {

float SolverVariableValues::sEpsilon = 0.001f;

SolverVariableValues::SolverVariableValues(ArrayRow* row, Cache* cache)
    : mRow(row), mCache(cache) {
    mKeys.assign(mHashSize, 0);
    mNextKeys.assign(mSize, 0);
    mVariables.assign(mSize, 0);
    mValues.assign(mSize, 0);
    mPrevious.assign(mSize, 0);
    mNext.assign(mSize, 0);
    clear();
}

int SolverVariableValues::getCurrentSize() {
    return mCount;
}

SolverVariable* SolverVariableValues::getVariable(int index) {
    const int count = mCount;
    if (count == 0) {
        return nullptr;
    }
    int j = mHead;
    for (int i = 0; i < count; i++) {
        if (i == index && j != mNone) {
            return mCache->mIndexedVariables[mVariables[j]];
        }
        j = mNext[j];
        if (j == mNone) {
            break;
        }
    }
    return nullptr;
}

float SolverVariableValues::getVariableValue(int index) {
    const int count = mCount;
    int j = mHead;
    for (int i = 0; i < count; i++) {
        if (i == index) {
            return mValues[j];
        }
        j = mNext[j];
        if (j == mNone) {
            break;
        }
    }
    return 0;
}

bool SolverVariableValues::contains(SolverVariable* variable) {
    return indexOf(variable) != mNone;
}

int SolverVariableValues::indexOf(SolverVariable* variable) {
    if (mCount == 0 || variable == nullptr) {
        return mNone;
    }
    int id = variable->id;
    int key = id % mHashSize;
    key = mKeys[key];
    if (key == mNone) {
        return mNone;
    }
    if (mVariables[key] == id) {
        return key;
    }
    while (mNextKeys[key] != mNone && mVariables[mNextKeys[key]] != id) {
        key = mNextKeys[key];
    }
    if (mNextKeys[key] == mNone) {
        return mNone;
    }
    if (mVariables[mNextKeys[key]] == id) {
        return mNextKeys[key];
    }
    return mNone;
}

float SolverVariableValues::get(SolverVariable* variable) {
    const int index = indexOf(variable);
    if (index != mNone) {
        return mValues[index];
    }
    return 0;
}

void SolverVariableValues::display() {
    // Java: System.out debug dump. No-op in port.
}

std::string SolverVariableValues::toString() {
    std::string str = "{ ";
    const int count = mCount;
    for (int i = 0; i < count; i++) {
        SolverVariable* v = getVariable(i);
        if (v == nullptr) {
            continue;
        }
        str += v->toString() + " = " + std::to_string(getVariableValue(i)) + " ";
    }
    str += "}";
    return str;
}

void SolverVariableValues::clear() {
    const int count = mCount;
    for (int i = 0; i < count; i++) {
        SolverVariable* v = getVariable(i);
        if (v != nullptr) {
            v->removeFromRow(mRow);
        }
    }
    for (int i = 0; i < mSize; i++) {
        mVariables[i] = mNone;
        mNextKeys[i] = mNone;
    }
    for (int i = 0; i < mHashSize; i++) {
        mKeys[i] = mNone;
    }
    mCount = 0;
    mHead = -1;
}

void SolverVariableValues::increaseSize() {
    int size = this->mSize * 2;
    mVariables.resize(size, 0);
    mValues.resize(size, 0);
    mPrevious.resize(size, 0);
    mNext.resize(size, 0);
    mNextKeys.resize(size, 0);
    for (int i = this->mSize; i < size; i++) {
        mVariables[i] = mNone;
        mNextKeys[i] = mNone;
    }
    this->mSize = size;
}

void SolverVariableValues::addToHashMap(SolverVariable* variable, int index) {
    int hash = variable->id % mHashSize;
    int key = mKeys[hash];
    if (key == mNone) {
        mKeys[hash] = index;
    } else {
        while (mNextKeys[key] != mNone) {
            key = mNextKeys[key];
        }
        mNextKeys[key] = index;
    }
    mNextKeys[index] = mNone;
}

void SolverVariableValues::removeFromHashMap(SolverVariable* variable) {
    int hash = variable->id % mHashSize;
    int key = mKeys[hash];
    if (key == mNone) {
        return;
    }
    int id = variable->id;
    if (mVariables[key] == id) {
        mKeys[hash] = mNextKeys[key];
        mNextKeys[key] = mNone;
    } else {
        while (mNextKeys[key] != mNone && mVariables[mNextKeys[key]] != id) {
            key = mNextKeys[key];
        }
        int currentKey = mNextKeys[key];
        if (currentKey != mNone && mVariables[currentKey] == id) {
            mNextKeys[key] = mNextKeys[currentKey];
            mNextKeys[currentKey] = mNone;
        }
    }
}

void SolverVariableValues::addVariable(int index, SolverVariable* variable, float value) {
    mVariables[index] = variable->id;
    mValues[index] = value;
    mPrevious[index] = mNone;
    mNext[index] = mNone;
    variable->addToRow(mRow);
    variable->usageInRowCount++;
    mCount++;
}

int SolverVariableValues::findEmptySlot() {
    for (int i = 0; i < mSize; i++) {
        if (mVariables[i] == mNone) {
            return i;
        }
    }
    return -1;
}

void SolverVariableValues::insertVariable(int index, SolverVariable* variable, float value) {
    int availableSlot = findEmptySlot();
    addVariable(availableSlot, variable, value);
    if (index != mNone) {
        mPrevious[availableSlot] = index;
        mNext[availableSlot] = mNext[index];
        mNext[index] = availableSlot;
    } else {
        mPrevious[availableSlot] = mNone;
        if (mCount > 0) {
            mNext[availableSlot] = mHead;
            mHead = availableSlot;
        } else {
            mNext[availableSlot] = mNone;
        }
    }
    if (mNext[availableSlot] != mNone) {
        mPrevious[mNext[availableSlot]] = availableSlot;
    }
    addToHashMap(variable, availableSlot);
}

void SolverVariableValues::put(SolverVariable* variable, float value) {
    if (value > -sEpsilon && value < sEpsilon) {
        remove(variable, true);
        return;
    }
    if (mCount == 0) {
        addVariable(0, variable, value);
        addToHashMap(variable, 0);
        mHead = 0;
    } else {
        const int index = indexOf(variable);
        if (index != mNone) {
            mValues[index] = value;
        } else {
            if (mCount + 1 >= mSize) {
                increaseSize();
            }
            const int count = mCount;
            int previousItem = -1;
            int j = mHead;
            for (int i = 0; i < count; i++) {
                if (mVariables[j] == variable->id) {
                    mValues[j] = value;
                    return;
                }
                if (mVariables[j] < variable->id) {
                    previousItem = j;
                }
                j = mNext[j];
                if (j == mNone) {
                    break;
                }
            }
            insertVariable(previousItem, variable, value);
        }
    }
}

int SolverVariableValues::sizeInBytes() {
    return 0;
}

float SolverVariableValues::remove(SolverVariable* v, bool removeFromDefinition) {
    int index = indexOf(v);
    if (index == mNone) {
        return 0;
    }
    removeFromHashMap(v);
    float value = mValues[index];
    if (mHead == index) {
        mHead = mNext[index];
    }
    mVariables[index] = mNone;
    if (mPrevious[index] != mNone) {
        mNext[mPrevious[index]] = mNext[index];
    }
    if (mNext[index] != mNone) {
        mPrevious[mNext[index]] = mPrevious[index];
    }
    mCount--;
    v->usageInRowCount--;
    if (removeFromDefinition) {
        v->removeFromRow(mRow);
    }
    return value;
}

void SolverVariableValues::add(SolverVariable* v, float value, bool removeFromDefinition) {
    if (value > -sEpsilon && value < sEpsilon) {
        return;
    }
    const int index = indexOf(v);
    if (index == mNone) {
        put(v, value);
    } else {
        mValues[index] += value;
        if (mValues[index] > -sEpsilon && mValues[index] < sEpsilon) {
            mValues[index] = 0;
            remove(v, removeFromDefinition);
        }
    }
}

float SolverVariableValues::use(ArrayRow* definition, bool removeFromDefinition) {
    float value = get(definition->mVariable);
    remove(definition->mVariable, removeFromDefinition);
    // Java has two `if (false)` dead branches; the live branch below iterates
    // the definition's backing arrays directly via a SolverVariableValues cast.
    SolverVariableValues* localDef = dynamic_cast<SolverVariableValues*>(definition->variables);
    if (localDef == nullptr) {
        return value;
    }
    const int definitionSize = localDef->getCurrentSize();
    int j = 0;
    for (int i = 0; j < definitionSize; i++) {
        if (localDef->mVariables[i] != mNone) {
            float definitionValue = localDef->mValues[i];
            SolverVariable* definitionVariable = mCache->mIndexedVariables[localDef->mVariables[i]];
            add(definitionVariable, definitionValue * value, removeFromDefinition);
            j++;
        }
    }
    return value;
}

void SolverVariableValues::invert() {
    const int count = mCount;
    int j = mHead;
    for (int i = 0; i < count; i++) {
        mValues[j] *= -1;
        j = mNext[j];
        if (j == mNone) {
            break;
        }
    }
}

void SolverVariableValues::divideByAmount(float amount) {
    const int count = mCount;
    int j = mHead;
    for (int i = 0; i < count; i++) {
        mValues[j] /= amount;
        j = mNext[j];
        if (j == mNone) {
            break;
        }
    }
}

} // namespace cdroid
