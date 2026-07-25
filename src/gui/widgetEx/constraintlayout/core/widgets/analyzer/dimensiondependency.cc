/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.DimensionDependency.
 */
#include <widgetEx/constraintlayout/core/widgets/analyzer/dimensiondependency.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/widgetrun.h>
#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

namespace cdroid {

DimensionDependency::DimensionDependency(WidgetRun* run)
    : DependencyNode(run) {
    // Java: if (run instanceof HorizontalWidgetRun) HORIZONTAL else VERTICAL.
    // WidgetRun carries an `orientation` field, so use it directly.
    if (run->orientation == ConstraintWidget::HORIZONTAL) {
        mType = Type::HORIZONTAL_DIMENSION;
    } else {
        mType = Type::VERTICAL_DIMENSION;
    }
}

void DimensionDependency::resolve(int value) {
    if (resolved) {
        return;
    }
    resolved = true;
    this->value = value;
    for (Dependency* node : mDependencies) {
        node->update(node);
    }
}

} // namespace cdroid
