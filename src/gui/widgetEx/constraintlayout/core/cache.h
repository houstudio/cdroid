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

#include <widgetEx/constraintlayout/core/pools.h>
#include <vector>

namespace cdroid {

class ArrayRow;        // forward (SimplePool<ArrayRow> only needs ArrayRow*)
class SolverVariable;  // forward

/**
 * Cache for common objects. Ported verbatim from
 * androidx.constraintlayout.core.Cache.
 *
 * The pools store raw pointers; ownership/lifetime is managed by LinearSystem
 * (which creates and resets the pool each solve).
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
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_CACHE_H__
