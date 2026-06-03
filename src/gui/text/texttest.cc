#include <text/textpaint.h>
#include <core/app.h>
int main(int argc,const char*argv[]){
    cdroid::App app(argc,argv);
    cdroid::Paint pt;
    pt.setTextSize(12);
    std::vector<float> advances(32);
    float advance1=pt.measureText((const char32_t*)L"Hello World!Haha",0,16);
    std::cout<<"advance="<<advance1<<std::endl;
    //float advance=pt.getTextRunAdvances((const char32_t*)L"Hello World",0,11,0,0,false,advances.data(),0);
    //std::cout<<"advance="<<advance<<std::endl;
    app.exec();
}
