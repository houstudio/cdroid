#include <imagedecoder.h>
#include <fstream>
#include <cdtypes.h>
#include <cdlog.h>
#include <framesequence.h>
#include <gui/cdroid.h>
#include <core/app.h>
#include <core/textutils.h>
#include <image-decoders/pngframesequence.h>
int main(int argc,const char*argv[]){
#if 0
    const char*fname =(argc>1)?argv[1]:"/home/houzh/pf.jpg";
    std::unique_ptr<cdroid::ImageDecoder>dec;
    dec=ImageDecoder::create(nullptr,fname);
    Cairo::RefPtr<Cairo::ImageSurface>image=dec->decode();
    LOGD("imageinfo:%dx%d",dec->getWidth(),dec->getHeight());
    image->write_to_png("111.png");
#else
    std::ifstream fin(argv[1]);
    cdroid::FrameSequence*seq=new cdroid::PngFrameSequence(&fin);
    LOGD("%dx%dx%d",seq->getWidth(),seq->getHeight(),seq->getFrameCount());
    Cairo::RefPtr<Cairo::ImageSurface>img=Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,seq->getWidth(),seq->getHeight());
    cdroid::FrameSequenceState*state=seq->createState();
    for(int i=0;i<seq->getFrameCount();i++){
        state->drawFrame(i,(uint32_t*)img->get_data(),img->get_stride()/4,0);
        img->write_to_png(std::to_string(i)+std::string(".png"));
    }
    //seq->createState()->drawFrame(0,(uint32_t*)img->get_data(),img->get_stride()/4,0);
    /*Window*w=new Window(0,0,-1,-1);
    AnimatedImageDrawable*ad=new AnimatedImageDrawable(&app,app.getParam(0,"./Honeycam1.gif"));
    cdroid::TextView*tv=new cdroid::TextView("Hello World!",100,100);
    w->addView(tv);
    tv->setBackground(ad);
    ad->start();*/
#endif
    return 0;
}
