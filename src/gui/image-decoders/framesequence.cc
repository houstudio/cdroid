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
#include <gui_features.h>
#include <image-decoders/framesequence.h>
#include <image-decoders/gifframesequence.h>
#include <image-decoders/pngframesequence.h>
#include <image-decoders/webpframesequence.h>
#include <core/iostreams.h>   // AssetInputStream / MemoryInputStream
#include <content/asset.h>
#include <content/resources.h>
#include <core/context.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <png.h>
#include <cdlog.h>
namespace cdroid{

uint32_t FrameSequence::mHeaderBytesRequired = 0;

std::map<const std::string,FrameSequence::Registry> FrameSequence::mFactories;

FrameSequence::Registry::Registry(uint32_t msize,Factory& fun,Verifier& v)
  :magicSize(msize),factory(fun),verifier(v){

}

FrameSequence::FrameSequence(std::istream&stream):mStream(stream){
}

int FrameSequence::registerFactory(const std::string&mime,uint32_t magicSize,Verifier v,Factory factory){
    auto it = mFactories.find(mime);
    if(it==mFactories.end()){
        mFactories.insert({mime,Registry(magicSize,factory,v)});
        mHeaderBytesRequired = std::max(magicSize,mHeaderBytesRequired);
        LOGD("Register FrameSequence factory[%d] %s", mFactories.size()-1,mime.c_str());
        return 0;
    }else{
        it->second.factory = factory;
    }
    return 0;
}

size_t FrameSequence::registerAllFrameSequences(std::map<const std::string,Registry>&entis){
#ifdef PNG_APNG_SUPPORTED
    // Sniff window large enough for the isPNG verifier to find the acTL chunk
    // (signature 8 + IHDR 25 puts the acTL type at ~37; aapt2 writes it right
    // behind IHDR) — PNG_HEADER_SIZE (8) only proved the plain png signature.
    FrameSequence::registerFactory(std::string("mime/apng"),
            256,
            PngFrameSequence::isPNG,
            [](std::istream&stream){
                return new PngFrameSequence(stream);
            });
#endif
#if ENABLE(WEBP)
    FrameSequence::registerFactory(std::string("mime/webp"),
            WebPFrameSequence::RIFF_HEADER_SIZE,
            WebPFrameSequence::isWEBP,
            [](std::istream&stream){
                return new WebPFrameSequence(stream);
            });
#endif

#if ENABLE(GIF)
    FrameSequence::registerFactory(std::string("mime/gif"),
            GifFrameSequence::GIF_HEADER_SIZE,
            GifFrameSequence::isGIF,
            [](std::istream&stream){
                return new GifFrameSequence(stream);
            });
#endif
    return entis.size();
}

FrameSequence* FrameSequence::create(const void* data, size_t size) {
    if((mHeaderBytesRequired==0)||(mFactories.size()==0)){
        registerAllFrameSequences(mFactories);
    }
    if((data==nullptr)||(size<mHeaderBytesRequired)) return nullptr;
    // Zero-copy view over the caller's buffer (the AOSP ImageDecoder
    // createSource(ByteBuffer) shape): the stream lives only inside this call.
    auto stream = std::make_unique<MemoryInputStream>((const char*)data, size);
    for(auto& f:mFactories){
        auto& dec = f.second;
        if(dec.verifier((const uint8_t*)data,mHeaderBytesRequired)){
           FrameSequence* fs = dec.factory(*stream);
           if(fs) fs->mOwnedStream = std::move(stream);
           return fs;
        }
    }
    return nullptr;
}

bool FrameSequence::isAnimated(const void* data, size_t size) {
    if((mHeaderBytesRequired==0)||(mFactories.size()==0)){
        registerAllFrameSequences(mFactories);
    }
    if((data==nullptr)||(size<mHeaderBytesRequired)) return false;
    const uint8_t* header = (const uint8_t*)data;
    for(auto& f:mFactories){
        if(f.second.verifier(header,mHeaderBytesRequired)) return true;
    }
    return false;
}

FrameSequence* FrameSequence::create(cdroid::Context*ctx,int resid) {
    if(ctx==nullptr) return nullptr;
    // Zero-copy resource open: stored pak entries hand getBuffer() a view
    // straight into the archive; the backends slurp it inside create(data),
    // so the Asset (RAII) may close as soon as this returns.
    std::unique_ptr<Asset> asset(ctx->getResources().openRawResource(resid));
    if(asset==nullptr) return nullptr;
    const void* data = asset->getBuffer(false);
    const size_t size = (size_t)asset->getLength();
    return (data&&size) ? create(data,size) : nullptr;
}

FrameSequence* FrameSequence::create(const std::string& path) {
    const int fd = ::open(path.c_str(), O_RDONLY);
    if(fd<0) return nullptr;
    std::unique_ptr<Asset> asset(Asset::createFromFd(fd,path.c_str(),Asset::ACCESS_BUFFER));
    if(asset==nullptr) return nullptr;
    const void* data = asset->getBuffer(false);
    const size_t size = (size_t)asset->getLength();
    return (data&&size) ? create(data,size) : nullptr;
}

}/*endof namespace*/

