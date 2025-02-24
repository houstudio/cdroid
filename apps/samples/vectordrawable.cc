#include <drawables/vectordrawable.h>

int main(int argc,char*argv[]){
    if(argc>1)
        cdroid::VectorDrawable::create(nullptr,argv[1]);
    return 0;
}
