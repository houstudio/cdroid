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

#ifndef __DEFAULT_RENDERER_H__
#define __DEFAULT_RENDERER_H__
#include <core/color.h>
#include <core/typeface.h>
#include <widget/achart/renderer/simpleseriesrenderer.h>

namespace cdroid{
/**
 * An abstract renderer to be extended by the multiple series classes.
 */
class DefaultRenderer {
public:
    /** A no color constant. */
    static constexpr int NO_COLOR = 0;
    /** The default background color. */
    static constexpr int BACKGROUND_COLOR = Color::BLACK;
    /** The default color for text. */
    static constexpr int TEXT_COLOR = Color::LTGRAY;
private:
    /** The chart title. */
    std::string mChartTitle = "";
    /** The chart title text size. */
    float mChartTitleTextSize = 15;
    /** A text font for regular text, like the chart labels. */
    //static constexpr Typeface REGULAR_TEXT_FONT = Typeface::create(Typeface.SERIF, Typeface.NORMAL);
    /** The typeface name for the texts. */
    std::string mTextTypefaceName;// = REGULAR_TEXT_FONT.toString();
    /** The typeface style for the texts. */
    int mTextTypefaceStyle = Typeface::NORMAL;
    /** The typeface for the texts */
    Typeface* mTextTypeface;
    /** The chart background color. */
    int mBackgroundColor=0;
    /** If the background color is applied. */
    bool mApplyBackgroundColor=false;
    /** If the axes are visible. */
    bool mShowAxes = true;
    /** The xaxis/yaxis color. */
    int mXAxisColor= TEXT_COLOR;
    int mYAxisColor= TEXT_COLOR;
    /** If the xLabels/yLabels are visible. */
    bool mShowXLabels = true;
    bool mShowYLabels=true;
    /** If the tick marks are visible. */
    bool mShowTickMarks = true;
    /** The labels color. */
    int mLabelsColor = TEXT_COLOR;
    /** The labels text size. */
    float mLabelsTextSize = 10;
    /** If the legend is visible. */
    bool mShowLegend = true;
    /** The legend text size. */
    float mLegendTextSize = 12;
    /** If the legend should size to fit. */
    bool mFitLegend = false;
    /** If the X axis grid should be displayed. */
    bool mShowGridX = false;
    /** If the Y axis grid should be displayed. */
    bool mShowGridY = false;
    /** The grid line width. */
    float mGridLineWidth=1.f;
    /** If the custom text grid should be displayed. */
    bool mShowCustomTextGridX = false;
    /** If the custom text grid should be displayed on the Y axis. */
    bool mShowCustomTextGridY = false;
    /** The simple renderers that are included in this multiple series renderer. */
    std::vector<std::shared_ptr<SimpleSeriesRenderer>> mRenderers;
    /** The antialiasing flag. */
    bool mAntialiasing = true;
    /** The legend height. */
    int mLegendHeight = 0;
    /** The margins size. */
    std::vector<int> mMargins = { 20, 30, 10, 20 };
    /** A value to be used for scaling the chart. */
    float mScale = 1;
    /** A flag for enabling the pan. */
    bool mPanEnabled = true;
    /** A flag for enabling the zoom. */
    bool mZoomEnabled = true;
    /** A flag for enabling the visibility of the zoom buttons. */
    bool mZoomButtonsVisible = false;
    /** The zoom rate. */
    float mZoomRate = 1.5f;
    /** A flag for enabling the external zoom. */
    bool mExternalZoomEnabled = false;
    /** The original chart scale. */
    float mOriginalScale = mScale;
    /** A flag for enabling the click on elements. */
    bool mClickEnabled = false;
    /** The selectable radius around a clickable point. */
    int mSelectableBuffer = 15;
    /** If the chart should display the values (available for pie chart). */
    bool mDisplayValues;

    /**
     * A flag to be set if the chart is inside a scroll and doesn't need to shrink
     * when not enough space.
     */
    bool mInScroll;
    /** The start angle for circular charts such as pie, doughnut, etc. */
    float mStartAngle = 0;
public:
    /**
     * Returns the chart title.
     *
     * @return the chart title
     */
    std::string getChartTitle() const{
        return mChartTitle;
    }

