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
