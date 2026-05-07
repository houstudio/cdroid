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
#ifndef __XYMULTIPLE_SERIES_RENDERER_H__
#define __XYMULTIPLE_SERIES_RENDERER_H__
#include <widget/achart/renderer/defaultrenderer.h>
namespace cdroid{
/**
 * Multiple XY series renderer.
 */
class XYMultipleSeriesRenderer :public DefaultRenderer {
public:
    enum Orientation{
        HORIZONTAL=0,
        VERTICAL=90
    };
private:
    /** The X axis title. */
    std::string mXTitle;
    /** The Y axis title. */
    std::vector<std::string> mYTitle;
    /** The axis title text size. */
    float mAxisTitleTextSize = 12;
    /** The start value in the X axis range. */
    std::vector<double> mMinX;
    /** The end value in the X axis range. */
    std::vector<double> mMaxX;
    /** The start value in the Y axis range. */
    std::vector<double> mMinY;
    /** The end value in the Y axis range. */
    std::vector<double> mMaxY;
    /** The approximative number of labels on the x axis. */
    int mXLabels = 5;
    /** The approximative number of labels on the y axis. */
    int mYLabels = 5;
    /** The current orientation of the chart. */
    Orientation mOrientation = Orientation::HORIZONTAL;
    /** The X axis text labels. */
    std::map<double, std::string> mXTextLabels;
    /** The Y axis text labels. */
    std::map<int, std::map<double, std::string>> mYTextLabels;
    /** A flag for enabling or not the pan on the X axis. */
    bool mPanXEnabled = true;
    /** A flag for enabling or not the pan on the Y axis. */
    bool mPanYEnabled = true;
    /** A flag for enabling or not the zoom on the X axis. */
    bool mZoomXEnabled = true;
    /** A flag for enabling or not the zoom on the Y axis . */
    bool mZoomYEnabled = true;
    /** The spacing between bars, in bar charts. */
    double mBarSpacing = 0;
    /** The margins colors. */
    int mMarginsColor = NO_COLOR;
    /** The pan limits. */
    std::vector<double> mPanLimits;
    /** The zoom limits. */
    std::vector<double> mZoomLimits;
    /** The X axis labels rotation angle. */
    float mXLabelsAngle=0.f;
    /** The Y axis labels rotation angle. */
    float mYLabelsAngle=0.f;
    /** The initial axis range. */
    std::map<int, std::vector<double>> initialRange;
    /** The point size for charts displaying points. */
    float mPointSize = 3;
    /** The grid color. */
    int mGridColor = 0x48c8c8c8;//Color.argb(75, 200, 200, 200);
    /** The number of scales. */
    int mScalesCount;
    /** The X axis labels alignment. */
    int/*Align*/ xLabelsAlign=1;// = Align.CENTER;
    /** The Y axis labels alignment. */
    std::vector<int/*Align*/> yLabelsAlign;
    /** The X text label padding. */
    float mXLabelsPadding = 0;
    /** The Y text label padding. */
    float mYLabelsPadding = 0;
    /** The Y axis labels vertical padding. */
    float mYLabelsVerticalPadding = 2;
    /** The Y axis alignment. */
    std::vector<int/*Align*/> yAxisAlign;
    /** The X axis labels color. */
    int mXLabelsColor = TEXT_COLOR;
    /** The Y axis labels color. */
    std::vector<int> mYLabelsColor = { TEXT_COLOR };
    /**
     * If X axis value selection algorithm to be used. Only used by the time
     * charts.
     */
    bool mXRoundedLabels = true;
    /** The label format. */
    NumberFormat* mLabelFormat=nullptr;
    /** A constant value for the bar chart items width. */
    float mBarWidth = -1;
    /** The zoom in limit permitted in the axis X */
    double mZoomInLimitX = 0;
    /** The zoom in limit permitted in the axis Y */
    double mZoomInLimitY = 0;
public:
    XYMultipleSeriesRenderer():XYMultipleSeriesRenderer(1){
    }