    /**
     * Sets the chart title.
     *
     * @param title the chart title
     */
    void setChartTitle(const std::string& title) {
        mChartTitle = title;
    }

    /**
     * Returns the chart title text size.
     *
     * @return the chart title text size
     */
    float getChartTitleTextSize() const{
        return mChartTitleTextSize;
    }

    /**
     * Sets the chart title text size.
     *
     * @param textSize the chart title text size
     */
    void setChartTitleTextSize(float textSize) {
        mChartTitleTextSize = textSize;
    }

    /**
     * Adds a simple renderer to the multiple renderer.
     *
     * @param renderer the renderer to be added
     */
    void addSeriesRenderer(const std::shared_ptr<SimpleSeriesRenderer>& renderer) {
        mRenderers.push_back(renderer);
    }

    /**
     * Adds a simple renderer to the multiple renderer.
     *
     * @param index the index in the renderers list
     * @param renderer the renderer to be added
     */
    void addSeriesRenderer(int index, const std::shared_ptr<SimpleSeriesRenderer>& renderer) {
        mRenderers.insert(mRenderers.begin()+index, renderer);
    }

    /**
     * Removes a simple renderer from the multiple renderer.
     *
     * @param renderer the renderer to be removed
     */
    void removeSeriesRenderer(const std::shared_ptr<SimpleSeriesRenderer>& renderer) {
        auto it =std::find(mRenderers.begin(),mRenderers.end(),renderer);
        if(it!=mRenderers.end())mRenderers.erase(it);
    }

    /**
     * Removes all renderers from the multiple renderer.
     */
    void removeAllRenderers() {
        mRenderers.clear();
    }

    /**
     * Returns the simple renderer from the multiple renderer list.
     *
     * @param index the index in the simple renderers list
     * @return the simple renderer at the specified index
     */
    const std::shared_ptr<SimpleSeriesRenderer>& getSeriesRendererAt(int index) const{
        return mRenderers.at(index);
    }

    /**
     * Returns the simple renderers count in the multiple renderer list.
     *
     * @return the simple renderers count
     */
    int getSeriesRendererCount() const{
        return mRenderers.size();
    }

    /**
     * Returns an array of the simple renderers in the multiple renderer list.
     *
     * @return the simple renderers array
     */
    std::vector<std::shared_ptr<SimpleSeriesRenderer>> getSeriesRenderers() const{
        return mRenderers;
    }

    /**
     * Returns the background color.
     *
     * @return the background color
     */
    int getBackgroundColor() const{
        return mBackgroundColor;
    }

    /**
     * Sets the background color.
     *
     * @param color the background color
     */
    void setBackgroundColor(int color) {
        mBackgroundColor = color;
    }

    /**
     * Returns if the background color should be applied.
     *
     * @return the apply flag for the background color.
     */
    bool isApplyBackgroundColor() const{
        return mApplyBackgroundColor;
    }

    /**
     * Sets if the background color should be applied.
     *
     * @param apply the apply flag for the background color
     */
    void setApplyBackgroundColor(bool apply) {
        mApplyBackgroundColor = apply;
    }

    /**
     * Returns the axes color.
     *
     * @return the axes color
     */
    int getAxesColor() const{
        return (mXAxisColor!=TEXT_COLOR)? mXAxisColor:mYAxisColor;
    }

    /**
     * Sets the axes color.
     *
     * @param color the axes color
     */
    void setAxesColor(int color) {
        mXAxisColor = color;
        mYAxisColor= color;
    }
    /**
     * Returns the color of the Y axis
     * 
     * @return the Y axis color
     */
    int getYAxisColor() const{
        return mYAxisColor;
    }
   
    /**
     * Sets the Y axis color.
     * 
     * @param color the Y axis color
     */
    void setYAxisColor(int color) {
        mYAxisColor = color;
    }
   
    /**
     * Returns the color of the X axis
     * 
     * @return the X axis color
     */
    int getXAxisColor() const{
        return mXAxisColor;
    }
   
    /**
     * Sets the X axis color.
     * 
     * @param color the X axis color
     */
    void setXAxisColor(int color) {
        mXAxisColor = color;
    }
   
    /**
     * Returns the labels color.
     *
     * @return the labels color
     */
    int getLabelsColor() const{
        return mLabelsColor;
    }

