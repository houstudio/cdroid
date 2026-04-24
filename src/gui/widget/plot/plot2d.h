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
#ifndef __CDPLOT_PLOT2D_H__
#define __CDPLOT_PLOT2D_H__

#include <algorithm>
#include <array>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <widget/plot/utility.h>
#include <widget/plot/figure.h>

namespace plotcpp {

/**
 * @brief A 2D plot consisting of one or more 2D sets of points
 */
class Plot2D : public Figure {
public:
    Plot2D();
    virtual ~Plot2D() = default;

    /**
     * @brief Add a plot consisting of an x-axis sequence and a y-axis sequence
     * of the same length
     *
     * @param x_data x-axis data
     * @param y_data y-axis data
     * @param color Stroke color
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void plot(const std::vector<Real> &x_data, const std::vector<Real> &y_data,
              int32_t color, float stroke_width,
              const std::vector<double> &dash_array = {});

    /**
     * @brief Add a plot consisting of an x-axis sequence and a y-axis sequence
     * of the same length
     *
     * @param x_data x-axis data
     * @param y_data y-axis data
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void plot(const std::vector<Real> &x_data, const std::vector<Real> &y_data,
              float stroke_width, const std::vector<double> &dash_array = {});

    /**
     * @brief Add a plot consisting of one y-axis sequence of size N. The x-axis
     * sequence will be automatically deduced as a 1-increment sequence from
     * 0 to N-1 or as the categories set by previous plots.
     *
     * @param x_data x-axis data
     * @param color Stroke color
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void plot(const std::vector<Real> &y_data, int32_t color,
              float stroke_width, const std::vector<double> &dash_array = {});

    /**
     * @brief Add a plot consisting of one y-axis sequence of size N. The x-axis
     * sequence will be automatically deduced as a 1-increment sequence from
     * 0 to N-1 or as the categories set by previous plots.
     *
     * @param x_data x-axis data
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void plot(const std::vector<Real> &y_data, float stroke_width,
              const std::vector<double> &dash_array = {});

    /**
     * @brief Add a plot using a vector as x axis values and a lambda function
     * such that y=function(x)
     *
     * @param x x values
     * @param function A function such that y=function(x)
     * @param color Stroke color
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void plot(const std::vector<Real> &x_data,
              const std::function<Real(Real)> &function, int32_t color,
              float stroke_width, const std::vector<double> &dash_array = {});

    /**
     * @brief Add a plot using a vector as x axis values and a lambda function
     * such that y=function(x)
     *
     * @param x x values
     * @param function A function such that y=function(x)
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void plot(const std::vector<Real> &x_data,
              const std::function<Real(Real)> &function,
              float stroke_width, const std::vector<double> &dash_array = {});

    /**
     * @brief Add a categorical plot with discrete text labels on the x axis and
     * Real numbers on the y axis.
     *
     * @param x_data categorical x
     * @param y_data y-axis data axis
     * @param color Stroke color
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void plot(const std::vector<std::string> &x_data,
              const std::vector<Real> &y_data, int32_t color,
              float stroke_width, const std::vector<double> &dash_array = {});

    /**
     * @brief Add a categorical plot with discrete text labels on the x axis and
     * Real numbers on the y axis.
     *
     * @param x_data categorical x
     * @param y_data y-axis data axis
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void plot(const std::vector<std::string> &x_data,
              const std::vector<Real> &y_data, float stroke_width = 2,
              const std::vector<double> &dash_array = {});

    /**
     * @brief Add a SCATTER plot consisting of one y-axis sequence of size N. The
     * x-axis sequence will be automatically deduced as a 1-increment sequence
     * from 0 to N-1.
     *
     * @param x_data categorical x
     * @param y_data y-axis data axis
     * @param color Stroke color
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void scatter(const std::vector<Real> &x_data, const std::vector<Real> &y_data,
                 int32_t color, float radius = 2);

    /**
     * @brief Add a SCATTER plot consisting of one y-axis sequence of size N. The
     * x-axis sequence will be automatically deduced as a 1-increment sequence
     * from 0 to N-1.
     *
     * @param x_data categorical x
     * @param y_data y-axis data axis
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void scatter(const std::vector<Real> &x_data, const std::vector<Real> &y_data,
                 float radius = 2);

    /**
     * @brief Add a categorical SCATTER plot with discrete text labels on the x
     * axis and Real numbers on the y axis.
     *
     * @param x_data categorical x
     * @param y_data y-axis data axis
     * @param color Stroke color
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void scatter(const std::vector<std::string> &x_data,
                 const std::vector<Real> &y_data,
                 int32_t color = 0xFF000000, float radius = 2);

    /**
     * @brief Add a categorical SCATTER plot with discrete text labels on the x
     * axis and Real numbers on the y axis.
     *
     * @param x_data categorical x
     * @param y_data y-axis data axis
     * @param stroke_width Line width
     * @param dash_array Dash array (length of draw / no-draw segments in pt
     * units)
     */
    void scatter(const std::vector<std::string> &x_data,
                 const std::vector<Real> &y_data, float radius = 2);

    /**
     * @brief Set hold on/off
     * Setting the hold on allows multiple data series to be plotted. If hold is
     * off, plotting overwrites the plot data.
     */
    void setHold(bool hold_on);

    /** Enable / disable the grid */
    void setGrid(bool enable);

    /**
     * @brief Set a range for the x axis
     *
     * @param x0 Minimum value
     * @param x1 Maximum value
     */
    void setXRange(Real x0, Real x1);

