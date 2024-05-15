#include <imagedecoder.h>
#include <fstream>
#include <cdtypes.h>
#include <cdlog.h>
#include <framesequence.h>
#include <gui/cdroid.h>
#include <core/app.h>
int main(int argc,const char*argv[]){
    cdroid::App app(argc,argv);
#if 0
    std::unique_ptr<std::ifstream> fstrm = std::make_unique<std::ifstream>(argv[1]);
    std::unique_ptr<std::istream> istm = std::move(fstrm);
    cdroid::GIFDecoder*dec = new cdroid::GIFDecoder(std::move(istm));
    dec->load();
    const int frmCount = dec->getFrameCount();
    LOGD("imageinfo:%dx%dx%d",dec->getWidth(),dec->getHeight(),frmCount);
    Cairo::RefPtr<Cairo::ImageSurface>image
	   =Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,dec->getWidth(),dec->getHeight());
    for(int loop=0;loop<10;loop++){
        for(int i=0;i<frmCount;i++)
	    dec->readImage(image,i);
	LOGD("loop %d",loop);
    }
    
    cdroid::FrameSequence*seq=cdroid::FrameSequence::create(&fin);
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
