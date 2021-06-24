#include <stdio.h>
#include <gtest/gtest.h>
#include <ngl_os.h>
#include <ngl_dmx.h>
#include <ngl_video.h>
#include <cdgraph.h>
#include <ngl_tuner.h>
#include <tvtestutils.h>
#include <ngl_misc.h>

class AV:public testing::Test{
   public :
   static void SetUpTestCase(){
       nglSysInit();
       nglTunerInit();
       nglDmxInit();
       nglAvInit();
       GFXInit();
   }
   static void TearDownTestCase(){
       sleep(20);
   }
   virtual void SetUp(){
       NGLTunerParam tp;//TRANSPONDER tp;
       tvutils::GetTuningParams(tp);
       nglTunerLock(0,&tp);
   }
   virtual void TearDown(){}
};

TEST_F(AV,Play){
   nglAvPlay(0,512,DECV_MPEG,650,DECA_MPEG1,128);
   nglSleep(10000);
}

TEST_F(AV,PlayGlobalAlpha){
   HANDLE mainsurface;
   GFXCreateSurface(&mainsurface,1280,720,GPF_ARGB,1);
   GFXFillRect(mainsurface,nullptr,0xFFFFFFFF);
   GFXFlip(mainsurface);
   GFXSurfaceSetOpacity(mainsurface,0x80);
   for(int i=255;i>0;i-=20){
      GFXSurfaceSetOpacity(mainsurface,i);
      sleep(1);
   }
   GFXDestroySurface(mainsurface);
}

TEST_F(AV,PlayUnderSurface){
   HANDLE mainsurface;
   nglAvPlay(0,512,DECV_MPEG,650,DECA_MPEG1,128);
   GFXCreateSurface(&mainsurface,1280,720,GPF_ARGB,1);
   GFXSurfaceSetOpacity(mainsurface,0x80);
   for(int i=255;i>0;i-=20){
      GFXFillRect(mainsurface,nullptr,0x00FFFFFF|(i<<24));
      GFXFlip(mainsurface);
      sleep(1);
   }
   GFXDestroySurface(mainsurface);
}

TEST_F(AV,PlayInWindow){
   HANDLE mainsurface;
   GFXRect rect={200,200,800,400};
   GFXCreateSurface(&mainsurface,1280,720,GPF_ARGB,1);
   GFXSurfaceSetOpacity(mainsurface,0x80);
   nglAvPlay(0,512,DECV_MPEG,650,DECA_MPEG1,128);
   for(int i=255;i>0;i-=20){
      GFXFillRect(mainsurface,&rect,0x00FFFFFF|(i<<24));
      GFXFlip(mainsurface);
      sleep(1);
   }
   GFXDestroySurface(mainsurface);
}

