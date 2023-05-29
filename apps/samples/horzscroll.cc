#include <cdroid.h>
#include <dirent.h>
#include <fstream>
using namespace Cairo;
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    HorizontalScrollView* hs=new HorizontalScrollView(1280,400);

    LinearLayout*layout=new LinearLayout(1280,100);
    layout->setOrientation(LinearLayout::HORIZONTAL);

    std::string path="/home/houzh/images";
    if(argc>1)path=argv[1];
    DIR*dir=opendir(path.c_str());
    struct dirent*ent;
    int count=0;
    hs->setBackgroundColor(0xFFFF0000);
    layout->setBackgroundColor(0xFF00FF00);
    printf("%s open=%p\n",path.c_str(),dir);
    while(dir&&(ent=readdir(dir))){
        std::string fullpath=path+"/"+ent->d_name;
	//LOGI("img:%s ",fullpath.c_str());
        if(ent->d_type==DT_DIR)continue;
	std::ifstream fs(fullpath.c_str(),std::ios::binary|std::ios::in);
	if(strstr(fullpath.c_str(),".png")==nullptr)continue;
        RefPtr<Cairo::ImageSurface>img=ImageSurface::create_from_stream(fs);
	//LOGI("img:%s =%p",fullpath.c_str(),img.get());
        if(img==nullptr)continue;
        ImageView*iv=new ImageView(150,30);
        iv->setImageBitmap(img);
        iv->setId(++count);
        layout->addView(iv,new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT));
    }
    hs->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    hs->setHorizontalScrollBarEnabled(true);
    hs->setVerticalScrollBarEnabled(true);
    hs->addView(layout,new LinearLayout::LayoutParams(LayoutParams::WRAP_CONTENT,LayoutParams::MATCH_PARENT));
    //w->addView(layout);
    w->addView(hs);
    layout->requestLayout();
    if(dir)closedir(dir);
    w->requestLayout();
    app.exec();
}
