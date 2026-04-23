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
#include <numeric>
#include <utils/textutils.h>
#include <widget/plot/barplot.h>

namespace plotcpp {

BarPlot::BarPlot() : m_color_selector(color_tables::VIBRANT) {
    m_round_y_markers = false;
    m_discrete_x_axis = true;
}

void BarPlot::plot(const std::vector<Real> &x_data, const std::vector<Real> &y_data, int32_t color) {
    if (x_data.size() != y_data.size()) {
        return;
    }

    if (m_y_data.size() == 0) {
        m_num_bars = x_data.size();
    } else {
        if (x_data.size() != m_num_bars) {
            return;
        }
    }

    if (m_data_type != DataType::NUMERIC) {
        m_y_data.clear();
    }

    m_data_type = DataType::NUMERIC;
    m_numeric_x_data = x_data;
    m_categorical_x_data.clear();
    m_y_data.emplace_back(DataSeries{y_data, color});
}

void BarPlot::plot(const std::vector<Real> &x_data, const std::vector<Real> &y_data) {
    plot(x_data, y_data, m_color_selector.NextColor());
}

void BarPlot::plot(const std::vector<std::string> &x_data, const std::vector<Real> &y_data, int32_t color) {
    if (x_data.size() != y_data.size()) {
        return;
    }

    if (m_y_data.size() == 0) {
        m_num_bars = x_data.size();
    } else {
        if (x_data.size() != m_num_bars) {
            return;
        }
    }

    if (m_data_type != DataType::CATEGORICAL) {
        m_y_data.clear();
    }

    m_data_type = DataType::CATEGORICAL;
    m_categorical_x_data = x_data;
    m_numeric_x_data.clear();
    m_y_data.emplace_back(DataSeries{y_data, color});
}

void BarPlot::plot(const std::vector<std::string> &x_data, const std::vector<Real> &y_data) {
    plot(x_data, y_data, m_color_selector.NextColor());
}

void BarPlot::plot(const std::vector<Real> &y_data, int32_t color) {
    if (m_y_data.size() == 0) {
        m_num_bars = y_data.size();
        m_numeric_x_data.clear();
        m_categorical_x_data.resize(m_num_bars);

        for (size_t i = 0; i < m_num_bars; ++i) {
            m_categorical_x_data[i] = cdroid::TextUtils::stringPrintf("%d",i+1);
        }

        m_data_type = DataType::CATEGORICAL;
    } else {
        if (y_data.size() != m_num_bars) {
            return;
        }
    }

    m_y_data.emplace_back(DataSeries{y_data, color});
}

void BarPlot::plot(const std::vector<Real> &y_data) {
    plot(y_data, m_color_selector.NextColor());
}

void BarPlot::setXData(const std::vector<Real> &x_data) {
    m_numeric_x_data = x_data;
    m_data_type = DataType::NUMERIC;
}

void BarPlot::setXData(const std::vector<std::string> &x_data) {
    m_categorical_x_data = x_data;
    m_data_type = DataType::CATEGORICAL;
}

void BarPlot::setBaseline(Real baseline) {
    m_baselines = std::vector<Real>(m_num_bars, baseline);
}

void BarPlot::setBaselines(const std::vector<Real> &baselines) {
    m_baselines = baselines;
}

void BarPlot::setLegend(const std::vector<std::string> &labels) {
    if (labels.empty()) {
        m_legend_labels.clear();
        return;
    }

    m_legend_labels = labels;
}

} // namespace plotcpp