    XYMultipleSeriesRenderer(int scaleNumber) {
        mScalesCount = scaleNumber;
        mLabelFormat = nullptr;
        initAxesRange(scaleNumber);
    }
    ~XYMultipleSeriesRenderer()override{
        delete mLabelFormat;
    }
    void initAxesRange(int scales) {
        mYTitle.resize(scales);// = new String[scales];
        yLabelsAlign.resize(scales);// = new Align[scales];
        yAxisAlign.resize(scales);//= new Align[scales];
        mYLabelsColor.resize(scales);// = new int[scales];
        mMinX.resize(scales);// = new double[scales];
        mMaxX.resize(scales);// = new double[scales];
        mMinY.resize(scales);// = new double[scales];
        mMaxY.resize(scales);// = new double[scales];
        for (int i = 0; i < scales; i++) {
            mYLabelsColor[i] = TEXT_COLOR;
            initAxesRangeForScale(i);
        }
    }

    void initAxesRangeForScale(int i) {
        mMinX[i] = MathHelper::NULL_VALUE;
        mMaxX[i] = -MathHelper::NULL_VALUE;
        mMinY[i] = MathHelper::NULL_VALUE;
        mMaxY[i] = -MathHelper::NULL_VALUE;
        std::vector<double> range = { mMinX[i], mMaxX[i], mMinY[i], mMaxY[i] };
        initialRange[i]= range;
        mYTitle[i] = "";
        mYTextLabels[i]= std::map<double, std::string>();
        yLabelsAlign[i] = 0;//Align.CENTER;
        yAxisAlign[i] = 0;//Align.LEFT;
    }

    /**
     * Returns the current orientation of the chart X axis.
     *
     * @return the chart orientation
     */
    Orientation getOrientation() const{
        return mOrientation;
    }

    /**
     * Sets the current orientation of the chart X axis.
     *
     * @param orientation the chart orientation
     */
    void setOrientation(Orientation orientation) {
        mOrientation = orientation;
    }

    /**
     * Returns the title for the X axis.
     *
     * @return the X axis title
     */
    std::string getXTitle() const{
        return mXTitle;
    }

    /**
     * Sets the title for the X axis.
     *
     * @param title the X axis title
     */
    void setXTitle(const std::string& title) {
        mXTitle = title;
    }

    /**
     * Returns the title for the Y axis.
     *
     * @return the Y axis title
     */
    std::string getYTitle() const{
        return getYTitle(0);
    }

    /**
     * Returns the title for the Y axis.
     *
     * @param scale the renderer scale
     * @return the Y axis title
     */
    std::string getYTitle(int scale) const{
        return mYTitle[scale];
    }

    /**
     * Sets the title for the Y axis.
     *
     * @param title the Y axis title
     */
    void setYTitle(const std::string& title) {
        setYTitle(title, 0);
    }

    /**
     * Sets the title for the Y axis.
     *
     * @param title the Y axis title
     * @param scale the renderer scale
     */
    void setYTitle(const std::string& title, int scale) {
        mYTitle[scale] = title;
    }

    /**
     * Returns the axis title text size.
     *
     * @return the axis title text size
     */
    float getAxisTitleTextSize() const{
        return mAxisTitleTextSize;
    }

    /**
     * Sets the axis title text size.
     *
     * @param textSize the chart axis text size
     */
    void setAxisTitleTextSize(float textSize) {
        mAxisTitleTextSize = textSize;
    }

    /**
     * Returns the start value of the X axis range.
     *
     * @return the X axis range start value
     */
    double getXAxisMin() const{
        return getXAxisMin(0);
    }

    /**
     * Sets the start value of the X axis range.
     *
     * @param min the X axis range start value
     */
    void setXAxisMin(double min) {
        setXAxisMin(min, 0);
    }

    /**
     * Returns if the minimum X value was set.
     *
     * @return the minX was set or not
     */
    bool isMinXSet() const{
        return isMinXSet(0);
    }

    /**
     * Returns the end value of the X axis range.
     *
     * @return the X axis range end value
     */
    double getXAxisMax() const{
        return getXAxisMax(0);
    }

