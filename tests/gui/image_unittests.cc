#include <gtest/gtest.h>
#include <cdtypes.h>
#include <cdgraph.h>
#include <core/canvas.h>
#include <core/gravity.h>
#include <sys/time.h>
#include <core/assets.h>
#include <cdinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <svg-cairo.h>
#include <fstream>
#include <gui/gui_features.h>
#ifdef ENABLE_CAIROSVG
#include <curl/curl.h>
#endif
#include <unistd.h>

#define SLEEP(x) usleep((x)*1000)

using namespace Cairo;
using namespace cdroid;
class IMAGE:public testing::Test{
protected:
   unsigned long long ts;
   std::vector<std::string>images;
public :
   static Canvas*ctx;
   static Assets *rm;
   static void SetUpTestCase(){
       GFXInit();
       InputInit();
       rm=new Assets("ntvplus.pak");
       ctx=new Canvas(800,600);
   }
   static void TearDownCase(){
       delete rm; 
   }
   virtual void SetUp(){
       images.clear();
   }
   virtual void TearDown(){
       postCompose();
       SLEEP(10000);
   }
   void tmstart(){
      ts=gettime();
   }
   void postCompose(){
       RECT rect={0,0,800,600};
       Canvas*primary=GraphDevice::getInstance().getPrimaryContext();
       primary->set_source(ctx->get_target(),0,0);
       primary->rectangle(0,0,800,600);
       primary->fill();
       GFXFlip(GraphDevice::getInstance().getPrimarySurface());
   }
   void tmend(const char*txt){
      printf("%s:used time %lldms\r\n",txt,gettime()-ts);
   }
   unsigned long long gettime(){
       struct timeval tv;
       gettimeofday(&tv,NULL);
       return tv.tv_sec*1000+tv.tv_usec/1000;
   }
   void loadImages(const std::string&path,const std::string&filter){
       struct stat st;
       if(stat(path.c_str(),&st)==-1)
           return ;
       if(S_ISREG(st.st_mode)){
           size_t pt=path.rfind('.');
           if(pt!= std::string::npos){
              std::string ext=path.substr(pt+1);
              if(filter.empty()||filter.compare(ext))images.push_back(path);
           }
           return ;
       }
       DIR *dir=opendir(path.c_str());
       struct dirent *entry;
       while(dir&&(entry=readdir(dir))){
           std::string s=path;
           if(entry->d_name[0]=='.')
              continue;
           s.append("/");
           s.append(entry->d_name);
           loadImages(s,filter);
       }
       if(dir)closedir(dir);
   }
};
Canvas*IMAGE::ctx=nullptr;
Assets *IMAGE::rm=nullptr;

TEST_F(IMAGE,Bitmap){
    loadImages("./","bmp");
    for(int i=0;i<images.size();i++){
       std::ifstream fs(images[i].c_str());
       RefPtr<ImageSurface>img=ImageSurface::create_from_stream(fs);
       RECT rect={0,0,800,600};
       for(int i=0;i<10;i++){
          ctx->set_color(0xFF000000|(i*20<<16));
          ctx->rectangle(rect);
          ctx->fill();
          ctx->draw_image(img,rect,nullptr);//,&rect,NULL,ST_FITXY);//ST_CENTER_INSIDE);
          postCompose();
          SLEEP(500);
       }
    }
    SLEEP(2000);
}
TEST_F(IMAGE,Image_PNG){
    loadImages("./","png");
    for(int i=0;i<images.size();i++){
        tmstart();
        std::ifstream fs(images[i].c_str());
        RefPtr<ImageSurface>img=ImageSurface::create_from_stream(fs);
        tmend("decodepng");
        RECT rect={0,0,800,600};
        ctx->rectangle(rect);
        ctx->fill();
        tmstart();
        ctx->draw_image(img,rect,nullptr);
        tmend("drawimage");
        postCompose();
    }
    //ctx->dump2png("test2.png");
    SLEEP(2500);
}

TEST_F(IMAGE,Image_JPG){
    loadImages("./","jpg");
    for(int i=0;i<images.size();i++){
        tmstart();
        std::ifstream fs(images[i].c_str());
        RefPtr<ImageSurface>imgj=ImageSurface::create_from_stream(fs);
        tmend("decodejpg");
        RECT rect={0,0,800,600};
        ctx->rectangle(rect);ctx->fill();
        tmstart();
        ctx->draw_image(imgj,rect,nullptr);
        tmend("drawimage");
        postCompose();
        SLEEP(1000);
    }
    SLEEP(2500);
}

TEST_F(IMAGE,draw){
    loadImages("/home/houzh/JPG/","");
    for(int i=0;i<images.size();i++){
        std::ifstream fs(images[i].c_str());
        RefPtr<ImageSurface>img=ImageSurface::create_from_stream(fs);
        RECT dst={100,100,200,200};
        RECT rs={img->get_width()/2,img->get_height()/2,img->get_width()/2,img->get_height()/2};
        ctx->draw_image(img,dst,&rs);
        postCompose();
        sleep(5);
    }
}

