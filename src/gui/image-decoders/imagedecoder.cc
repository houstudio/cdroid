#include <memory>
#include <cstring>
#include <fstream>
#include <gui_features.h>
#include <drawables/drawable.h>
#include <drawables/bitmapdrawable.h>
#include <drawables/ninepatchdrawable.h>
#include <drawables/animatedimagedrawable.h>
#include <image-decoders/imagedecoder.h>
#include <core/textutils.h>
#include <core/context.h>
#include <core/atexit.h>
#include <png.h>
#include <porting/cdlog.h>
#if ENABLE(LCMS)
#include <lcms2.h>
#endif

namespace cdroid{
using namespace Cairo;
void*ImageDecoder::mCMSProfile = nullptr;

ImageDecoder::ImageDecoder(Context*ctx,const std::string&resourceId){
    mImageWidth = -1;
    mImageHeight= -1;
    mPrivate = nullptr;
    mTransform= nullptr;
    mContext = ctx;
    if(ctx)
        istream = ctx->getInputStream(resourceId);
    else
        istream = std::move(std::make_unique<std::ifstream>(resourceId));

#if ENABLE(LCMS)
    if(mCMSProfile==nullptr){
        mCMSProfile=cmsOpenProfileFromFile("/home/houzh/sRGB Color Space Profile.icm","r");
        if(mCMSProfile){
            AtExit::registerCallback([](){
                cmsCloseProfile(mCMSProfile);
            });
        }
    }
#endif
}

ImageDecoder::~ImageDecoder(){
#if ENABLE(LCMS)
    if(mTransform)cmsDeleteTransform(mTransform);
#endif
}

int ImageDecoder::getWidth()const{
    return mImageWidth;
}

int ImageDecoder::getHeight()const{
    return mImageHeight;
}

int ImageDecoder::computeTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp){
    if((bmp==nullptr)||(bmp->get_width()==0)||(bmp->get_height()==0))
        return PixelFormat::TRANSPARENT;
    if((bmp->get_content()&&(Cairo::Content::CONTENT_ALPHA)==0))
        return PixelFormat::OPAQUE;

    if( (bmp->get_content()&CONTENT_COLOR) ==0){
        switch(bmp->get_format()){
        case Surface::Format::A1: return PixelFormat::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA;
        case Surface::Format::A8:
            for(int y=0;y<bmp->get_height();y++){
                uint8_t*alpha=bmp->get_data()+bmp->get_stride()*y;
                for(int x=0;x<bmp->get_width();x++,alpha++)
                    if(*alpha > 0 && *alpha < 255)
                        return PixelFormat::TRANSLUCENT;//CAIRO_IMAGE_HAS_ALPHA;
            }
            return PixelFormat::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA;
        default:return PixelFormat::TRANSLUCENT;
        }
    }

    if((bmp->get_format()==Surface::Format::RGB16_565)||(bmp->get_format()==Surface::Format::RGB24))
        return PixelFormat::OPAQUE;

    if(bmp->get_format()!=Surface::Format::ARGB32)
        return PixelFormat::TRANSLUCENT;

    for(int y = 0;y < bmp->get_height() ;y++){
        uint32_t*pixel = (uint32_t*)(bmp->get_data() + bmp->get_stride()*y);
        for (int x = 0; x < bmp->get_width(); x++, pixel++){
            int a = (*pixel & 0xff000000) >> 24;
            if (a > 0 && a < 255)return PixelFormat::TRANSLUCENT;//CAIRO_IMAGE_HAS_ALPHA;
            else if(a==0)return PixelFormat::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA
        }
    }

    return  PixelFormat::OPAQUE;
}

#define TRANSPARENCY "TRANSPARENCY"

int ImageDecoder::getTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp){
    unsigned long len;
    const unsigned char*data= bmp->get_mime_data((const char*)TRANSPARENCY,len);
    const int transparency  = int((unsigned long)data);
    return transparency?transparency:int(PixelFormat::OPAQUE);
}

void ImageDecoder::setTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp,int transparency){
    bmp->set_mime_data((const char*)TRANSPARENCY,(unsigned char*)(long(transparency)),0,nullptr);
}

static bool matchesGIFSignature(char* contents){
    return !memcmp(contents, "GIF87a", 6) || !memcmp(contents, "GIF89a", 6);
}

static bool matchesPNGSignature(char* contents){
    return !memcmp(contents, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8);
}

static bool matchesJPEGSignature(char* contents){
    return !memcmp(contents, "\xFF\xD8\xFF", 3);
}

