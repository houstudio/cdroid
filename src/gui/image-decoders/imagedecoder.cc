#include <memory>
#include <cstring>
#include <fstream>
#include <drawables/drawable.h>
#include <drawables/bitmapdrawable.h>
#include <drawables/ninepatchdrawable.h>
#include <drawables/animatedimagedrawable.h>
#include <image-decoders/imagedecoder.h>
#include <core/textutils.h>
#include <gui/gui_features.h>

namespace cdroid{

ImageDecoder::ImageDecoder(std::unique_ptr<std::istream>stm){
    mImageWidth = -1;
    mImageHeight= -1;
    mFrameCount = 0;
    mScale = 1.f;
    mPrivate = nullptr;
    istream = std::move(stm);
}

ImageDecoder::~ImageDecoder(){
}

int ImageDecoder::getWidth()const{
    return mImageWidth;
}

int ImageDecoder::getHeight()const{
    return mImageHeight;
}

int ImageDecoder::getFrameCount()const{
    return mFrameCount;
}

float ImageDecoder::getScale()const{
    return mScale;
}

void ImageDecoder::setScale(float s){
    mScale =s;
}

int ImageDecoder::getFrameDuration(int idx)const{
    int duration = 0;//( (idx>=0) && (idx<mFrameCount) ) ? mFrames[idx]->duration:INT_MAX;
    // Many annoying ads specify a 0 duration to make an image flash as quickly as possible.
    // We follow Firefox's behavior and use a duration of 100 ms for any frames that specify
    // a duration of <= 10 ms. See <rdar://problem/7689300> and <http://webkit.org/b/36082>
    // for more information.
    if(duration<=10)duration = 100;
    return duration;
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
    unsigned length = istm->gcount();
    if (length < lengthOfLongestSignature)
        return nullptr;
    istm->seekg(0,std::ios::beg);
#if ENABLE(GIF)
    if (matchesGIFSignature(contents))
        decoder = new GIFDecoder(std::move(istm));
#endif
    if (matchesPNGSignature(contents))
        decoder = new APNGDecoder(std::move(istm));
#if USE(ICO)
    if (matchesICOSignature(contents) || matchesCURSignature(contents))
        return ICOImageDecoder::create(alphaOption, gammaAndColorProfileOption);
#endif
#if ENABLE(JPEG)
    if (matchesJPEGSignature(contents))
        decoder = new JPEGDecoder(std::move(istm));
#endif
#if USE(OPENJPEG)
    if (matchesJP2Signature(contents))
        return JPEG2000ImageDecoder::create(JPEG2000ImageDecoder::Format::JP2, alphaOption, gammaAndColorProfileOption);

    if (matchesJ2KSignature(contents))
        return JPEG2000ImageDecoder::create(JPEG2000ImageDecoder::Format::J2K, alphaOption, gammaAndColorProfileOption);
#endif

#if ENABLE(WEBP)
    if (matchesWebPSignature(contents))
        return new WebpDecoder(std::move(istm));
#endif
#if USE(BITMAP)
    if (matchesBMPSignature(contents))
        return BMPImageDecoder::create(alphaOption, gammaAndColorProfileOption);
#endif
    return decoder;
}

Drawable*ImageDecoder::createAsDrawable(Context*ctx,const std::string&resourceId){
    int frameIndex = 0;
    ImageDecoder*decoder = create(ctx,resourceId);
    if(decoder->getFrameCount()>1){
        return new AnimatedImageDrawable(ctx,resourceId);    	    
    }
    Cairo::RefPtr<Cairo::ImageSurface>image=Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,decoder->getWidth(),decoder->getHeight());
    decoder->readImage(image,frameIndex);
    if(TextUtils::endWith(resourceId,"9.png"))
	return new NinePatchDrawable(image);
    return new BitmapDrawable(image);
}

}/*endof namespace*/
