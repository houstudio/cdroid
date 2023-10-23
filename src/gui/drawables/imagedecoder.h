#ifndef __IMAGE_READER_H__
#define __IMAGE_READER_H__
namespace cdroid{

class ImageDecoder{
protected:
    void*mPrivate;
    int mFrameCount;
    int mImageWidth;
    int mImageHeight;
public:
    ImageDecoder();
    virtual ~ImageDecoder();
    virtual int load(std::istream&)=0;
    int getFrameCount()const;
    int getWidth()const;
    int getHeight()const;
    virtual int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)=0;
};

class GIFDecoder:public ImageDecoder{
public:
    GIFDecoder();
    ~GIFDecoder()override;
    int load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)override;
};

class APNGDecoder:public ImageDecoder{
public:
    APNGDecoder();
    ~APNGDecoder()override;
    int load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)override;
};

}/*endof namespace*/
#endif
