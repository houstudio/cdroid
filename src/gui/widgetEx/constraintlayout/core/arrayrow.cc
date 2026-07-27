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
#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>
#include <widgetEx/constraintlayout/core/cache.h>
#include <widgetEx/constraintlayout/core/arraylinkedvariables.h>
#include <widgetEx/constraintlayout/core/linearsystem.h>

#include <string>

namespace cdroid {

ArrayRow::ArrayRow() {
}

ArrayRow::ArrayRow(Cache* cache) {
    variables = new ArrayLinkedVariables(this, cache);
}

ArrayRow::~ArrayRow() {
    delete variables;
    variables = nullptr;
}

bool ArrayRow::hasKeyVariable() {
    return !(
               (mVariable == nullptr)
               || (mVariable->mType != SolverVariable::Type::UNRESTRICTED
                   && mConstantValue < 0)
           );
}

std::string ArrayRow::toString() {
    return toReadableString();
}

std::string ArrayRow::toReadableString() {
    std::string s;
    if (mVariable == nullptr) {
        s += "0";
    } else {
        s += mVariable->toString();
    }
    s += " = ";
    bool addedVariable = false;
    if (mConstantValue != 0) {
        s += std::to_string(mConstantValue);
        addedVariable = true;
    }
    int count = variables->getCurrentSize();
    for (int i = 0; i < count; i++) {
        SolverVariable* v = variables->getVariable(i);
        if (v == nullptr) {
            continue;
        }
        float amount = variables->getVariableValue(i);
        if (amount == 0) {
            continue;
        }
        std::string name = v->toString();
        if (!addedVariable) {
            if (amount < 0) {
                s += "- ";
                amount *= -1;
            }
        } else {
            if (amount > 0) {
                s += " + ";
            } else {
                s += " - ";
                amount *= -1;
            }
        }
        if (amount == 1) {
            s += name;
        } else {
            s += std::to_string(amount) + " " + name;
        }
        addedVariable = true;
    }
    if (!addedVariable) {
        s += "0.0";
    }
    // Java: if (DEBUG) variables.display();  (DEBUG == false, omitted)
    return s;
}

void ArrayRow::reset() {
    mVariable = nullptr;
    variables->clear();
    mConstantValue = 0;
    mIsSimpleDefinition = false;
}

bool ArrayRow::hasVariable(SolverVariable* v) {
    return variables->contains(v);
}

ArrayRow* ArrayRow::createRowDefinition(SolverVariable* variable, int value) {
    this->mVariable = variable;
    variable->computedValue = value;
    mConstantValue = value;
    mIsSimpleDefinition = true;
    return this;
}

ArrayRow* ArrayRow::createRowEquals(SolverVariable* variable, int value) {
    if (value < 0) {
        mConstantValue = -1 * value;
        variables->put(variable, 1);
    } else {
        mConstantValue = value;
        variables->put(variable, -1);
    }
    return this;
}

ArrayRow* ArrayRow::createRowEquals(SolverVariable* variableA, SolverVariable* variableB, int margin) {
    bool inverse = false;
    if (margin != 0) {
        int m = margin;
        if (m < 0) {
            m = -1 * m;
            inverse = true;
        }
        mConstantValue = m;
    }
    if (!inverse) {
        variables->put(variableA, -1);
        variables->put(variableB, 1);
    } else {
        variables->put(variableA, 1);
        variables->put(variableB, -1);
    }
    return this;
}

ArrayRow* ArrayRow::addSingleError(SolverVariable* error, int sign) {
    variables->put(error, (float)sign);
    return this;
}

ArrayRow* ArrayRow::createRowGreaterThan(SolverVariable* variableA, SolverVariable* variableB,
        SolverVariable* slack, int margin) {
    bool inverse = false;
    if (margin != 0) {
        int m = margin;
        if (m < 0) {
            m = -1 * m;
            inverse = true;
        }
        mConstantValue = m;
    }
    if (!inverse) {
        variables->put(variableA, -1);
        variables->put(variableB, 1);
        variables->put(slack, 1);
    } else {
        variables->put(variableA, 1);
        variables->put(variableB, -1);
        variables->put(slack, -1);
    }
    return this;
}

ArrayRow* ArrayRow::createRowGreaterThan(SolverVariable* a, int b, SolverVariable* slack) {
    (void)slack;
    mConstantValue = b;
    variables->put(a, -1);
    return this;
}

ArrayRow* ArrayRow::createRowLowerThan(SolverVariable* variableA, SolverVariable* variableB,
                                       SolverVariable* slack, int margin) {
    bool inverse = false;
    if (margin != 0) {
        int m = margin;
        if (m < 0) {
            m = -1 * m;
            inverse = true;
        }
        mConstantValue = m;
    }
    if (!inverse) {
        variables->put(variableA, -1);
        variables->put(variableB, 1);
        variables->put(slack, -1);
    } else {
        variables->put(variableA, 1);
        variables->put(variableB, -1);
        variables->put(slack, 1);
    }
    return this;
}

ArrayRow* ArrayRow::createRowEqualMatchDimensions(float currentWeight, float totalWeights, float nextWeight,
        SolverVariable* variableStartA, SolverVariable* variableEndA,
        SolverVariable* variableStartB, SolverVariable* variableEndB) {
    mConstantValue = 0;
    if (totalWeights == 0 || (currentWeight == nextWeight)) {
        variables->put(variableStartA, 1);
        variables->put(variableEndA, -1);
        variables->put(variableEndB, 1);
        variables->put(variableStartB, -1);
    } else {
        if (currentWeight == 0) {
            variables->put(variableStartA, 1);
            variables->put(variableEndA, -1);
        } else if (nextWeight == 0) {
            variables->put(variableStartB, 1);
            variables->put(variableEndB, -1);
        } else {
            float cw = currentWeight / totalWeights;
            float nw = nextWeight / totalWeights;
            float w = cw / nw;
            variables->put(variableStartA, 1);
            variables->put(variableEndA, -1);
            variables->put(variableEndB, w);
            variables->put(variableStartB, -w);
        }
    }
    return this;
}

ArrayRow* ArrayRow::createRowEqualDimension(float currentWeight, float totalWeights, float nextWeight,
        SolverVariable* variableStartA, int marginStartA,
        SolverVariable* variableEndA, int marginEndA,
        SolverVariable* variableStartB, int marginStartB,
        SolverVariable* variableEndB, int marginEndB) {
    if (totalWeights == 0 || (currentWeight == nextWeight)) {
        mConstantValue = -marginStartA - marginEndA + marginStartB + marginEndB;
        variables->put(variableStartA, 1);
        variables->put(variableEndA, -1);
        variables->put(variableEndB, 1);
        variables->put(variableStartB, -1);
    } else {
        float cw = currentWeight / totalWeights;
        float nw = nextWeight / totalWeights;
        float w = cw / nw;
        mConstantValue = -marginStartA - marginEndA + w * marginStartB + w * marginEndB;
        variables->put(variableStartA, 1);
        variables->put(variableEndA, -1);
        variables->put(variableEndB, w);
        variables->put(variableStartB, -w);
    }
    return this;
}

ArrayRow* ArrayRow::createRowCentering(SolverVariable* variableA, SolverVariable* variableB,
                                       int marginA, float bias, SolverVariable* variableC,
                                       SolverVariable* variableD, int marginB) {
    if (variableB == variableC) {
        variables->put(variableA, 1);
        variables->put(variableD, 1);
        variables->put(variableB, -2);
        return this;
    }
    if (bias == 0.5f) {
        variables->put(variableA, 1);
        variables->put(variableB, -1);
        variables->put(variableC, -1);
        variables->put(variableD, 1);
        if (marginA > 0 || marginB > 0) {
            mConstantValue = -marginA + marginB;
        }
    } else if (bias <= 0) {
        variables->put(variableA, -1);
        variables->put(variableB, 1);
        mConstantValue = marginA;
    } else if (bias >= 1) {
        variables->put(variableD, -1);
        variables->put(variableC, 1);
        mConstantValue = -marginB;
    } else {
        variables->put(variableA, 1 * (1 - bias));
        variables->put(variableB, -1 * (1 - bias));
        variables->put(variableC, -1 * bias);
        variables->put(variableD, 1 * bias);
        if (marginA > 0 || marginB > 0) {
            mConstantValue = -marginA * (1 - bias) + marginB * bias;
        }
    }
    return this;
}

ArrayRow* ArrayRow::addError(LinearSystem* system, int strength) {
    variables->put(system->createErrorVariable(strength, "ep"), 1);
    variables->put(system->createErrorVariable(strength, "em"), -1);
    return this;
}

ArrayRow* ArrayRow::createRowDimensionPercent(SolverVariable* variableA, SolverVariable* variableC, float percent) {
    variables->put(variableA, -1);
    variables->put(variableC, percent);
    return this;
}

ArrayRow* ArrayRow::createRowDimensionRatio(SolverVariable* variableA, SolverVariable* variableB,
        SolverVariable* variableC, SolverVariable* variableD, float ratio) {
    variables->put(variableA, -1);
    variables->put(variableB, 1);
    variables->put(variableC, ratio);
    variables->put(variableD, -ratio);
    return this;
}

ArrayRow* ArrayRow::createRowWithAngle(SolverVariable* at, SolverVariable* ab,
                                       SolverVariable* bt, SolverVariable* bb, float angleComponent) {
    variables->put(bt, 0.5f);
    variables->put(bb, 0.5f);
    variables->put(at, -0.5f);
    variables->put(ab, -0.5f);
    mConstantValue = -angleComponent;
    return this;
}

int ArrayRow::sizeInBytes() {
    int size = 0;
    if (mVariable != nullptr) {
        size += 4; // object
    }
    size += 4; // constantValue
    size += 4; // used
    size += variables->sizeInBytes();
    return size;
}

void ArrayRow::ensurePositiveConstant() {
    if (mConstantValue < 0) {
        mConstantValue *= -1;
        variables->invert();
    }
}

bool ArrayRow::chooseSubject(LinearSystem* system) {
    bool addedExtra = false;
    SolverVariable* pivotCandidate = chooseSubjectInVariables(system);
    if (pivotCandidate == nullptr) {
        addedExtra = true;
    } else {
        pivot(pivotCandidate);
    }
    if (variables->getCurrentSize() == 0) {
        mIsSimpleDefinition = true;
    }
    return addedExtra;
}

SolverVariable* ArrayRow::chooseSubjectInVariables(LinearSystem* system) {
    SolverVariable* restrictedCandidate = nullptr;
    SolverVariable* unrestrictedCandidate = nullptr;
    float unrestrictedCandidateAmount = 0;
    float restrictedCandidateAmount = 0;
    bool unrestrictedCandidateIsNew = false;
    bool restrictedCandidateIsNew = false;

    const int currentSize = variables->getCurrentSize();
    for (int i = 0; i < currentSize; i++) {
        float amount = variables->getVariableValue(i);
        SolverVariable* variable = variables->getVariable(i);
        if (variable->mType == SolverVariable::Type::UNRESTRICTED) {
            if (unrestrictedCandidate == nullptr) {
                unrestrictedCandidate = variable;
                unrestrictedCandidateAmount = amount;
                unrestrictedCandidateIsNew = isNew(variable, system);
            } else if (unrestrictedCandidateAmount > amount) {
                unrestrictedCandidate = variable;
                unrestrictedCandidateAmount = amount;
                unrestrictedCandidateIsNew = isNew(variable, system);
            } else if (!unrestrictedCandidateIsNew && isNew(variable, system)) {
                unrestrictedCandidate = variable;
                unrestrictedCandidateAmount = amount;
                unrestrictedCandidateIsNew = true;
            }
        } else if (unrestrictedCandidate == nullptr) {
            if (amount < 0) {
                if (restrictedCandidate == nullptr) {
                    restrictedCandidate = variable;
                    restrictedCandidateAmount = amount;
                    restrictedCandidateIsNew = isNew(variable, system);
                } else if (restrictedCandidateAmount > amount) {
                    restrictedCandidate = variable;
                    restrictedCandidateAmount = amount;
                    restrictedCandidateIsNew = isNew(variable, system);
                } else if (!restrictedCandidateIsNew && isNew(variable, system)) {
                    restrictedCandidate = variable;
                    restrictedCandidateAmount = amount;
                    restrictedCandidateIsNew = true;
                }
            }
        }
    }
    if (unrestrictedCandidate != nullptr) {
        return unrestrictedCandidate;
    }
    return restrictedCandidate;
}

bool ArrayRow::isNew(SolverVariable* variable, LinearSystem* /*system*/) {
    // Java FULL_NEW_CHECK (== false) path is omitted; the maintained usage count is authoritative.
    return variable->usageInRowCount <= 1;
}

void ArrayRow::pivot(SolverVariable* v) {
    if (mVariable != nullptr) {
        variables->put(mVariable, -1.0f);
        mVariable->mDefinitionId = -1;
        mVariable = nullptr;
    }
    float amount = variables->remove(v, true) * -1;
    mVariable = v;
    if (amount == 1) {
        return;
    }
    mConstantValue = mConstantValue / amount;
    variables->divideByAmount(amount);
}

bool ArrayRow::isEmpty() {
    return (mVariable == nullptr && mConstantValue == 0 && variables->getCurrentSize() == 0);
}

void ArrayRow::updateFromRow(LinearSystem* system, ArrayRow* definition, bool removeFromDefinition) {
    float value = variables->use(definition, removeFromDefinition);
    mConstantValue += definition->mConstantValue * value;
    if (removeFromDefinition) {
        definition->mVariable->removeFromRow(this);
    }
    if (LinearSystem::SIMPLIFY_SYNONYMS
            && mVariable != nullptr && variables->getCurrentSize() == 0) {
        mIsSimpleDefinition = true;
        system->hasSimpleDefinition = true;
    }
}

void ArrayRow::updateFromFinalVariable(LinearSystem* system, SolverVariable* variable,
                                       bool removeFromDefinition) {
    if (variable == nullptr || !variable->isFinalValue) {
        return;
    }
    float value = variables->get(variable);
    mConstantValue += variable->computedValue * value;
    variables->remove(variable, removeFromDefinition);
    if (removeFromDefinition) {
        variable->removeFromRow(this);
    }
    if (LinearSystem::SIMPLIFY_SYNONYMS
            && variables->getCurrentSize() == 0) {
        mIsSimpleDefinition = true;
        system->hasSimpleDefinition = true;
    }
}

void ArrayRow::updateFromSynonymVariable(LinearSystem* system, SolverVariable* variable,
        bool removeFromDefinition) {
    if (variable == nullptr || !variable->mIsSynonym) {
        return;
    }
    float value = variables->get(variable);
    mConstantValue += variable->mSynonymDelta * value;
    variables->remove(variable, removeFromDefinition);
    if (removeFromDefinition) {
        variable->removeFromRow(this);
    }
    variables->add(system->mCache->mIndexedVariables[variable->mSynonym],
                   value, removeFromDefinition);
    if (LinearSystem::SIMPLIFY_SYNONYMS
            && variables->getCurrentSize() == 0) {
        mIsSimpleDefinition = true;
        system->hasSimpleDefinition = true;
    }
}

SolverVariable* ArrayRow::pickPivotInVariables(bool* avoid, SolverVariable* exclude) {
    bool all = true;
    float value = 0;
    SolverVariable* pivot = nullptr;
    SolverVariable* pivotSlack = nullptr;
    float valueSlack = 0;

    const int currentSize = variables->getCurrentSize();
    for (int i = 0; i < currentSize; i++) {
        float currentValue = variables->getVariableValue(i);
        if (currentValue < 0) {
            SolverVariable* v = variables->getVariable(i);
            if (!((avoid != nullptr && avoid[v->id]) || (v == exclude))) {
                if (all) {
                    if (v->mType == SolverVariable::Type::SLACK
                            || v->mType == SolverVariable::Type::ERROR) {
                        if (currentValue < value) {
                            value = currentValue;
                            pivot = v;
                        }
                    }
                } else {
                    if (v->mType == SolverVariable::Type::SLACK) {
                        if (currentValue < valueSlack) {
                            valueSlack = currentValue;
                            pivotSlack = v;
                        }
                    } else if (v->mType == SolverVariable::Type::ERROR) {
                        if (currentValue < value) {
                            value = currentValue;
                            pivot = v;
                        }
                    }
                }
            }
        }
    }
    if (all) {
        return pivot;
    }
    return pivot != nullptr ? pivot : pivotSlack;
}

SolverVariable* ArrayRow::pickPivot(SolverVariable* exclude) {
    return pickPivotInVariables(nullptr, exclude);
}

SolverVariable* ArrayRow::getPivotCandidate(LinearSystem* system, bool* avoid) {
    (void)system;
    return pickPivotInVariables(avoid, nullptr);
}

void ArrayRow::clear() {
    variables->clear();
    mVariable = nullptr;
    mConstantValue = 0;
}

void ArrayRow::initFromRow(Row* row) {
    ArrayRow* copiedRow = dynamic_cast<ArrayRow*>(row);
    if (copiedRow != nullptr) {
        mVariable = nullptr;
        variables->clear();
        for (int i = 0; i < copiedRow->variables->getCurrentSize(); i++) {
            SolverVariable* var = copiedRow->variables->getVariable(i);
            float val = copiedRow->variables->getVariableValue(i);
            variables->add(var, val, true);
        }
    }
}

void ArrayRow::addError(SolverVariable* error) {
    float weight = 1;
    if (error->strength == SolverVariable::STRENGTH_LOW) {
        weight = 1.0f;
    } else if (error->strength == SolverVariable::STRENGTH_MEDIUM) {
        weight = 1e3f;
    } else if (error->strength == SolverVariable::STRENGTH_HIGH) {
        weight = 1e6f;
    } else if (error->strength == SolverVariable::STRENGTH_HIGHEST) {
        weight = 1e9f;
    } else if (error->strength == SolverVariable::STRENGTH_EQUALITY) {
        weight = 1e12f;
    }
    variables->put(error, weight);
}

SolverVariable* ArrayRow::getKey() {
    return mVariable;
}

void ArrayRow::updateFromSystem(LinearSystem* system) {
    if (system->mRows.empty()) {
        return;
    }
    bool done = false;
    while (!done) {
        int currentSize = variables->getCurrentSize();
        for (int i = 0; i < currentSize; i++) {
            SolverVariable* variable = variables->getVariable(i);
            if (variable->mDefinitionId != -1 || variable->isFinalValue || variable->mIsSynonym) {
                mVariablesToUpdate.push_back(variable);
            }
        }
        const int size = (int)mVariablesToUpdate.size();
        if (size > 0) {
            for (int i = 0; i < size; i++) {
                SolverVariable* variable = mVariablesToUpdate[i];
                if (variable->isFinalValue) {
                    updateFromFinalVariable(system, variable, true);
                } else if (variable->mIsSynonym) {
                    updateFromSynonymVariable(system, variable, true);
                } else {
                    updateFromRow(system, system->mRows[variable->mDefinitionId], true);
                }
            }
            mVariablesToUpdate.clear();
        } else {
            done = true;
        }
    }
    if (LinearSystem::SIMPLIFY_SYNONYMS
            && mVariable != nullptr && variables->getCurrentSize() == 0) {
        mIsSimpleDefinition = true;
        system->hasSimpleDefinition = true;
    }
}

} // namespace cdroid
