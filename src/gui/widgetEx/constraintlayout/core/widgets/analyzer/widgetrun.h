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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.WidgetRun.
 *
 * Abstract base for the horizontal/vertical/chain runs of the graph solver (OPTIMIZATION_GRAPH).
 * Owns its start/end DependencyNodes and a DimensionDependency. The HVRun-dependent methods
 * (getTarget / updateRunCenter / resolveDimension / wrapSize) are stubbed here and completed
 * once HorizontalWidgetRun / VerticalWidgetRun / RunGroup are ported.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_WIDGET_RUN_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_WIDGET_RUN_H

#include <widgetEx/constraintlayout/core/widgets/analyzer/dependency.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/dependencynode.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/dimensiondependency.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

class ConstraintAnchor;
class RunGroup;
class DimensionDependency;
class DependencyNode;

class WidgetRun : public Dependency {
public:
    enum class RunType { NONE, START, END, CENTER };

    int matchConstraintsType = 0;
    ConstraintWidget* mWidget;
    RunGroup* mRunGroup = nullptr;
    ConstraintWidget::DimensionBehaviour mDimensionBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
    DimensionDependency mDimension;
    int orientation = ConstraintWidget::HORIZONTAL;
    bool mResolved = false;
    DependencyNode start;
    DependencyNode end;

    explicit WidgetRun(ConstraintWidget* widget);

    // --- abstract (implemented by Horizontal/Vertical/Chain run) ---
    virtual void clear() = 0;
    virtual void apply() = 0;
    virtual void applyToWidget() = 0;
    virtual void reset() = 0;
    virtual bool supportsWrapComputation() = 0;

    // --- concrete ---
    bool isDimensionResolved();
    bool isCenterConnection();
    long wrapSize(int direction);
    DependencyNode* getTarget(ConstraintAnchor* anchor);
    void updateRunCenter(Dependency* dependency, ConstraintAnchor* startAnchor,
                         ConstraintAnchor* endAnchor, int orientation);
    void updateRunStart(Dependency* dependency);
    void updateRunEnd(Dependency* dependency);
    void update(Dependency* dependency) override;
    int getLimitedDimension(int dimension, int orientation);
    DependencyNode* getTarget(ConstraintAnchor* anchor, int orientation);
    void addTarget(DependencyNode* node, DependencyNode* target, int margin);
    void addTarget(DependencyNode* node, DependencyNode* target, int marginFactor,
                   DimensionDependency* dimensionDependency);
    long getWrapDimension();
    bool isResolved();

protected:
    void resolveDimension(int orientation, int distance);
    RunType mRunType = RunType::NONE;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_WIDGET_RUN_H
