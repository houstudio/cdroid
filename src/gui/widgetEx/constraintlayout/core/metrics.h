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
#ifndef __CONSTRAINTLAYOUT_CORE_METRICS_H__
#define __CONSTRAINTLAYOUT_CORE_METRICS_H__

#include <cstdint>
#include <string>
#include <vector>

namespace cdroid {

/**
 * Utility class to track metrics during the system resolution.
 * Ported verbatim from androidx.constraintlayout.core.Metrics.
 */
class Metrics {
  public:
    int64_t measuresWidgetsDuration = 0;  // time spent in child measures (ns)
    int64_t measuresLayoutDuration  = 0;  // time spent in child measures (ns)
    int64_t measuredWidgets         = 0;
    int64_t measuredMatchWidgets    = 0;
    int64_t measures                = 0;
    int64_t additionalMeasures      = 0;
    int64_t resolutions             = 0;
    int64_t tableSizeIncrease       = 0;
    int64_t minimize                = 0;
    int64_t constraints             = 0;
    int64_t simpleconstraints       = 0;
    int64_t optimize                = 0;
    int64_t iterations              = 0;
    int64_t pivots                  = 0;
    int64_t bfs                     = 0;
    int64_t variables               = 0;
    int64_t errors                  = 0;
    int64_t slackvariables          = 0;
    int64_t extravariables          = 0;
    int64_t maxTableSize            = 0;
    int64_t fullySolved             = 0;
    int64_t graphOptimizer          = 0;
    int64_t graphSolved             = 0;
    int64_t linearSolved            = 0;
    int64_t resolvedWidgets         = 0;
    int64_t minimizeGoal            = 0;
    int64_t maxVariables            = 0;
    int64_t maxRows                 = 0;
    int64_t nonresolvedWidgets      = 0;
    std::vector<std::string> problematicLayouts;
    int64_t lastTableSize           = 0;
    int64_t widgets                 = 0;
    int64_t measuresWrap            = 0;
    int64_t measuresWrapInfeasible  = 0;
    int64_t infeasibleDetermineGroups = 0;
    int64_t determineGroups         = 0;
    int64_t layouts                 = 0;
    int64_t grouping                = 0;
    int  mNumberOfLayouts = 0; // times ConstraintLayout onLayout is called
    int  mNumberOfMeasures = 0; // times child measures is called
    int64_t mMeasureDuration = 0; // time spent in measure (ns)
    int64_t mChildCount = 0; // number of child Views of ConstraintLayout
    int64_t mMeasureCalls = 0; // number of times CL onMeasure is called
    int64_t mSolverPasses = 0;
    int64_t mEquations = 0;
    int64_t mVariables = 0;
    int64_t mSimpleEquations = 0;

    std::string toString();
    void reset();
    void copy(const Metrics& metrics);
};

} // namespace cdroid

#endif // __CONSTRAINTLAYOUT_CORE_METRICS_H__
