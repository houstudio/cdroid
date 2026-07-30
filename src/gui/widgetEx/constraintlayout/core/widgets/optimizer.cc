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
#include <widgetEx/constraintlayout/core/widgets/optimizer.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

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
