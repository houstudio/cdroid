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
#ifndef __CDPLOT_BAR_PLOT_H__
#define __CDPLOT_BAR_PLOT_H__

#include <string>
#include <utility>
#include <vector>

#include <widget/plot/utility.h>
#include <widget/plot/barplotbase.h>

namespace plotcpp {

/**
 * @brief A bar plot.
 *
 * BarPlot supports a standard representation consisting of
 * a single data series and a stacked bar representation with multiple segments
 * stacked on top of each other.
 *
 * In both modes the baseline is 0 by default and can be changed to a custom
 * value. In standard mode the bars are drawn from the baseline value and will
 * have a weight dictated by the values provided. In stacked mode the base are
 * drawn from the baseline value and will be stacked in order they were plotted,
 * their weights determined by the values provided.
 *
 * Both modes support numeric and categorical x axis. The y axis is always
 * numeric.
 *
 * Since BarPlot can have vertical and horizontal orientations, the weights of
 * the bar are considered the y axis while the discrete categorisation of each
 * bar is represented by the x axis.
 */
class BarPlot : public BarPlotBase {
public:
    BarPlot();
    virtual ~BarPlot() = default;

    void plot(const std::vector<Real> &x_data, const std::vector<Real> &y_data,int32_t color);
    void plot(const std::vector<Real> &x_data, const std::vector<Real> &y_data);
    void plot(const std::vector<std::string> &x_data, const std::vector<Real> &y_data, int32_t color);
    void plot(const std::vector<std::string> &x_data, const std::vector<Real> &y_data);
    void plot(const std::vector<Real> &y_data, int32_t color);
    void plot(const std::vector<Real> &y_data);


    void setXData(const std::vector<Real> &x_data);
    void setXData(const std::vector<std::string> &x_data);

    /**
     * @brief Set a baseline value for all bars.
     * The baseline is the value from which all bar segments, positive and
     * negative will be stacked.
     *
     * @param baseline Baseline value.
     */
    void setBaseline(Real baseline);

    /**
     * @brief Set a baseline value for every individual bar.
     * The baseline is the value from which all bar segments, positive and
     * negative will be stacked.
     *
     * @param baseline Baseline value.
     */
    void setBaselines(const std::vector<Real> &baselines);

    /** Set the figure legend */
    void setLegend(const std::vector<std::string> &labels);

protected:
    ColorSelector m_color_selector;
};

} // namespace plotcpp

#endif // __CDPLOT_BAR_PLOT_H__
