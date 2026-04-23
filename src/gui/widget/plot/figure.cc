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
#include <fstream>
#include <string>
#include <thread>
#include <widget/plot/figure.h>

namespace plotcpp {

void Figure::setTitle(const std::string &title) {
    m_title = title;
}

std::string Figure::getTitle() const {
    return m_title;
}

void Figure::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;
}

uint32_t Figure::width() const {
    return m_width;
}

uint32_t Figure::height() const {
    return m_height;
}

} // namespace plotcpp
