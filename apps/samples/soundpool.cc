#include <media/soundpool.h>
#include <unistd.h>
int main(int argc,char*argv[]){
    cdroid::SoundPool sp;
    for(int i=0;i<argc;i++){
        int sid =sp.load(argv[i]);
        sp.play(sid);
    }
    while(1)sleep(1);
    return 0;
}
