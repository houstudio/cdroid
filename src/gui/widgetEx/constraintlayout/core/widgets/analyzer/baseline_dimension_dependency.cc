/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.BaselineDimensionDependency.
 */
#include <widgetEx/constraintlayout/core/widgets/analyzer/baseline_dimension_dependency.h>
#include <widgetEx/constraintlayout/core/widgets/analyzer/widget_run.h>
#include <widgetEx/constraintlayout/core/widgets/constraint_widget.h>

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
