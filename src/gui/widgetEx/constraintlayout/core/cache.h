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
 */
#ifndef __CONSTRAINTLAYOUT_CORE_CACHE_H__
#define __CONSTRAINTLAYOUT_CORE_CACHE_H__

#include <core/pools.h>
#include <vector>

#include <widgetEx/constraintlayout/core/arrayrow.h>
#include <widgetEx/constraintlayout/core/solvervariable.h>

namespace cdroid {

/**
 * Cache for common objects. Ported verbatim from
 * androidx.constraintlayout.core.Cache.
 *
 * Uses CDROID's shared cdroid::Pools::SimplePool (gui/core/pools.h) — an owning pool whose
 * destructor frees its contents — rather than a constraintlayout-local duplicate. ArrayRow and
 * SolverVariable are included in full (no forward decl) so the owning pool destructors can delete
 * them; the include chain is acyclic (linear_system.h forward-declares Cache, never includes it).
 */
class Cache {
  public:
    Pools::SimplePool<ArrayRow>      mOptimizedArrayRowPool;
    Pools::SimplePool<ArrayRow>      mArrayRowPool;
    Pools::SimplePool<SolverVariable> mSolverVariablePool;
    std::vector<SolverVariable*>     mIndexedVariables; // Java: SolverVariable[32], grows

    Cache()
        : mOptimizedArrayRowPool(256)
        , mArrayRowPool(256)
        , mSolverVariablePool(256)
        , mIndexedVariables(32, nullptr) {
    }
    ~Cache(); // out-of-line (cache.cc) so the owning pool destructors see complete types
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_CACHE_H__
