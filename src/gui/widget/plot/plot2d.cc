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
#include <algorithm>
#include <functional>
#include <limits>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
#include <sstream>

#include <core/color.h>
#include <cairo/cairo.h>
#include <utils/textutils.h>
#include <widget/plot/frame.h>
#include <widget/plot//legend.h>
#include <widget/plot/fonts.h>
#include <widget/plot/utility.h>
#include <widget/plot/plot2d.h>

namespace plotcpp {

template <typename T> static bool IsInfinity(T x) {
    return (x == std::numeric_limits<T>::infinity()) ||
           (x == -std::numeric_limits<T>::infinity());
}

Plot2D::Plot2D() : Figure(), m_color_selector(color_tables::BRIGHT) {}

void Plot2D::plot(const std::vector<Real> &x_data, const std::vector<Real> &y_data,
        int32_t color, float stroke_width, const std::vector<double> &dash_array) {
    if (x_data.size() != y_data.size()) {
        return;
    }

    if (m_hold == false) {
        m_numeric_data.clear();
    }

    const Style style = {color, stroke_width, dash_array, false};
    m_numeric_data.emplace_back(DataSeries{x_data, y_data, style});

    m_data_type = DataType::NUMERIC;
    m_categorical_data.clear();
}

void Plot2D::plot(const std::vector<Real> &x_data, const std::vector<Real> &y_data,
        float stroke_width,const std::vector<double> &dash_array) {
    plot(x_data, y_data, m_color_selector.NextColor(), stroke_width, dash_array);
}

void Plot2D::plot(const std::vector<Real> &y_data, int32_t color,
                  const float stroke_width, const std::vector<double> &dash_array) {
    if (m_data_type == DataType::NUMERIC) {
        std::vector<Real> x_data;
        x_data.resize(y_data.size());
        std::iota(x_data.begin(), x_data.end(), 1.0f);
        plot(x_data, y_data, color, stroke_width, dash_array);
    } else if (m_data_type == DataType::CATEGORICAL) {
        if ((m_categorical_labels.size() > 0) &&
                (m_categorical_labels.size() != y_data.size())) {
            return;
        }

        const Style style = {color, stroke_width, dash_array, false};
        m_categorical_data.emplace_back(CategoricalDataSeries{y_data, style});
    }
}

void Plot2D::plot(const std::vector<Real> &y_data, float stroke_width,
                  const std::vector<double> &dash_array) {
    plot(y_data, m_color_selector.NextColor(), stroke_width, dash_array);
}

void Plot2D::plot(const std::vector<Real> &x_data,
                  const std::function<Real(Real)> &function, int32_t color,
                  float stroke_width, const std::vector<double> &dash_array) {
    const auto y_data = ranges::Generate(x_data, function);
    plot(x_data, y_data, color, stroke_width, dash_array);
}

void Plot2D::plot(const std::vector<Real> &x_data,
                  const std::function<Real(Real)> &function,
                  float stroke_width, const std::vector<double> &dash_array) {
    plot(x_data, function, m_color_selector.NextColor(), stroke_width,
         dash_array);
}

void Plot2D::plot(const std::vector<std::string> &x_data,
                  const std::vector<Real> &y_data, int32_t color,
                  float stroke_width, const std::vector<double> &dash_array) {
    if (x_data.size() != y_data.size()) {
        return;
    }

    const bool should_reset_data =
        (x_data.size() != m_categorical_labels.size()) &&
        (m_categorical_data.size() > 0);
    if (should_reset_data) {
        m_categorical_data.clear();
    }

    m_categorical_data.clear();
    m_categorical_labels = x_data;

    const Style style = {color, stroke_width, dash_array, false};
    m_categorical_data.emplace_back(CategoricalDataSeries{y_data, style});
    m_numeric_data.clear();
    m_data_type = DataType::CATEGORICAL;
}

void Plot2D::plot(const std::vector<std::string> &x_data, const std::vector<Real> &y_data,
        float stroke_width, const std::vector<double> &dash_array) {
    plot(x_data, y_data, m_color_selector.NextColor(), stroke_width, dash_array);
}

void Plot2D::scatter(const std::vector<Real> &x_data, const std::vector<Real> &y_data,
        int32_t color, float radius) {
    if (x_data.size() != y_data.size()) {
        return;
    }

    if (m_hold == false) {
        m_numeric_data.clear();
    }

    const Style style = {color, radius, {}, true};
    m_numeric_data.emplace_back(DataSeries{x_data, y_data, style});

    m_data_type = DataType::NUMERIC;
    m_categorical_data.clear();
}

void Plot2D::scatter(const std::vector<Real> &x_data, const std::vector<Real> &y_data, const float radius) {
    scatter(x_data, y_data, m_color_selector.NextColor(), radius);
}

void Plot2D::scatter(const std::vector<std::string> &x_data, const std::vector<Real> &y_data,
        int32_t color, float radius) {
    if (x_data.size() != y_data.size()) {
        return;
    }

    const bool labels_are_equal = (x_data == m_categorical_labels);
    if (!labels_are_equal) {
        m_categorical_data.clear();
        m_categorical_labels = x_data;
    }

    const Style style = {color, radius, {}, true};
    m_categorical_data.emplace_back(CategoricalDataSeries{y_data, style});
    m_numeric_data.clear();
    m_data_type = DataType::CATEGORICAL;
}

void Plot2D::scatter(const std::vector<std::string> &x_data,
                     const std::vector<Real> &y_data, float radius) {
    scatter(x_data, y_data, m_color_selector.NextColor(), radius);
}

void Plot2D::setXRange(Real x0, Real x1) {
    const Real x_min = std::min(x0, x1);
    const Real x_max = std::max(x0, x1);
    mXRangeSetted =true;
    m_x_set_range = ranges::Interval<Real> {x_min, x_max};
}

void Plot2D::setYRange(Real y0, Real y1) {
    const Real y_min = std::min(y0, y1);
    const Real y_max = std::max(y0, y1);
    mYRangeSetted =true;
    m_y_set_range = ranges::Interval<Real> {y_min, y_max};
}

const ranges::Interval<Real>* Plot2D::getXRange() const {
    return mXRangeSetted?&m_x_set_range:nullptr;
}

const ranges::Interval<Real>* Plot2D::getYRange() const {
    return mYRangeSetted?&m_y_set_range:nullptr;
}

void Plot2D::setXLabel(const std::string &label) {
    m_x_label = label;
}

void Plot2D::setYLabel(const std::string &label) {
    m_y_label = label;
}

std::string Plot2D::getXLabel() const {
    return m_x_label;
}

std::string Plot2D::getYLabel() const {
    return m_y_label;
}

void Plot2D::addXMarker(Real x) {
    m_x_custom_markers.insert(x);
}

void Plot2D::addYMarker(Real y) {
    m_y_custom_markers.insert(y);
}

void Plot2D::setGrid(bool enable) {
    m_grid_enable = enable;
}

void Plot2D::setHold(bool hold) {
    m_hold = hold;
}

void Plot2D::setLegend(const std::vector<std::string> &labels) {
    if (labels.empty()) {
        m_legend_labels.clear();
        return;
    }

    size_t num_plots = 0;
    switch (m_data_type) {
    case DataType::NUMERIC:
        num_plots = m_numeric_data.size();
        break;
    case DataType::CATEGORICAL:
        num_plots = m_categorical_data.size();
        break;
    }

    const size_t num_labels = std::min(labels.size(), num_plots);
    m_legend_labels.resize(num_labels);
    for (size_t i = 0; i < num_labels; ++i) {
        m_legend_labels[i] = labels[i];
    }
}

void Plot2D::clear() {
    clearData();
    m_title.clear();
    setSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
    m_x_label.clear();
    m_y_label.clear();
    mXRangeSetted = mYRangeSetted = false;
    clearMarkers();
}

void Plot2D::clearMarkers() {
    m_x_markers.clear();
    m_y_markers.clear();
    m_x_custom_markers.clear();
    m_y_custom_markers.clear();
}

void Plot2D::clearData() {
    m_numeric_data.clear();
}

void Plot2D::build(cairo_t *cr) {
    calculateFrame();

    drawBackground(cr);
    drawTitle(cr);
    drawFrame(cr);
    drawLabels(cr);
    drawData(cr);
    drawLegend(cr);
}

void Plot2D::calculateFrame() {
    m_frame_x = static_cast<float>(m_width) * FRAME_LEFT_MARGIN_REL;
    m_frame_y = static_cast<float>(m_height) * FRAME_TOP_MARGIN_REL;
    m_frame_w = static_cast<float>(m_width) * (1.0f - FRAME_LEFT_MARGIN_REL - FRAME_RIGHT_MARGIN_REL);
    m_frame_h = static_cast<float>(m_height) * (1.0f - FRAME_TOP_MARGIN_REL - FRAME_BOTTOM_MARGIN_REL);

    switch (m_data_type) {
    case DataType::NUMERIC:
        calculateNumericFrame();
        break;
    case DataType::CATEGORICAL:
        calculateCategoricalFrame();
        break;
    default:
        break;
    }

    const unsigned int num_x_markers = std::min((uint32_t)MAX_NUM_X_MARKERS, static_cast<unsigned int>(m_frame_w / PIXELS_PER_X_MARKER));
    m_x_markers = ranges::PartitionRange(m_x_range, num_x_markers);
    const unsigned int num_y_markers = std::min((uint32_t)MAX_NUM_Y_MARKERS, static_cast<unsigned int>(m_frame_h / PIXELS_PER_Y_MARKER));
    m_y_markers = ranges::PartitionRange(m_y_range, num_y_markers);

    const std::string DUMMY_TEXT = "-000.00";
    const float y_axis_font_size = fonts::ConstrainedFontSize(
                                       BASE_AXIS_FONT_SIZE, DUMMY_TEXT, TEXT_FONT,
                                       (3.0f * m_frame_x / 4.0f) - MARKER_LENGTH,
                                       m_frame_h / static_cast<float>(num_y_markers));
    m_axis_font_size = std::min({y_axis_font_size});
}

void Plot2D::calculateNumericFrame() {
    Real min_x = std::numeric_limits<Real>::max();
    Real max_x = std::numeric_limits<Real>::lowest();
    Real min_y = std::numeric_limits<Real>::max();
    Real max_y = std::numeric_limits<Real>::lowest();
    for (auto &plot : m_numeric_data) {
        const size_t size = plot.x.size();
        for (size_t i = 0; i < size; ++i) {
            const Real x = plot.x[i];
            const Real y = plot.y[i];

            if (!IsInfinity(x)) {
                min_x = std::min(x, min_x);
                max_x = std::max(x, max_x);
            }
            if (!IsInfinity(y)) {
                min_y = std::min(y, min_y);
                max_y = std::max(y, max_y);
            }
        }
    }
    m_x_data_range = {min_x, max_x};
    m_y_data_range = {min_y, max_y};

    m_x_range = mXRangeSetted ? m_x_set_range : m_x_data_range;
    m_y_range = mYRangeSetted ? m_y_set_range : m_y_data_range;

    m_zoom_x = fabsf(static_cast<float>(m_frame_w / (m_x_range.second - m_x_range.first)));
    m_zoom_y = fabsf(static_cast<float>(m_frame_h / (m_y_range.second - m_y_range.first)));
}

std::pair<float, float> Plot2D::translateToFrame(Real x, Real y) const {
    float tx = static_cast<float>(m_zoom_x * (x - m_x_range.first));
    float ty = static_cast<float>(m_frame_h - (m_zoom_y * (y - m_y_range.first)));
    return {tx, ty};
}

void Plot2D::calculateCategoricalFrame() {
    Real min_y = std::numeric_limits<Real>::max();
    Real max_y = std::numeric_limits<Real>::lowest();
    for (const auto &data : m_categorical_data) {
        for (const auto &y : data.y) {
            if (!IsInfinity(y)) {
                min_y = std::min(y, min_y);
                max_y = std::max(y, max_y);
            }
        }
    }
    m_y_data_range = {min_y, max_y};

    m_y_range = mYRangeSetted ? m_y_set_range : m_y_data_range;

    m_zoom_y = fabsf(static_cast<float>(m_frame_h / (m_y_range.second - m_y_range.first)));

    const uint32_t num_y_markers = std::min((uint32_t)MAX_NUM_Y_MARKERS, static_cast<uint32_t>(m_frame_h / PIXELS_PER_Y_MARKER));
    m_y_markers = ranges::PartitionRange(m_y_range, num_y_markers);
}

void Plot2D::drawBackground(cairo_t *cr) {
    cairo_rectangle(cr, 0, 0, m_width, m_height);
    cdroid::Color c(BACKGROUND_COLOR);
    cairo_set_source_rgba(cr,c.red(),c.green(),c.blue(),c.alpha());
    cairo_fill(cr);
}

void Plot2D::drawFrame(cairo_t *cr) {
    PlotFrame frame(m_frame_w, m_frame_h, m_grid_enable);

    std::set<Real> left_markers;
    left_markers.insert(m_y_markers.begin(), m_y_markers.end());
    left_markers.insert(m_y_custom_markers.begin(), m_y_custom_markers.end());

    for (const auto &marker : left_markers) {
        if ((marker < m_y_range.first) || (marker > m_y_range.second)) {
            continue;
        }
        auto v = translateToFrame(0, marker);
        frame.addLeftMarker(v.second, cdroid::TextUtils::stringPrintf("%.2f",marker));
    }

    if (m_data_type == DataType::NUMERIC) {
        std::set<Real> bottom_markers;
        bottom_markers.insert(m_x_markers.begin(), m_x_markers.end());
        bottom_markers.insert(m_x_custom_markers.begin(), m_x_custom_markers.end());

        for (const auto &marker : bottom_markers) {
            if ((marker < m_x_range.first) || (marker > m_x_range.second)) {
                continue;
            }
            const auto v = translateToFrame(marker, 0);
            frame.addBottomMarker(v.first, cdroid::TextUtils::stringPrintf("%.2g",marker));
        }
    } else if (m_data_type == DataType::CATEGORICAL) {
        const size_t num_labels = m_categorical_labels.size();
        if (num_labels <= 1) {
            return;
        }
        for (size_t i = 0; i < num_labels; ++i) {
            const auto x = static_cast<float>(i) *
                           (m_frame_w / static_cast<float>(num_labels - 1));
            frame.addBottomMarker(x, m_categorical_labels[i]);
        }
    }

    frame.draw(cr, m_frame_x, m_frame_y);
}

void Plot2D::drawData(cairo_t *cr) {
    // 设置裁剪区域为绘图区
    cairo_save(cr);
    cairo_rectangle(cr, m_frame_x, m_frame_y, m_frame_w, m_frame_h);
    cairo_clip(cr);

    switch (m_data_type) {
    case DataType::NUMERIC:
        drawNumericData(cr);
        break;
    case DataType::CATEGORICAL:
        drawCategoricalData(cr);
        break;
    default:
        break;
    }

    cairo_restore(cr);
}

void Plot2D::drawNumericData(cairo_t *cr) {
    for (const auto &plot : m_numeric_data) {
        if (!plot.style.scatter) {
            drawNumericPath(plot, cr);
        } else {
            drawNumericScatter(plot, cr);
        }
    }
}

void Plot2D::drawNumericPath(const DataSeries &plot, cairo_t *cr) {
    const std::vector<Real> &data_x = plot.x;
    const std::vector<Real> &data_y = plot.y;
    const size_t size = data_x.size();
    if (size == 0) return;

    cairo_save(cr);

    cairo_set_line_width(cr, plot.style.stroke);
    cdroid::Color c(plot.style.color);
    cairo_set_source_rgba(cr,c.red(),c.green(),c.blue(),c.alpha());

    const std::vector<double> dashes =plot.style.dash_array;
    if (!dashes.empty()) {
        cairo_set_dash(cr, dashes.data(), dashes.size(), 0);
    }

    bool first = true;
    for (size_t i = 0; i < size; ++i) {
        if (IsInfinity(data_y[i])) {
            first = true;
            continue;
        }

        auto v = translateToFrame(data_x[i], data_y[i]);
        float abs_x = v.first + m_frame_x;
        float abs_y = v.second + m_frame_y;

        if (first) {
            cairo_move_to(cr, abs_x, abs_y);
            first = false;
        } else {
            cairo_line_to(cr, abs_x, abs_y);
        }
    }

    cairo_stroke(cr);
    cairo_restore(cr);
}

void Plot2D::drawNumericScatter(const DataSeries &plot, cairo_t *cr) {
    const std::vector<Real> &data_x = plot.x;
    const std::vector<Real> &data_y = plot.y;
    const size_t size = data_x.size();

    cairo_save(cr);
    cdroid::Color c(plot.style.color);
    cairo_set_source_rgba(cr,c.red(),c.green(),c.blue(),c.alpha());

    for (size_t i = 0; i < size; ++i) {
        if (IsInfinity(data_y[i])) continue;

        const auto  v= translateToFrame(data_x[i], data_y[i]);
        float cx = v.first + m_frame_x;
        float cy = v.second + m_frame_y;
        float radius = plot.style.stroke;

        cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
        cairo_fill(cr);
    }
    cairo_restore(cr);
}

void Plot2D::drawCategoricalData(cairo_t *cr) {
    for (const auto &plot : m_categorical_data) {
        if (!plot.style.scatter) {
            drawCategoricalPath(plot, cr);
        } else {
            drawCategoricalScatter(plot, cr);
        }
    }
}

void Plot2D::drawCategoricalPath(const CategoricalDataSeries &plot, cairo_t *cr) {
    const std::vector<Real> &data_y = plot.y;
    const size_t size = data_y.size();
    if (size == 0) return;

    cairo_save(cr);
    cdroid::Color c(plot.style.color);
    cairo_set_line_width(cr, plot.style.stroke);
    cairo_set_source_rgba(cr,c.red(),c.green(),c.blue(),c.alpha());

    const std::vector<double>& dashes=plot.style.dash_array;
    if (!dashes.empty()) {
        cairo_set_dash(cr, dashes.data(), dashes.size(), 0);
    }

    bool first = true;
    for (size_t i = 0; i < size; ++i) {
        if (IsInfinity(data_y[i])) {
            first = true;
            continue;
        }

        auto v = translateToFrame(0, data_y[i]);
        float tx = static_cast<float>(i) * (m_frame_w / static_cast<float>(size - 1));
        float abs_x = tx + m_frame_x;
        float abs_y = v.second + m_frame_y;

        if (first) {
            cairo_move_to(cr, abs_x, abs_y);
            first = false;
        } else {
            cairo_line_to(cr, abs_x, abs_y);
        }
    }

    cairo_stroke(cr);
    cairo_restore(cr);
}

void Plot2D::drawCategoricalScatter(const CategoricalDataSeries &plot, cairo_t *cr) {
    const std::vector<Real> &data_y = plot.y;
    const size_t size = data_y.size();

    cairo_save(cr);
    cdroid::Color c(plot.style.color);
    cairo_set_source_rgba(cr,c.red(),c.green(),c.blue(),c.alpha());

    for (size_t i = 0; i < size; ++i) {
        if (IsInfinity(data_y[i])) continue;

        auto v = translateToFrame(0, data_y[i]);
        float tx = static_cast<float>(i) * (m_frame_w / static_cast<float>(size - 1));
        float cx = tx + m_frame_x;
        float cy = v.second + m_frame_y;
        float radius = plot.style.stroke;

        cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
        cairo_fill(cr);
    }
    cairo_restore(cr);
}

void Plot2D::drawTitle(cairo_t *cr) {
    if (m_title.empty()) return;

    const float x = static_cast<float>(m_width) / 2;
    const float y = static_cast<float>(m_height) * FRAME_TOP_MARGIN_REL / 2;

    const float font_size = fonts::ConstrainedFontSize(
                                BASE_TITLE_FONT_SIZE, m_title, TEXT_FONT,
                                static_cast<float>(m_width),
                                static_cast<float>(m_height) * FRAME_TOP_MARGIN_REL);

    cairo_save(cr);
    cairo_select_font_face(cr, TEXT_FONT,
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, font_size);
    cairo_set_source_rgb(cr, 0, 0, 0); // 黑色

    cairo_text_extents_t extents;
    cairo_text_extents(cr, m_title.c_str(), &extents);
    float text_x = x - extents.width / 2;
    float text_y = y + extents.height / 2;
    cairo_move_to(cr, text_x, text_y);
    cairo_show_text(cr, m_title.c_str());
    cairo_restore(cr);
}

void Plot2D::drawLabels(cairo_t *cr) {
    cairo_save(cr);
    cairo_select_font_face(cr, TEXT_FONT,
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, m_axis_font_size);
    cairo_set_source_rgb(cr, 0, 0, 0);

    if (!m_x_label.empty()) {
        const float frame_bottom = m_frame_y + m_frame_h;
        const float x = m_frame_x + (m_frame_w / 2);
        const float y = frame_bottom + 0.75f * (static_cast<float>(m_height) - frame_bottom);

        cairo_text_extents_t extents;
        cairo_text_extents(cr, m_x_label.c_str(), &extents);
        float text_x = x - extents.width / 2;
        float text_y = y + extents.height / 2;
        cairo_move_to(cr, text_x, text_y);
        cairo_show_text(cr, m_x_label.c_str());
    }

    if (!m_y_label.empty()) {
        const float x = (1 - 0.75f) * m_frame_x;
        const float y = m_frame_y + (m_frame_h / 2);

        cairo_save(cr);
        cairo_translate(cr, x, y);
        cairo_rotate(cr, -M_PI / 2);
        cairo_text_extents_t extents;
        cairo_text_extents(cr, m_y_label.c_str(), &extents);
        float text_x = -extents.width / 2;
        float text_y = extents.height / 2;
        cairo_move_to(cr, text_x, text_y);
        cairo_show_text(cr, m_y_label.c_str());
        cairo_restore(cr);
    }

    cairo_restore(cr);
}

void Plot2D::drawLegend(cairo_t *cr) {
    Legend legend;

    switch (m_data_type) {
    case DataType::NUMERIC: {
        const size_t num_labels = std::min(m_legend_labels.size(), m_numeric_data.size());
        for (size_t i = 0; i < num_labels; ++i) {
            Legend::DataType type = (m_numeric_data[i].style.scatter == false)
                ? Legend::DataType::LINE : Legend::DataType::POINT;
            legend.addEntry(m_legend_labels[i], {type, m_numeric_data[i].style.color,
                                                 m_numeric_data[i].style.dash_array});
        }
        break;
    }
    case DataType::CATEGORICAL: {
        const size_t num_labels = std::min(m_legend_labels.size(), m_categorical_data.size());
        for (size_t i = 0; i < num_labels; ++i) {
            Legend::DataType type = (m_categorical_data[i].style.scatter == false)
                ? Legend::DataType::LINE : Legend::DataType::POINT;
            legend.addEntry(m_legend_labels[i], {type, m_categorical_data[i].style.color,
                                                 m_categorical_data[i].style.dash_array});
        }
        break;
    }
    }

    legend.draw(cr, m_frame_x + m_frame_w, m_frame_y, LEGEND_MARGIN,
                Legend::Alignment::TOP_RIGHT);
}

} // namespace plotcpp
