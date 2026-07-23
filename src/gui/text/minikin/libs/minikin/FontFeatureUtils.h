/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MINIKIN_FONT_FEATURE_UTILS_H
#define MINIKIN_FONT_FEATURE_UTILS_H

#include <hb.h>

#include "minikin/MinikinPaint.h"

namespace minikin {

/**
 * Returns the final set of font features based on the features requested by this paint object and
 * extra defaults or implied font features.
 *
 * Features are included from the paint object if they are:
 *   1) in a supported range
 *
 * Default features are added based if they are:
 *   1) implied due to Paint settings such as letterSpacing
 *   2) default features that do not conflict with requested features
 */
std::vector<hb_feature_t> cleanAndAddDefaultFontFeatures(const MinikinPaint& paint);

}  // namespace minikin
#endif  // MINIKIN_LAYOUT_UTILS_H
