/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <memory>
#include <cstring>
#include <fstream>
#include <gui_features.h>
#include <drawable/drawable.h>
#include <drawable/bitmapdrawable.h>
#include <drawable/ninepatchdrawable.h>
#include <drawable/animatedimagedrawable.h>
#include <image-decoders/framesequence.h>
#include <image-decoders/imagedecoder.h>
#include <content/typedvalue.h>  // TypedValue (id-based decodeDrawable path)
#include <content/asset.h>            // Asset (openRawResource / openAsset face)
#include <core/iostreams.h>           // AssetInputStream
#include <text/textutils.h>
#include <core/context.h>
#include <png.h>
#include <porting/cdlog.h>
#if ENABLE(LCMS)
#include <lcms2.h>
#endif

namespace cdroid{
using namespace Cairo;

static std::unique_ptr<void,std::function<void(void*)>>mLCMSProfile;

ImageDecoder::ImageDecoder(std::istream&stream):mStream(stream){
    mImageWidth = -1;
    mImageHeight= -1;
    mFrameCount = 1;
    mPrivate = nullptr;
#if ENABLE(LCMS)
    if(mLCMSProfile == nullptr){
        auto cmsprofile = cmsOpenProfileFromFile("/home/houzh/sRGB Color Space Profile.icm","r");
        mLCMSProfile = std::unique_ptr<void,std::function<void(void*p)>>(cmsprofile,
            [](void* lcms){ if(lcms)cmsCloseProfile(lcms); });
    }
#endif
}

ImageDecoder::~ImageDecoder(){
}

uint32_t ImageDecoder::mHeaderBytesRequired = 0;
std::unordered_map<std::string,ImageDecoder::Registry> ImageDecoder::mFactories;

ImageDecoder::Registry::Registry(uint32_t msize,Factory& fun,Verifier& v)
  :magicSize(msize),factory(fun),verifier(v){

}

int ImageDecoder::registerFactory(const std::string&mime,uint32_t magicSize,Verifier v,Factory factory){
    auto it = mFactories.find(mime);
    if(it == mFactories.end()){
        mFactories.insert({mime,Registry(magicSize,factory,v)});
        mHeaderBytesRequired = std::max(magicSize,mHeaderBytesRequired);
        LOGD("Register FrameSequence factory[%d] %s", mFactories.size()-1,mime.c_str());
        return 0;
    }else{
        it->second.factory = factory;
    }
    return 0;
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

int ImageDecoder::computeTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp){
    if( (bmp == nullptr) || (bmp->get_width() == 0 )|| (bmp->get_height() == 0) )
        return PixelFormat::TRANSPARENT;
    if((bmp->get_content()&&(Cairo::Content::CONTENT_ALPHA)==0))
        return PixelFormat::OPAQUE;

    if( (bmp->get_content()&CONTENT_COLOR) ==0){
        switch(bmp->get_format()){
        case Surface::Format::A1:
            return PixelFormat::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA;
        case Surface::Format::A8:
            for(int y = 0;y < bmp->get_height(); ++y){
                uint8_t*alpha = bmp->get_data() + bmp->get_stride()*y;
                for(int x = 0;x < bmp->get_width(); ++x, ++alpha)
                    if(*alpha > 0 && *alpha < 255)
                        return PixelFormat::TRANSLUCENT;//CAIRO_IMAGE_HAS_ALPHA;
            }
            return PixelFormat::TRANSPARENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA;
        default:
            return PixelFormat::TRANSLUCENT;
        }
    }

    if((bmp->get_format()==Surface::Format::RGB16_565)||(bmp->get_format()==Surface::Format::RGB24))
        return PixelFormat::OPAQUE;

    if(bmp->get_format()!=Surface::Format::ARGB32)
        return PixelFormat::TRANSLUCENT;

    int transparentCount = 0, opaqueCount = 0;
    for(int y = 0;y < bmp->get_height() ;y++){
        uint8_t*pixels = (bmp->get_data() + bmp->get_stride()*y);
        for (int x = 0; x < bmp->get_width(); ++x, pixels+=4){
            const uint8_t a = pixels[3];
            if(a==0) transparentCount++;
            else if(a!=255) return PixelFormat::TRANSLUCENT;//CAIRO_IMAGE_HAS_BILEVEL_ALPHA
            else opaqueCount++;//return PixelFormat::TRANSLUCENT;//CAIRO_IMAGE_HAS_ALPHA;

            if(transparentCount&&opaqueCount) return PixelFormat::TRANSLUCENT;
        }
    }
    return PixelFormat::OPAQUE;
}

#define TRANSPARENCY "TRANSPARENCY"

int ImageDecoder::getTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp){
    if(bmp){
        unsigned long len;
        const unsigned char*data= bmp->get_mime_data((const char*)TRANSPARENCY,len);
        const int transparency = int((unsigned long)data);
        return transparency?transparency:int(PixelFormat::OPAQUE);
    }
    return PixelFormat::TRANSPARENT;
}

