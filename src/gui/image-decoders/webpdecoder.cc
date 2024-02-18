#include <imagedecoder.h>
#include <gui/gui_features.h>
#include <cdlog.h>
#if ENABLE(WEBP)
#include <webp/demux.h>
namespace cdroid{
WebpDecoder::WebpDecoder(){
   LOGD("===WEBP");
}

WebpDecoder::~WebpDecoder(){
}

int WebpDecoder::load(std::istream&stream){
    WebPDecoderConfig config;
    WebPInitDecoderConfig(&config);
    WebPData inputData;// = { reinterpret_cast<const uint8_t*>(protectedData->data()), protectedData->size() };
    WebPDemuxState demuxerState;
    LOGD("WEBP::loading...");
    while(!stream.eof()){
        char buffer[256];
        stream.read(buffer,sizeof(buffer));
        WebPDecode((uint8_t*)buffer,stream.gcount(),&config);
        inputData.bytes=(uint8_t*)buffer;
        inputData.size=stream.gcount();
        WebPDemuxer* demuxer = WebPDemuxPartial(&inputData,&demuxerState);
        LOGD("demuxer=%p state=%d",demuxer,demuxerState);// != WEBP_DEMUX_DONE);
    }
    return 0;
}

int WebpDecoder::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex){
    return 0;
}
}/*endof namespace*/
#endif
