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
 */
#include <widgetEx/constraintlayout/core/arraylinkedvariables.h>
#include <widgetEx/constraintlayout/core/cache.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

#include <string>

namespace cdroid {

float ArrayLinkedVariables::sEpsilon = 0.001f;

ArrayLinkedVariables::ArrayLinkedVariables(ArrayRow* arrayRow, Cache* cache)
    : mRow(arrayRow)
    , mCache(cache)
    , mArrayIndices(mRowSize, 0)
    , mArrayNextIndices(mRowSize, 0)
    , mArrayValues(mRowSize, 0.0f) {
    // Java DEBUG path (fill mArrayIndices with NONE) skipped, DEBUG == false.
}

void ArrayLinkedVariables::put(SolverVariable* variable, float value) {
    if (value == 0) {
        remove(variable, true);
        return;
    }
    const int len = (int)mArrayIndices.size();
    // Special casing empty list...
    if (mHead == NONE) {
        mHead = 0;
        mArrayValues[mHead] = value;
        mArrayIndices[mHead] = variable->id;
        mArrayNextIndices[mHead] = NONE;
        variable->usageInRowCount++;
        variable->addToRow(mRow);
        mCurrentSize++;
        if (!mDidFillOnce) {
            mLast++;
            if (mLast >= len) {
                mDidFillOnce = true;
                mLast = len - 1;
            }
        }
        return;
    }
    int current = mHead;
    int previous = NONE;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        if (mArrayIndices[current] == variable->id) {
            mArrayValues[current] = value;
            return;
        }
        if (mArrayIndices[current] < variable->id) {
            previous = current;
        }
        current = mArrayNextIndices[current];
        counter++;
    }

    // Not found, we need to insert. First, find an available spot.
    int availableIndice = mLast + 1;
    if (mDidFillOnce) {
        if (mArrayIndices[mLast] == NONE) {
            availableIndice = mLast;
        } else {
            availableIndice = len;
        }
    }
    if (availableIndice >= len) {
        if (mCurrentSize < len) {
            for (int i = 0; i < len; i++) {
                if (mArrayIndices[i] == NONE) {
                    availableIndice = i;
                    break;
                }
            }
        }
    }
    // grow as needed
    if (availableIndice >= len) {
        availableIndice = len;
        mRowSize *= 2;
        mDidFillOnce = false;
        mLast = availableIndice - 1;
        mArrayValues.resize(mRowSize, 0.0f);
        mArrayIndices.resize(mRowSize, 0);
        mArrayNextIndices.resize(mRowSize, 0);
    }

    mArrayIndices[availableIndice] = variable->id;
    mArrayValues[availableIndice] = value;
    if (previous != NONE) {
        mArrayNextIndices[availableIndice] = mArrayNextIndices[previous];
        mArrayNextIndices[previous] = availableIndice;
    } else {
        mArrayNextIndices[availableIndice] = mHead;
        mHead = availableIndice;
    }
    variable->usageInRowCount++;
    variable->addToRow(mRow);
    mCurrentSize++;
    if (!mDidFillOnce) {
        mLast++;
    }
    if (mCurrentSize >= (int)mArrayIndices.size()) {
        mDidFillOnce = true;
    }
    if (mLast >= (int)mArrayIndices.size()) {
        mDidFillOnce = true;
        mLast = (int)mArrayIndices.size() - 1;
    }
}

