#include <memory>
#include <cstring>
#include <fstream>
#include <drawables/drawable.h>
#include <drawables/bitmapdrawable.h>
#include <drawables/ninepatchdrawable.h>
#include <drawables/animatedimagedrawable.h>
#include <image-decoders/imagedecoder.h>
#include <core/textutils.h>
#include <core/context.h>
#include <lcms2.h>
namespace cdroid{

ImageDecoder::ImageDecoder(Context*ctx,const std::string&resourceId){
    mImageWidth = -1;
    mImageHeight= -1;
    mPrivate = nullptr;
    mTransform= nullptr;
    if(ctx)
        istream = ctx->getInputStream(resourceId);
    else
        istream = std::move(std::make_unique<std::ifstream>(resourceId));
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

ImageDecoder*ImageDecoder::create(Context*ctx,const std::string&resourceId){
    constexpr unsigned lengthOfLongestSignature = 14; /* To wit: "RIFF????WEBPVP"*/
    char contents[lengthOfLongestSignature];
    ImageDecoder*decoder = nullptr;
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
#if ENABLE(GIF)&&0
    if (matchesGIFSignature(contents))
        decoder = new GIFDecoder(ctx,resourceId);
#endif
    if (matchesPNGSignature(contents))
        decoder = new PNGDecoder(ctx,resourceId);
#if USE(ICO)
    if (matchesICOSignature(contents) || matchesCURSignature(contents))
        return ICOImageDecoder::create(alphaOption, gammaAndColorProfileOption);
#endif
#if ENABLE(JPEG)
    if (matchesJPEGSignature(contents))
        decoder = new JPEGDecoder(ctx,resourceId);
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
    ImageDecoder*decoder = create(ctx,resourceId);
    if(decoder){
        static int png,png9,jpg;
        Cairo::RefPtr<Cairo::ImageSurface>image = decoder->decode();
        delete decoder;
        if(TextUtils::endWith(resourceId,"9.png"))png9++;
        else if(TextUtils::endWith(resourceId,".png"))png++;
        else if(TextUtils::endWith(resourceId,".jpg"))jpg++;
        LOGD("%d png, %d 9png,%d jpg",png,png9,jpg);
        if(TextUtils::endWith(resourceId,"9.png"))
            return new NinePatchDrawable(image);
        else if(TextUtils::endWith(resourceId,".png")||TextUtils::endWith(resourceId,".jpg"))
            return new BitmapDrawable(image);
    }
    if(TextUtils::endWith(resourceId,".gif")||TextUtils::endWith(resourceId,".webp"))
	    return new AnimatedImageDrawable(ctx,resourceId);
    return nullptr;
}

}/*endof namespace*/
