#ifndef __IMAGE_READER_H__
#define __IMAGE_READER_H__
#include <string>
#include <iostream>
#include <vector>
#include <cairomm/surface.h>
#include <core/context.h>

namespace cdroid{

class ImageDecoder{
public:
    enum DisposalMethod{
        Unspecified,
        DoNotDispose,
        RestoreToBackground,
        RestoreToPrevious
    };
    struct ImageFrame {
        unsigned char * pixels;
        unsigned int x;
        unsigned int y;
        unsigned int width;
        unsigned int height;
        unsigned int duration;
        unsigned int disposalMethod;
    };

protected:
    void*mPrivate;
    int mFrameCount;
    int mImageWidth;
    int mImageHeight;
    std::vector<ImageFrame*>mFrames;
public:
    ImageDecoder();
    virtual ~ImageDecoder();
    virtual int load(std::istream&)=0;
    int getWidth()const;
    int getHeight()const;
    int getFrameCount()const;
    int getFrameDuration(int)const;
    virtual int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)=0;
    static ImageDecoder*create(Context*ctx,const std::string&resourceId);
    static Drawable*createAsDrawable(Context*ctx,const std::string&resourceId);
};

class GIFDecoder:public ImageDecoder{
public:
    GIFDecoder();
    ~GIFDecoder()override;
    int load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)override;
    void setFramePixels(int frameIndex);
};

class APNGDecoder:public ImageDecoder{
public:
    APNGDecoder();
    ~APNGDecoder()override;
    int load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)override;
};

class WebpDecoder:public ImageDecoder{
public:
    WebpDecoder();
    ~WebpDecoder();
    int load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)override;
};
}/*endof namespace*/
#endif
