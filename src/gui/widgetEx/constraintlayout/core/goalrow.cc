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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.GoalRow.
 */
#include <widgetEx/constraintlayout/core/goalrow.h>
#include <widgetEx/constraintlayout/core/cache.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

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
