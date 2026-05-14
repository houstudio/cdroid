/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
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
 */
#ifndef SIMPLE_SERIES_RENDERER_H__
#define SIMPLE_SERIES_RENDERER_H__
#include <core/color.h>
#include <core/numberformat.h>
#include <widget/achart/renderer/basicstroke.h>
namespace cdroid{

/**
 * A simple series renderer.
 */
class SimpleSeriesRenderer{
private:
    /** The series color. */
    int mColor = Color::BLUE;
    /** The minimum distance between displaying chart values. */
    int mDisplayChartValuesDistance = 100;
    /** The chart values text size. */
    float mChartValuesTextSize = 10;
    /** The chart values text alignment. */
    int/*Align*/ mChartValuesTextAlign;// = Align.CENTER;
    /** The chart values spacing from the data point. */
    float mChartValuesSpacing = 5.f;
    /** The gradient start value. */
    double mGradientStartValue;
    /** The gradient start color. */
    int mGradientStartColor;
    /** The gradient stop value. */
    double mGradientStopValue;
    /** The gradient stop color. */
    int mGradientStopColor;
    /** If the values should be displayed above the chart points. */
    bool mDisplayChartValues=false;
    /** If gradient is enabled. */
    bool mGradientEnabled = false;
    /** If the legend item for this renderer is visible. */
    bool mShowLegendItem = true;
    /** If this is a highlighted slice (pie chart displays slice as exploded). */
    bool mHighlighted;
    /** If the bounding points to the first and last visible ones should be displayed. */
    bool mDisplayBoundingPoints = true;
    /** The chart values format. */
    std::string mChartValuesFormat;
    /** The stroke style. */
    BasicStroke* mStroke=nullptr;
public:
    virtual ~SimpleSeriesRenderer()=default;
    /**
     * Returns the series color.
     *
     * @return the series color
     */
    int getColor() const{
        return mColor;
    }

    /**
     * Sets the series color.
     *
     * @param color the series color
     */
    void setColor(int color) {
        mColor = color;
    }

    /**
     * Returns if the chart point values should be displayed as text.
     *
     * @return if the chart point values should be displayed as text
     */
    bool isDisplayChartValues() const{
        return mDisplayChartValues;
    }

    /**
     * Sets if the chart point values should be displayed as text.
     *
     * @param display if the chart point values should be displayed as text
     */
    void setDisplayChartValues(bool display) {
        mDisplayChartValues = display;
    }

    /**
     * Returns the chart values minimum distance.
     *
     * @return the chart values minimum distance
     */
    int getDisplayChartValuesDistance() const{
        return mDisplayChartValuesDistance;
    }

    /**
     * Sets chart values minimum distance.
     *
     * @param distance the chart values minimum distance
     */
    void setDisplayChartValuesDistance(int distance) {
        mDisplayChartValuesDistance = distance;
    }

    /**
     * Returns the chart values text size.
     *
     * @return the chart values text size
     */
    float getChartValuesTextSize() const{
        return mChartValuesTextSize;
    }

    /**
     * Sets the chart values text size.
     *
     * @param textSize the chart values text size
     */
    void setChartValuesTextSize(float textSize) {
        mChartValuesTextSize = textSize;
    }

    /**
     * Returns the chart values text align.
     *
     * @return the chart values text align
     */
    int/*Align*/ getChartValuesTextAlign() const{
        return mChartValuesTextAlign;
    }

    /**
     * Sets the chart values text align.
     *
     * @param align the chart values text align
     */
    void setChartValuesTextAlign(int/*Align*/ align) {
        mChartValuesTextAlign = align;
    }

    /**
     * Returns the chart values spacing from the data point.
     *
     * @return the chart values spacing
     */
    float getChartValuesSpacing() const{
        return mChartValuesSpacing;
    }

    /**
     * Sets the chart values spacing from the data point.
     *
     * @param spacing the chart values spacing (in pixels) from the chart data
     *          point
     */
    void setChartValuesSpacing(float spacing) {
        mChartValuesSpacing = spacing;
    }

    /**
     * Returns the stroke style.
     *
     * @return the stroke style
     */
    BasicStroke* getStroke() const{
        return mStroke;
    }

    /**
     * Sets the stroke style.
     *
     * @param stroke the stroke style
     */
    void setStroke(BasicStroke* stroke) {
        mStroke = stroke;
    }

    /**
     * Returns the gradient is enabled value.
     *
     * @return the gradient enabled
     */
    bool isGradientEnabled() const{
        return mGradientEnabled;
    }

    /**
     * Sets the gradient enabled value.
     *
     * @param enabled the gradient enabled
     */
    void setGradientEnabled(bool enabled) {
        mGradientEnabled = enabled;
    }

    /**
     * Returns the gradient start value.
     *
     * @return the gradient start value
     */
    double getGradientStartValue() const{
        return mGradientStartValue;
    }

    /**
     * Returns the gradient start color.
     *
     * @return the gradient start color
     */
    int getGradientStartColor() const{
        return mGradientStartColor;
    }

    /**
     * Sets the gradient start value and color.
     *
     * @param start the gradient start value
     * @param color the gradient start color
     */
    void setGradientStart(double start, int color) {
        mGradientStartValue = start;
        mGradientStartColor = color;
    }

    /**
     * Returns the gradient stop value.
     *
     * @return the gradient stop value
     */
    double getGradientStopValue() const{
        return mGradientStopValue;
    }

    /**
     * Returns the gradient stop color.
     *
     * @return the gradient stop color
     */
    int getGradientStopColor() const{
        return mGradientStopColor;
    }

    /**
     * Sets the gradient stop value and color.
     *
     * @param start the gradient stop value
     * @param color the gradient stop color
     */
    void setGradientStop(double start, int color) {
        mGradientStopValue = start;
        mGradientStopColor = color;
    }

    /**
     * Returns if the legend item for this renderer should be visible.
     *
     * @return the visibility flag for the legend item for this renderer
     */
    bool isShowLegendItem() const{
        return mShowLegendItem;
    }

    /**
     * Sets if the legend item for this renderer should be visible.
     *
     * @param showLegend the visibility flag for the legend item for this renderer
     */
    void setShowLegendItem(bool showLegend) {
        mShowLegendItem = showLegend;
    }

    /**
     * Returns if the item is displayed highlighted.
     *
     * @return the highlighted flag for the item for this renderer
     */
    bool isHighlighted() const{
        return mHighlighted;
    }

    /**
     * Sets if the item for this renderer should be highlighted. Pie chart is supported for now.
     *
     * @param highlighted the highlighted flag for the item for this renderer
     */
    void setHighlighted(bool highlighted) {
        mHighlighted = highlighted;
    }

    /**
     * Returns if the bounding points of the first and last visible ones should be displayed.
     *
     * @return the bounding points display
     */
    bool isDisplayBoundingPoints() const{
        return mDisplayBoundingPoints;
    }

    /**
     * Sets if the bounding points of the first and last visible ones should be displayed.
     *
     * @param display the bounding points display
     */
    void setDisplayBoundingPoints(bool display) {
        mDisplayBoundingPoints = display;
    }

    /**
     * Returns the number format for displaying chart values.
     *
     * @return the number format for chart values
     */
    std::string getChartValuesFormat() const{
        return mChartValuesFormat;
    }

    /**
     * Sets the number format for displaying chart values.
     *
     * @param format the number format for chart values
     */
    void setChartValuesFormat(const std::string& format) {
        mChartValuesFormat = format;
    }
};
}/*endof namespace*/
#endif/*SIMPLE_SERIES_RENDERER_H__*/
