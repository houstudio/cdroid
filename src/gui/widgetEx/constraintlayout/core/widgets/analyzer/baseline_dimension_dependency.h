/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.BaselineDimensionDependency.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_BASELINE_DIMENSION_DEPENDENCY_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_BASELINE_DIMENSION_DEPENDENCY_H

#include <widgetEx/constraintlayout/core/widgets/analyzer/dimension_dependency.h>

namespace cdroid {

class DependencyNode;

class BaselineDimensionDependency : public DimensionDependency {
public:
    explicit BaselineDimensionDependency(WidgetRun* run);

    // Specialized callback (overload, not the Dependency::update(Dependency*) override) invoked
    // by VerticalWidgetRun when the baseline dependency is ready.
    void update(DependencyNode* node);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_BASELINE_DIMENSION_DEPENDENCY_H
