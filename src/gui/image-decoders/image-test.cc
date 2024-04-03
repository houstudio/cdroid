#include <imagedecoder.h>
#include <fstream>
#include <cdtypes.h>
#include <cdlog.h>
#include <framesequence.h>
int main(int argc,char*argv[]){
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
    std::ifstream fin(argv[1]);
    cdroid::FrameSequence*seq=cdroid::FrameSequence::create(&fin);
    return 0;
}
