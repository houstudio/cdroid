/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.WidgetRun.
 */
#include <widgetEx/constraintlayout/core/widgets/analyzer/widgetrun.h>
#include <widgetEx/constraintlayout/core/widgets/constraintanchor.h>

#include <algorithm>

namespace cdroid {

WidgetRun::WidgetRun(ConstraintWidget* widget)
    : mWidget(widget)
    , mDimension(this)
    , start(this)
    , end(this) {
}

bool WidgetRun::isDimensionResolved() {
    return mDimension.resolved;
}

bool WidgetRun::isCenterConnection() {
    int connections = 0;
    for (DependencyNode* dependency : start.mTargets) {
        if (dependency->mRun != this) {
            connections++;
        }
    }
    for (DependencyNode* dependency : end.mTargets) {
        if (dependency->mRun != this) {
            connections++;
        }
    }
    return connections >= 2;
}

long WidgetRun::wrapSize(int /*direction*/) {
    // DEFERRED(run-group): Java returns mDimension.value adjusted by start/end margins depending
    // on `direction == RunGroup.START`. Needs RunGroup::START constant + isCenterConnection; both
    // available, but this is only called by RunGroup (next batch). Stub for now.
    if (mDimension.resolved) {
        return mDimension.value;
    }
    return 0;
}

DependencyNode* WidgetRun::getTarget(ConstraintAnchor* /*anchor*/) {
    // DEFERRED(hvrun): Java switches on the target anchor type and returns the corresponding
    // HorizontalWidgetRun/VerticalWidgetRun start/end/baseline node. Needs HVRun complete.
    // TODO(hvrun): restore once HorizontalWidgetRun/VerticalWidgetRun are ported.
    return nullptr;
}

void WidgetRun::updateRunCenter(Dependency* /*dependency*/, ConstraintAnchor* /*startAnchor*/,
                                ConstraintAnchor* /*endAnchor*/, int /*orientation*/) {
    // DEFERRED(hvrun): centers the run between its resolved start/end targets (resolves
    // dimension via resolveDimension, then start/end via bias). Needs getTarget() (HVRun).
    // TODO(hvrun): restore verbatim from WidgetRun.updateRunCenter.
}

void WidgetRun::updateRunStart(Dependency* /*dependency*/) {
    // base no-op (overridden by run subclasses if needed)
}

void WidgetRun::updateRunEnd(Dependency* /*dependency*/) {
    // base no-op
}

void WidgetRun::update(Dependency* /*dependency*/) {
    // base no-op
}

int WidgetRun::getLimitedDimension(int dimension, int orientation) {
    if (orientation == ConstraintWidget::HORIZONTAL) {
        int max = mWidget->mMatchConstraintMaxWidth;
        int min = mWidget->mMatchConstraintMinWidth;
        int value = std::max(min, dimension);
        if (max > 0) {
            value = std::min(max, dimension);
        }
        if (value != dimension) {
            dimension = value;
        }
    } else {
        int max = mWidget->mMatchConstraintMaxHeight;
        int min = mWidget->mMatchConstraintMinHeight;
        int value = std::max(min, dimension);
        if (max > 0) {
            value = std::min(max, dimension);
        }
        if (value != dimension) {
            dimension = value;
        }
    }
    return dimension;
}

DependencyNode* WidgetRun::getTarget(ConstraintAnchor* /*anchor*/, int /*orientation*/) {
    // DEFERRED(hvrun): returns targetWidget.{mHorizontalRun|mVerticalRun}.start|end based on the
    // target anchor type. Needs HVRun complete.
    // TODO(hvrun): restore verbatim from WidgetRun.getTarget(anchor, orientation).
    return nullptr;
}

void WidgetRun::addTarget(DependencyNode* node, DependencyNode* target, int margin) {
    node->mTargets.push_back(target);
    node->mMargin = margin;
    target->mDependencies.push_back(node);
}

void WidgetRun::addTarget(DependencyNode* node, DependencyNode* target, int marginFactor,
                          DimensionDependency* dimensionDependency) {
    node->mTargets.push_back(target);
    node->mTargets.push_back(&mDimension);
    node->mMarginFactor = marginFactor;
    node->mMarginDependency = dimensionDependency;
    target->mDependencies.push_back(node);
    dimensionDependency->mDependencies.push_back(node);
}

long WidgetRun::getWrapDimension() {
    if (mDimension.resolved) {
        return mDimension.value;
    }
    return 0;
}

bool WidgetRun::isResolved() {
    return mResolved;
}

void WidgetRun::resolveDimension(int /*orientation*/, int /*distance*/) {
    // DEFERRED(hvrun): resolves mDimension per matchConstraintsType (SPREAD/PERCENT/WRAP/RATIO);
    // the PERCENT/RATIO branches read parent.mHorizontalRun/mVerticalRun. Needs HVRun.
    // TODO(hvrun): restore verbatim from WidgetRun.resolveDimension.
}

} // namespace cdroid
