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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.DependencyNode.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DEPENDENCY_NODE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DEPENDENCY_NODE_H

#include <string>
#include <vector>

#include <widgetEx/constraintlayout/core/widgets/analyzer/dependency.h>

namespace cdroid {

class WidgetRun;
class DimensionDependency;

class DependencyNode : public Dependency {
public:
    enum class Type {
        UNKNOWN, HORIZONTAL_DIMENSION, VERTICAL_DIMENSION,
        LEFT, RIGHT, TOP, BOTTOM, BASELINE
    };

    Dependency* updateDelegate = nullptr;
    bool delegateToWidgetRun = false;
    bool readyToSolve = false;

    WidgetRun* mRun;
    Type mType = Type::UNKNOWN;
    int mMargin = 0;
    int value = 0;
    int mMarginFactor = 1;
    DimensionDependency* mMarginDependency = nullptr;
    bool resolved = false;

    std::vector<Dependency*> mDependencies;
    std::vector<DependencyNode*> mTargets;

    explicit DependencyNode(WidgetRun* run);

    std::string toString();
    virtual void resolve(int value);
    void update(Dependency* node) override;
    void addDependency(Dependency* dependency);
    std::string name();
    void clear();
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_DEPENDENCY_NODE_H
