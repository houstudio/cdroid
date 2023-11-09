#include <cdroid.h>
#include <string>
#include <cdlog.h>
#include <iostream>
#include <curl/curl.h>
#include <curldownload.h>
int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,-1,-1);
    CurlDownloader dld;
    CurlDownloader::ConnectionData cnn(argc<=1?"https://www.baidu.com/":argv[1]);
    dld.addConnection(&cnn);
    return app.exec();
}

