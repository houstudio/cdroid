#ifndef __IMAGE_READER_H__
#define __IMAGE_READER_H__
#include <string>
#include <iostream>
#include <vector>
#include <cairomm/surface.h>
#include <core/context.h>

namespace cdroid{
class ImageDecoder{
protected:
    struct PRIVATE*mPrivate;
    int mImageWidth;
    int mImageHeight;
    int mCurrScanline;
    void*mTransform;
    std::unique_ptr<std::istream>istream;
public:
    ImageDecoder(Context*ctx,const std::string&res);
    virtual ~ImageDecoder();
    int getWidth()const;
    int getHeight()const;

    virtual bool decodeSize()=0;
    virtual Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)=0;

    static ImageDecoder*create(Context*ctx,const std::string&resourceId);
    static Drawable*createAsDrawable(Context*ctx,const std::string&resourceId);
};

class GIFDecoder:public ImageDecoder{
public:
    GIFDecoder(Context*ctx,const std::string&res);
    ~GIFDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
};

class JPEGDecoder:public ImageDecoder{
public:
    JPEGDecoder(Context*ctx,const std::string&res);
    ~JPEGDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
};

class PNGDecoder:public ImageDecoder{
private:
    void*getColorProfile(PRIVATE*,uint8_t colorType);
public:
    PNGDecoder(Context*ctx,const std::string&res);
    ~PNGDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
};

}/*endof namespace*/
#endif
