#include <media/soundpool.h>
#include <media/audiomanager.h>
#include <core/environment.h>
#include <gui_features.h>
#include <cdroid.h>
#include <unistd.h>
int main(int argc,const char*argv[]){
#if ENABLE(AUDIO)
    if(argc<2){
        App app(argc,argv);
        auto& am=AudioManager::getInstance();
        am.playSoundEffect(AudioManager::FX_KEY_CLICK,1.f);
        return app.exec();
    }
    cdroid::SoundPool sp(8,0,0);
    for(int i=1;i<argc;i++){
        int sid =sp.load(argv[i],0);
        int streamid=sp.play(sid,2.f,2.f,0,10,1.f);
        sp.setRate(streamid,0.5f);
    }
#endif
    while(1)sleep(1);
    return 0;
}
