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
#ifndef __XY_CHART_H__
#define __XY_CHART_H__
#include <map>
#include <vector>
#include <core/canvas.h>
#include <widget/achart/model/xymultipleseriesdataset.h>
#include <widget/achart/renderer/xyseriesrenderer.h>
#include <widget/achart/renderer/xymultipleseriesrenderer.h>
#include <widget/achart/chart/clickablearea.h>
#include <widget/achart/chart/abstractchart.h>
namespace cdroid{
class ScatterChart;
/**
 * The XY chart rendering class.
 */
class XYChart :public AbstractChart {
private:
    friend class CombinedXYChart;
    /** The current scale value. */
    float mScale;
    /** The current translate value. */
    float mTranslate;
    /** The canvas center point. */
    PointF mCenter;
    /** The visible chart area, in screen coordinates. */
    Rect mScreenR;
    /** The calculated range. */
    std::map<int, std::vector<double>> mCalcRange;
protected:
    /**
     * The clickable areas for all points. The array index is the series index,
     * and the RectF list index is the point index in that series.
     */
    std::map<int, std::vector<ClickableArea>> mClickableAreas;
private:
    std::vector<double> getValidLabels(const std::vector<double>& labels)const;
    //void setStroke(Cap cap, Join join, float miter, Style style, PathEffect pathEffect,Paint paint);
    void transform(Canvas& canvas, float angle, bool inverse);
    int getLabelLinePos(int/*Align*/ align)const;
protected:
    /** The multiple series dataset. */
    std::shared_ptr<XYMultipleSeriesDataset> mDataset;
    /** The multiple series renderer. */
    std::shared_ptr<XYMultipleSeriesRenderer> mRenderer;

    XYChart();
    virtual void setDatasetRenderer(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);
    virtual std::vector<double> getXLabels(double min, double max, int count)const;
    std::map<int, std::vector<double>> getYLabels(const std::vector<double>& minY, const std::vector<double>& maxY, int maxScaleNumber)const;

    Rect getScreenR()const;
    void setScreenR(const Rect& screenR);

    /**
     * Draws the series.
     *
     * @param series the series
     * @param canvas the canvas
     * @param paint the paint object
     * @param pointsList the points to be rendered
     * @param seriesRenderer the series renderer
     * @param yAxisValue the y axis value in pixels
     * @param seriesIndex the series index
     * @param or the orientation
     * @param startIndex the start index of the rendering points
     */
    virtual void drawSeries(const std::shared_ptr<XYSeries>& series, Canvas& canvas, Paint& paint,std::vector<float>& pointsList,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int orientation ,int startIndex);
    void drawPoints(Canvas& canvas, Paint& paint, std::vector<float>& pointsList,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex);
    /**
     * The graphical representation of the series values as text.
     *
     * @param canvas the canvas to paint to
     * @param series the series to be painted
     * @param renderer the series renderer
     * @param paint the paint to be used for drawing
     * @param points the array of points to be used for drawing the series
     * @param seriesIndex the index of the series currently being drawn
     * @param startIndex the start index of the rendering points
     */
    virtual void drawChartValuesText(Canvas& canvas, const std::shared_ptr<XYSeries>& series, const std::shared_ptr<XYSeriesRenderer>& renderer,
             Paint& paint,const std::vector<float>& points, int seriesIndex, int startIndex);

    /**
     * The graphical representation of a text, to handle both HORIZONTAL and
     * VERTICAL orientations and extra rotation angles.
     *
     * @param canvas the canvas to paint to
     * @param text the text to be rendered
     * @param x the X axis location of the text
     * @param y the Y axis location of the text
     * @param paint the paint to be used for drawing
     * @param extraAngle the text angle
     */
    void drawText(Canvas& canvas, const std::string& text, float x, float y, Paint& paint,float extraAngle);

    /**
     * The graphical representation of the labels on the X axis.
     *
     * @param xLabels the X labels values
     * @param xTextLabelLocations the X text label locations
     * @param canvas the canvas to paint to
     * @param paint the paint to be used for drawing
     * @param left the left value of the labels area
     * @param top the top value of the labels area
     * @param bottom the bottom value of the labels area
     * @param xPixelsPerUnit the amount of pixels per one unit in the chart labels
     * @param minX the minimum value on the X axis in the chart
     * @param maxX the maximum value on the X axis in the chart
     */
    virtual void drawXLabels(const std::vector<double>& xLabels,const std::vector<double>& xTextLabelLocations,
            Canvas& canvas, Paint& paint, int left, int top, int bottom, double xPixelsPerUnit, double minX, double maxX);

