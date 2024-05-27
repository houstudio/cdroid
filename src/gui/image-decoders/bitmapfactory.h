#ifndef __BITMAP_FACTORY_H__
#define __BITMAP_FACTORY_H__
namespace cdroid{
class BitmapFactory{
public:
    class Options;
};

class BitmapFactory::Options{
public:
    Cairo::RefPtr<ImageSurface>inBitmap;
    bool inMutable;
    bool inJustDecodeBounds;
    bool inPremultiplied;
    bool inScaled;
    int inSampleSize;
    int inDensity;
    int inTargetDensity;
    int inScreenDensity;
    int outWidth;
    int outHeight;
public:
    Options(){
        inScaled = true;
	inPremultiplied = true;
    }
};
}
#endif /*__BITMAP_FACTORY_H__*/
