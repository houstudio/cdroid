#include <stdio.h>
#include <gtest/gtest.h>
#include <dtvos.h>
#include <dtvdmx.h>
#include <dtvtuner.h>
#include <cdmisc.h>
#include <tvtestutils.h>
#define MAX_PROGRAM 32

class DMXTSPES:public testing::Test{
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

static void FilterCBK(HANDLE dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData){
   int i;
   void**params=(void**)pUserData;
   printf("FilterCBK flt=%p  len=%d params[0]=%p event=%p\r\n",dwVaFilterHandle,uiBufferLength,params[0],params[1]);
   memcpy(params[0],pBuffer,uiBufferLength);
   nglSetEvent((HANDLE)params[1]);
}

int DMXTSPES::GetSection(int pid,BYTE tbid,BYTE*data){
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

static int getElements(BYTE*data,USHORT *pids){
    INT i,len1,len2;
    USHORT svc_caid=0,svc_ecmpid=0;
    USHORT es_caid=0,es_emcpid=0;
    len1=((data[10]&0x0F)<<8)|data[11];
    BYTE*des=data+12+len1;
    USHORT sectionLen= (data[1]&0x0F)<<8|data[2];
    for(i=0;des<data+sectionLen-1;i++){
        BYTE*pca;
        USHORT dlen=(des[3]&0x0F)<<8|des[4];
        //es->stream_type=des[0];
        pids[i]=(des[1]&0x1F)<<8|des[2];//es->pid=(des[1]&0x1F)<<8|des[2];
        des+=dlen+5;//es->getLength()+5;
    }
    return i;
}

static void SectionCounterCBK(HANDLE dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData){
    INT*counter=(INT*)pUserData;
    (*counter)+=1;
    printf("SectionCounterCBK datalen=%d\r\n",uiBufferLength);
}

TEST_F(DMXTSPES,AllocFilter_1){
   flt=nglAllocateSectionFilter(dmxid,0x10,SectionCounterCBK,NULL,DMX_TS);

   ASSERT_NE((HANDLE)nullptr,flt);
   ASSERT_EQ(nglFreeSectionFilter(flt),E_OK);
}

TEST_F(DMXTSPES,AllocFilter_2){
   flt=nglAllocateSectionFilter(dmxid,0x10,SectionCounterCBK,NULL,DMX_PES);

   ASSERT_NE((HANDLE)nullptr,flt);
   ASSERT_EQ(nglFreeSectionFilter(flt),E_OK);
}


TEST_F(DMXTSPES,TS){
   BYTE mask[8],value[8];
   INT i=0,count=0;

   flt=nglAllocateSectionFilter(dmxid,0x00,SectionCounterCBK,&count,DMX_TS);
   ASSERT_NE((HANDLE)nullptr,flt);
   ASSERT_EQ(0,nglStartSectionFilter(flt));
   do{
       nglWaitEvent(eventHandle,1000);
   }while(count<2&&i++<10);
   ASSERT_GT(count,1);
   ASSERT_EQ(0,nglStopSectionFilter(flt));
   nglFreeSectionFilter(flt);

}

TEST_F(DMXTSPES,PES){
   BYTE mask[8],value[8];
   INT i=0,count=0,esc;
   BYTE PAT[1024];
   BYTE PMT[1024];
   USHORT pmtpids[128];
   USHORT espids[32];
   ASSERT_EQ(0,GetSection(0,0,PAT));
   GetPMTPids(PAT,pmtpids);
   
   ASSERT_EQ(0,GetSection(pmtpids[1],2,PMT));
   ASSERT_GT(esc=getElements(PMT,espids),0);
   for(int i=0;i<esc;i++){
      printf("espid[%d]=%d/%x\r\n",i,espids[i],espids[i]);
      count=0;
      flt=nglAllocateSectionFilter(0,espids[0],SectionCounterCBK,&count,DMX_PES);
      ASSERT_NE((HANDLE)nullptr,flt);
      ASSERT_EQ(0,nglStartSectionFilter(flt));
      while(count==0)nglWaitEvent(eventHandle,2000);
      ASSERT_EQ(0,nglStopSectionFilter(flt));
      nglFreeSectionFilter(flt);
   }
}



