#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<windows.h>
#include<asynsource.h>
#include<iostream>

int main(int argc,const char*argv[]){
    App app(argc,argv);
    AsyncEventSource*as=new AsyncEventSource();
    Looper::getDefault()->add_event_source(as,[](EventSource&){return true;});
    for(int i=0;i<10;i++){
        std::future<int> res=as->enqueue([i](int a)->int{
            usleep((a+500)*1000);
            std::cout<<i<<" 'sresult:="<<a<<std::endl;
            return a;
        },i);
        //std::cout<<i<<" 'sresult:="<<res.get()<<std::endl;
    }
    std::cout<<"before exec"<<std::endl;
    return app.exec();
}
