#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
#include <curl/curl.h>
#include <curldownload.h>
class MyAdapter:public ArrayAdapter<std::string>{
public:
    MyAdapter():ArrayAdapter(){
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv = new TextView("",600,20);
            tv->setPadding(20,0,0,0);
        }
        tv->setId(position);
        tv->setText("position :"+std::to_string(position));
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0x80002222);
        tv->setTextSize(40);
        return tv;
    }
};

const char*urls[]={
    "https://img-blog.csdnimg.cn/img_convert/2a388361afd21ca04cbc01e7330f77c6.png",
    "https://static-upyun.ricken.cn/Web/Blog/article/picture/image-20211130133158971.png",
    nullptr
};

const std::vector<CLA::Argument> ARGS={
   {CLA::EntryType::Option, "u", "url",  "url to download", CLA::ValueType::String, (int)CLA::EntryFlags::Optional},
   {CLA::EntryType::Option, "", "loop",  "loops to download", CLA::ValueType::Int, (int)CLA::EntryFlags::Optional},
};

int main(int argc,const char*argv[]){
    App app(argc,argv,ARGS);
    Window*w=new Window(0,0,-1,-1);
     MyAdapter*adapter=new MyAdapter();
    ListView*lv=(ListView*)&w->addView(new ListView(460,500)).setId(100);
    adapter->setNotifyOnChange(true);
    lv->setAdapter(adapter);
    for(int i=0;i<56;i++) adapter->add("");
    CurlDownloader dld(Looper::getForThread());
    std::string url = app.getArg("url");
    LOGI_IF(!url.empty(),"downloading %s...",url.c_str());
    if(!url.empty()){
       CurlDownloader::ConnectionData* cnn=new CurlDownloader::ConnectionData(url);
       dld.addConnection(cnn);
    }
    const int loops =app.getArgAsInt("loop",url.empty()?1:0);
    for(int j=0;j<loops;j++){
       for(int i=0;urls[i];i++){
          CurlDownloader::ConnectionData* cnn=new CurlDownloader::ConnectionData(urls[i]);
          dld.addConnection(cnn);
       }
    }
    return app.exec();
}

