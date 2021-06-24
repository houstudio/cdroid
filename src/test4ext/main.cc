#include<stdio.h>
#include<ngl_graph.h>
#include<ngl_os.h>
#include<ngl_misc.h>
#include<ngl_log.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/time.h>
void TestFill();
void TestBlit();

NGL_MODULE(MAIN)

int main(int argc,const char*argv[]){
    nglLogParseModules(argc,argv);
    nglSysInit();
    nglGraphInit();
    TestBlit();
    TestFill();
}
#define TEST_TIMES 2000
void TestBlit(){
   HANDLE mainsurface=0;
   HANDLE surface2;
   UINT width,height,pitch;
   struct timeval t1,t2;

   NGLOG_DEBUG("");
   nglGetScreenSize(&width,&height);
   nglCreateSurface(&mainsurface,width,height,0,1);//1-->main surface
   nglCreateSurface(&surface2,600,600,0,0);//soft layer
   
   nglFillRect(surface2,NULL,0xFFFF0000);
   gettimeofday(&t1,NULL);
   for(int i=0;i<TEST_TIMES;i++)   
   nglBlit(mainsurface,100,50,surface2,NULL);
   gettimeofday(&t2,NULL);
   int usedtime=(t2.tv_sec*1000+t2.tv_usec/1000-t1.tv_sec*1000+t1.tv_usec/1000);
   printf("BlitSpeed=%fms/frame\r\n",usedtime/(float)TEST_TIMES);
   sleep(2);
}
void TestFill(){
   HANDLE surface=0;
   UINT width,height,pitch;
   UINT *buffer;
   struct timeval t1,t2;
   NGLRect r={100,50,600,600};
   nglGetScreenSize(&width,&height);
   nglCreateSurface(&surface,width,height,0,1);
   NGLOG_DEBUG("ScreenSize %dx%d mainsurface=%p buffer=%p pitch=%d",width,height,surface,buffer,pitch);
   gettimeofday(&t1,NULL);
   for(int i=0;i<TEST_TIMES;i++){
      nglFillRect(surface,&r,0xFF000000|(i<<8)|(i+i<<16));
   }
   gettimeofday(&t2,NULL);
   int usedtime=(t2.tv_sec*1000+t2.tv_usec/1000-t1.tv_sec*1000+t1.tv_usec/1000);
   printf("FillSpeed=%fms/frame\r\n",usedtime/(float)TEST_TIMES);
   sleep(2);
   nglDestroySurface(surface);
}