#if USE(OPENJPEG)
static bool matchesJP2Signature(char* contents){
    return !memcmp(contents, "\x00\x00\x00\x0C\x6A\x50\x20\x20\x0D\x0A\x87\x0A", 12)
        || !memcmp(contents, "\x0D\x0A\x87\x0A", 4);
}

static bool matchesJ2KSignature(char* contents){
    return !memcmp(contents, "\xFF\x4F\xFF\x51", 4);
}
#endif

#if ENABLE(WEBP)
static bool matchesWebPSignature(char* contents){
    return !memcmp(contents, "RIFF", 4) && !memcmp(contents + 8, "WEBPVP", 6);
}
#endif

static bool matchesBMPSignature(char* contents){
    return !memcmp(contents, "BM", 2);
}

static bool matchesICOSignature(char* contents){
    return !memcmp(contents, "\x00\x00\x01\x00", 4);
}

static bool matchesCURSignature(char* contents){
    return !memcmp(contents, "\x00\x00\x02\x00", 4);
}

static bool isApng(std::istream*istm,const std::string&name){
    char buf[64];
    //Chuk IHDR is the pngs's 1st Chunk
    //Chunk acTL is the 1st Chunk after IHDR
    istm->read(buf,48);
    const uint32_t frames = png_get_uint_32(buf+41);
    LOGV("chunk:%c%c%c%c isapng=%d frames:%d",buf[37],buf[38],buf[39],buf[40],!memcmp(buf+37,"acTL",4),frames);
    return (memcmp(buf+37,"acTL",4)==0) && (frames>1);
}

std::unique_ptr<ImageDecoder>ImageDecoder::create(Context*ctx,const std::string&resourceId){
    constexpr unsigned lengthOfLongestSignature = 14; /* To wit: "RIFF????WEBPVP"*/
    char contents[lengthOfLongestSignature];
    std::unique_ptr<ImageDecoder>decoder;
    std::unique_ptr<std::istream>istm;
    if(ctx)
        istm = ctx->getInputStream(resourceId);
    else
        istm = std::move(std::make_unique<std::ifstream>(resourceId));
    istm->read(contents,lengthOfLongestSignature);
    const unsigned length = istm->gcount();
    if (length < lengthOfLongestSignature)
        return nullptr;
    istm->seekg(0,std::ios::beg);
#if ENABLE(GIF)
    if (matchesGIFSignature(contents))
        decoder = std::make_unique<GIFDecoder>(ctx,resourceId);
#endif
    if (matchesPNGSignature(contents)&&(isApng(istm.get(),resourceId) == false))
        decoder = std::make_unique<PNGDecoder>(ctx,resourceId);
#if USE(ICO)
    if (matchesICOSignature(contents) || matchesCURSignature(contents))
        return ICOImageDecoder::create(alphaOption, gammaAndColorProfileOption);
#endif
#if ENABLE(JPEG)
    if (matchesJPEGSignature(contents))
        decoder = std::make_unique<JPEGDecoder>(ctx,resourceId);
#endif
#if USE(OPENJPEG)
    if (matchesJP2Signature(contents))
        return JPEG2000ImageDecoder::create(JPEG2000ImageDecoder::Format::JP2, alphaOption, gammaAndColorProfileOption);

    if (matchesJ2KSignature(contents))
        return JPEG2000ImageDecoder::create(JPEG2000ImageDecoder::Format::J2K, alphaOption, gammaAndColorProfileOption);
#endif

#if USE(BITMAP)
    if (matchesBMPSignature(contents))
        return BMPImageDecoder::create(alphaOption, gammaAndColorProfileOption);
#endif
    return decoder;
}

Drawable*ImageDecoder::createAsDrawable(Context*ctx,const std::string&resourceId){
    const std::unique_ptr<ImageDecoder>decoder = create(ctx,resourceId);
    if(decoder){
        Cairo::RefPtr<Cairo::ImageSurface>image = decoder->decode();
        if(TextUtils::endWith(resourceId,".9.png"))
            return new NinePatchDrawable(image);
        else if(TextUtils::endWith(resourceId,".png")||TextUtils::endWith(resourceId,".jpg"))
            return new BitmapDrawable(image);
    }
    if(TextUtils::endWith(resourceId,".gif")||TextUtils::endWith(resourceId,".webp")
            ||TextUtils::endWith(resourceId,".apng")||TextUtils::endWith(resourceId,".png"))
	    return new AnimatedImageDrawable(ctx,resourceId);
    return nullptr;
}

}/*endof namespace*/
