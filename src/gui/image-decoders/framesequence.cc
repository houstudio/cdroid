/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gui_features.h>
#include <image-decoders/framesequence.h>
#include <image-decoders/gifframesequence.h>
#include <image-decoders/pngframesequence.h>
#include <image-decoders/webpframesequence.h>
#include <fstream>
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
        LOGD("Register FrameSequence factory[%d] %s", mFactories.size(),mime.c_str());
        return 0;
    }else{
        it->second.factory = factory;
    }
    return 0;
}

static int registerAllFrameSequences(std::map<const std::string,FrameSequence::Registry>&entis){
    FrameSequence::registerFactory(std::string("mime/apng"),
            PngFrameSequence::PNG_HEADER_SIZE,
            PngFrameSequence::isPNG,
            [](std::istream&stream){
                return new PngFrameSequence(stream);
            });
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
    uint8_t header[32];
    std::unique_ptr<std::istream>stream;
    if((mHeaderBytesRequired==0)||(mFactories.size()==0)){
        registerAllFrameSequences(mFactories);
    }
    if(ctx)
        stream = ctx->getInputStream(resid);
    else
        stream = std::make_unique<std::ifstream>(resid);
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

