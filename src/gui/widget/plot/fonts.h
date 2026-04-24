/*
 * plotcpp is a 2D plotting library for modern C++
 *
 * Copyright 2022  Javier Lancha Vázquez <javier.lancha@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CDPLOT_FONTS_H__
#define __CDPLOT_FONTS_H__

#include <map>
#include <string>
#include <utility>

namespace plotcpp {
namespace fonts {

struct FontData {
    float width_em;
};

/** Convert em to px */
constexpr float EmToPx(float em) {
    return em * 16.0f;
};

/**
 * @brief Calculate an approximation of the bounding box of a text in em.
 *
 * @param text Text string
 * @param font Font family
 * @param size Font size
 * @return A std::pair with the width and height of the bounding box.
 */
std::pair<float, float> CalculateTextSize(const std::string &text,
        const std::string &font, float size);

float ConstrainedFontSize(float base_size, const std::string &text,
                          const std::string &font, float width, float height);

} // namespace fonts
} // namespace plotcpp

#endif // __CDPLOT_FONTS_H__
