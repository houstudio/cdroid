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

#ifndef MINIKIN_MINIKIN_FONT_FACTORY_H
#define MINIKIN_MINIKIN_FONT_FACTORY_H

#include "minikin/Buffer.h"
#include "minikin/MinikinFont.h"

namespace minikin {

// A class to serialize / deserialize MinikinFont instance into / from memory buffer.
class MinikinFontFactory {
public:
    MinikinFontFactory() {}

    virtual ~MinikinFontFactory() = 0;

    // Create MinikinFont instance from the buffer.
    virtual std::shared_ptr<MinikinFont> create(BufferReader reader) const = 0;

    // Skip a MinikinFont region in the buffer and advance the reader to the
    // next position.
    virtual void skip(BufferReader* reader) const = 0;

    // Serialize MinikinFont into the buffer.
    virtual void write(BufferWriter* writer, const MinikinFont* minikinFont) const = 0;

    // Return the singleton MinikinFontFactory instance.
    // setInstance() must be called before any MinikinFont instance is
    // serialized or deserialized.
    static const MinikinFontFactory& getInstance();

    // Set the factory instance.
    // The factory must be a singleton and cannot be changed during the process lifetime.
    // It is safe to call this method multiple times with the same instance.
    // This method itself is not thread safe. The call to this method, as well
    // as deserialized MinikinFont objects, must be synchronized by the caller.
    static void setInstance(const MinikinFontFactory* factory);
};

}  // namespace minikin

#endif  // MINIKIN_MINIKIN_FONT_FACTORY_H
