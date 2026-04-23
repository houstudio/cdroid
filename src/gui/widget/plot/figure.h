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
#ifndef __CDPLOT_FIGURE_H__
#define __CDPLOT_FIGURE_H__

#include <string>
#include <thread>
#include <cairo/cairo.h>
#include <widget/plot/utility.h>
namespace plotcpp {
const char*const TEXT_FONT="monospace";
class Figure {
public:
    virtual ~Figure() = default;

    /**
     * @brief Build the figure with its current data and configuration. This sets
     * the figure ready to be displayed or saved.
     */
    virtual void build(cairo_t*cr) = 0;

    /** @brief Clear the figure */
    virtual void clear() = 0;

    /**
     * @brief Set figure title.
     *
     * @param title
     */
    void setTitle(const std::string &title);

    /**
     * @brief Returns the figure's title.
     *
     * @return title
     */
    std::string getTitle() const;

    /**
     * @brief Set figure size in pixels.
     *
     * @param width
     * @param height
     */
    void setSize(uint32_t width, uint32_t height);

    /**
     * @brief Returns the figure width in pixels.
     *
     * @return width
     */
    uint32_t width() const;

    /**
     * @brief Returns the figure height in pixels.
     *
     * @return height
     */
    uint32_t height() const;

protected:
    std::string m_title;

    static constexpr uint32_t DEFAULT_WIDTH = 600;
    static constexpr uint32_t DEFAULT_HEIGHT = 450;

    uint32_t m_width = DEFAULT_WIDTH;
    uint32_t m_height = DEFAULT_HEIGHT;

    explicit Figure() = default;
};

} // namespace plotcpp

#endif // __CDPLOT_FIGURE_H__