    /**
     * @brief Set a range for the y axis
     *
     * @param y0 Minimum value
     * @param y1 Maximum value
     */
    void setYRange(Real y0, Real y1);

    /** Returns the x axis range. */
    const ranges::Interval<Real>* getXRange() const;

    /** Returns the x axis range. */
    const ranges::Interval<Real>* getYRange() const;

    /** Adds a custom marker to the x axis */
    void addXMarker(Real x);

    /** Adds a custom marker to the y axis */
    void addYMarker(Real y);

    /** Clears all custom markers */
    void clearMarkers();

    /**
     * @brief Set the legend text for all plots.
     *
     * @param labels Vector of labels, in the same order they were `plotted`, that
     * will appear in the legend box.
     * Excess labels will be ignored. If not enough labels are provided, the
     * corresponding plots will not shot a label. Calling this function deletes
     * all previously set labels.
     */
    void setLegend(const std::vector<std::string> &labels);

    /**
     * @brief Set a label for the x axis.
     * @param label
     */
    void setXLabel(const std::string &label);

    /**
     * @brief Set a label for the y axis.
     * @param label
     */
    void setYLabel(const std::string &label);

    /**
     * @brief Returns the x axis label
     *
     * @return x axis label
     */
    std::string getXLabel() const;

    /**
     * @brief Returns the y axis label
     *
     * @return y axis label
     */
    std::string getYLabel() const;

    /** Clear figure configuration */
    void clear() override;

    /** Clear plot data */
    void clearData();

    /** Build the figure */
    void build(cairo_t*cr) override;

protected:
    struct Style {
        int32_t color;
        float stroke;
        std::vector<double> dash_array;
        bool scatter;
    };

    struct DataSeries {
        std::vector<Real> x;
        std::vector<Real> y;
        Style style;
    };

    struct CategoricalDataSeries {
        std::vector<Real> y;
        Style style;
    };

    bool m_hold = true;

    std::string m_x_label;
    std::string m_y_label;

    enum class DataType {
        NUMERIC,
        CATEGORICAL,
    };
    DataType m_data_type = DataType::NUMERIC;

    // Numeric data series
    std::vector<DataSeries> m_numeric_data;
    ranges::Interval<Real> m_x_data_range, m_y_data_range;
    ranges::Interval<Real> m_x_set_range, m_y_set_range;
    ranges::Interval<Real> m_x_range, m_y_range;
    float m_zoom_x = 1.0f;
    float m_zoom_y = 1.0f;

    ColorSelector m_color_selector;

    // Categorical data series
    std::vector<std::string> m_categorical_labels;
    std::vector<CategoricalDataSeries> m_categorical_data;

    std::vector<std::string> m_legend_labels;

    float m_frame_x, m_frame_y, m_frame_w, m_frame_h;

    std::pair<ranges::Interval<Real>, ranges::Interval<Real>> XDataRange() const;
    std::pair<ranges::Interval<Real>, ranges::Interval<Real>> YDataRange() const;

    std::set<Real> m_x_markers;
    std::set<Real> m_y_markers;
    std::set<Real> m_x_custom_markers;
    std::set<Real> m_y_custom_markers;

    bool m_grid_enable = false;
    bool mXRangeSetted=false;
    bool mYRangeSetted=false;
    /** Translate the (x, y) coordinates from the plot function to (x, y) in the
     * svg image */
    std::pair<float, float> translateToFrame(Real x, Real y) const;

    // Constraints
    static constexpr float FRAME_TOP_MARGIN_REL = 0.10f;
    static constexpr float FRAME_BOTTOM_MARGIN_REL = 0.12f;
    static constexpr float FRAME_LEFT_MARGIN_REL = 0.15f;
    static constexpr float FRAME_RIGHT_MARGIN_REL = 0.05f;

    static constexpr int32_t BACKGROUND_COLOR=0xFFFFFFFF;

    float m_axis_font_size;

    static constexpr float PIXELS_PER_X_MARKER = 80.0f;
    static constexpr float PIXELS_PER_Y_MARKER = 80.0f;
    static constexpr float MARKER_LENGTH = 5.0f;
    static constexpr uint32_t MAX_NUM_Y_MARKERS = 5;
    static constexpr uint32_t MAX_NUM_X_MARKERS = 10;

    static constexpr float BASE_TITLE_FONT_SIZE = 20.0f;
    static constexpr float BASE_AXIS_FONT_SIZE = 11.0f;

    static constexpr float LEGEND_MARGIN = 5.0f;

    /** Calculate all frame parameters needed to draw the plots. */
    void calculateFrame();
    void calculateNumericFrame();
    void calculateCategoricalFrame();

    void drawBackground(cairo_t*);
    void drawFrame(cairo_t*);

    void drawData(cairo_t*);
    void drawNumericData(cairo_t*cr);
    void drawNumericPath(const DataSeries &plot,cairo_t*);
    void drawNumericScatter(const DataSeries &plot,cairo_t*);
    void drawCategoricalData(cairo_t*);
    void drawCategoricalPath(const CategoricalDataSeries &plot,cairo_t*);
    void drawCategoricalScatter(const CategoricalDataSeries &plot,cairo_t*);

    void drawTitle(cairo_t*);
    void drawLabels(cairo_t*);

    void drawLegend(cairo_t*);
};

} // namespace plotcpp

#endif // __CDPLOT_PLOT2D_H__
