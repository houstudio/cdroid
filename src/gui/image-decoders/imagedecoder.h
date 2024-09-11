#ifndef __IMAGE_READER_H__
#define __IMAGE_READER_H__
#include <string>
#include <iostream>
#include <vector>
#include <cairomm/surface.h>
#include <core/context.h>

typedef struct png_struct_def* png_structp;
typedef struct png_info_def* png_infop;

namespace cdroid{
class ImageDecoder{
protected:
    struct PRIVATE*mPrivate;
    int mImageWidth;
    int mImageHeight;
    int mCurrScanline;
    void*mTransform;
    static void*mCMSProfile;
    cdroid::Context*mContext;
    std::unique_ptr<std::istream>istream;
public:
    ImageDecoder(Context*ctx,const std::string&res);
    virtual ~ImageDecoder();
    int getWidth()const;
    int getHeight()const;

    virtual bool decodeSize()=0;
    virtual Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)=0;

    static std::unique_ptr<ImageDecoder>create(Context*ctx,const std::string&resourceId);
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
private:
    void*getColorProfile(PRIVATE*);
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
    static double setGamma(Context*ctx,png_structp png_ptr,png_infop info_ptr);
public:
    PNGDecoder(Context*ctx,const std::string&res);
    ~PNGDecoder()override;
    bool decodeSize()override;
    Cairo::RefPtr<Cairo::ImageSurface> decode(float scale=1.f,void*targetProfile=nullptr)override;
};

}/*endof namespace*/
#endif
