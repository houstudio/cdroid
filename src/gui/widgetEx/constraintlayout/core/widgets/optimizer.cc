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
#include <widgetEx/constraintlayout/core/widgets/optimizer.h>
#include <widgetEx/constraintlayout/core/widgets/constraint_widget.h>

namespace cdroid {

// DEFERRED: ConstraintWidgetContainer is not yet ported (Stage 2 driver = Stage 3 analyzer).
// The Java body resolves MATCH_PARENT widgets directly (no solver pivot) by pinning their
// left/right or top/bottom anchors to the container edges and marking the widget DIRECT.
// Restore verbatim once ConstraintWidgetContainer exists and the driver invokes this.
void Optimizer::checkMatchParent(ConstraintWidgetContainer* /*container*/, LinearSystem* /*system*/,
                                 ConstraintWidget* /*widget*/) {
    // TODO(container): see androidx.constraintlayout.core.widgets.Optimizer.checkMatchParent
}

} // namespace cdroid
