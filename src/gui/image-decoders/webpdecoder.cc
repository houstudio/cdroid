#include <imagedecoder.h>
#include <gui/gui_features.h>

#if ENABLE(WEBP)
#include <webp/demux.h>
namespace cdroid{
WebpDecoder::WebpDecoder(){
}

WebpDecoder::~WebpDecoder(){
}

int WebpDecoder::load(std::istream&stream){
    WebPData inputData;// = { reinterpret_cast<const uint8_t*>(protectedData->data()), protectedData->size() };
    WebPDemuxState demuxerState;
    WebPDemuxer* demuxer = WebPDemuxPartial(&inputData, &demuxerState);
    return 0;
}

int WebpDecoder::readImage(Cairo::RefPtr<Cairo::ImageSurface>image,int& frameIndex){
    return 0;
}
}/*endof namespace*/
#endif