    /**
     * Sets the end value of the X axis range.
     *
     * @param max the X axis range end value
     */
    void setXAxisMax(double max) {
        setXAxisMax(max, 0);
    }

    /**
     * Returns if the maximum X value was set.
     *
     * @return the maxX was set or not
     */
    bool isMaxXSet() const{
        return isMaxXSet(0);
    }

    /**
     * Returns the start value of the Y axis range.
     *
     * @return the Y axis range end value
     */
    double getYAxisMin() const{
        return getYAxisMin(0);
    }

    /**
     * Sets the start value of the Y axis range.
     *
     * @param min the Y axis range start value
     */
    void setYAxisMin(double min) {
        setYAxisMin(min, 0);
    }

    /**
     * Returns if the minimum Y value was set.
     *
     * @return the minY was set or not
     */
    bool isMinYSet() const{
        return isMinYSet(0);
    }

    /**
     * Returns the end value of the Y axis range.
     *
     * @return the Y axis range end value
     */
    double getYAxisMax() const{
        return getYAxisMax(0);
    }

    /**
     * Sets the end value of the Y axis range.
     *
     * @param max the Y axis range end value
     */
    void setYAxisMax(double max) {
        setYAxisMax(max, 0);
    }

    /**
     * Returns if the maximum Y value was set.
     *
     * @return the maxY was set or not
     */
    bool isMaxYSet() const{
        return isMaxYSet(0);
    }

    /**
     * Returns the start value of the X axis range.
     *
     * @param scale the renderer scale
     * @return the X axis range start value
     */
    double getXAxisMin(int scale) const{
        return mMinX[scale];
    }

    /**
     * Sets the start value of the X axis range.
     *
     * @param min the X axis range start value
     * @param scale the renderer scale
     */
    void setXAxisMin(double min, int scale) {
        if (!isMinXSet(scale)) {
            initialRange[scale][0] = min;
        }
        mMinX[scale] = min;
    }

    /**
     * Returns if the minimum X value was set.
     *
     * @param scale the renderer scale
     * @return the minX was set or not
     */
    bool isMinXSet(int scale) const{
        return mMinX[scale] != MathHelper::NULL_VALUE;
    }

    /**
     * Returns the end value of the X axis range.
     *
     * @param scale the renderer scale
     * @return the X axis range end value
     */
    double getXAxisMax(int scale) const{
        return mMaxX[scale];
    }

    /**
     * Sets the end value of the X axis range.
     *
     * @param max the X axis range end value
     * @param scale the renderer scale
     */
    void setXAxisMax(double max, int scale) {
        if (!isMaxXSet(scale)) {
            initialRange[scale][1] = max;
        }
        mMaxX[scale] = max;
    }

    /**
     * Returns if the maximum X value was set.
     *
     * @param scale the renderer scale
     * @return the maxX was set or not
     */
    bool isMaxXSet(int scale) const{
        return mMaxX[scale] != -MathHelper::NULL_VALUE;
    }

    /**
     * Returns the start value of the Y axis range.
     *
     * @param scale the renderer scale
     * @return the Y axis range end value
     */
    double getYAxisMin(int scale) const{
        return mMinY[scale];
    }

    /**
     * Sets the start value of the Y axis range.
     *
     * @param min the Y axis range start value
     * @param scale the renderer scale
     */
    void setYAxisMin(double min, int scale) {
        if (!isMinYSet(scale)) {
            initialRange[scale][2] = min;
        }
        mMinY[scale] = min;
    }

    /**
     * Returns if the minimum Y value was set.
     *
     * @param scale the renderer scale
     * @return the minY was set or not
     */
    bool isMinYSet(int scale) const{
        return mMinY[scale] != MathHelper::NULL_VALUE;
    }

    /**
     * Returns the end value of the Y axis range.
     *
     * @param scale the renderer scale
     * @return the Y axis range end value
     */
    double getYAxisMax(int scale) const{
        return mMaxY[scale];
    }