void ArrayLinkedVariables::add(SolverVariable* variable, float value, bool removeFromDefinition) {
    if (value > -sEpsilon && value < sEpsilon) {
        return;
    }
    const int len = (int)mArrayIndices.size();
    // Special casing empty list...
    if (mHead == NONE) {
        mHead = 0;
        mArrayValues[mHead] = value;
        mArrayIndices[mHead] = variable->id;
        mArrayNextIndices[mHead] = NONE;
        variable->usageInRowCount++;
        variable->addToRow(mRow);
        mCurrentSize++;
        if (!mDidFillOnce) {
            mLast++;
            if (mLast >= len) {
                mDidFillOnce = true;
                mLast = len - 1;
            }
        }
        return;
    }
    int current = mHead;
    int previous = NONE;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        int idx = mArrayIndices[current];
        if (idx == variable->id) {
            float v = mArrayValues[current] + value;
            if (v > -sEpsilon && v < sEpsilon) {
                v = 0;
            }
            mArrayValues[current] = v;
            // Possibly delete immediately
            if (v == 0) {
                if (current == mHead) {
                    mHead = mArrayNextIndices[current];
                } else {
                    mArrayNextIndices[previous] = mArrayNextIndices[current];
                }
                if (removeFromDefinition) {
                    variable->removeFromRow(mRow);
                }
                if (mDidFillOnce) {
                    mLast = current;
                }
                variable->usageInRowCount--;
                mCurrentSize--;
            }
            return;
        }
        if (mArrayIndices[current] < variable->id) {
            previous = current;
        }
        current = mArrayNextIndices[current];
        counter++;
    }

    // Not found, we need to insert. First, find an available spot.
    int availableIndice = mLast + 1;
    if (mDidFillOnce) {
        if (mArrayIndices[mLast] == NONE) {
            availableIndice = mLast;
        } else {
            availableIndice = len;
        }
    }
    if (availableIndice >= len) {
        if (mCurrentSize < len) {
            for (int i = 0; i < len; i++) {
                if (mArrayIndices[i] == NONE) {
                    availableIndice = i;
                    break;
                }
            }
        }
    }
    if (availableIndice >= len) {
        availableIndice = len;
        mRowSize *= 2;
        mDidFillOnce = false;
        mLast = availableIndice - 1;
        mArrayValues.resize(mRowSize, 0.0f);
        mArrayIndices.resize(mRowSize, 0);
        mArrayNextIndices.resize(mRowSize, 0);
    }

    mArrayIndices[availableIndice] = variable->id;
    mArrayValues[availableIndice] = value;
    if (previous != NONE) {
        mArrayNextIndices[availableIndice] = mArrayNextIndices[previous];
        mArrayNextIndices[previous] = availableIndice;
    } else {
        mArrayNextIndices[availableIndice] = mHead;
        mHead = availableIndice;
    }
    variable->usageInRowCount++;
    variable->addToRow(mRow);
    mCurrentSize++;
    if (!mDidFillOnce) {
        mLast++;
    }
    if (mLast >= (int)mArrayIndices.size()) {
        mDidFillOnce = true;
        mLast = (int)mArrayIndices.size() - 1;
    }
}

float ArrayLinkedVariables::use(ArrayRow* definition, bool removeFromDefinition) {
    float value = get(definition->mVariable);
    remove(definition->mVariable, removeFromDefinition);
    ArrayRow::ArrayRowVariables* definitionVariables = definition->variables;
    int definitionSize = definitionVariables->getCurrentSize();
    for (int i = 0; i < definitionSize; i++) {
        SolverVariable* definitionVariable = definitionVariables->getVariable(i);
        float definitionValue = definitionVariables->get(definitionVariable);
        this->add(definitionVariable, definitionValue * value, removeFromDefinition);
    }
    return value;
}

float ArrayLinkedVariables::remove(SolverVariable* variable, bool removeFromDefinition) {
    if (mCandidate == variable) {
        mCandidate = nullptr;
    }
    if (mHead == NONE) {
        return 0;
    }
    int current = mHead;
    int previous = NONE;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        int idx = mArrayIndices[current];
        if (idx == variable->id) {
            if (current == mHead) {
                mHead = mArrayNextIndices[current];
            } else {
                mArrayNextIndices[previous] = mArrayNextIndices[current];
            }
            if (removeFromDefinition) {
                variable->removeFromRow(mRow);
            }
            variable->usageInRowCount--;
            mCurrentSize--;
            mArrayIndices[current] = NONE;
            if (mDidFillOnce) {
                mLast = current;
            }
            return mArrayValues[current];
        }
        previous = current;
        current = mArrayNextIndices[current];
        counter++;
    }
    return 0;
}

