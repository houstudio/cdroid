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
#ifndef __CDPLOT_COMPONENTS_LEGEND_H__
#define __CDPLOT_COMPONENTS_LEGEND_H__

#include <widget/plot/fonts.h>
#include <widget/plot/utility.h>
#include <cairo/cairo.h>
namespace plotcpp {

class Legend {
public:
    enum class Alignment {
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
    };

    enum class DataType {
        LINE,
        POINT,
        BAR,
    };

    struct Style {
        DataType type;
        int32_t color;
        std::vector<double> dash_array;
    };

    Legend() = default;

    void addEntry(const std::string &label, const Style &style);

    void draw(cairo_t*cr, float x, float y, float margin, Alignment alignment) const;
protected:
    using LegendEntry = std::pair<std::string, Style>;
    std::vector<LegendEntry> m_legend_labels;

    static constexpr float FONT_SIZE = 12.0f;
    static constexpr float FONT_EM = FONT_SIZE / 12.0f;
    static constexpr float FONT_MARGIN_EM = 0.5f * FONT_EM;
    static constexpr float SYMBOL_LENTH_EM = 1.5f * FONT_EM;
    static constexpr float RECT_LENGTH_EM = 3.0f * FONT_EM / 4.0f;
    static constexpr float RECT_LENGTH_PX = fonts::EmToPx(RECT_LENGTH_EM);
    static constexpr float SPACING_LENGTH_EM = 0.5f * FONT_EM;
    static constexpr int32_t STROKE_COLOR = BORDER_COLOR;
    static constexpr int32_t BOX_COLOR =0xFFFFFFFF;
    static constexpr float BOX_OPACITY = 0.90f;
    static constexpr float BOX_RADIUS = 3.0f;
};

} // namespace plotcpp

#endif // __CDPLOT_COMPONENTS_LEGEND_H__
