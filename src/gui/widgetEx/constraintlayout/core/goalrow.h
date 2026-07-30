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
