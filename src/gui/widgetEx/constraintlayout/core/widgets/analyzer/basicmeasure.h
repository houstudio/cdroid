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
 * Ported to C++ for CDROID from androidx.constraintlayout.core.widgets.analyzer.BasicMeasure.
 *
 * The measure+solve orchestrator called by ConstraintLayout.onMeasure(). The OPTIMIZATION_GRAPH
 * fast-path (directMeasure / DependencyGraph) is stubbed; the default linear-solve path
 * (measureChildren -> updateHierarchy -> solveLinearSystem -> container.layout()) is real.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_BASIC_MEASURE_H
#define CDROID_CONSTRAINTLAYOUT_CORE_WIDGETS_ANALYZER_BASIC_MEASURE_H

#include <vector>

#include <widgetEx/constraintlayout/core/widgets/constraintwidget.h>

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
