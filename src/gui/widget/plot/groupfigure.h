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
#ifndef __CDPLOT_GROUPFIGURE_H__
#define __CDPLOT_GROUPFIGURE_H__

#include <vector>
#include <string>
#include <widget/plot/figure.h>

namespace plotcpp {

class GroupFigure : public Figure {
public:
    GroupFigure(uint32_t rows,uint32_t cols);

    /**
     * @brief Add a subplot at the specified location in the group.
     *
     * @param figure A pointer to a figure.
     * @param row Row
     * @param col Column
     */
    void subplot(Figure *figure, size_t row, size_t col);

    /**
     * @brief Returns a pointer to the figure at the specified location in the
     * group.
     *
     * @param row Row
     * @param col Column
     */
    Figure *getFigure(size_t row, size_t col)const;

    void clear() override;

    void build(cairo_t*cr) override;
protected:
    std::vector<Figure*> m_figures;
    static constexpr uint32_t HORIZONTAL_MARGIN = 20;
    static constexpr uint32_t VERTICAL_MARGIN = 20;
    uint32_t mCols;
    uint32_t mRows;
    void clearFigures();
}; // namespace plotcpp

} // namespace plotcpp

#endif // __CDPLOT_GROUPFIGURE_H__