    /**
     * Sets the end value of the Y axis range.
     *
     * @param max the Y axis range end value
     * @param scale the renderer scale
     */
    void setYAxisMax(double max, int scale) {
        if (!isMaxYSet(scale)) {
            initialRange[scale][3] = max;
        }
        mMaxY[scale] = max;
    }

    /**
     * Returns if the maximum Y value was set.
     *
     * @param scale the renderer scale
     * @return the maxY was set or not
     */
    bool isMaxYSet(int scale) const{
        return mMaxY[scale] != -MathHelper::NULL_VALUE;
    }

    /**
     * Returns the approximate number of labels for the X axis.
     *
     * @return the approximate number of labels for the X axis
     */
    int getXLabels() const{
        return mXLabels;
    }

    /**
     * Sets the approximate number of labels for the X axis.
     *
     * @param xLabels the approximate number of labels for the X axis
     */
    void setXLabels(int xLabels) {
        mXLabels = xLabels;
    }

    /**
     * Adds a new text label for the specified X axis value.
     *
     * @param x the X axis value
     * @param text the text label
     * @deprecated use addXTextLabel instead
     */
    void addTextLabel(double x, const std::string& text) {
        addXTextLabel(x, text);
    }

    /**
     * Adds a new text label for the specified X axis value.
     *
     * @param x the X axis value
     * @param text the text label
     */
    void addXTextLabel(double x, const std::string& text) {
        mXTextLabels[x]= text;
    }

    /**
     * Removes text label for the specified X axis value.
     *
     * @param x the X axis value
     */
    void removeXTextLabel(double x) {
        auto it = mXTextLabels.find(x);
        mXTextLabels.erase(it);
    }

    /**
     * Returns the X axis text label at the specified X axis value.
     *
     * @param x the X axis value
     * @return the X axis text label
     */
    std::string getXTextLabel(double x) const{
        return mXTextLabels.find(x)->second;
    }

    /**
     * Returns the X text label locations.
     *
     * @return the X text label locations
     */
    std::vector<double> getXTextLabelLocations() const{
        //return mXTextLabels.keySet().toArray(new Double[0]);
        std::vector<double> result(mXTextLabels.size());
        //result.reserve(mXTextLabels.size());
        for (const auto &it: mXTextLabels) {
            result.push_back(it.first);
        }
        return result;
    }

    /**
     * Clears the existing text labels.
     *
     * @deprecated use clearXTextLabels instead
     */
    void clearTextLabels() {
        clearXTextLabels();
    }

    /**
     * Clears the existing text labels on the X axis.
     */
    void clearXTextLabels() {
        mXTextLabels.clear();
    }

    /**
     * If X axis labels should be rounded.
     *
     * @return if rounded time values to be used
     */
    bool isXRoundedLabels() const{
        return mXRoundedLabels;
    }

    /**
     * Sets if X axis rounded time values to be used.
     *
     * @param rounded rounded values to be used
     */
    void setXRoundedLabels(bool rounded) {
        mXRoundedLabels = rounded;
    }

    /**
     * Adds a new text label for the specified Y axis value.
     *
     * @param y the Y axis value
     * @param text the text label
     */
    void addYTextLabel(double y, const std::string& text) {
        addYTextLabel(y, text, 0);
    }

    /**
     * Removes text label for the specified Y axis value.
     *
     * @param y the Y axis value
     */
    void removeYTextLabel(double y) {
        removeYTextLabel(y, 0);
    }

    /**
     * Adds a new text label for the specified Y axis value.
     *
     * @param y the Y axis value
     * @param text the text label
     * @param scale the renderer scale
     */
    void addYTextLabel(double y, const std::string& text, int scale) {
        mYTextLabels[scale][y]= text;
    }

    /**
     * Removes text label for the specified Y axis value.
     *
     * @param y the Y axis value
     * @param scale the renderer scale
     */
    void removeYTextLabel(double y, int scale) {
        mYTextLabels.find(scale)->second.erase(y);
    }

