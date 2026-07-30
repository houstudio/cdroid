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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.BaselineDimensionDependency.
 */
#include <widgetEx/constraintlayout/core/widgets/analyzer/baselinedimensiondependency.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/widgetrun.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

BaselineDimensionDependency::BaselineDimensionDependency(WidgetRun* run)
    : DimensionDependency(run) {
}

void BaselineDimensionDependency::update(DependencyNode* /*node*/) {
    // DEFERRED: Java casts mRun to VerticalWidgetRun and sets verticalRun.baseline.mMargin =
    // mRun.mWidget.getBaselineDistance(), then resolved = true. VerticalWidgetRun is ported in
    // the next analyzer batch; restore then.
    // TODO(verticalrun): VerticalWidgetRun* verticalRun = static_cast<VerticalWidgetRun*>(mRun);
    //   verticalRun->baseline.mMargin = mWidget->mBaselineDistance; resolved = true;
    resolved = true;
}

} // namespace cdroid
