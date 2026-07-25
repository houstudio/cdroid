/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Ported to C++ for CDROID from androidx.constraintlayout.core.Metrics.
 */
#include <widgetEx/constraintlayout/core/metrics.h>

namespace cdroid {

std::string Metrics::toString() {
    return "\n*** Metrics ***\n"
           "measures: " + std::to_string(measures) + "\n"
           "measuresWrap: " + std::to_string(measuresWrap) + "\n"
           "measuresWrapInfeasible: " + std::to_string(measuresWrapInfeasible) + "\n"
           "determineGroups: " + std::to_string(determineGroups) + "\n"
           "infeasibleDetermineGroups: " + std::to_string(infeasibleDetermineGroups) + "\n"
           "graphOptimizer: " + std::to_string(graphOptimizer) + "\n"
           "widgets: " + std::to_string(widgets) + "\n"
           "graphSolved: " + std::to_string(graphSolved) + "\n"
           "linearSolved: " + std::to_string(linearSolved) + "\n";
}

void Metrics::reset() {
    measures = 0;
    widgets = 0;
    additionalMeasures = 0;
    resolutions = 0;
    tableSizeIncrease = 0;
    maxTableSize = 0;
    lastTableSize = 0;
    maxVariables = 0;
    maxRows = 0;
    minimize = 0;
    minimizeGoal = 0;
    constraints = 0;
    simpleconstraints = 0;
    optimize = 0;
    iterations = 0;
    pivots = 0;
    bfs = 0;
    variables = 0;
    errors = 0;
    slackvariables = 0;
    extravariables = 0;
    fullySolved = 0;
    graphOptimizer = 0;
    graphSolved = 0;
    resolvedWidgets = 0;
    nonresolvedWidgets = 0;
    linearSolved = 0;
    problematicLayouts.clear();
    mNumberOfMeasures = 0;
    mNumberOfLayouts = 0;
    measuresWidgetsDuration = 0;
    measuresLayoutDuration = 0;
    mChildCount = 0;
    mMeasureDuration = 0;
    mMeasureCalls = 0;
    mSolverPasses = 0;
    mVariables = 0;
    mEquations = 0;
    mSimpleEquations = 0;
}

void Metrics::copy(const Metrics& metrics) {
    mVariables = metrics.mVariables;
    mEquations = metrics.mEquations;
    mSimpleEquations = metrics.mSimpleEquations;
    mNumberOfMeasures = metrics.mNumberOfMeasures;
    mNumberOfLayouts = metrics.mNumberOfLayouts;
    mMeasureDuration = metrics.mMeasureDuration;
    mChildCount = metrics.mChildCount;
    mMeasureCalls = metrics.mMeasureCalls;
    measuresWidgetsDuration = metrics.measuresWidgetsDuration;
    mSolverPasses = metrics.mSolverPasses;

    measuresLayoutDuration = metrics.measuresLayoutDuration;
    measures = metrics.measures;
    widgets = metrics.widgets;
    additionalMeasures = metrics.additionalMeasures;
    resolutions = metrics.resolutions;
    tableSizeIncrease = metrics.tableSizeIncrease;
    maxTableSize = metrics.maxTableSize;
    lastTableSize = metrics.lastTableSize;
    maxVariables = metrics.maxVariables;
    maxRows = metrics.maxRows;
    minimize = metrics.minimize;
    minimizeGoal = metrics.minimizeGoal;
    constraints = metrics.constraints;
    simpleconstraints = metrics.simpleconstraints;
    optimize = metrics.optimize;
    iterations = metrics.iterations;
    pivots = metrics.pivots;
    bfs = metrics.bfs;
    variables = metrics.variables;
    errors = metrics.errors;
    slackvariables = metrics.slackvariables;
    extravariables = metrics.extravariables;
    fullySolved = metrics.fullySolved;
    graphOptimizer = metrics.graphOptimizer;
    graphSolved = metrics.graphSolved;
    resolvedWidgets = metrics.resolvedWidgets;
    nonresolvedWidgets = metrics.nonresolvedWidgets;
}

} // namespace cdroid
