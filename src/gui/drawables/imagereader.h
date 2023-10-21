#ifndef __IMAGE_READER_H__
#define __IMAGE_READER_H__
namespace cdroid{

class ImageReader{
protected:
    void*mPrivate;
    int mFrameCount;
    int mImageWidth;
    int mImageHeight;
public:
    ImageReader();
    virtual ~ImageReader();
    virtual bool load(std::istream&)=0;
    int getFrameCount()const;
    int getWidth()const;
    int getHeight()const;
    virtual int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)=0;
};

class GIFReader:public ImageReader{
public:
    GIFReader();
    ~GIFReader()override;
    bool load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)override;
};

class APNGReader:public ImageReader{
public:
    APNGReader();
    ~APNGReader()override;
    bool load(std::istream&)override;
    int readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex)override;
};

}/*endof namespace*/
#endif
