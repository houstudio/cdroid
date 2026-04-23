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
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <algorithm>
#include <core/color.h>
#include <utils/textutils.h>
#include <widget/plot/frame.h>
#include <widget/plot/legend.h>
#include <widget/plot/fonts.h>
#include <widget/plot/utility.h>
#include <widget/plot/barplotbase.h>

namespace plotcpp {

void BarPlotBase::setXLabel(const std::string &label) {
    m_x_label = label;
}

void BarPlotBase::setYLabel(const std::string &label) {
    m_y_label = label;
}

void BarPlotBase::setGrid(bool enable) {
    m_grid_enable = enable;
}

void BarPlotBase::setRoundedEdges(bool enable) {
    m_rounded_borders = enable;
}

void BarPlotBase::setBarRelativeWidth(float rel_width) {
    m_bar_width_rel = std::max(0.0f, std::min(1.0f, rel_width));
}

void BarPlotBase::addYMarker(Real marker) {
    m_y_custom_markers.insert(marker);
}

void BarPlotBase::setLegend(const std::vector<std::string> &labels) {
    m_legend_labels = labels;
}

void BarPlotBase::clear() {
    clearData();

    m_x_label.clear();
    m_y_label.clear();
}

void BarPlotBase::clearData() {
    m_baselines.clear();
    m_numeric_x_data.clear();
    m_categorical_x_data.clear();
    m_y_data.clear();
}

void BarPlotBase::build(cairo_t*cr) {
    //m_svg.Reset();
    //m_svg.SetSize(m_width, m_height);

    calculateFrame();

    drawBackground(cr);
    drawTitle(cr);
    drawLabels(cr);
    drawFrame(cr);
    drawBars(cr);
    drawLegend(cr);
}

float BarPlotBase::translateToFrame(Real y) const {
    return (BAR_FRAME_Y_MARGIN_REL * m_frame_h) -
           (m_zoom_y * static_cast<float>(y - m_y_range.second));
}

void BarPlotBase::calculateFrame() {
    // Set default baselines if not set
    if (m_baselines.size() == 0) {
        m_baselines = std::vector<Real>(m_num_bars, 0.0f);
    }

    // Frame rectangle
    m_frame_x = static_cast<float>(m_width) * FRAME_LEFT_MARGIN_REL;
    m_frame_y = static_cast<float>(m_height) * (FRAME_TOP_MARGIN_REL);
    m_frame_w = static_cast<float>(m_width) *
                (1.0f - FRAME_LEFT_MARGIN_REL - FRAME_RIGHT_MARGIN_REL);
    m_frame_h = static_cast<float>(m_height) *
                (1.0f - FRAME_TOP_MARGIN_REL - FRAME_BOTTOM_MARGIN_REL);

    // Calculate y range
    std::vector<Real> pos_acc(m_num_bars, 0.0f);
    std::vector<Real> neg_acc(m_num_bars, 0.0f);
    for (const auto &series : m_y_data) {
        for (size_t i = 0; i < m_num_bars; ++i) {
            const Real value = series.values[i];
            if (value >= 0.0f) {
                pos_acc[i] += series.values[i];
            } else {
                neg_acc[i] += series.values[i];
            }
        }
    }
    for (size_t i = 0; i < m_num_bars; ++i) {
        pos_acc[i] += m_baselines[i];
        neg_acc[i] += m_baselines[i];
    }

    m_y_range =
        std::pair<Real, Real> {*std::min_element(neg_acc.begin(), neg_acc.end()),
                               *std::max_element(pos_acc.begin(), pos_acc.end())
                              };

    m_zoom_y = std::abs((m_frame_h * (1 - 2 * BAR_FRAME_Y_MARGIN_REL)) /
                        static_cast<float>(m_y_range.second - m_y_range.first));

    const unsigned int num_y_markers =std::min((uint32_t)MAX_NUM_Y_MARKERS,
                 static_cast<uint32_t>(m_frame_h / PIXELS_PER_Y_MARKER));
    m_y_markers = ranges::PartitionRange(m_y_range, num_y_markers);

    // Calculate font sizes
    const std::string DUMMY_TEXT = "-000.00";
    const float y_axis_font_size = fonts::ConstrainedFontSize(
                                       BASE_AXIS_FONT_SIZE, DUMMY_TEXT, TEXT_FONT,
                                       (3.0f * m_frame_x / 4.0f) - MARKER_LENGTH,
                                       m_frame_h / static_cast<float>(num_y_markers));
    m_axis_font_size = std::min({y_axis_font_size});
}

static void quadratic_curve_to_relative(cairo_t *cr, double dx1, double dy1, double dx2, double dy2) {
    double x0, y0;
    cairo_get_current_point(cr, &x0, &y0);
    
    double x1 = x0 + dx1;
    double y1 = y0 + dy1;
    double x2 = x0 + dx2;
    double y2 = y0 + dy2;
    
    double c1_x = x0 + (2.0/3.0) * (x1 - x0);
    double c1_y = y0 + (2.0/3.0) * (y1 - y0);
    double c2_x = x2 + (2.0/3.0) * (x1 - x2);
    double c2_y = y2 + (2.0/3.0) * (y1 - y2);
    
    cairo_curve_to(cr, c1_x, c1_y, c2_x, c2_y, x2, y2);
}

void BarPlotBase::drawBars(cairo_t*cr) {
    // Horizontal space for a bar including a relative margin
    const float bar_horizontal_space =  (m_frame_w * (1 - 2 * BAR_FRAME_Y_MARGIN_REL)) / static_cast<float>(m_num_bars);
    const float bar_width = bar_horizontal_space * m_bar_width_rel;

    std::vector<size_t> remaining_positive_segment_counts =  std::vector<size_t>(m_num_bars, 0);
    std::vector<size_t> remaining_negative_segment_counts =  std::vector<size_t>(m_num_bars, 0);
    std::vector<Real> positive_bar_sizes(m_num_bars, 0.0f);
    std::vector<Real> negative_bar_sizes(m_num_bars, 0.0f);
    for (const auto &data_series : m_y_data) {
        for (size_t i = 0; i < m_num_bars; ++i) {
            const Real value = data_series.values[i];
            if (value > 0) {
                ++remaining_positive_segment_counts[i];
                positive_bar_sizes[i] += value;
            } else if (value < 0) {
                ++remaining_negative_segment_counts[i];
                negative_bar_sizes[i] += value;
            }
        }
    }

    std::vector<Real> pos_acc(m_num_bars, 0.0f);
    std::vector<Real> neg_acc(m_num_bars, 0.0f);
    for (const auto &series : m_y_data) {
        for (size_t i = 0; i < m_num_bars; ++i) {
            const Real value = series.values[i];

            float start_y;
            float end_y;
            bool should_round_border = m_rounded_borders;
            Real bar_height = 0.0f;
            if (value > 0) {
                start_y = m_frame_y + translateToFrame(m_baselines[i] + pos_acc[i]);
                end_y =  m_frame_y + translateToFrame(m_baselines[i] + pos_acc[i] + value);
                pos_acc[i] += value;
                bar_height = translateToFrame(positive_bar_sizes[i]) -
                             translateToFrame(m_baselines[i]);
                should_round_border &= (--remaining_positive_segment_counts[i] == 0);
            } else if (value < 0) {
                start_y = m_frame_y + translateToFrame(m_baselines[i] + neg_acc[i]);
                end_y =  m_frame_y + translateToFrame(m_baselines[i] + neg_acc[i] + value);
                neg_acc[i] += value;
                bar_height = translateToFrame(negative_bar_sizes[i]) -
                             translateToFrame(m_baselines[i]);
                should_round_border &= (--remaining_negative_segment_counts[i] == 0);
            } else {
                continue;
            }

            const float bar_center_x = m_frame_x + (m_frame_w * BAR_FRAME_X_MARGIN_REL) +
                          (bar_horizontal_space / 2.0f) + (static_cast<float>(i) * bar_horizontal_space);

            static constexpr float max_radius = 5.0f;
            const float radius = std::min({max_radius, bar_width / 2.0f, static_cast<float>(std::abs(bar_height))});
            const float delta = (value >= 0) ? -radius : radius;

            should_round_border &= (std::abs(bar_height) >= std::abs(delta));

            cdroid::Color sc(series.color);
            cairo_set_source_rgb(cr,sc.red(),sc.green(),sc.blue());
            if(!should_round_border){
                cairo_move_to(cr,bar_center_x - (bar_width / 2.0f), start_y);
                cairo_line_to(cr,bar_center_x - (bar_width / 2.0f),end_y);
                cairo_rel_line_to(cr,bar_width,0);
                cairo_line_to(cr,bar_center_x +bar_width/2.0f,start_y);
            }else{
                double current_x = bar_center_x - (bar_width / 2.0f);
                double current_y = start_y;
                cairo_move_to(cr,bar_center_x - (bar_width / 2.0f), start_y);
                cairo_line_to(cr,bar_center_x - (bar_width / 2.0f), current_y=end_y);
                quadratic_curve_to_relative(cr,0, delta,std::abs(delta), delta);
                current_x += std::abs(delta);
                current_y += delta;
                double target_x = bar_center_x + bar_width / 2.0f - std::abs(delta);
                current_x = bar_center_x + bar_width / 2.0f - std::abs(delta);
                cairo_line_to(cr,current_x,current_y);
                quadratic_curve_to_relative(cr,std::abs(delta), 0, std::abs(delta), -delta);
                current_x += std::abs(delta);
                cairo_line_to(cr,current_x,start_y);
            }
            cairo_close_path(cr);
            cairo_stroke_preserve(cr);
            cairo_fill(cr);
        }
    }
}

void BarPlotBase::drawBackground(cairo_t*cr) {
    //m_svg.DrawBackground(BACKGROUND_COLOR);
    cdroid::Color c(BACKGROUND_COLOR);
    cairo_set_source_rgb(cr,c.red(),c.green(),c.blue());
    cairo_paint(cr);
}

void BarPlotBase::drawLabels(cairo_t*cr) {
    if (!m_x_label.empty()) {
        const float frame_bottom = m_frame_y + m_frame_h;
        const float x = m_frame_x + (m_frame_w / 2);
        const float y = frame_bottom + (0.75f) * (static_cast<float>(m_height) - frame_bottom);
        cairo_set_source_rgb(cr,0,0,0);
        cairo_move_to(cr,x,y);
        cairo_show_text(cr,m_x_label.c_str());
    }

    if (!m_y_label.empty()) {
        const float x = (1 - 0.75f) * m_frame_x;
        const float y = m_frame_y + (m_frame_h / 2);
        cairo_text_extents_t extents;
        cairo_save(cr);
        cairo_set_font_size(cr, m_axis_font_size);
        cairo_text_extents(cr, m_y_label.c_str(), &extents);
        cairo_set_source_rgb(cr,0,0,0);
        cairo_move_to(cr,x,y);
        cairo_rotate(cr,-M_PI/2.0);
        cairo_translate(cr, -extents.width/2.0,0);
        cairo_show_text(cr,m_y_label.c_str());
        cairo_restore(cr);
    }
}

void BarPlotBase::drawFrame(cairo_t*cr) {
    PlotFrame frame(m_frame_w, m_frame_h, m_grid_enable);

    // Left markers
    std::set<Real> left_markers;
    left_markers.insert(m_y_markers.begin(), m_y_markers.end());
    left_markers.insert(m_y_custom_markers.begin(), m_y_custom_markers.end());

    for (const auto &marker : left_markers) {
        if ((marker < m_y_range.first) || (marker > m_y_range.second)) {
            continue;
        }

        const auto y = translateToFrame(marker);
        const std::string marker_text = (!m_round_y_markers)
            ? cdroid::TextUtils::stringPrintf("%.2f",marker)
            : cdroid::TextUtils::stringPrintf("%d",static_cast<int>(std::round(marker)));
        frame.addLeftMarker(y, marker_text);
    }

    // Bottom markers
    const size_t max_num_x_markers = static_cast<size_t>(m_frame_w / PIXELS_PER_X_MARKER);
    const size_t marker_step = std::max(1UL, m_num_bars / max_num_x_markers);

    const float bar_horizontal_space = (m_frame_w * (1 - 2 * BAR_FRAME_Y_MARGIN_REL)) /
        static_cast<float>(m_num_bars);

    for (size_t i = 0; i < m_num_bars; i += marker_step) {
        const float x = (m_frame_w * BAR_FRAME_X_MARGIN_REL) + (bar_horizontal_space / 2.0f) +
            (static_cast<float>(i) * bar_horizontal_space);
        const std::string marker_text = (m_data_type == DataType::NUMERIC)
            ? cdroid::TextUtils::stringPrintf("%.2f",m_numeric_x_data[i])
            : m_categorical_x_data[i];
        frame.addBottomMarker(x, marker_text);
    }

    frame.draw(cr, m_frame_x, m_frame_y);
}

void BarPlotBase::drawTitle(cairo_t*cr) {
    if (m_title.empty()) {
        return;
    }

    const float x = static_cast<float>(m_width) / 2;
    const float y = static_cast<float>(m_height) * FRAME_TOP_MARGIN_REL / 2;

    const float font_size = fonts::ConstrainedFontSize( BASE_TITLE_FONT_SIZE, m_title, TEXT_FONT,
            static_cast<float>(m_width), static_cast<float>(m_height) * FRAME_TOP_MARGIN_REL);
    //auto node_ptr = m_svg.DrawText(svg::Text{m_title, x, y, font_size, components::TEXT_FONT});
    //svg::SetAttribute(node_ptr, "font-weight", "bold");
    //svg::SetAttribute(node_ptr, "text-anchor", "middle");
    cairo_move_to(cr,x,y);
    cairo_show_text(cr,m_title.c_str());
}

void BarPlotBase::drawLegend(cairo_t*cr) {
    Legend legend;

    const size_t num_labels = std::min(m_legend_labels.size(), m_y_data.size());
    for (size_t i = 0; i < num_labels; ++i) {
        legend.addEntry(m_legend_labels[i],
        {Legend::DataType::BAR, m_y_data[i].color});
    }

    legend.draw(cr, m_frame_x + m_frame_w, m_frame_y, LEGEND_MARGIN,
                Legend::Alignment::TOP_RIGHT);
}
} // namespace plotcpp
