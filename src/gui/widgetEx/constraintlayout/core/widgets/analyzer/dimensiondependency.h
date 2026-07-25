/*
 * Copyright (C) 2019 The Android Open Source Project
 *
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
