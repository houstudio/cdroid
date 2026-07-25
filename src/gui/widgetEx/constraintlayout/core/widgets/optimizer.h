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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Optimizer.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_OPTIMIZER_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_OPTIMIZER_H

namespace cdroid {

class ConstraintWidget;
class ConstraintWidgetContainer;
class LinearSystem;

class Optimizer {
public:
    static const int OPTIMIZATION_NONE               = 0;
    static const int OPTIMIZATION_DIRECT             = 1;
    static const int OPTIMIZATION_BARRIER            = 1 << 1;
    static const int OPTIMIZATION_CHAIN              = 1 << 2;
    static const int OPTIMIZATION_DIMENSIONS         = 1 << 3;
    static const int OPTIMIZATION_RATIO              = 1 << 4;
    static const int OPTIMIZATION_GROUPS             = 1 << 5;
    static const int OPTIMIZATION_GRAPH              = 1 << 6;
    static const int OPTIMIZATION_GRAPH_WRAP         = 1 << 7;
    static const int OPTIMIZATION_CACHE_MEASURES     = 1 << 8;
    static const int OPTIMIZATION_DEPENDENCY_ORDERING = 1 << 9;
    static const int OPTIMIZATION_GROUPING           = 1 << 10;
    static const int OPTIMIZATION_STANDARD           = OPTIMIZATION_DIRECT
            | OPTIMIZATION_CACHE_MEASURES;

    // Looks at optimizing match_parent. Implemented in optimizer.cc; currently a stub
    // pending ConstraintWidgetContainer (it dereferences the container and is only invoked
    // from the Stage-3 solver driver).
    static void checkMatchParent(ConstraintWidgetContainer* container, LinearSystem* system,
                                 ConstraintWidget* widget);

    static bool enabled(int optimizationLevel, int optimization) {
        return (optimizationLevel & optimization) == optimization;
    }
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_OPTIMIZER_H
