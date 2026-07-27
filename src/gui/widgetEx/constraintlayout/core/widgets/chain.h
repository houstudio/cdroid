/*
 * Copyright (C) 2017 The Android Open Source Project
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
    static const bool USE_CHAIN_OPTIMIZATION = false;

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
