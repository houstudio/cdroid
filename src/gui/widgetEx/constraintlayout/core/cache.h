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
