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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.Cache.
 *
 * Out-of-line Cache destructor: instantiates the owning SimplePool destructors here, where ArrayRow
 * and SolverVariable are complete (cache.h forward-declares them), so the pools can delete their
 * contents. ~LinearSystem has already returned the last solve's rows + acquireSolverVariable
 * variables to the pools (mPoolVariables excludes the anchor-owned variables, which ConstraintAnchor
 * frees itself — so they are never pooled and not double-freed).
 */
#include <widgetEx/constraintlayout/core/cache.h>

#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

namespace cdroid {

Cache::~Cache() = default; // member dtors run here (complete types) -> owning pools free contents

} // namespace cdroid
