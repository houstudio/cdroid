#include <cdroid.h>
#include <dirent.h>
using namespace Cairo;
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,600);
    HorizontalScrollView* hs=new HorizontalScrollView(1280,400);

    LinearLayout*layout=new LinearLayout(1280,100);
    layout->setOrientation(LinearLayout::HORIZONTAL);

    std::string path="/home/houzh/images";
    if(argc>1)path=argv[1];
    DIR*dir=opendir(path.c_str());
    struct dirent*ent;
    int count=0;
    while(dir&&(ent=readdir(dir))){
        std::string fullpath=path+"/"+ent->d_name;
        if(ent->d_type!=DT_REG)continue;
        RefPtr<Cairo::ImageSurface>img=app.getImage(fullpath);
        if(img==nullptr)continue;
        ImageView*iv=new ImageView(150,30);
        iv->setImageBitmap(img);
        iv->setId(++count);
	LOGI("img:%s",fullpath.c_str());
        layout->addView(iv,new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT));
    }
    hs->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    hs->addView(layout,new LinearLayout::LayoutParams(-1,-1));
    //w->addView(layout);
    w->addView(hs);
    layout->requestLayout();
    if(dir)closedir(dir);
    w->requestLayout();
    app.exec();
}