    /**
     * Returns the Y axis text label at the specified Y axis value.
     *
     * @param y the Y axis value
     * @return the Y axis text label
     */
    std::string getYTextLabel(double y) const{
        return getYTextLabel(y, 0);
    }

    /**
     * Returns the Y axis text label at the specified Y axis value.
     *
     * @param y the Y axis value
     * @param scale the renderer scale
     * @return the Y axis text label
     */
    std::string getYTextLabel(double y, int scale) const{
        if(scale>=0&&scale<mYTextLabels.size()){
            auto sit=mYTextLabels.find(scale);
            auto it =sit->second.find(y);//std::map<int, std::map<double, std::string>> mYTextLabels
            return (it!=sit->second.end())?it->second:std::string("");
        }
        return "";//ifit==mYTextLabels.end()?"":it->second.at(y);
    }

    /**
     * Returns the Y text label locations.
     *
     * @return the Y text label locations
     */
    std::vector<double> getYTextLabelLocations() const{
        return getYTextLabelLocations(0);
    }

    /**
     * Returns the Y text label locations.
     *
     * @param scale the renderer scale
     * @return the Y text label locations
     */
    std::vector<double> getYTextLabelLocations(int scale) const{
        if(scale>=0&&scale<mYTextLabels.size()){
            auto sit=mYTextLabels.find(scale);
            std::vector<double> keys;
            keys.reserve(sit->second.size());
            keys.reserve(sit->second.size());
            for (const auto& pair : sit->second) {
               keys.push_back(pair.first);
            }
            return keys;
        }
        return {};
    }

    /**
     * Clears the existing text labels on the Y axis.
     */
    void clearYTextLabels() {
        clearYTextLabels(0);
    }

    /**
     * Clears the existing text labels on the Y axis.
     *
     * @param scale the renderer scale
     */
    void clearYTextLabels(int scale) {
        if(scale>=0&&scale<=mYTextLabels.size()){
           mYTextLabels[scale].clear();
        }
    }

    /**
     * Returns the approximate number of labels for the Y axis.
     *
     * @return the approximate number of labels for the Y axis
     */
    int getYLabels() const{
        return mYLabels;
    }

    /**
     * Sets the approximate number of labels for the Y axis.
     *
     * @param yLabels the approximate number of labels for the Y axis
     */
    void setYLabels(int yLabels) {
        mYLabels = yLabels;
    }

    /**
     * Sets if the chart point values should be displayed as text.
     *
     * @param display if the chart point values should be displayed as text
     * @deprecated use SimpleSeriesRenderer.setDisplayChartValues() instead
     */
    void setDisplayChartValues(bool display) {
        auto renderers = getSeriesRenderers();
        for (auto renderer : renderers) {
            renderer->setDisplayChartValues(display);
        }
    }

    /**
     * Sets the chart values text size.
     *
     * @param textSize the chart values text size
     * @deprecated use SimpleSeriesRenderer.setChartValuesTextSize() instead
     */
    void setChartValuesTextSize(float textSize) {
        auto renderers = getSeriesRenderers();
        for (auto renderer : renderers) {
            renderer->setChartValuesTextSize(textSize);
        }
    }

    /**
     * Returns the constant bar chart item width in pixels.
     *
     * @return the bar width
     */
    float getBarWidth() const{
        return mBarWidth;
    }

    /**
     * Sets the bar chart item constant width in pixels.
     *
     * @param width width in pixels
     */
    void setBarWidth(float width) {
        mBarWidth = width;
    }

    /**
     * Returns the enabled state of the pan on at least one axis.
     *
     * @return if pan is enabled
     */
    bool isPanEnabled() const{
        return isPanXEnabled() || isPanYEnabled();
    }

    /**
     * Returns the enabled state of the pan on X axis.
     *
     * @return if pan is enabled on X axis
     */
    bool isPanXEnabled() const{
        return mPanXEnabled;
    }

    /**
     * Returns the enabled state of the pan on Y axis.
     *
     * @return if pan is enabled on Y axis
     */
    bool isPanYEnabled() const{
        return mPanYEnabled;
    }