    /**
     * Sets the labels color.
     *
     * @param color the labels color
     */
    void setLabelsColor(int color) {
        mLabelsColor = color;
    }

    /**
     * Returns the labels text size.
     *
     * @return the labels text size
     */
    float getLabelsTextSize() const{
        return mLabelsTextSize;
    }

    /**
     * Sets the labels text size.
     *
     * @param textSize the labels text size
     */
    void setLabelsTextSize(float textSize) {
        mLabelsTextSize = textSize;
    }

    /**
     * Returns if the axes should be visible.
     *
     * @return the visibility flag for the axes
     */
    bool isShowAxes() const{
        return mShowAxes;
    }

    /**
     * Sets if the axes should be visible.
     *
     * @param showAxes the visibility flag for the axes
     */
    void setShowAxes(bool showAxes) {
        mShowAxes = showAxes;
    }

    /**
     * Returns if either of the labels should be visible.
     *
     * @return the visibility flag for the labels
     */
    bool isShowLabels()const{
        return mShowXLabels||mShowYLabels;
    }

    /**
     * Returns if the labels should be visible.
     *
     * @return the visibility flag for the labels
     */
    bool isShowXLabels() const{
        return mShowXLabels;
    }
    bool isShowYLabels()const{
        return mShowYLabels;
    }

    /**
     * Sets if the labels should be visible.
     *
     * @param showXLabels the visibility flag for the X labels
     * @param showYLabels the visibility flag for the Y labels
     */
    void setShowLabels(bool showXLabels, bool showYLabels) {
      mShowXLabels = showXLabels;
      mShowYLabels = showYLabels;
    }

    /**
     * Sets if the labels should be visible.
     *
     * @param showLabels the visibility flag for the labels
     */
    void setShowLabels(bool showLabels) {
        mShowXLabels = showLabels;
        mShowYLabels = showLabels;
    }

    /**
     * Returns if the tick marks should be visible.
     * 
     * @return isShowTickMarks
     */
    bool isShowTickMarks() const{
        return mShowTickMarks;
    }
    /**
     * Sets if the tick marks should be visible.
     * 
     * @param showTickMarks the visibility flag for the tick marks
     */
    void setShowTickMarks(bool mShowTickMarks) {
        mShowTickMarks = mShowTickMarks;
    }
    /**
     * Sets the grid line width.
     *
     * @param width the grid size
     */
    void setGridLineWidth(float width) {
        mGridLineWidth = width;
    }
    
    /**
     * Gets the grid line width.
     *
     * @return the grid line width
     */
    float getGridLineWidth() const{
        return mGridLineWidth;
    }
    /**
     * Returns if the X axis grid should be visible.
     *
     * @return the visibility flag for the X axis grid
     */
    bool isShowGridX() const{
        return mShowGridX;
    }

    /**
     * Returns if the Y axis grid should be visible.
     *
     * @return the visibility flag for the Y axis grid
     */
    bool isShowGridY() const{
        return mShowGridY;
    }

    /**
     * Sets if the X axis grid should be visible.
     *
     * @param showGrid the visibility flag for the X axis grid
     */
    void setShowGridX(bool showGrid) {
        mShowGridX = showGrid;
    }

    /**
     * Sets if the Y axis grid should be visible.
     *
     * @param showGrid the visibility flag for the Y axis grid
     */
    void setShowGridY(bool showGrid) {
        mShowGridY = showGrid;
    }

    /**
     * Sets if the grid should be visible.
     *
     * @param showGrid the visibility flag for the grid
     */
    void setShowGrid(bool showGrid) {
        setShowGridX(showGrid);
        setShowGridY(showGrid);
    }

    /**
     * Returns if the grid should be visible for custom X or Y labels.
     *
     * @return the visibility flag for the custom text grid
     */
    bool isShowCustomTextGridX() const{
        return mShowCustomTextGridX;
    }
    /**
     * Returns if the Y axis custom text grid should be visible.
     * 
     * @return the visibility flag for the custom text Y axis grid
     */
    bool isShowCustomTextGridY() const{
        return mShowCustomTextGridY;
    }
  
    /**
     * Sets if the X axis custom text grid should be visible.
     * 
     * @param showGrid the visibility flag for the X axis custom text grid
     */
    void setShowCustomTextGridX(bool showGrid) {
        mShowCustomTextGridX = showGrid;
    }
  
