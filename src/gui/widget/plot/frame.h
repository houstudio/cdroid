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
#ifndef __CDPLOT_COMPONENTS_FRAME_H__
#define __CDPLOT_COMPONENTS_FRAME_H__

#include <set>
#include <string>
#include <cairo/cairo.h>
#include <widget/plot/utility.h>

namespace plotcpp {

class PlotFrame {
public:
    enum class FrameType {
        FULL_FRAME,
        AXES_ONLY,
    };

    PlotFrame(float width, float height, bool enable_grid);

    void draw(cairo_t*cr, float x, float y) const;

    void addLeftMarker(float pos, const std::string &text);
    void addTopMarker(float pos, const std::string &text);
    void addRightMarker(float pos, const std::string &text);
    void addBottomMarker(float pos, const std::string &text);

    void setFrameColor(int32_t);
    int32_t getFrameColor()const;
    void setFrameType(FrameType frame_type);
protected:
    float m_width, m_height;
    float m_axis_font_size = 11.0f;
    bool m_grid_enable;
    FrameType m_frame_type = FrameType::FULL_FRAME;
    int32_t m_frame_color;
    using Marker = std::pair<float, std::string>;
    std::set<Marker> m_left_markers;
    std::set<Marker> m_top_markers;
    std::set<Marker> m_right_markers;
    std::set<Marker> m_bottom_markers;

    static constexpr float STROKE_WIDTH = 0.75f;
    const std::string DASH_ARRAY{"0.75,0.75"};
    static constexpr float MARKER_LENGTH = 5.0f;

    void drawFullFrame(cairo_t*cr, float x, float y) const;
    void drawAxesOnly(cairo_t*, float x, float y) const;
    void drawAxes(cairo_t*, float x, float y) const;
};

} // namespace plotcpp

#endif // __CDPLOT_COMPONENTS_FRAME_H__