    /**
     * Sets the enabled state of the pan.
     *
     * @param enabledX pan enabled on X axis
     * @param enabledY pan enabled on Y axis
     */
    void setPanEnabled(bool enabledX, bool enabledY) {
        mPanXEnabled = enabledX;
        mPanYEnabled = enabledY;
    }

    /**
     * Override {@link DefaultRenderer#setPanEnabled(bool)} so it can be
     * delegated to {@link #setPanEnabled(bool, bool)}.
     */
    void setPanEnabled(bool enabled) override{
        setPanEnabled(enabled, enabled);
    }

    /**
     * Returns the enabled state of the zoom on at least one axis.
     *
     * @return if zoom is enabled
     */
    bool isZoomEnabled() const{
        return isZoomXEnabled() || isZoomYEnabled();
    }

    /**
     * Returns the enabled state of the zoom on X axis.
     *
     * @return if zoom is enabled on X axis
     */
    bool isZoomXEnabled() const{
        return mZoomXEnabled;
    }

    /**
     * Returns the enabled state of the zoom on Y axis.
     *
     * @return if zoom is enabled on Y axis
     */
    bool isZoomYEnabled() const{
        return mZoomYEnabled;
    }

    /**
     * Sets the enabled state of the zoom.
     *
     * @param enabledX zoom enabled on X axis
     * @param enabledY zoom enabled on Y axis
     */
    void setZoomEnabled(bool enabledX, bool enabledY) {
        mZoomXEnabled = enabledX;
        mZoomYEnabled = enabledY;
    }

    /**
     * Returns the spacing between bars, in bar charts.
     *
     * @return the spacing between bars
     * @deprecated use getBarSpacing instead
     */
    double getBarsSpacing() const{
        return getBarSpacing();
    }

    /**
     * Returns the spacing between bars, in bar charts.
     *
     * @return the spacing between bars
     */
    double getBarSpacing() const{
        return mBarSpacing;
    }

    /**
     * Sets the spacing between bars, in bar charts. Only available for bar
     * charts. This is a coefficient of the bar width. For instance, if you want
     * the spacing to be a half of the bar width, set this value to 0.5.
     *
     * @param spacing the spacing between bars coefficient
     */
    void setBarSpacing(double spacing) {
        mBarSpacing = spacing;
    }

    /**
     * Returns the margins color.
     *
     * @return the margins color
     */
    int getMarginsColor() const{
        return mMarginsColor;
    }

    /**
     * Sets the color of the margins.
     *
     * @param color the margins color
     */
    void setMarginsColor(int color) {
        mMarginsColor = color;
    }

    /**
     * Returns the grid color.
     *
     * @return the grid color
     */
    int getGridColor() const{
        return mGridColor;
    }

    /**
     * Sets the color of the grid.
     *
     * @param color the grid color
     */
    void setGridColor(int color) {
        mGridColor = color;
    }

    /**
     * Returns the pan limits.
     *
     * @return the pan limits
     */
    std::vector<double> getPanLimits() const{
        return mPanLimits;
    }

    /**
     * Sets the pan limits as an array of 4 values. Setting it to null or a
     * different size array will disable the panning limitation. Values:
     * [panMinimumX, panMaximumX, panMinimumY, panMaximumY]
     *
     * @param panLimits the pan limits
     */
    void setPanLimits(const std::vector<double>& panLimits) {
        mPanLimits = panLimits;
    }

    /**
     * Returns the zoom limits.
     *
     * @return the zoom limits
     */
    std::vector<double> getZoomLimits() const{
        return mZoomLimits;
    }

    /**
     * Sets the zoom limits as an array of 4 values. Setting it to null or a
     * different size array will disable the zooming limitation. Values:
     * [zoomMinimumX, zoomMaximumX, zoomMinimumY, zoomMaximumY]
     *
     * @param zoomLimits the zoom limits
     */
    void setZoomLimits(const std::vector<double>& zoomLimits) {
        mZoomLimits = zoomLimits;
    }