void ArrayLinkedVariables::clear() {
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        SolverVariable* variable = mCache->mIndexedVariables[mArrayIndices[current]];
        if (variable != nullptr) {
            variable->removeFromRow(mRow);
        }
        current = mArrayNextIndices[current];
        counter++;
    }
    mHead = NONE;
    mLast = NONE;
    mDidFillOnce = false;
    mCurrentSize = 0;
}

bool ArrayLinkedVariables::contains(SolverVariable* variable) {
    if (mHead == NONE) {
        return false;
    }
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        if (mArrayIndices[current] == variable->id) {
            return true;
        }
        current = mArrayNextIndices[current];
        counter++;
    }
    return false;
}

int ArrayLinkedVariables::indexOf(SolverVariable* variable) {
    if (mHead == NONE) {
        return -1;
    }
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        if (mArrayIndices[current] == variable->id) {
            return current;
        }
        current = mArrayNextIndices[current];
        counter++;
    }
    return -1;
}

bool ArrayLinkedVariables::hasAtLeastOnePositiveVariable() {
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        if (mArrayValues[current] > 0) {
            return true;
        }
        current = mArrayNextIndices[current];
        counter++;
    }
    return false;
}

void ArrayLinkedVariables::invert() {
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        mArrayValues[current] *= -1;
        current = mArrayNextIndices[current];
        counter++;
    }
}

void ArrayLinkedVariables::divideByAmount(float amount) {
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        mArrayValues[current] /= amount;
        current = mArrayNextIndices[current];
        counter++;
    }
}

int ArrayLinkedVariables::getCurrentSize() {
    return mCurrentSize;
}

int ArrayLinkedVariables::getHead() {
    return mHead;
}

int ArrayLinkedVariables::getId(int index) {
    return mArrayIndices[index];
}

float ArrayLinkedVariables::getValue(int index) {
    return mArrayValues[index];
}

int ArrayLinkedVariables::getNextIndice(int index) {
    return mArrayNextIndices[index];
}

SolverVariable* ArrayLinkedVariables::getPivotCandidate() {
    if (mCandidate == nullptr) {
        int current = mHead;
        int counter = 0;
        SolverVariable* pivot = nullptr;
        while (current != NONE && counter < mCurrentSize) {
            if (mArrayValues[current] < 0) {
                SolverVariable* v = mCache->mIndexedVariables[mArrayIndices[current]];
                if (pivot == nullptr || pivot->strength < v->strength) {
                    pivot = v;
                }
            }
            current = mArrayNextIndices[current];
            counter++;
        }
        return pivot;
    }
    return mCandidate;
}

SolverVariable* ArrayLinkedVariables::getVariable(int index) {
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        if (counter == index) {
            return mCache->mIndexedVariables[mArrayIndices[current]];
        }
        current = mArrayNextIndices[current];
        counter++;
    }
    return nullptr;
}

float ArrayLinkedVariables::getVariableValue(int index) {
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        if (counter == index) {
            return mArrayValues[current];
        }
        current = mArrayNextIndices[current];
        counter++;
    }
    return 0;
}

float ArrayLinkedVariables::get(SolverVariable* v) {
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        if (mArrayIndices[current] == v->id) {
            return mArrayValues[current];
        }
        current = mArrayNextIndices[current];
        counter++;
    }
    return 0;
}

int ArrayLinkedVariables::sizeInBytes() {
    int size = 0;
    size += 3 * ((int)mArrayIndices.size() * 4);
    size += 9 * 4;
    return size;
}

void ArrayLinkedVariables::display() {
    // Java: System.out.print debug dump. No-op in port (debug only).
}

std::string ArrayLinkedVariables::toString() {
    std::string result;
    int current = mHead;
    int counter = 0;
    while (current != NONE && counter < mCurrentSize) {
        result += " -> ";
        result += std::to_string(mArrayValues[current]) + " : ";
        SolverVariable* v = mCache->mIndexedVariables[mArrayIndices[current]];
        result += (v != nullptr) ? v->toString() : std::string("null");
        current = mArrayNextIndices[current];
        counter++;
    }
    return result;
}

} // namespace cdroid
