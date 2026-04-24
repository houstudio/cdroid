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
#ifndef __CDPLOT_BAR_PLOT_BASE_H__
#define __CDPLOT_BAR_PLOT_BASE_H__

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <widget/plot/figure.h>
#include <widget/plot/utility.h>

namespace plotcpp {

class BarPlotBase : public Figure {
public:
    virtual ~BarPlotBase() = default;

    void clear() override;
    void build(cairo_t*cr) override;

    /** Set the x axis label */
    void setXLabel(const std::string &label);

    /** Set the y axis label */
    void setYLabel(const std::string &label);

    /** Set the grid enabled / disabled*/
    void setGrid(bool enable);

    /** Enable / disable rounded edges in the bars*/
    void setRoundedEdges(bool enable);

    /**
     * @brief Set the relative size (percentage) of the bar in relation with its
     * total available space. Setting this parameter will affect the spacing
     * between bars.
     *
     * @param rel_width Percentage of the bar available space.
     */
    void setBarRelativeWidth(float rel_width);

    /** Add a y axis marker */
    void addYMarker(Real marker);

    /** Set the figure legend */
    void setLegend(const std::vector<std::string> &labels);

protected:
    explicit BarPlotBase() = default;

    void clearData();

    enum class DataType {
        NUMERIC,
        CATEGORICAL,
    };
    DataType m_data_type = DataType::NUMERIC;

    struct DataSeries {
        std::vector<Real> values;
        int32_t color;
    };

    size_t m_num_bars;
    std::vector<Real> m_baselines;
    std::vector<Real> m_numeric_x_data;
    std::vector<std::string> m_categorical_x_data;
    std::vector<DataSeries> m_y_data;
    std::vector<std::string> m_legend_labels;

    std::string m_x_label;
    std::string m_y_label;

    float m_frame_x, m_frame_y, m_frame_w, m_frame_h;

    std::pair<Real, Real> m_y_range;

    std::set<Real> m_y_markers;
    std::set<Real> m_y_custom_markers;
    bool m_round_y_markers = false;

    bool m_grid_enable = false;
    bool m_rounded_borders = true;

    // Constraints
    static constexpr float FRAME_TOP_MARGIN_REL = 0.10f;
    static constexpr float FRAME_BOTTOM_MARGIN_REL = 0.12f;
    static constexpr float FRAME_LEFT_MARGIN_REL = 0.15f;
    static constexpr float FRAME_RIGHT_MARGIN_REL = 0.05f;
    static constexpr float BAR_FRAME_Y_MARGIN_REL = 0.05f;
    static constexpr float BAR_FRAME_X_MARGIN_REL = 0.05f;
    static constexpr float DEFAULT_BAR_WIDTH_REL = 0.65f;

    float m_bar_width_rel = DEFAULT_BAR_WIDTH_REL;

    static constexpr int32_t BACKGROUND_COLOR = 0xFFFFFFFF;

    float m_axis_font_size = 11.0f;

    static constexpr float PIXELS_PER_X_MARKER = 80.0f;
    static constexpr float PIXELS_PER_Y_MARKER = 80.0f;
    static constexpr float MARKER_LENGTH = 5.0f;
    static constexpr uint32_t MAX_NUM_Y_MARKERS = 5;
    static constexpr uint32_t MAX_NUM_X_MARKERS = 10;

    static constexpr float BASE_TITLE_FONT_SIZE = 20.0f;
    static constexpr float BASE_AXIS_FONT_SIZE = 11.0f;
    static constexpr float LEGEND_MARGIN = 5.0f;

    float translateToFrame(Real y) const;

    float m_zoom_y = 1.0f;
    float m_bar_top_y;

    bool m_discrete_x_axis = true;

    /** Calculate all frame parameters needed to draw the plots. */
    void calculateFrame();

    void drawBackground(cairo_t*cr);
    void drawFrame(cairo_t*cr);

    void drawTitle(cairo_t*cr);
    void drawBars(cairo_t*cr);

    void drawXAxis(cairo_t*cr);
    void drawYAxis(cairo_t*cr);
    void drawLabels(cairo_t*cr);

    void drawLegend(cairo_t*cr);
};

} // namespace plotcpp

#endif // __CDPLOT_BAR_PLOT_BASE_H__
