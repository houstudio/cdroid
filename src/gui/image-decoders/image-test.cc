#include <imagedecoder.h>
#include <fstream>
#include <cdtypes.h>
#include <cdlog.h>
#include <framesequence.h>
#include <gui/cdroid.h>
#include <core/app.h>
#include <core/textutils.h>
int main(int argc,const char*argv[]){
    cdroid::App app(argc,argv);
#if 1
    const char*fname =(argc>1)?argv[1]:"/home/houzh/pf.jpg";
    std::ifstream fstrm (fname);
    cdroid::ImageDecoder*dec=nullptr;
    if(TextUtils::endWith(fname,"png")) dec=new cdroid::PNGDecoder(fstrm);
    else dec=new cdroid::JPEGDecoder(fstrm);
    Cairo::RefPtr<Cairo::ImageSurface>image=dec->decode(1.234);
    LOGD("imageinfo:%dx%d",dec->getWidth(),dec->getHeight());
    image->write_to_png("111.png");
    //cdroid::FrameSequence*seq=cdroid::FrameSequence::create(&fin);
#else
    Window*w=new Window(0,0,-1,-1);
    AnimatedImageDrawable*ad=new AnimatedImageDrawable(&app,app.getParam(0,"./Honeycam1.gif"));
    cdroid::TextView*tv=new cdroid::TextView("Hello World!",100,100);
    w->addView(tv);
    tv->setBackground(ad);
    ad->start();
#endif
    return app.exec();
}