    /**
     * Returns the rotation angle of labels for the X axis.
     *
     * @return the rotation angle of labels for the X axis
     */
    float getXLabelsAngle() const{
        return mXLabelsAngle;
    }

    /**
     * Sets the rotation angle (in degrees) of labels for the X axis.
     *
     * @param angle the rotation angle of labels for the X axis
     */
    void setXLabelsAngle(float angle) {
        mXLabelsAngle = angle;
    }

    /**
     * Returns the rotation angle of labels for the Y axis.
     *
     * @return the approximate number of labels for the Y axis
     */
    float getYLabelsAngle() const{
        return mYLabelsAngle;
    }

    /**
     * Sets the rotation angle (in degrees) of labels for the Y axis.
     *
     * @param angle the rotation angle of labels for the Y axis
     */
    void setYLabelsAngle(float angle) {
        mYLabelsAngle = angle;
    }

    /**
     * Returns the size of the points, for charts displaying points.
     *
     * @return the point size
     */
    float getPointSize() const{
        return mPointSize;
    }

    /**
     * Sets the size of the points, for charts displaying points.
     *
     * @param size the point size
     */
    void setPointSize(float size) {
        mPointSize = size;
    }

    void setRange(const std::vector<double>& range) {
        setRange(range, 0);
    }

    /**
     * Sets the axes range values.
     *
     * @param range an array having the values in this order: minX, maxX, minY,
     *          maxY
     * @param scale the renderer scale
     */
    void setRange(const std::vector<double>&range, int scale) {
        setXAxisMin(range[0], scale);
        setXAxisMax(range[1], scale);
        setYAxisMin(range[2], scale);
        setYAxisMax(range[3], scale);
    }

    bool isInitialRangeSet() const{
        return isInitialRangeSet(0);
    }

    /**
     * Returns if the initial range is set.
     *
     * @param scale the renderer scale
     * @return the initial range was set or not
     */
    bool isInitialRangeSet(int scale) const{
        return initialRange.find(scale) != initialRange.end();
    }

    /**
     * Returns the initial range.
     *
     * @return the initial range
     */
    std::vector<double> getInitialRange() const{
        return getInitialRange(0);
    }

    /**
     * Returns the initial range.
     *
     * @param scale the renderer scale
     * @return the initial range
     */
    std::vector<double> getInitialRange(int scale) const{
        return initialRange.at(scale);
    }

    /**
     * Sets the axes initial range values. This will be used in the zoom fit tool.
     *
     * @param range an array having the values in this order: minX, maxX, minY,
     *          maxY
     */
    void setInitialRange(const std::vector<double>& range) {
        setInitialRange(range, 0);
    }

    /**
     * Sets the axes initial range values. This will be used in the zoom fit tool.
     *
     * @param range an array having the values in this order: minX, maxX, minY,
     *          maxY
     * @param scale the renderer scale
     */
    void setInitialRange(const std::vector<double>& range, int scale) {
        initialRange[scale]= range;
    }

    /**
     * Returns the X axis labels color.
     *
     * @return the X axis labels color
     */
    int getXLabelsColor() const{
        return mXLabelsColor;
    }

    /**
     * Returns the Y axis labels color.
     *
     * @return the Y axis labels color
     */
    int getYLabelsColor(int scale) const{
        return mYLabelsColor[scale];
    }

    /**
     * Sets the X axis labels color.
     *
     * @param color the X axis labels color
     */
    void setXLabelsColor(int color) {
        mXLabelsColor = color;
    }

    /**
     * Sets the Y axis labels color.
     *
     * @param scale the renderer scale
     * @param color the Y axis labels color
     */
    void setYLabelsColor(int scale, int color) {
        mYLabelsColor[scale] = color;
    }

    /**
     * Returns the X axis labels alignment.
     *
     * @return X labels alignment
     */
    int/*Align*/ getXLabelsAlign() const{
        return xLabelsAlign;
    }

    /**
     * Sets the X axis labels alignment.
     *
     * @param align the X labels alignment
     */
    void setXLabelsAlign(int/*Align*/ align) {
        xLabelsAlign = align;
    }

