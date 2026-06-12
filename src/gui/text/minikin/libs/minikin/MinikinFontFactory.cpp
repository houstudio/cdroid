/*
 * Copyright (C) 2022 The Android Open Source Project
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

#define LOG_TAG "Minikin"

#include "minikin/MinikinFontFactory.h"

#include <log/log.h>

#include "MinikinInternal.h"

namespace minikin {

namespace {
static const MinikinFontFactory* gMinikinFontFactory = nullptr;
}

MinikinFontFactory::~MinikinFontFactory() {}

// static
const MinikinFontFactory& MinikinFontFactory::getInstance() {
    MINIKIN_ASSERT(gMinikinFontFactory != nullptr, "setInstance should have been called.");
    return *gMinikinFontFactory;
}

// static
void MinikinFontFactory::setInstance(const MinikinFontFactory* factory) {
    MINIKIN_ASSERT(gMinikinFontFactory == nullptr || gMinikinFontFactory == factory,
                   "MinikinFontFactory cannot be changed after it is set.");
    gMinikinFontFactory = factory;
}

}  // namespace minikin