void ImageDecoder::setTransparency(Cairo::RefPtr<Cairo::ImageSurface>bmp,int transparency){
    if(bmp)
        bmp->set_mime_data((const char*)TRANSPARENCY,(unsigned char*)(long(transparency)),0,nullptr);
}

static int registerBuildinCodesc(){
    ImageDecoder::registerFactory(std::string("mime/png"),8,PNGDecoder::isPNG,
            [](std::istream&stream){return std::make_unique<PNGDecoder>(stream);});
#if ENABLE(GIF)
    ImageDecoder::registerFactory("mime/gif",6,GIFDecoder::isGIF,
            [](std::istream&stream){return std::make_unique<GIFDecoder>(stream);});
#endif

#if ENABLE(JPEG)
    ImageDecoder::registerFactory("mime/jpeg",12,JPEGDecoder::isJPEG,
            [](std::istream&stream){return std::make_unique<JPEGDecoder>(stream);});
#endif

#if ENABLE(WEBP)
    ImageDecoder::registerFactory("mime/webp",14,WEBPDecoder::isWEBP,
            [](std::istream&stream){return std::make_unique<WEBPDecoder>(stream);});
#endif

#if ENABLE(OPENJPEG)
    ImageDecoder::registerFactory("mime/jp2",12,JPEG2000Decoder::isJP2,
            [](std::istream&stream){return std::make_unique<JPEG2000Decoder>(stream);});
    ImageDecoder::registerFactory("mime/j2k",4,JPEG2000Decoder::isJ2K,
            [](std::istream&stream){return std::make_unique<JPEG2000Decoder>(stream);});
#endif

#if USE(ICO)
    if (matchesICOSignature(contents) || matchesCURSignature(contents))
        return ICOImageDecoder::create(alphaOption, gammaAndColorProfileOption);
#endif

    /* register BMP decoder */
    ImageDecoder::registerFactory("mime/bmp",2,BMPDecoder::isBMP,
            [](std::istream&stream){return std::make_unique<BMPDecoder>(stream);});
    return 0;
}

std::unique_ptr<ImageDecoder>ImageDecoder::getDecoder(std::istream&istm){
    constexpr unsigned lengthOfLongestSignature = 14; /* To wit: "RIFF????WEBPVP"*/
    uint8_t contents[lengthOfLongestSignature];
    std::unique_ptr<ImageDecoder>decoder;
    
    istm.read((char*)contents,lengthOfLongestSignature);
    const std::streamsize length = istm.gcount();
    if (length < lengthOfLongestSignature)
        return nullptr;
    istm.seekg(0,std::ios::beg);
    if(mFactories.empty()){
        registerBuildinCodesc();
    }
    for(auto fac:mFactories){
        auto f = fac.second;
        if(f.verifier(contents,lengthOfLongestSignature)){
            float scale = 1.f;
            decoder = f.factory(istm);
            return decoder;
        }
    }
    return nullptr;
}

