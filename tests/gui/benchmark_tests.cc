#include <gtest/gtest.h>
#include <cdroid.h>
#include <core/systemclock.h>
#include <image-decoders/imagedecoder.h>
#include <ngl_os.h>

using namespace cdroid;
using namespace Cairo;
class BENCHMARK:public testing::Test{

   public :
   static void SetUpTestCase(){
       GFXInit();
   }
   virtual void SetUp(){

   }
   virtual void TearDown(){
   }
};

TEST_F(BENCHMARK,FillBlit){
    RefPtr<ImageSurface>dstsuf=ImageSurface::create(Surface::Format::ARGB32,1280,720);
    RefPtr<ImageSurface>srcsuf=ImageSurface::create(Surface::Format::ARGB32,1280,720);
    RefPtr<Cairo::Context>dst=Cairo::Context::create(dstsuf);
    RefPtr<Cairo::Context>src=Cairo::Context::create(srcsuf);
    src->set_source_rgb(1,0.5,1);
    src->rectangle(0,0,1280,720);
    src->fill();
    dst->set_source(srcsuf,0,0);
    int64_t t1=SystemClock::uptimeMillis();
    for(int i=0;i<100;i++){
        dst->set_source_rgb(i*.01f,i*.01f,i*.01f);
        dst->rectangle(0,0,1280,720);
        dst->fill();
    }
    int64_t t2=SystemClock::uptimeMillis();
    printf("fill speed=%f\r\n",(t2-t1)/100.f);

    t1=SystemClock::uptimeMillis();
    for(int i=0;i<100;i++){
        dst->rectangle(0,0,1280,720);
        dst->paint();
    }
    t2=SystemClock::uptimeMillis();
    printf("paint speed=%f\r\n",(t2-t1)/100.f);

    t1=SystemClock::uptimeMillis();
    for(int i=0;i<100;i++){
        dst->rectangle(0,0,1280,720);
        dst->paint_with_alpha(0.5f);
    }
    t2=SystemClock::uptimeMillis();
    printf("alpha paint speed=%f\r\n",(t2-t1)/100.f);
}

#if defined(ENABLE_JPEG)||defined(ENABLE_TURBOJPEG)
TEST_F(BENCHMARK,Jpeg){
   int64_t t1=SystemClock::uptimeMillis();
   for(int i=0;i<100;i++){
       auto dec =ImageDecoder::create(nullptr,"radio_logo.jpg");
       RefPtr<ImageSurface>img=dec->decode();
   }
   int64_t t2=SystemClock::uptimeMillis();
   printf("jpeg decoe time:%f \r\n",(t2-t1)/100.f);
}
#endif
