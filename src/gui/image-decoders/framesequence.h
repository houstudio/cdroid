/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef __FRAME_SEQUENCE_H__
#define __FRAME_SEQUENCE_H__

#include <iostream>

#define  COLOR_8888_ALPHA_MASK  0xff000000 // TODO: handle endianness
#define  COLOR_TRANSPARENT 0x0

namespace cdroid{
class FrameSequenceState;
class FrameSequence {
public:
    struct RegistryEntry;
    class Registry;
private:
    static Registry*mHead;
    static int mHeaderBytesRequired;
public:
    /**
     * Creates a FrameSequence using data from the data stream
     *
     * Type determined by header information in the stream
     */
    static FrameSequence* create(std::istream* stream);
    static bool isSupport(std::istream* stream);

    virtual ~FrameSequence() {}
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual bool isOpaque() const = 0;
    virtual int getFrameCount() const = 0;
    virtual int getDefaultLoopCount() const = 0;

    virtual FrameSequenceState* createState() const = 0;
};

class FrameSequenceState {
public:
    /**
    * Produces a frame of animation in the output buffer, drawing (at minimum) the delta since
    * previousFrameNr (the current contents of the buffer), or from scratch if previousFrameNr is
    * negative
    * Returns frame's delay time in milliseconds.
    */
    virtual long drawFrame(int frameNr, uint32_t* outputPtr, int outputPixelStride, int previousFrameNr) = 0;
    virtual ~FrameSequenceState() {}
};

struct FrameSequence::RegistryEntry {
    int requiredHeaderBytes;
    bool (*checkHeader)(void* header, int header_size);
    FrameSequence* (*createFrameSequence)(std::istream* stream);
    bool (*acceptsBuffer)();
};

class FrameSequence::Registry {
public:
    Registry(const RegistryEntry& entry);
    static const RegistryEntry* find(std::istream* stream);
private:
    RegistryEntry mImpl;
    Registry* mNext;
};

}/*endof namespace*/
#endif //__FRAME_SEQUENCE_H__
