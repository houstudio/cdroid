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

#include <widget/plot/groupfigure.h>

namespace plotcpp {

GroupFigure::GroupFigure(uint32_t rows,uint32_t cols) {
    m_figures.assign(rows*cols,nullptr);
    mRows=rows;
    mCols=cols;
    clearFigures();
};

/**
 * @brief Add a subplot at the specified location in the group.
 *
 * @param figure A pointer to a figure.
 * @param row Row
 * @param col Column
 */
void GroupFigure::subplot(Figure *figure, size_t row, size_t col) {
    if ((row >= mRows) && (col >= mCols)) {
        return;
    }

    m_figures[mCols * row + col] = figure;
}

/**
 * @brief Returns a pointer to the figure at the specified location in the
 * group.
 *
 * @param row Row
 * @param col Column
 */
Figure *GroupFigure::getFigure(size_t row, size_t col) const{
    return m_figures[mCols * row + col];
}

void GroupFigure::clear() {
    clearFigures();
}

void GroupFigure::build(cairo_t*cr) {
    //m_svg.DrawBackground({255, 255, 255});
    cairo_set_source_rgb(cr,1,1,1);
    cairo_paint(cr);
    const uint32_t subplot_width = m_width / mCols;
    const uint32_t subplot_height = m_height / mRows;

    for (uint32_t i = 0; i < mRows; ++i) {
        for (uint32_t j = 0; j < mCols; ++j) {
            Figure *figure = getFigure(i, j);
            if (figure == nullptr) {
                continue;
            }

            const uint32_t x = j * subplot_width;
            const uint32_t y = i * subplot_height;

            figure->setSize(subplot_width, subplot_height);
            cairo_save(cr);
            cairo_translate(cr,x,y);
            figure->build(cr);
            cairo_restore(cr);
        }
    }
}

void GroupFigure::clearFigures() {
    for (auto &figure_ptr : m_figures) {
        figure_ptr = nullptr;
    }
}

} // namespace plotcpp