    /**
     * Sets if the Y axis custom text grid should be visible.
     * 
     * @param showGrid the visibility flag for the Y axis custom text grid
     */
    void setShowCustomTextGridY(bool showGrid) {
        mShowCustomTextGridY = showGrid;
    }
  
    /**
     * Sets if the grid for custom X or Y labels should be visible.
     *
     * @param showGrid the visibility flag for the custom text grid
     */
    void setShowCustomTextGrid(bool showGrid) {
        mShowCustomTextGridX = showGrid;
        mShowCustomTextGridY = showGrid;
    }

    /**
     * Returns if the legend should be visible.
     *
     * @return the visibility flag for the legend
     */
    bool isShowLegend() const{
        return mShowLegend;
    }

    /**
     * Sets if the legend should be visible.
     *
     * @param showLegend the visibility flag for the legend
     */
    void setShowLegend(bool showLegend) {
        mShowLegend = showLegend;
    }

    /**
     * Returns if the legend should size to fit.
     *
     * @return the fit behavior
     */
    bool isFitLegend() const{
        return mFitLegend;
    }

    /**
     * Sets if the legend should size to fit.
     *
     * @param fit the fit behavior
     */
    void setFitLegend(bool fit) {
        mFitLegend = fit;
    }

    /**
     * Returns the text typeface name.
     *
     * @return the text typeface name
     */
    std::string getTextTypefaceName() const{
        return mTextTypefaceName;
    }

    /**
     * Returns the text typeface style.
     *
     * @return the text typeface style
     */
    int getTextTypefaceStyle() const{
        return mTextTypefaceStyle;
    }

    /**
     * Returns the text typeface.
     *
     * @return the text typeface
     */
    Typeface* getTextTypeface() const{
        return mTextTypeface;
    }

    /**
     * Returns the legend text size.
     *
     * @return the legend text size
     */
    float getLegendTextSize() const{
        return mLegendTextSize;
    }

    /**
     * Sets the legend text size.
     *
     * @param textSize the legend text size
     */
    void setLegendTextSize(float textSize) {
        mLegendTextSize = textSize;
    }

    /**
     * Sets the text typeface name and style.
     *
     * @param typefaceName the text typeface name
     * @param style the text typeface style
     */
    void setTextTypeface(const std::string& typefaceName, int style) {
        mTextTypefaceName = typefaceName;
        mTextTypefaceStyle = style;
    }

    /**
     * Sets the text typeface.
     *
     * @param typeface the typeface
     */
    void setTextTypeface(Typeface* typeface) {
        mTextTypeface = typeface;
    }

    /**
     * Returns the antialiasing flag value.
     *
     * @return the antialiasing value
     */
    bool isAntialiasing() const{
        return mAntialiasing;
    }

    /**
     * Sets the antialiasing value.
     *
     * @param antialiasing the antialiasing
     */
    void setAntialiasing(bool antialiasing) {
        mAntialiasing = antialiasing;
    }

    /**
     * Returns the value to be used for scaling the chart.
     *
     * @return the scale value
     */
    float getScale() const{
        return mScale;
    }

    /**
     * Returns the original value to be used for scaling the chart.
     *
     * @return the original scale value
     */
    float getOriginalScale() const{
        return mOriginalScale;
    }

    /**
     * Sets the value to be used for scaling the chart. It works on some charts
     * like pie, doughnut, dial.
     *
     * @param scale the scale value
     */
    void setScale(float scale) {
        mScale = scale;
    }

    /**
     * Returns the enabled state of the zoom.
     *
     * @return if zoom is enabled
     */
    bool isZoomEnabled() const{
        return mZoomEnabled;
    }

    /**
     * Sets the enabled state of the zoom.
     *
     * @param enabled zoom enabled
     */
    void setZoomEnabled(bool enabled) {
        mZoomEnabled = enabled;
    }

    /**
     * Returns the visible state of the zoom buttons.
     *
     * @return if zoom buttons are visible
     */
    bool isZoomButtonsVisible() const{
        return mZoomButtonsVisible;
    }

    /**
     * Sets the visible state of the zoom buttons.
     *
     * @param visible if the zoom buttons are visible
     */
    void setZoomButtonsVisible(bool visible) {
        mZoomButtonsVisible = visible;
    }

