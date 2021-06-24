#include <stdio.h>
#include <gtest/gtest.h>
#include <ngl_os.h>
#include <ngl_dmx.h>
#include <ngl_tuner.h>
#include <ngl_misc.h>
#include <tvtestutils.h>
#define MAX_PROGRAM 32

class DMX:public testing::Test{
public :
   HANDLE flt;
   HANDLE eventHandle;
   INT dmxid;
   BYTE*data;
   static void SetUpTestCase(){
       NGLTunerParam tp;//TRANSPONDER tp;
       tvutils::GetTuningParams(tp);
       nglSysInit();
       nglTunerInit();
       nglTunerLock(0,&tp);
       nglDmxInit();
       printf("DMX::SetUpTestCase\r\n");
   }
   static void TearDownTestCase() {
       printf("DMX::TearDownTestCase\r\n");
   }
   virtual void SetUp(){
       flt=0;
       dmxid=0;
       data=(BYTE*)nglMalloc(4096);
       eventHandle=nglCreateEvent(0,TRUE);
       printf("data=%p,handle=%p\r\n",data,eventHandle);
   }
   virtual void TearDown(){
      if(0!=flt)
          nglFreeSectionFilter(flt);
      nglFree(data);
      nglDestroyEvent(eventHandle);
   }
   int GetPMTPids(BYTE*pmt,USHORT*pmtpids){
      int seclen=((pmt[1]&0x0F)<<8)|pmt[2];
      int cnt=0;
      BYTE*p=pmt+8;
      unsigned short* pids=pmtpids;
      for(;p<pmt+seclen-1;p+=4){
         pids[0]=(p[0]<<8)|p[1]; //serviceid
         pids[1]=((p[2]&0x1F)<<8)|p[3]; //pmtpid
         if(pids[1]==0x10)continue;
         printf("serviceid=0x%x/%d pmt_pid=0x%x/%d\r\n",pids[0],pids[0],pids[1],pids[1]);
         pids+=2;cnt++;
      }
      return cnt;
   }
   int GetSection(int pid,BYTE tbid,BYTE*data);
};

static void SectionCBK(HANDLE dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData)
{
   printf("SectionCBK flt=0x%x data=%p 0x%02x\n",dwVaFilterHandle,pBuffer,pBuffer[0]);  
}

TEST_F(DMX,AllocFilter_1){
   flt=nglAllocateSectionFilter(dmxid,0x10,SectionCBK,NULL,DMX_SECTION);

   ASSERT_NE((HANDLE)nullptr,flt);
   ASSERT_EQ(nglFreeSectionFilter(flt),E_OK);

}

TEST_F(DMX,Alloc_Max){
   HANDLE flts[MAX_FILTERS]={NULL};
   for(int i=0;i<MAX_FILTERS/2;i++){
       flts[i]=nglAllocateSectionFilter(dmxid,i,SectionCBK,NULL,DMX_SECTION);
       ASSERT_FALSE(0==flts[i]);
   }
   for(int i=0;i<MAX_FILTERS/2;i++)
       ASSERT_EQ(nglFreeSectionFilter(flts[i]),E_OK);
}

TEST_F(DMX,AllocFilter_2){
   ASSERT_EQ(NULL,nglAllocateSectionFilter(dmxid,0x10,NULL,NULL,DMX_SECTION));
   ASSERT_EQ(NULL,nglAllocateSectionFilter(dmxid,0x1FFF,NULL,NULL,DMX_SECTION));
   ASSERT_EQ(NULL,nglAllocateSectionFilter(dmxid,0x2000,NULL,NULL,DMX_SECTION));
}

