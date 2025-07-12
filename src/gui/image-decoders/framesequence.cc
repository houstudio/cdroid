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
    FrameSequence::registerFactory(std::string("mime/apng"),
            PngFrameSequence::PNG_HEADER_SIZE,
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

FrameSequence* FrameSequence::create(cdroid::Context*ctx,const std::string&resid) {
    uint8_t header[32]={0};
    std::unique_ptr<std::istream>stream;
    if((mHeaderBytesRequired==0)||(mFactories.size()==0)){
        registerAllFrameSequences(mFactories);
    }
    if(ctx)
        stream = ctx->getInputStream(resid);
    else
        stream = std::make_unique<std::ifstream>(resid);
    if((stream==nullptr)||(!*stream))
        return nullptr;
    stream->read((char*)header,mHeaderBytesRequired);
    stream->seekg(int(-mHeaderBytesRequired),std::ios::cur);
    for(auto& f:mFactories){
        auto& dec = f.second;
        if(dec.verifier(header,mHeaderBytesRequired))
           return dec.factory(*stream);
    }
    return nullptr;
}

}/*endof namespace*/

