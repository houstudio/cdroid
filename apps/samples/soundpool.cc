#include <media/soundpool.h>

int main(int argc,char*argv[]){
    cdroid::SoundPool sp;
    if(argc>1){
        int sid =sp.load(argv[1]);
        sp.play(sid);
    }
    return 0;
}
