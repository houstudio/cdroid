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
#ifndef __FRAME_SEQUENCE_H__
#define __FRAME_SEQUENCE_H__

#include <map>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>

namespace cdroid{
class FrameSequenceState;
class Context;
class FrameSequence {
public:
    static constexpr uint32_t COLOR_8888_ALPHA_MASK = 0xff000000;
    static constexpr uint32_t COLOR_TRANSPARENT = 0x0;
public:
    typedef std::function<bool(const uint8_t*,uint32_t)>Verifier;
    typedef std::function<FrameSequence*(std::istream&)> Factory;
    struct Registry{
        Factory factory;
        Verifier verifier;
        uint32_t magicSize;
        Registry(uint32_t,Factory&,Verifier&);
    };
private:
    static std::map<const std::string,Registry>mFactories;
    static uint32_t mHeaderBytesRequired;
protected:
    std::istream& mStream;
public:
    /**
     * Creates a FrameSequence using data from the data stream
     *
     * Type determined by header information in the stream
     */
    FrameSequence(std::istream&);
    virtual ~FrameSequence()=default;
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual bool isOpaque() const = 0;
    virtual int getFrameCount() const = 0;
    virtual int getDefaultLoopCount() const = 0;
    virtual FrameSequenceState* createState() const = 0;
    static int registerFactory(const std::string&mime,uint32_t,Verifier,Factory);
    static size_t registerAllFrameSequences(std::map<const std::string,Registry>&entis);
    static FrameSequence* create(cdroid::Context*,const std::string&resid);
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
    virtual ~FrameSequenceState() =default;
};

}/*endof namespace*/
#endif //__FRAME_SEQUENCE_H__