    /**
     * Returns the Y axis labels alignment.
     *
     * @param scale the renderer scale
     * @return Y labels alignment
     */
    int/*Align*/ getYLabelsAlign(int scale) const{
        return yLabelsAlign[scale];
    }

    void setYLabelsAlign(int/*Align*/ align) {
        setYLabelsAlign(align, 0);
    }

    int/*Align*/ getYAxisAlign(int scale) const{
        return yAxisAlign[scale];
    }

    void setYAxisAlign(int/*Align*/ align, int scale) {
        yAxisAlign[scale] = align;
    }

    /**
     * Sets the Y axis labels alignment.
     *
     * @param align the Y labels alignment
     */
    void setYLabelsAlign(int/*Align*/ align, int scale) {
        yLabelsAlign[scale] = align;
    }

    /**
     * Returns the X labels padding.
     *
     * @return X labels padding
     */
    float getXLabelsPadding() const{
        return mXLabelsPadding;
    }

    /**
     * Sets the X labels padding
     *
     * @param padding the amount of padding between the axis and the label
     */
    void setXLabelsPadding(float padding) {
        mXLabelsPadding = padding;
    }

    /**
     * Returns the Y labels padding.
     *
     * @return Y labels padding
     */
    float getYLabelsPadding() const{
        return mYLabelsPadding;
    }

    /**
     * Sets the Y labels vertical padding
     *
     * @param padding the amount of vertical padding
     */
    void setYLabelsVerticalPadding(float padding) {
        mYLabelsVerticalPadding = padding;
    }

    /**
     * Returns the Y labels vertical padding.
     *
     * @return Y labels vertical padding
     */
    float getYLabelsVerticalPadding() const{
        return mYLabelsVerticalPadding;
    }

    /**
     * Sets the Y labels padding
     *
     * @param padding the amount of padding between the axis and the label
     */
    void setYLabelsPadding(float padding) {
        mYLabelsPadding = padding;
    }

    /**
     * Returns the number format for displaying labels.
     *
     * @return the number format for labels
     */
    NumberFormat* getLabelFormat() const{
        return mLabelFormat;
    }

    /**
     * Sets the number format for displaying labels.
     *
     * @param format the number format for labels
     */
    void setLabelFormat(NumberFormat* format) {
        mLabelFormat = format;
    }

    /**
     * Returns the zoom in limit permitted in the axis X.
     *
     * @return the maximum zoom in permitted in the axis X
     *
     * @see #setZoomInLimitX(double)
     */
    double getZoomInLimitX() const{
        return mZoomInLimitX;
    }

    /**
     * Sets the zoom in limit permitted in the axis X.
     *
     * This function prevent that the distance between {@link #getXAxisMin()} and
     * {@link #getXAxisMax()} can't be greater or equal than
     * {@link #getZoomInLimitX()}
     *
     * @param zoomInLimitX the maximum distance permitted between
     * {@link #getXAxisMin()} and {@link #getXAxisMax()}.
     */
    void setZoomInLimitX(double zoomInLimitX) {
        mZoomInLimitX = zoomInLimitX;
    }

    /**
     * Returns the zoom in limit permitted in the axis Y.
     *
     * @return the maximum in zoom permitted in the axis Y
     *
     * @see #setZoomInLimitY(double)
     */
    double getZoomInLimitY() const{
        return mZoomInLimitY;
    }

    /**
     * Sets zoom in limit permitted in the axis Y.
     *
     * This function prevent that the distance between {@link #getYAxisMin()} and
     * {@link #getYAxisMax()} can't be greater or equal than
     * {@link #getZoomInLimitY()}
     *
     * @param zoomInLimitY the maximum distance permitted between
     * {@link #getYAxisMin()} and {@link #getYAxisMax()}
     */
    void setZoomInLimitY(double zoomInLimitY) {
        mZoomInLimitY = zoomInLimitY;
    }

    int getScalesCount() const{
        return mScalesCount;
    }
};
}/*endof namespace*/
#endif/*__XYMULTIPLE_SERIES_RENDERER_H__*/