    /**
     * The graphical representation of the labels on the Y axis.
     *
     * @param allYLabels the Y labels values
     * @param canvas the canvas to paint to
     * @param paint the paint to be used for drawing
     * @param maxScaleNumber the maximum scale number
     * @param left the left value of the labels area
     * @param right the right value of the labels area
     * @param bottom the bottom value of the labels area
     * @param yPixelsPerUnit the amount of pixels per one unit in the chart labels
     * @param minY the minimum value on the Y axis in the chart
     */
    void drawYLabels(const std::map<int, std::vector<double>>& allYLabels, Canvas& canvas,
             Paint& paint,int maxScaleNumber, int left, int right, int bottom,
            const std::vector<double>& yPixelsPerUnit,const std::vector<double>& minY);

    /**
     * The graphical representation of the text labels on the X axis.
     *
     * @param xTextLabelLocations the X text label locations
     * @param canvas the canvas to paint to
     * @param paint the paint to be used for drawing
     * @param left the left value of the labels area
     * @param top the top value of the labels area
     * @param bottom the bottom value of the labels area
     * @param xPixelsPerUnit the amount of pixels per one unit in the chart labels
     * @param minX the minimum value on the X axis in the chart
     * @param maxX the maximum value on the X axis in the chart
     */
    void drawXTextLabels(const std::vector<double>& xTextLabelLocations, Canvas& canvas, Paint& paint,
            bool showLabels, int left, int top, int bottom, double xPixelsPerUnit, double minX,double maxX);
    /**
     * Returns the clickable areas for all passed points
     *
     * @param points the array of points
     * @param values the array of values of each point
     * @param yAxisValue the minimum value of the y axis
     * @param seriesIndex the index of the series to which the points belong
     * @return an array of rectangles with the clickable area
     * @param startIndex the start index of the rendering points
     */
    virtual std::vector<ClickableArea> clickableAreasForPoints(const std::vector<float>& points,
            const std::vector<double>& values, float yAxisValue, int seriesIndex, int startIndex)=0;

    /**
     * Returns if the chart should display the null values.
     *
     * @return if null values should be rendered
     */
    virtual bool isRenderNullValues() const;
public:
    XYChart(const std::shared_ptr<XYMultipleSeriesDataset>& dataset,const std::shared_ptr<XYMultipleSeriesRenderer>& renderer);
    void draw(Canvas& canvas, int x, int y, int width, int height, Paint& paint)override;

    const std::shared_ptr<XYMultipleSeriesRenderer>& getRenderer() const;

    const std::shared_ptr<XYMultipleSeriesDataset>& getDataset() const;

    std::vector<double> getCalcRange(int scale) const;

    void setCalcRange(const std::vector<double>& range, int scale);

    std::vector<double> toRealPoint(float screenX, float screenY)const;
    std::vector<double> toScreenPoint(const std::vector<double>& realPoint)const;

    std::vector<double> toRealPoint(float screenX, float screenY, int scale)const;
    std::vector<double> toScreenPoint(const std::vector<double>& realPoint, int scale)const;
    bool getSeriesAndPointForScreenCoordinate(const PointF& screenPoint,SeriesSelection&)const override;

    /**
     * The graphical representation of a series.
     *
     * @param canvas the canvas to paint to
     * @param paint the paint to be used for drawing
     * @param points the array of points to be used for drawing the series
     * @param seriesRenderer the series renderer
     * @param yAxisValue the minimum value of the y axis
     * @param seriesIndex the index of the series currently being drawn
     * @param startIndex the start index of the rendering points
     */
    virtual void drawSeries(Canvas& canvas, Paint& paint,std::vector<float>& points,
            const std::shared_ptr<XYSeriesRenderer>& seriesRenderer, float yAxisValue, int seriesIndex, int startIndex)=0;


    /**
     * Returns if the chart should display the points as a certain shape.
     *
     * @param renderer the series renderer
     */
    virtual bool isRenderPoints(const std::shared_ptr<SimpleSeriesRenderer>& renderer)const;
    /**
     * Returns the default axis minimum.
     *
     * @return the default axis minimum
     */
    virtual double getDefaultMinimum() const;

    /**
     * Returns the scatter chart to be used for drawing the data points.
     *
     * @return the data points scatter chart
     */
    virtual ScatterChart* getPointsChart() const;

    /**
     * Returns the chart type identifier.
     *
     * @return the chart type
     */
    virtual std::string getChartType()const =0;
    void setSelection(int seriesIndex,int dataIndex)override;
};
}/*endof namespace*/
#endif/*__XY_CHART_H__*/