Cairo::RefPtr<Cairo::ImageSurface> ImageDecoder::loadImage(std::istream&istm,int width,int height,
                                                          std::vector<uint8_t>* ninePatchChunk){
    float scale = 1.f;
    // getDetector reads the magic then seeks back to 0, so the source stream
    // must seek reliably. Slurp into a seekable istringstream first — same fix
    // decodeDrawable uses.
    std::string data((std::istreambuf_iterator<char>(istm)), std::istreambuf_iterator<char>());
    std::istringstream seekable(std::move(data));
    std::unique_ptr<ImageDecoder>decoder = getDecoder(seekable);
    if(decoder == nullptr)
        return nullptr;
    if((width > 0) && (height > 0))
        scale = std::min(float(width)/decoder->getWidth(),float(height)/decoder->getHeight());
    else if(width > 0)
        scale = std::min(scale,float(width)/decoder->getWidth());
    else if(height > 0)
        scale = std::max(scale,float(height)/decoder->getHeight());
    Cairo::RefPtr<Cairo::ImageSurface> image = decoder->decode(scale,mLCMSProfile.get());
    // Read the 9-patch chunk AFTER decode — the user-chunk callback (npTc/cdNp)
    // fires during png_read_info inside decode, so it isn't available before.
    if(ninePatchChunk){
        const std::vector<uint8_t>* c = decoder->getNinePatchChunk();
        if(c) *ninePatchChunk = *c;
    }
    return image;
}

Cairo::RefPtr<Cairo::ImageSurface>ImageDecoder::loadImage(Context*ctx,const std::string&resourceId,int width,int height){
    std::unique_ptr<ImageDecoder>decoder;
    std::unique_ptr<std::istream>istm;
    if(ctx){
        Asset*asset = ctx->openAsset(resourceId);
        if(asset) istm = std::unique_ptr<std::istream>(new AssetInputStream(asset));
    }else istm = std::make_unique<std::ifstream>(resourceId);
    if((istm == nullptr)||(!*istm))
        return nullptr;
    return loadImage(*istm,width,height);
}

Drawable*ImageDecoder::decodeDrawable(Context*ctx,const std::string&resourceId){
    std::unique_ptr<std::istream> istm;
    if(ctx){
        Asset*asset = ctx->openAsset(resourceId);
        if(asset) istm = std::unique_ptr<std::istream>(new AssetInputStream(asset));
    }else istm = std::make_unique<std::ifstream>(resourceId);
    if((istm==nullptr)||(!*istm)) return nullptr;
    return decodeDrawableStream(ctx, std::move(istm), resourceId);
}

// Shared decode core for decodeDrawable(string) and decodeDrawable(int).
// `path` is the file path (for 9-patch/.gif/.webp/.png checks + AnimatedImage).
Drawable* ImageDecoder::decodeDrawableStream(Context* ctx,
        std::unique_ptr<std::istream> istm, const std::string& path) {
    if ((istm == nullptr) || (!*istm)) return nullptr;
    // Slurp into a seekable in-memory buffer. getDetector reads the magic then
    // seeks back to 0 (libpng re-reads the signature), so the decode needs a
    // reliably seekable stream; an istringstream seeks regardless of the
    // source stream's seek behavior.
    auto seekable = std::make_unique<std::istringstream>(
        std::string((std::istreambuf_iterator<char>(*istm)), std::istreambuf_iterator<char>()));
    std::unique_ptr<ImageDecoder> decoder = getDecoder(*seekable);
    Cairo::RefPtr<Cairo::ImageSurface> image = decoder?decoder->decode(1.0):nullptr;

    if(image && decoder && (decoder->getFrameCount()==1)){
        Drawable*d = nullptr;
        // A 9-patch is detected by the cdNp chunk (aapt-stripped assets are renamed to
        // .png, so the filename no longer carries .9). Keep the .9.png fallback for any
        // bordered source loaded directly without an embedded chunk.
        const std::vector<uint8_t>* npChunk = decoder ? decoder->getNinePatchChunk() : nullptr;
        if(npChunk != nullptr || TextUtils::endWith(path,".9.png"))
            d = new NinePatchDrawable(image, npChunk);
        else if( (image->get_width() >0) && (image->get_height() > 0) ){
            d = new BitmapDrawable(image);
        }
        if(d != nullptr) {
#if !defined(NDEBUG)
            d->getConstantState()->mResource=path;
#endif
            return d;
        }
    }

    if( ((istm!=nullptr)&&(*istm)) && (TextUtils::endWith(path,".gif")||TextUtils::endWith(path,".webp")
            ||TextUtils::endWith(path,".apng")||TextUtils::endWith(path,".png"))){
        // AOSP ImageDecoder.createSource(File) vs (Resources,resId): a Context
        // means the path is a pak asset, otherwise it is a plain file.
        Drawable* d = ctx ? (Drawable*)new AnimatedImageDrawable(ctx->openAsset(path))
                          : (Drawable*)new AnimatedImageDrawable(path);
        LOGD_IF(d==nullptr,"%s load failed!",path.c_str());
        if(d != nullptr){
            d->getConstantState()->mResource=path;
            return d;
        }
    }
    return nullptr;
}

