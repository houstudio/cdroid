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
#include <vector>
#include <algorithm>
#include <widget/plot/utility.h>
#include <widget/plot/histogramplot.h>

namespace plotcpp {

HistogramPlot::HistogramPlot() {
    setBarRelativeWidth(1.0f);
    m_round_y_markers = true;
    m_discrete_x_axis = false;
}

void HistogramPlot::clear() {
    clearData();
}

void HistogramPlot::plot(const std::vector<Real> &values, uint32_t num_bins, int32_t color) {
    clearData();

    std::vector<Real> intervals = calculateIntervals(values, num_bins);
    m_numeric_x_data = calculateBins(intervals);
    std::vector<Real> counts = calculateHistogram(values, intervals);
    m_num_bars = m_numeric_x_data.size();
    m_y_data.push_back(DataSeries{counts, color});
    m_data_type = DataType::NUMERIC;
}

std::vector<Real>
HistogramPlot::calculateIntervals(const std::vector<Real> &values, uint32_t num_bins) {
    std::vector<Real> intervals;
    intervals.resize(num_bins + 1);

    const auto /*[min, max]*/v =
        std::pair<Real, Real> {*std::min_element(values.begin(), values.end()),
                               *std::max_element(values.begin(), values.end())
                              };
    const auto min = v.first;
    const auto max = v.second;
    if (min == max) {
        return {min};
    }

    const Real interval = (max - min) / num_bins;
    for (unsigned int i = 0; (i < num_bins + 1); ++i) {
        intervals[i] = min + static_cast<float>(i) * interval;
    }

    return intervals;
}

void HistogramPlot::plot(const std::vector<Real> &values,uint32_t num_bins) {
    plot(values, num_bins, (int32_t)DEFAULT_COLOR);
}

std::vector<Real> HistogramPlot::calculateBins(const std::vector<Real> &intervals) {
    const size_t num_intervals = intervals.size();
    if (num_intervals == 0) {
        return {};
    } else if (num_intervals == 1) {
        return {intervals.front()};
    }

    const size_t num_bins = num_intervals - 1;
    std::vector<Real> bins;
    bins.resize(num_bins);

    for (size_t i = 0; i < num_bins; ++i) {
        bins[i] = (intervals[i + 1] + intervals[i]) / 2.0f;
    }

    return bins;
}

std::vector<Real> HistogramPlot::calculateHistogram(const std::vector<Real> &values,const std::vector<Real> &intervals) {
    if (intervals.size() == 0) {
        return {};
    }

    const size_t num_bins = intervals.size() - 1;
    std::vector<size_t> counts(num_bins, 0);

    for (const auto &value : values) {
        const auto /*[index, found]*/v = BinarySearchInterval(value, intervals);
        if (v.second/*found*/) {
            ++counts[v.first/*index*/];
        }
    }

    return adaptor::Real(counts);
}

} // namespace plotcpp
