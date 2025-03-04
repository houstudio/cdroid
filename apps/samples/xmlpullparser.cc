#include <core/xmlpullparser.h>
#include <core/app.h>
#include <iostream>
int main(int argc,const char*argv[]){
    cdroid::App app(argc,argv);
    cdroid::XmlPullParser parser(nullptr,argv[1]);
    cdroid::XmlPullParser::XmlEvent event;
    while(parser.next(event)){
        const int depth=parser.getDepth();
        if(event.type==cdroid::XmlPullParser::START_TAG)
            std::cout<<std::string(depth*4,' ')<<"<"<<event.name<<">"<<depth<<std::endl;
        else if(event.type==cdroid::XmlPullParser::END_TAG)
            std::cout<<std::string(depth*4,' ')<<"</"<<event.name<<">"<<depth<<std::endl;
        else std::cout<<"type="<<event.type<<" text="<<event.text<<std::endl;
    }
    return app.exec();
}
