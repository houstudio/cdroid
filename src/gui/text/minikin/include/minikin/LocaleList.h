/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef MINIKIN_LOCALE_H
#define MINIKIN_LOCALE_H

#include <string>

namespace minikin {

// A special ID for the empty locale list.
// This value must be 0 since the empty locale list is inserted into mLocaleLists by default.
const static uint32_t kEmptyLocaleListId = 0;

// Looks up a locale list from an internal cache and returns its ID.
// If the passed locale list is not in the cache, registers it and returns newly assigned ID.
// TODO: Introduce LocaleId type.
uint32_t registerLocaleList(const std::string& locales);

std::string getLocaleString(uint32_t id);

}  // namespace minikin

#endif  // MINIKIN_LOCALE_H
