/**
 * Copyright (C) 2009 - 2012 SC 4ViewSoft SRL
 * Copyright (C) 2013 Henning Dodenhof
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

#ifndef __XYSERIES_RENDERER_H__
#define __XYSERIES_RENDERER_H__
#include <widget/achart/chart/pointstyle.h>
#include <widget/achart/renderer/simpleseriesrenderer.h>
namespace cdroid{
/**
 * A renderer for the XY type series.
 */
class XYSeriesRenderer :public SimpleSeriesRenderer {
public:
    class FillOutsideLine{
    public:
        enum Type {
            NONE, BOUNDS_ALL, BOUNDS_BELOW, BOUNDS_ABOVE, BELOW, ABOVE
        };
    private:
        /** The fill type. */
        Type mType=NONE;
        /** The fill color. */
        int mColor = 0x800000c8;//Color.argb(125, 0, 0, 200);
        /** The fill points index range. */
        std::vector<int> mFillRange;
    public:
        FillOutsideLine()=default;
        FillOutsideLine(const FillOutsideLine&)=default;
        FillOutsideLine(FillOutsideLine&&)=default;
        FillOutsideLine(Type type) {
            mType = type;
        }

        int getColor() const{
            return mColor;
        }

        void setColor(int color) {
            mColor = color;
        }

        Type getType() const{
            return mType;
        }

        std::vector<int> getFillRange() const{
            return mFillRange;
        }

        void setFillRange(const std::vector<int>& range) {
            mFillRange = range;
        }
    };
private:
    /** If the chart points should be filled. */
    bool mFillPoints = false;
    /** If the chart should be filled outside its line. */
    std::vector<FillOutsideLine> mFillBelowLine;
    /** The point style. */
    PointStyle mPointStyle = PointStyle::POINT;
    /** The point size */
    float mPointSize = 3;
    /** The point stroke width */
    float mPointStrokeWidth = 1;
    /** The chart line width. */
    float mLineWidth = 1;
public:
    /**
     * Returns if the chart should be filled below the line.
     *
     * @return the fill below line status
     *
     * @deprecated Use {@link #getFillOutsideLine()} instead.
     */
    //@Deprecated
    bool isFillBelowLine() const{
        return mFillBelowLine.size() > 0;
    }

    /**
     * Sets if the line chart should be filled below its line. Filling below the
     * line transforms a line chart into an area chart.
     *
     * @param fill the fill below line flag value
     *
     * @deprecated Use {@link #setFillOutsideLine(FillOutsideLine)} instead.
     */
    //@Deprecated
    void setFillBelowLine(bool fill) {
        mFillBelowLine.clear();
        if (fill) {
            mFillBelowLine.push_back(FillOutsideLine(FillOutsideLine::Type::BOUNDS_ALL));
        } else {
            mFillBelowLine.push_back(FillOutsideLine(FillOutsideLine::Type::NONE));
        }
    }

    /**
     * Returns the type of the outside fill of the line.
     *
     * @return the type of the outside fill of the line.
     */
    std::vector<FillOutsideLine> getFillOutsideLine() const{
        return mFillBelowLine;
    }

    /**
     * Sets if the line chart should be filled outside its line. Filling outside
     * with FillOutsideLine.INTEGRAL the line transforms a line chart into an area
     * chart.
     *
     * @param the type of the filling
     */
    void addFillOutsideLine(const FillOutsideLine& fill) {
        mFillBelowLine.push_back(fill);
    }

    /**
     * Returns if the chart points should be filled.
     *
     * @return the points fill status
     */
    bool isFillPoints() const{
        return mFillPoints;
    }

    /**
     * Sets if the chart points should be filled.
     *
     * @param fill the points fill flag value
     */
    void setFillPoints(bool fill) {
        mFillPoints = fill;
    }

    /**
     * Sets the fill below the line color.
     *
     * @param color the fill below line color
     *
     * @deprecated Use FillOutsideLine.setColor instead
     */
    //@Deprecated
    void setFillBelowLineColor(int color) {
        if (mFillBelowLine.size() > 0) {
            mFillBelowLine.at(0).setColor(color);
        }
    }

    /**
     * Returns the point style.
     *
     * @return the point style
     */
    PointStyle getPointStyle() const{
        return mPointStyle;
    }

    /**
     * Sets the point style.
     *
     * @param style the point style
     */
    void setPointStyle(PointStyle style) {
        mPointStyle = style;
    }

    /**
     * Returns the point stroke width in pixels.
     *
     * @return the point stroke width in pixels
     */
    float getPointStrokeWidth() const{
        return mPointStrokeWidth;
    }

    /**
     * Sets the point stroke width in pixels.
     *
     * @param strokeWidth the point stroke width in pixels
     */
    void setPointStrokeWidth(float strokeWidth) {
        mPointStrokeWidth = strokeWidth;
    }

    /**
     * Returns the point size in pixels.
     *
     * @return the point size in pixels
     */
    float getPointSize() const{
        return mPointSize;
    }

    /**
     * Sets the point size in pixels.
     *
     * @param strokeWidth the point size in pixels
     */
    void setPointSize(float pointSize) {
        mPointSize = pointSize;
    }

    /**
     * Returns the chart line width.
     *
     * @return the line width
     */
    float getLineWidth() const{
        return mLineWidth;
    }

    /**
     * Sets the chart line width.
     *
     * @param lineWidth the line width
     */
    void setLineWidth(float lineWidth) {
        mLineWidth = lineWidth;
    }
};
}/*endof namespace*/
#endif/*__XYSERIES_RENDERER_H__*/
