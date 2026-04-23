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
#ifndef __CDPLOT_HISTOGRAM_PLOT_H__
#define __CDPLOT_HISTOGRAM_PLOT_H__

#include <vector>
#include <widget/plot/figure.h>
#include <widget/plot/utility.h>
#include <widget/plot/barplotbase.h>

namespace plotcpp {

class HistogramPlot : public BarPlotBase {
public:
    HistogramPlot();
    virtual ~HistogramPlot() = default;

    void clear() override;

    /**
     * @brief Plot a histogram of a sequence of values
     *
     * @param values Vector of values
     * @param num_bins Number of bins
     * @param color Bar colour
     */
    void plot(const std::vector<Real> &values, uint32_t num_bins, int32_t color);

    /**
     * @brief Plot a histogram of a sequence of values
     *
     * @param values Vector of values
     * @param num_bins Number of bins
     */
    void plot(const std::vector<Real> &values, uint32_t num_bins);
protected:
    std::vector<Real> calculateIntervals(const std::vector<Real> &values,uint32_t num_bins);
    std::vector<Real> calculateBins(const std::vector<Real> &intervals);
    std::vector<Real> calculateHistogram(const std::vector<Real> &values, const std::vector<Real> &intervals);

    static constexpr int DEFAULT_COLOR =0x332288;
};

} // namespace plotcpp

#endif // __CDPLOT_HISTOGRAM_PLOT_H__
