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
#ifndef __IMAGE_READER_H__
#define __IMAGE_READER_H__
#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cairomm/surface.h>
#include <core/context.h>

typedef struct png_struct_def* png_structp;
typedef struct png_info_def* png_infop;

namespace cdroid{
class ImageDecoder{
public:
    typedef std::function<int(const uint8_t*,uint32_t)>Verifier;
    typedef std::function<std::unique_ptr<ImageDecoder>(std::istream&)> Factory;
    struct Registry{
        Factory factory;
        Verifier verifier;
        uint32_t magicSize;
        Registry(uint32_t,Factory&,Verifier&);
    };
private:
    static std::unordered_map<std::string,Registry>mFactories;
    static uint32_t mHeaderBytesRequired;
protected:
    struct PRIVATE*mPrivate;
    int mImageWidth;
    int mImageHeight;
    int mFrameCount;
    std::istream&mStream;
    static std::unique_ptr<ImageDecoder>getDecoder(std::istream&);
public:
    ImageDecoder(std::istream&);
    virtual ~ImageDecoder();
    int getWidth()const;
    int getHeight()const;
    int getFrameCount()const;
    virtual bool decodeSize()=0;
    virtual Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)=0;

    static int  registerFactory(const std::string&mime,uint32_t magicSize,Verifier,Factory factory);
    static int  computeTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp);
    static int  getTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp);
    static void setTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp,int);
    static Cairo::RefPtr<Cairo::ImageSurface>loadImage(std::istream&,int width=-1,int height=-1);
    static Cairo::RefPtr<Cairo::ImageSurface>loadImage(Context*ctx,const std::string&,int width=-1,int height=-1);
    static Drawable*createAsDrawable(Context*ctx,const std::string&resourceId);
};

class GIFDecoder:public ImageDecoder{
public:
    GIFDecoder(std::istream&);
    ~GIFDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
    static bool isGIF(const uint8_t*,uint32_t);
};

class JPEGDecoder:public ImageDecoder{
private:
    void*getColorProfile(PRIVATE*);
public:
    JPEGDecoder(std::istream&);
    ~JPEGDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
    static bool isJPEG(const uint8_t*,uint32_t);
};

class PNGDecoder:public ImageDecoder{
private:
    void*getColorProfile(PRIVATE*,uint8_t colorType);
public:
    static double setGamma(Context*ctx,png_structp png_ptr,png_infop info_ptr);
public:
    PNGDecoder(std::istream&);
    ~PNGDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
    static bool isPNG(const uint8_t*,uint32_t);
};

// BMP decoder supporting classic and VxD headers, paletted & truecolor, RLE4/RLE8, BITFIELDS, and
// BI_JPEG/BI_PNG embedded-compressed bitmaps (delegates to JPEG/PNG decoders when present).
class BMPDecoder:public ImageDecoder{
private:
    struct PRIVATE;
    PRIVATE* mPrivate;
public:
    BMPDecoder(std::istream&);
    ~BMPDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
    static bool isBMP(const uint8_t*,uint32_t);
};

// JPEG2000 decoder wrapper around OpenJPEG
class JPEG2000Decoder: public ImageDecoder {
private:
    struct PRIVATE;
    PRIVATE* mPrivate;
public:
    enum class Format { JP2, J2K };
    JPEG2000Decoder(std::istream&);
    ~JPEG2000Decoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
    static bool isJP2(const uint8_t*, uint32_t);
    static bool isJ2K(const uint8_t*, uint32_t);
};

class WEBPDecoder:public ImageDecoder{
private:
    void*getColorProfile(PRIVATE*,uint8_t colorType);
public:
    WEBPDecoder(std::istream&);
    ~WEBPDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
    static bool isWEBP(const uint8_t*,uint32_t);
};

}/*endof namespace*/
#endif