TEST_F(IMAGE,ninepatch1){
    std::ifstream fs("/home/houzh/Miniwin/apps/ntvplus/assets/drawable/paopao1.9.png");
    RefPtr<ImageSurface>img=ImageSurface::create_from_stream(fs);
    ASSERT_EQ(1,(int)fs.good());
    RECT rect={50,50,400,100};
    //std::vector<NinePatchBlock> horz,vert;
    //img->get_ninepatch(horz,vert);
    //ctx->draw_ninepatch(img,rect,horz,vert);
}

TEST_F(IMAGE,ninepatch2){
    std::ifstream fs("/home/houzh/Miniwin/apps/ntvplus/assets/drawable/btn_normal.9.png");
    RefPtr<ImageSurface>img=ImageSurface::create_from_stream(fs);
    ASSERT_EQ(1,(int)fs.good());
    RECT rect={50,50,400,100};
    //std::vector<NinePatchBlock> horz,vert;
    //img->get_ninepatch(horz,vert);
    //ctx->draw_ninepatch(img,rect,horz,vert);
}
#ifdef ENABLE_CAIROSVG
TEST_F(IMAGE,SVG){
     svg_cairo_t *svg;
     CURL *hcurl;
     RefPtr<Canvas>ctx(GraphDevice::getInstance().createContext(800,600));
     const static char*svgs[]={
        "https://www.w3school.com.cn/svg/rect1.svg",
        "https://www.w3school.com.cn/svg/rect2.svg",
        "https://www.w3school.com.cn/svg/rect3.svg",
        "https://www.w3school.com.cn/svg/rect4.svg",
        "https://www.w3school.com.cn/svg/circle1.svg",
        "https://www.w3school.com.cn/svg/ellipse1.svg",
        "https://www.w3school.com.cn/svg/ellipse2.svg",
        "https://www.w3school.com.cn/svg/ellipse3.svg",
        "https://www.w3school.com.cn/svg/line1.svg",
        "https://www.w3school.com.cn/svg/polygon1.svg",
        "https://www.w3school.com.cn/svg/polygon2.svg",
        "https://www.w3school.com.cn/svg/polyline1.svg",
        "https://www.w3school.com.cn/svg/path1.svg",
        "https://www.w3school.com.cn/svg/path2.svg",
        "https://www.w3school.com.cn/svg/filter1.svg",
        "https://www.w3school.com.cn/svg/filter2.svg",
        "https://www.w3school.com.cn/svg/feblend_1.svg",
        "https://www.w3school.com.cn/svg/fecolormatrix_1.svg",
        "https://www.w3school.com.cn/svg/fecomponenttransfer_1.svg",
        "https://www.w3school.com.cn/svg/feoffset_1.svg",
        "https://www.w3school.com.cn/svg/femerge_1.svg",
        "https://www.w3school.com.cn/svg/femorphology_1.svg",
        "https://www.w3school.com.cn/svg/filter_1.svg",
        "https://www.w3school.com.cn/svg/filter_2.svg",
        "https://www.w3school.com.cn/svg/filter_3.svg",
        "https://www.w3school.com.cn/svg/filter_4.svg",
        "https://www.w3school.com.cn/svg/filter_5.svg",
        "https://www.w3school.com.cn/svg/filter_6.svg",
        "https://www.w3school.com.cn/svg/linear1.svg",
        "https://www.w3school.com.cn/svg/linear3.svg",
        "https://www.w3school.com.cn/svg/radial1.svg",
        "https://www.w3school.com.cn/svg/radial2.svg",
        "https://www.w3school.com.cn/svg/a_1.svg",
        "https://www.w3school.com.cn/svg/a_2.svg",
        "https://www.w3school.com.cn/svg/animate_1.svg",
        "https://www.w3school.com.cn/svg/animate_2.svg",
        "https://www.w3school.com.cn/svg/animatecolor_1.svg",
        "https://www.w3school.com.cn/svg/animatemotion_0.svg",
        "https://www.w3school.com.cn/svg/animatemotion_1.svg",
        "https://www.w3school.com.cn/svg/animatemotion_2.svg",
        "https://www.levien.com/svg/tiger.svg","file://./svgs/rect1.svg"
     };
     unsigned int width,height;
     RECT rect={0,0,800,600};
     mkdir("pngs",0777);
     for(int i=0;i<sizeof(svgs)/sizeof(svgs[0]);i++){
         svg_cairo_create(&svg);
         char fpng[256];
         const char*file=strrchr(svgs[i],'/');
         ctx->set_source_rgb(1,1,1);
         ctx->rectangle(0,0,800,600);
         ctx->fill();

         svg_cairo_parse(svg,svgs[i]);
      
         svg_cairo_get_size(svg,&width,&height);
         printf("%s  imagesize=%dx%d\r\n",svgs[i],width,height);
         svg_cairo_render(svg,(cairo_t*)ctx->cobj());
         svg_cairo_destroy(svg);
         ctx->move_to(100,540);
         ctx->set_source_rgb(0,.5,1);
         ctx->set_font_size(24);
         ctx->show_text(svgs[i]);
         ctx->invalidate(rect);

         sprintf(fpng,"pngs/%s",file+1);
         strcpy((char*)strrchr(fpng,'.'),".png");
         ctx->invalidate(rect);
         GraphDevice::getInstance().composeSurfaces();
         
         ctx->dump2png(fpng);
         SLEEP(500);
     }
     SLEEP(3000);
}
#endif

