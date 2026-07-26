/*
 * Copyright (C) 2016 The Android Open Source Project
 *
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