TEST_F(DMX,StartFilter_1){
   flt=nglAllocateSectionFilter(dmxid,0x10,SectionCBK,NULL,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,StopFilter_1){
   flt=nglAllocateSectionFilter(dmxid,0x10,SectionCBK,NULL,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   ASSERT_EQ(0,nglStopSectionFilter(flt));
   ASSERT_EQ(0,nglFreeSectionFilter(flt));
}

TEST_F(DMX,NullFilter){
   ASSERT_NE(E_OK,nglStartSectionFilter(0));
   ASSERT_NE(E_OK,nglSetSectionFilterParameters(NULL,NULL,NULL,0));
   ASSERT_NE(E_OK,nglStopSectionFilter(0));
   ASSERT_NE(E_OK,nglFreeSectionFilter(0));
}

TEST_F(DMX,SetFilterParam_1){
   BYTE mask[16],value[16];
   flt=nglAllocateSectionFilter(dmxid,0x10,SectionCBK,NULL,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,NULL,NULL,0));
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,NULL,value,0));
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,NULL,0));
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,0));
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,MAX_FILTER_DEPTH));
   ASSERT_EQ(E_OK,nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,SetFilterParam_Error_1){
   BYTE mask[8],value[8];
   flt=nglAllocateSectionFilter(dmxid,0x10,SectionCBK,NULL,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,NULL,NULL,1));
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,mask,NULL,1));
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,NULL,value,1));
   ASSERT_NE(0,nglSetSectionFilterParameters(flt,mask,value,MAX_FILTER_DEPTH+1));
   ASSERT_EQ(0,nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

static void FilterCBK(HANDLE dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData){
   int i;
   void**params=(void**)pUserData;
   printf("FilterCBK flt=%p  len=%d params[0]=%p event=%p\r\n",dwVaFilterHandle,uiBufferLength,params[0],params[1]);
   memcpy(params[0],pBuffer,uiBufferLength);
   nglSetEvent((HANDLE)params[1]);
}

int DMX::GetSection(int pid,BYTE tbid,BYTE*data){
   void* params[2];
   BYTE mask[8],value[8];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   mask[0]=0xFF;
   value[0]=tbid;
   data[0]=0xFF;
   flt=nglAllocateSectionFilter(dmxid,pid,FilterCBK,params,DMX_SECTION);
   nglSetSectionFilterParameters(flt,mask,value,1);
   nglStartSectionFilter(flt);
   int rc=nglWaitEvent(eventHandle,2000);
   nglStopSectionFilter(flt);
   nglFreeSectionFilter(flt);
   return rc;
}

static void SectionCounterCBK(HANDLE dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData){
    INT*counter=(INT*)pUserData;
    (*counter)+=1;
}
TEST_F(DMX,ShotTimes){
   BYTE mask[8],value[8];
   INT i=0,count=0;

   data[0]=0xFF;
   flt=nglAllocateSectionFilter(dmxid,0x00,SectionCounterCBK,&count,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   mask[0]=0xFF;value[0]=0x00;//for PAT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   do{
       nglWaitEvent(eventHandle,1000);
   }while(count<2&&i++<10);
   ASSERT_GT(count,1);
   ASSERT_EQ(0,nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);

}

TEST_F(DMX,TDT){
   BYTE mask[8],value[8];
   void* params[2];
   INT i=0,count=0;
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   data[0]=0xFF;
   flt=nglAllocateSectionFilter(dmxid,0x14,FilterCBK,params,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   mask[0]=0xFF;value[0]=0x70;//for TDT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_EQ(E_OK,nglWaitEvent(eventHandle,20000));
   ASSERT_EQ(0,nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,Filter_MASK_1){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;

   data[0]=0xFF;
   flt=nglAllocateSectionFilter(dmxid,0x00,FilterCBK,params,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   mask[0]=0xFF;value[0]=0x00;//for PAT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_EQ(0,nglWaitEvent(eventHandle,2000));
   ASSERT_EQ(data[0],0);
   ASSERT_EQ(0,nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
}

TEST_F(DMX,Filter_MASK_3){
   BYTE PAT[1024];
   BYTE mask[8],value[8];
   void*params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;

   ASSERT_EQ(GetSection(0,0,PAT),E_OK);

   data[0]=0xFF;
   flt=nglAllocateSectionFilter(dmxid,0x00,FilterCBK,params,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   mask[0]=mask[1]=mask[2]=0xFF;
   value[0]=0x00;//for PAT filter by transport id
   value[1]=PAT[3];value[2]=PAT[4];//section length is skipped

   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,3));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_EQ(0,nglWaitEvent(eventHandle,2000));

   ASSERT_EQ(data[0],0);
   ASSERT_EQ(data[3],PAT[3]);
   ASSERT_EQ(data[4],PAT[4]);
   ASSERT_EQ(0,nglStopSectionFilter(flt));

   nglFreeSectionFilter(flt);
}

TEST_F(DMX,Filter_Parallel1){//recv two or more data at same time
   HANDLE flt1;
   HANDLE flt2;
   BYTE mask[8],value[8];
   INT i=0,count1=0,count2=0;
   flt1=nglAllocateSectionFilter(dmxid,0x00,SectionCounterCBK,&count1,DMX_SECTION);
   flt2=nglAllocateSectionFilter(dmxid,0x11,SectionCounterCBK,&count2,DMX_SECTION);

   ASSERT_TRUE(flt1&&flt2);

   mask[0]=0xFF; value[0]=0x00;//for PAT filter by transport id
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt1,mask,value,1));

   value[0]=0x42;//sdt receiver
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt2,mask,value,1));

   ASSERT_EQ(0,nglStartSectionFilter(flt1));
   ASSERT_EQ(0,nglStartSectionFilter(flt2));
   do{
       nglWaitEvent(eventHandle,1000);
       i++;
   }while(i<10&&count1<10&&count2<10);
   ASSERT_GT(count1,0);
   ASSERT_GT(count2,0);
   ASSERT_EQ(0,nglStopSectionFilter(flt1));
   ASSERT_EQ(0,nglStopSectionFilter(flt2));
   ASSERT_EQ(0,nglFreeSectionFilter(flt1));
   ASSERT_EQ(0,nglFreeSectionFilter(flt2));
}