Drawable* ImageDecoder::decodeDrawable(Resources& res, int id) {
    // AOSP ImageDecoder.createSource(Resources, resId): resolve the file path
    // (for 9-patch / animated-extension checks) + open the asset by id, all
    // through the Resources face.
    TypedValue tv;
    if (!res.getValue(id, &tv, true) || tv.type != TypedValue::TYPE_STRING) return nullptr;
    std::string path = TextUtils::utf16_utf8(reinterpret_cast<const uint16_t*>(tv.string), tv.stringLen);
    std::unique_ptr<Asset> asset(res.openRawResource(id));
    if (asset == nullptr) return nullptr;
    const off64_t sz = asset->getLength();
    if (sz <= 0) return nullptr;
    // Animated formats take the zero-copy fast path: one asset open, the
    // buffer handed straight to AnimatedImageDrawable (no slurp, no reopen).
    if (FrameSequence::isAnimated(asset->getBuffer(false), (size_t)sz)) {
        Drawable* d = new AnimatedImageDrawable(asset.release());
        if (d) {
            d->getConstantState()->mResource = path;
            return d;
        }
        return nullptr;
    }
    // Static formats decode through the same zero-copy view (the Asset outlives
    // the synchronous decode below) — the old slurp-into-string copy is gone.
    Drawable* d = decodeDrawableStream(res.getContext(),
            std::unique_ptr<std::istream>(new MemoryInputStream(
                    (const char*)asset->getBuffer(false), (size_t)sz)), path);
    // Decode-time density fixup for 9-patches (AOSP BitmapFactory.decodeResourceStream):
    // source density comes from the TypedValue, target from the display metrics.
    if (auto* npd = dynamic_cast<NinePatchDrawable*>(d)) {
        npd->setSourceDensity(tv.density);
        npd->setTargetDensity(res.getDisplayMetrics().densityDpi);
    } else if (auto* bd = dynamic_cast<BitmapDrawable*>(d)) {
        // BitmapDrawable.updateStateFromTypedArray's density normalize
        // (android-36 :843-848): DENSITY_DEFAULT -> DisplayMetrics default,
        // DENSITY_NONE stays none (raw sizes), anything else is the bucket.
        int density = 0;   // Bitmap.DENSITY_NONE stand-in
        if (tv.density == TypedValue::DENSITY_DEFAULT) {
            density = DisplayMetrics::DENSITY_DEFAULT;
        } else if (tv.density != TypedValue::DENSITY_NONE) {
            density = tv.density;
        }
        bd->setSourceDensity(density);
        bd->setTargetDensity(res.getDisplayMetrics().densityDpi);
    }
    return d;
}

}/*endof namespace*/
