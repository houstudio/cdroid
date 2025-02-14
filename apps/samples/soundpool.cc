#include <media/soundpool.h>
#include <unistd.h>
int main(int argc,char*argv[]){
    cdroid::SoundPool sp;
    if(argc>1){
        int sid =sp.load(argv[1]);
        sp.play(sid);
    }
    while(1)sleep(1);
    return 0;
}
