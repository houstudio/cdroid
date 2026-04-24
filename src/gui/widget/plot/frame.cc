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
#include <cmath>
#include <set>
#include <string>
#include <vector>
#include <cairo/cairo.h>
#include <core/color.h>
#include <widget/plot/frame.h>
#include <widget/plot/fonts.h>
#include <widget/plot/figure.h>

namespace plotcpp {

PlotFrame::PlotFrame(float width, float height, bool enable_grid)
    : m_width(width), m_height(height), m_grid_enable(enable_grid) {
    m_frame_color =0xff808080;
}

void PlotFrame::addLeftMarker(float pos, const std::string &text) {
    m_left_markers.insert({pos, text});
}

void PlotFrame::addTopMarker(float pos, const std::string &text) {
    m_top_markers.insert({pos, text});
}

void PlotFrame::addRightMarker(float pos, const std::string &text) {
    m_right_markers.insert({pos, text});
}

void PlotFrame::addBottomMarker(float pos, const std::string &text) {
    m_bottom_markers.insert({pos, text});
}

void PlotFrame::setFrameColor(int32_t color){
    m_frame_color = color;
}

int32_t PlotFrame::getFrameColor()const{
    return m_frame_color;
}

void PlotFrame::setFrameType(FrameType frame_type) {
    m_frame_type = frame_type;
}

void PlotFrame::draw(cairo_t*cr, float x, float y) const {
    drawAxes(cr, x, y);

    switch (m_frame_type) {
    case FrameType::FULL_FRAME:
        drawFullFrame(cr, x, y);
        break;
    case FrameType::AXES_ONLY:
        drawAxesOnly(cr, x, y);
        break;
    }
}

void PlotFrame::drawFullFrame(cairo_t *cr, float x, float y) const {
    cairo_rectangle(cr, x, y, m_width, m_height);
    cdroid::Color cs(m_frame_color);
    cairo_set_source_rgba(cr, cs.red(), cs.green(),cs.blue(),cs.alpha());
    cairo_set_line_width(cr, STROKE_WIDTH);
    cairo_stroke(cr);
}

void PlotFrame::drawAxesOnly(cairo_t *cr, float x, float y) const {
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x, y + m_height);
    cairo_line_to(cr, x + m_width, y + m_height);
    cdroid::Color cs(m_frame_color);
    cairo_set_source_rgba(cr,cs.red(), cs.green(),cs.blue(),cs.alpha());
    cairo_set_line_width(cr, STROKE_WIDTH);
    cairo_stroke(cr);
}

void PlotFrame::drawAxes(cairo_t*cr, float x, float y) const {
    cairo_save(cr);

    cairo_select_font_face(cr, TEXT_FONT,
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, m_axis_font_size);

    // Left markers
    cdroid::Color cs(m_frame_color);
    for (const Marker &marker : m_left_markers) {
        float mark_y = y + marker.first;
        cairo_move_to(cr, x, mark_y);
        cairo_line_to(cr, x - MARKER_LENGTH, mark_y);
        cairo_set_source_rgba(cr,cs.red(), cs.green(),cs.blue(),cs.alpha());
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);
        // TODO: Draw text
        cairo_text_extents_t extents;
        cairo_text_extents(cr, marker.second.c_str(), &extents);
        float text_x = x - 2 * MARKER_LENGTH - extents.width;
        float text_y = mark_y + extents.height / 2.0;
        cairo_move_to(cr, text_x, text_y);
        //cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_show_text(cr, marker.second.c_str());

        if (m_grid_enable) {
            cairo_move_to(cr, x, mark_y);
            cairo_line_to(cr, x + m_width, mark_y);
            cairo_set_source_rgba(cr,cs.red(), cs.green(),cs.blue(),cs.alpha());
            cairo_set_line_width(cr, 0.75);
            double dashes[] = {0.75, 0.75};
            cairo_set_dash(cr, dashes, 2, 0);
            cairo_stroke(cr);
            cairo_set_dash(cr, nullptr, 0, 0);
        }
    }

    for (const Marker &marker : m_bottom_markers) {
        float mark_x = x + marker.first;

        cairo_move_to(cr, mark_x, y + m_height);
        cairo_line_to(cr, mark_x, y + m_height + MARKER_LENGTH);
        cairo_set_source_rgba(cr,cs.red(), cs.green(),cs.blue(),cs.alpha());
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);

        cairo_text_extents_t extents;
        cairo_text_extents(cr, marker.second.c_str(), &extents);
        float text_x = x + marker.first;
        float text_y = y + m_height + MARKER_LENGTH + extents.height;
        cairo_move_to(cr, text_x, text_y);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_show_text(cr, marker.second.c_str());

        if (m_grid_enable) {
            cairo_move_to(cr, mark_x, y);
            cairo_line_to(cr, mark_x, y + m_height);
            cairo_set_source_rgb(cr,cs.red(), cs.green(),cs.blue());
            cairo_set_line_width(cr, 0.75);
            double dashes[] = {0.75, 0.75};
            cairo_set_dash(cr, dashes, 2, 0);
            cairo_stroke(cr);
            cairo_set_dash(cr, nullptr, 0, 0);
        }
    }

    // 可选：顶轴和右轴标记（原代码未实现，这里留空）
    // ...

    cairo_restore(cr);
}

} // namespace plotcpp