TEST_F(DMX,Filter_Parallel2){//test for multi sectin filter ,rcv all pmt at same time
   BYTE PAT[1024];
   BYTE mask[8],value[8];
   USHORT pmtpids[128];
   INT program_count;
   INT counters[128];
   HANDLE flts[64];
   ASSERT_EQ(GetSection(0,0,PAT),E_OK);
   mask[0]=mask[1]=mask[2]=0xFF;
   value[0]=0x02;
   program_count=GetPMTPids(PAT,pmtpids);
   ASSERT_GT(program_count,0);
   for(int i=0;i<program_count&&i<64;i++){
       int masklen=3;
       mask[0]=mask[1]=mask[2]=0xFF;
       value[0]=0x02;

       value[1]=pmtpids[i*2]>>8;
       value[2]=pmtpids[i*2]&0xFF;

       counters[i]=0;
       flts[i]=nglAllocateSectionFilter(dmxid,pmtpids[i*2+1],SectionCounterCBK,counters+i,DMX_SECTION);
       ASSERT_NE((HANDLE)nullptr,flts[i]);
       ASSERT_EQ(0,nglSetSectionFilterParameters(flts[i],mask,value,masklen));
       ASSERT_EQ(0,nglStartSectionFilter(flts[i]));
   }
   nglWaitEvent(eventHandle,5000);
   for(int i=0;i<program_count;i++){
       ASSERT_GT(counters[i],0);
       nglStopSectionFilter(flts[i]);
       nglFreeSectionFilter(flts[i]);
   }

}

TEST_F(DMX,Filter_MASK_3_Error){ 
   BYTE PAT[1024];
   BYTE mask[8],value[8];
   ASSERT_EQ(GetSection(0,0,PAT),E_OK);
   ASSERT_TRUE(data);
   
   void*params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
  
   flt=nglAllocateSectionFilter(dmxid,0x00,FilterCBK,params,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
 
   mask[0]=mask[1]=mask[2]=0xFF;
   value[0]=0x00;//for PAT filter by transport id
   value[1]=~PAT[3];value[2]=~PAT[4];//filter by transport id(section length is sipped)

   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,3));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_NE(0,nglWaitEvent(eventHandle,2000));// no data wait timeout
   ASSERT_EQ(0,nglStopSectionFilter(flt));

   nglFreeSectionFilter(flt);
}

