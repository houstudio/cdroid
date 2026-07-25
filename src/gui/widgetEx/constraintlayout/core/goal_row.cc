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
#include <widgetEx/constraintlayout/core/goal_row.h>
#include <widgetEx/constraintlayout/core/cache.h>
#include <widgetEx/constraintlayout/core/solver_variable.h>

namespace cdroid {

GoalRow::GoalRow(Cache* cache)
    : ArrayRow(cache) {
}

void GoalRow::addError(SolverVariable* error) {
    ArrayRow::addError(error);
    // error variables in the goal shouldn't be tracked (we only care if they
    // are in the system rows)
    error->usageInRowCount--;
}

} // namespace cdroid
