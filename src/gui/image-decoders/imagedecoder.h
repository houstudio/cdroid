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
    void*mPrivate;
    int mFrameCount;
    int mImageWidth;
    int mImageHeight;
    float mScale;
public:
    ImageDecoder();
    virtual ~ImageDecoder();
    virtual int load(std::istream&)=0;
    int getWidth()const;
    int getHeight()const;
    float getScale()const;
    void setScale(float);
    virtual int getFrameCount()const;
    virtual int getFrameDuration(int)const;
    virtual int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)=0;
    static ImageDecoder*create(Context*ctx,const std::string&resourceId);
    static Drawable*createAsDrawable(Context*ctx,const std::string&resourceId);
};

class GIFDecoder:public ImageDecoder{
public:
    GIFDecoder();
    ~GIFDecoder()override;
    int load(std::istream&)override;
    virtual int getFrameDuration(int)const;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)override;
};

class JPEGDecoder:public ImageDecoder{
public:
    JPEGDecoder();
    int load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)override;
};

class APNGDecoder:public ImageDecoder{
public:
    APNGDecoder();
    ~APNGDecoder()override;
    int load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)override;
};

class WebpDecoder:public ImageDecoder{
public:
    WebpDecoder();
    ~WebpDecoder();
    int load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)override;
};
}/*endof namespace*/
#endif