TEST_F(DMX,Filter_Data_2){
   BYTE mask[8],value[8];
   void* params[2];
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   for(int i=0;i<3;i++){
      data[0]=0xFF;
      flt=nglAllocateSectionFilter(dmxid,0x11,FilterCBK,params,DMX_SECTION);
      ASSERT_FALSE(0==flt);
      mask[0]=0xFF;value[0]=0x42;//for SDT
      ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
      ASSERT_EQ(0,nglStartSectionFilter(flt));
      ASSERT_EQ(0,nglWaitEvent(eventHandle,5000));
      ASSERT_EQ(data[0],0x42);
      ASSERT_EQ(0,nglStopSectionFilter(flt));
      ASSERT_EQ(0,nglFreeSectionFilter(flt));
      sleep(2);
   }
}

TEST_F(DMX,Filter_Data_PAT_PMT){
   BYTE mask[8],value[8];
   void* params[2];
   USHORT pmtpids[128];
   INT program_count;
   params[0]=(void*)data;
   params[1]=(void*)eventHandle;
   flt=nglAllocateSectionFilter(dmxid,0,FilterCBK,params,DMX_SECTION);

   ASSERT_NE((HANDLE)nullptr,flt);

   data[0]=0xFF;
   mask[0]=0xFF;
   value[0]=0x0;//for PAT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_EQ(0,nglWaitEvent(eventHandle,2000));
   ASSERT_EQ(data[0],0x00);
   ASSERT_EQ(0,nglStopSectionFilter(flt));
   ASSERT_EQ(0,nglFreeSectionFilter(flt));
   ASSERT_EQ(0,data[0]);
   program_count=GetPMTPids(data,pmtpids);
   ASSERT_GT(program_count,0);
   for(int i=0;i<program_count;i++){
       int masklen=3;
       mask[0]=mask[1]=mask[2]=0xFF;
       value[0]=0x02;

       value[1]=pmtpids[i*2]>>8;
       value[2]=pmtpids[i*2]&0xFF;

       flt=nglAllocateSectionFilter(dmxid,pmtpids[i*2+1],FilterCBK,params,DMX_SECTION);
       ASSERT_NE((HANDLE)nullptr,flt);
       ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,masklen));
       ASSERT_EQ(0,nglStartSectionFilter(flt));
       ASSERT_EQ(0,nglWaitEvent(eventHandle,2000));
       ASSERT_EQ(data[0],2);
       ASSERT_EQ(data[3]<<8|data[4],pmtpids[i*2]);
       ASSERT_EQ(0,nglStopSectionFilter(flt));
       ASSERT_EQ(0,nglFreeSectionFilter(flt));
   }
}

static void EIT_CBK(HANDLE dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData){
   static int count=0;
   memcpy(pUserData,pBuffer,uiBufferLength); 
   if(count++%1000==0)printf("%02x %02x %02x %02x  %p/%p %d\r\n",pBuffer[0],pBuffer[1],pBuffer[2],pBuffer[3],
       pUserData,pBuffer,uiBufferLength);
}
TEST_F(DMX,GX){
   BYTE buffer[4096];
   BYTE mask[8],value[8];

   data[0]=0xFF;
   flt=nglAllocateSectionFilter(dmxid,0x12,EIT_CBK,buffer,DMX_SECTION);
   ASSERT_NE((HANDLE)nullptr,flt);
   mask[0]=0x00;value[0]=0x00;//for PAT
   ASSERT_EQ(0,nglSetSectionFilterParameters(flt,mask,value,1));
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   ASSERT_EQ(0,nglWaitEvent(eventHandle,2000*1000));
   ASSERT_EQ(data[0],0);
   ASSERT_EQ(0,nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);
  
}
