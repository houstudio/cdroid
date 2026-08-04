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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.Chain.
 *
 * Chain management and constraints creation. Dead branches omitted: USE_CHAIN_OPTIMIZATION
 * (false → Direct.solveChain fast-path) and DEBUG/FULL_DEBUG print blocks.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CHAIN_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CHAIN_H

#include <vector>

namespace cdroid {

class ConstraintWidget;
class ConstraintWidgetContainer;
class LinearSystem;
class ChainHead;

class Chain {
  public:
    static const bool USE_CHAIN_OPTIMIZATION = true;

    // Iterate every chain head recorded on the container and apply chain constraints.
    // `widgets` is nullable (Java ArrayList<ConstraintWidget>); when non-null only chains whose
    // first widget is in the set are applied.
    static void applyChainConstraints(ConstraintWidgetContainer* constraintWidgetContainer,
                                      LinearSystem* system,
                                      std::vector<ConstraintWidget*>* widgets,
                                      int orientation);

    // Apply chain constraints for a single chain (the core builder).
    static void applyChainConstraints(ConstraintWidgetContainer* container, LinearSystem* system,
                                      int orientation, int offset, ChainHead* chainHead);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_CHAIN_H
