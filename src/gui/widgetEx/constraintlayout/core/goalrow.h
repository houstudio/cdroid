/*
 * Copyright (C) 2017 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.GoalRow.
 */
#ifndef __CONSTRAINTLAYOUT_CORE_GOAL_ROW_H__
#define __CONSTRAINTLAYOUT_CORE_GOAL_ROW_H__

#include <widgetEx/constraintlayout/core/arrayrow.h>

namespace cdroid {

class Cache;

/**
 * Ported verbatim from androidx.constraintlayout.core.GoalRow.
 * Error variables in the goal shouldn't be tracked (we only care if they are
 * in the system rows), so addError decrements usageInRowCount after delegating.
 */
class GoalRow : public ArrayRow {
public:
    explicit GoalRow(Cache* cache);
    void addError(SolverVariable* error) override;
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_GOAL_ROW_H__
