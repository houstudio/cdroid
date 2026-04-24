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
#include <core/color.h>
#include <widget/plot/legend.h>
#include <widget/plot/fonts.h>
#include <widget/plot/figure.h>

namespace plotcpp {

void Legend::addEntry(const std::string &label, const Style &style) {
    m_legend_labels.emplace_back(LegendEntry{label, style});
}

void Legend::draw(cairo_t*cr, float x, float y, float margin, Alignment alignment) const {
    if (m_legend_labels.empty()) {
        return;
    }
    cairo_save(cr);
    // Calculate sizes
    float max_font_width_em = 0;
    for (const auto &label : m_legend_labels) {
        const auto v =fonts::CalculateTextSize(label.first, TEXT_FONT, FONT_SIZE);
        max_font_width_em = std::max(max_font_width_em, v.first);
    }

    const size_t num_labels = m_legend_labels.size();
    const float box_w = fonts::EmToPx(2 * FONT_MARGIN_EM + SYMBOL_LENTH_EM +
                                      SPACING_LENGTH_EM + max_font_width_em);
    const float box_h = fonts::EmToPx(static_cast<float>(num_labels) * FONT_EM +
                                      2 * FONT_MARGIN_EM);

    // Apply alignment
    switch (alignment) {
    case Alignment::TOP_LEFT:
        x += margin;
        y += margin;
        break;
    case Alignment::TOP_RIGHT:
        x -= (box_w + margin);
        y += margin;
        break;
    case Alignment::BOTTOM_LEFT:
        x += margin;
        y -= (box_h + margin);
        break;
    case Alignment::BOTTOM_RIGHT:
        x -= (box_w + margin);
        y -= (box_h - margin);
        break;
    }

    const float radius = BOX_RADIUS;
    cairo_new_path(cr);

    cairo_move_to(cr, x + radius, y);
    cairo_line_to(cr, x + box_w - radius, y);
    cairo_arc(cr, x + box_w - radius, y + radius, radius, -M_PI/2, 0);
    cairo_line_to(cr, x + box_w, y + box_h - radius);
    cairo_arc(cr, x + box_w - radius, y + box_h - radius, radius, 0, M_PI/2);
    cairo_line_to(cr, x + radius, y + box_h);
    cairo_arc(cr, x + radius, y + box_h - radius, radius, M_PI/2, M_PI);
    cairo_line_to(cr, x, y + radius);
    cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3*M_PI/2);
    cairo_close_path(cr);

    cdroid::Color cb(BOX_COLOR);
    cairo_set_source_rgba(cr,cb.red(),cb.green(),cb.blue(),cb.alpha());
    cairo_fill_preserve(cr);
    cdroid::Color cs(STROKE_COLOR);
    cairo_set_source_rgb(cr,cs.red(),cs.green(),cs.blue());
    cairo_set_line_width(cr, 1.0);
    cairo_stroke(cr);

    cairo_select_font_face(cr, TEXT_FONT,CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, FONT_SIZE);
    for (size_t i = 0; i < num_labels; ++i) {
        const auto& label = m_legend_labels[i];
        const float element_y = y + fonts::EmToPx(FONT_MARGIN_EM + FONT_EM/4.0f +
                                static_cast<float>(i) * FONT_EM);

        float symbol_x = x + fonts::EmToPx(FONT_MARGIN_EM);
        float symbol_size = fonts::EmToPx(SYMBOL_LENTH_EM);
        cdroid::Color cl(label.second.color);
        cairo_set_source_rgb(cr,cl.red(),cl.green(),cl.blue());
        if (label.second.type == DataType::LINE) {
            cairo_set_line_width(cr, 2.0f);
            const std::vector<double>& dashes =label.second.dash_array;
            if (!dashes.empty()) {
                cairo_set_dash(cr, dashes.data(), (int)dashes.size(), 0);
            }
            cairo_move_to(cr, symbol_x, element_y);
            cairo_line_to(cr, symbol_x + symbol_size, element_y);
            cairo_stroke(cr);
            cairo_set_dash(cr, nullptr, 0, 0);
        } else if (label.second.type == DataType::POINT) {
            float circle_x = symbol_x + symbol_size/2.0f;
            float circle_r = fonts::EmToPx(FONT_EM/3.0f);

            cairo_arc(cr, circle_x, element_y, circle_r, 0, 2*M_PI);
            cairo_fill(cr);
        } else if (label.second.type == DataType::BAR) {
            float rect_size = fonts::EmToPx(FONT_EM/2.0f);
            float rect_x = symbol_x + symbol_size/2.0f - rect_size/2.0f;
            float rect_y = element_y - rect_size/2.0f;

            cairo_rectangle(cr, rect_x, rect_y, rect_size, rect_size);
            cairo_fill_preserve(cr);
            cdroid::Color cs(BORDER_COLOR);
            cairo_set_source_rgb(cr,cs.red(), cs.green(),cs.blue());
            cairo_set_line_width(cr, 1.0f);
            cairo_stroke(cr);
        }

        const float text_x = symbol_x + symbol_size + fonts::EmToPx(SPACING_LENGTH_EM);
        const float text_y = element_y + fonts::EmToPx(FONT_EM/3.0f);

        cairo_move_to(cr, text_x, text_y);
        cairo_show_text(cr, label.first.c_str());
    }
    cairo_restore(cr);
}
} // namespace plotcpp
