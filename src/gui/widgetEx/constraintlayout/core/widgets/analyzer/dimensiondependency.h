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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.DimensionDependency.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DIMENSION_DEPENDENCY_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DIMENSION_DEPENDENCY_H

#include <widgetEx/constraintlayout/core/widgets/analyzer/dependencynode.h>

namespace cdroid {

class WidgetRun;

class DimensionDependency : public DependencyNode {
public:
    int wrapValue = 0;

    explicit DimensionDependency(WidgetRun* run);

    void resolve(int value) override;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DIMENSION_DEPENDENCY_H