    /**
     * Returns the enabled state of the external (application implemented) zoom.
     *
     * @return if external zoom is enabled
     */
    bool isExternalZoomEnabled() const{
        return mExternalZoomEnabled;
    }

    /**
     * Sets the enabled state of the external (application implemented) zoom.
     *
     * @param enabled external zoom enabled
     */
    void setExternalZoomEnabled(bool enabled) {
        mExternalZoomEnabled = enabled;
    }

    /**
     * Returns the zoom rate.
     *
     * @return the zoom rate
     */
    float getZoomRate() const{
        return mZoomRate;
    }

    /**
     * Returns the enabled state of the pan.
     *
     * @return if pan is enabled
     */
    bool isPanEnabled() const{
        return mPanEnabled;
    }

    /**
     * Sets the enabled state of the pan.
     *
     * @param enabled pan enabled
     */
    virtual void setPanEnabled(bool enabled) {
        mPanEnabled = enabled;
    }

    /**
     * Sets the zoom rate.
     *
     * @param rate the zoom rate
     */
    void setZoomRate(float rate) {
        mZoomRate = rate;
    }

    /**
     * Returns the enabled state of the click.
     *
     * @return if click is enabled
     */
    bool isClickEnabled() const{
        return mClickEnabled;
    }

    /**
     * Sets the enabled state of the click.
     *
     * @param enabled click enabled
     */
    void setClickEnabled(bool enabled) {
        mClickEnabled = enabled;
    }

    /**
     * Returns the selectable radius value around clickable points.
     *
     * @return the selectable radius
     */
    int getSelectableBuffer() const{
        return mSelectableBuffer;
    }

    /**
     * Sets the selectable radius value around clickable points.
     *
     * @param buffer the selectable radius
     */
    void setSelectableBuffer(int buffer) {
        mSelectableBuffer = buffer;
    }

    /**
     * Returns the legend height.
     *
     * @return the legend height
     */
    int getLegendHeight() const{
        return mLegendHeight;
    }

    /**
     * Sets the legend height, in pixels.
     *
     * @param height the legend height
     */
    void setLegendHeight(int height) {
        mLegendHeight = height;
    }

    /**
     * Returns the margin sizes. An array containing the margins in this order:
     * top, left, bottom, right
     *
     * @return the margin sizes
     */
    const std::vector<int>& getMargins() const{
        return mMargins;
    }

    /**
     * Sets the margins, in pixels.
     *
     * @param margins an array containing the margin size values, in this order:
     *          top, left, bottom, right
     */
    void setMargins(const std::vector<int>& margins) {
        mMargins = margins;
    }

    /**
     * Returns if the chart is inside a scroll view and doesn't need to shrink.
     *
     * @return if it is inside a scroll view
     */
    bool isInScroll() const{
        return mInScroll;
    }

    /**
     * To be set if the chart is inside a scroll view and doesn't need to shrink
     * when not enough space.
     *
     * @param inScroll if it is inside a scroll view
     */
    void setInScroll(bool inScroll) {
        mInScroll = inScroll;
    }

    /**
     * Returns the start angle for circular charts such as pie, doughnut. An angle
     * of 0 degrees correspond to the geometric angle of 0 degrees (3 o'clock on a
     * watch.)
     *
     * @return the start angle in degrees
     */
    float getStartAngle() const{
        return mStartAngle;
    }

    /**
     * Sets the start angle for circular charts such as pie, doughnut, etc. An
     * angle of 0 degrees correspond to the geometric angle of 0 degrees (3
     * o'clock on a watch.)
     *
     * @param startAngle the start angle in degrees
     */
    void setStartAngle(float startAngle) {
        mStartAngle = startAngle;
    }

    /**
     * Returns if the values should be displayed as text.
     *
     * @return if the values should be displayed as text
     */
    bool isDisplayValues() const{
        return mDisplayValues;
    }

    /**
     * Sets if the values should be displayed as text (supported by pie chart).
     *
     * @param display if the values should be displayed as text
     */
    void setDisplayValues(bool display) {
        mDisplayValues = display;
    }
};
}/*endof namespace*/
#endif/*__DEFAULT_RENDERER_H__*/
