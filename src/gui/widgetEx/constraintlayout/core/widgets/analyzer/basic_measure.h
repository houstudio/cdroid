/*
 * Copyright (C) 2016 The Android Open Source Project
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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.BasicMeasure.
 *
 * The measure+solve orchestrator called by ConstraintLayout.onMeasure(). The OPTIMIZATION_GRAPH
 * fast-path (directMeasure / DependencyGraph) is stubbed; the default linear-solve path
 * (measureChildren -> updateHierarchy -> solveLinearSystem -> container.layout()) is real.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_BASIC_MEASURE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_BASIC_MEASURE_H

#include <vector>

#include <widgetEx/constraintlayout/core/widgets/constraint_widget.h>

namespace cdroid {

class ConstraintWidgetContainer;

class BasicMeasure {
public:
    // MeasureSpec modes (top 2 bits) — matches Android View.MeasureSpec encoding.
    static const int MODE_SHIFT = 30;
    static const int UNSPECIFIED = 0;
    static const int EXACTLY     = 1 << MODE_SHIFT;
    static const int AT_MOST     = 2 << MODE_SHIFT;

    static const int MATCH_PARENT = -1;
    static const int WRAP_CONTENT = -2;
    static const int FIXED        = -3;

    /** Holds a widget's measure request + result, passed to/from the Measurer. */
    class Measure {
    public:
        static int SELF_DIMENSIONS;
        static int TRY_GIVEN_DIMENSIONS;
        static int USE_GIVEN_DIMENSIONS;

        ConstraintWidget::DimensionBehaviour horizontalBehavior = ConstraintWidget::DimensionBehaviour::FIXED;
        ConstraintWidget::DimensionBehaviour verticalBehavior   = ConstraintWidget::DimensionBehaviour::FIXED;
        int horizontalDimension = 0;
        int verticalDimension   = 0;
        int measuredWidth       = 0;
        int measuredHeight      = 0;
        int measuredBaseline    = 0;
        bool measuredHasBaseline   = false;
        bool measuredNeedsSolverPass = false;
        int measureStrategy    = 0;
    };

    /** Callback the host (ConstraintLayout) implements to measure a child View. */
    class Measurer {
    public:
        virtual ~Measurer() = default;
        virtual void measure(ConstraintWidget* widget, Measure* measure) = 0;
        virtual void didMeasures() = 0;
    };

    explicit BasicMeasure(ConstraintWidgetContainer* constraintWidgetContainer);

    void updateHierarchy(ConstraintWidgetContainer* layout);
    long solverMeasure(ConstraintWidgetContainer* layout, int optimizationLevel,
                       int paddingX, int paddingY,
                       int widthMode, int widthSize,
                       int heightMode, int heightSize,
                       int lastMeasureWidth, int lastMeasureHeight);

    Measure& getMeasure() { return mMeasure; }

private:
    ConstraintWidgetContainer* mConstraintWidgetContainer;
    std::vector<ConstraintWidget*> mVariableDimensionsWidgets;
    Measure mMeasure;

    void measureChildren(ConstraintWidgetContainer* layout);
    void solveLinearSystem(ConstraintWidgetContainer* layout, const char* reason, int pass,
                           int w, int h);
    bool measure(Measurer* measurer, ConstraintWidget* widget, int measureStrategy);
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_BASIC_MEASURE_H
