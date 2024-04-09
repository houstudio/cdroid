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
    int mFrameCount;
    int mImageWidth;
    int mImageHeight;
    float mScale;
    std::istream*istream;
public:
    ImageDecoder(std::istream&);
    virtual ~ImageDecoder();
    virtual int load()=0;
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
    GIFDecoder(std::istream&);
    ~GIFDecoder()override;
    int load()override;
    virtual int getFrameDuration(int)const;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)override;
};

class JPEGDecoder:public ImageDecoder{
public:
    JPEGDecoder(std::istream&);
    int load()override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)override;
};

class APNGDecoder:public ImageDecoder{
public:
    APNGDecoder(std::istream&);
    ~APNGDecoder()override;
    int load()override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)override;
};

class WebpDecoder:public ImageDecoder{
public:
    WebpDecoder(std::istream&);
    ~WebpDecoder();
    int load()override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int frameIndex)override;
};
}/*endof namespace*/
#endif
