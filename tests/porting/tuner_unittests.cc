#include <stdio.h>
#include <gtest/gtest.h>
#include<gtest/internal/gtest-port.h>
#include <ngl_os.h>
#include <ngl_tuner.h>
#include <ngl_dmx.h>
#include <tvtestutils.h>

static void TunningCBK(INT tuneridx,INT lockedState,void*param){
    DWORD *params=(DWORD*)param;
    *((INT*)params[0])=lockedState;
    printf("lockedState=%d\r\n");
}
static void SectionCBK(HANDLE dwVaFilterHandle,const BYTE *pBuffer,UINT uiBufferLength, void *pUserData)
{
   printf("SectionCBK flt=0x%x data=%p 0x%02x\n",dwVaFilterHandle,pBuffer,pBuffer[0]);
   nglSetEvent((HANDLE)pUserData);
}


class TUNER:public testing::Test{
   public :
   HANDLE eventHandle;
   INT lockstate;
   static HANDLE params[2];
   HANDLE flt;
   BYTE mask[8],value[8];
   static void SetUpTestCase(){
      nglTunerInit();
      nglDmxInit();
      nglTunerRegisteCBK(0,TunningCBK,params);
   }
   static void TearDownTestCase(){
   }
   virtual void SetUp(){
      lockstate=0;
      eventHandle=nglCreateEvent(0,0);
      params[0]=(HANDLE)&lockstate;
      params[1]=eventHandle;
   }

   HANDLE createFilter(){
      flt=nglAllocateSectionFilter(0,0/*patpid*/,SectionCBK,(void*)eventHandle,DMX_SECTION);
      mask[0]=0xFF;value[0]=0x00;//for PAT
      nglSetSectionFilterParameters(flt,mask,value,1); 
      nglStartSectionFilter(flt);
      return flt;
   }
   virtual void TearDown(){
      nglTunerUnRegisteCBK(0,TunningCBK);
   }
   int GetTuningParams(NGLTunerParam&tp){
       const char*params=testing::internal::StringFromGTestEnv("tunning","C:685000,6875,64");
       const char*p=strpbrk(params,"CST");
       if(p==NULL)return 0;
       switch(*p){
       case 'C':tp.delivery_type=DELIVERY_C;
                tp.frequency=atoi(p+1);
                p=strpbrk(p+1,",;");
                tp.u.c.symbol_rate=atoi(p+1);
                p=strpbrk(p+1,",;");
                //tp.u.c.modulation=0;
                break;
       case 'S':tp.delivery_type=DELIVERY_S;break;
       case 'T':tp.delivery_type=DELIVERY_T;break;
       default:return 0;
       }
       testing::internal::StringFromGTestEnv("tunning","S:685000,6875,hor,22k");
       testing::internal::StringFromGTestEnv("tunning","T:685000,8");
   }
};
HANDLE TUNER::params[2];

TEST_F(TUNER,LNB){
   //TUNER_Polarity 水平时为1，垂直时为2 off 时为0
   ASSERT_TRUE(nglTunerSetLNB(0,0)==E_OK);
   ASSERT_TRUE(nglTunerSetLNB(0,1)==E_OK);
   ASSERT_TRUE(nglTunerSetLNB(0,2)==E_OK);
}

TEST_F(TUNER,22K){
   ASSERT_TRUE(nglTunerSet22K(0,0)==E_OK);//22k off
   ASSERT_TRUE(nglTunerSet22K(0,1)==E_OK);//22k on
}

TEST_F(TUNER,Tunning_C1){
   NGLTunerParam tp;
   tvutils::GetTuningParams(tp);
   printf("flt=%p\r\n",flt); 

   ASSERT_EQ(E_OK,nglTunerLock(0,&tp));
   flt=createFilter();
   ASSERT_TRUE(E_OK==nglWaitEvent(eventHandle,5000));
   nglFreeSectionFilter(flt);
   ASSERT_EQ(1,lockstate);
}

TEST_F(TUNER,Tunning_S1){
   NGLTunerParam tp;
   tvutils::GetTuningParams(tp);
   flt=createFilter();
   printf("flt=%p\r\n",flt); 

   nglTunerSetLNB(0,1);//1--HORZ 2--VERT
   nglTunerSet22K(0,1);
   ASSERT_EQ(E_OK,nglTunerLock(0,&tp));
   //diseqc_set_diseqc10(0,TUNER_DISEQC10_PORT_A,TUNER_POL_VERTICAL,TUNER_TONE_22K_ON);//TUNER_POL_VERTICAL/HORIZONTAL
   flt=createFilter();
   ASSERT_TRUE(E_OK==nglWaitEvent(eventHandle,5000));
   nglFreeSectionFilter(flt);
   ASSERT_EQ(1,lockstate);
}

TEST_F(TUNER,Tunning_T1){
   NGLTunerParam tp;
   tvutils::GetTuningParams(tp);
   flt=createFilter();
   printf("flt=%p\r\n",flt); 

   ASSERT_EQ(E_OK,nglTunerLock(0,&tp));
   flt=createFilter();
   ASSERT_TRUE(E_OK==nglWaitEvent(eventHandle,5000));
   nglFreeSectionFilter(flt);
   ASSERT_EQ(1,lockstate);
}

TEST_F(TUNER,Tunning_Err_1){
   NGLTunerParam tp;
   tp.delivery_type=DELIVERY_S;
   tp.u.s.symbol_rate=27500;//27500;//26040;
   tp.u.s.polar=NGL_NIM_POLAR_VERTICAL;// NGL_NIM_POLAR_HORIZONTAL NGL_NIM_POLAR_VERTICAL;
   tp.frequency=38400*1000;
   ASSERT_EQ(E_ERROR,nglTunerLock(0,&tp));
   flt=createFilter();
   ASSERT_TRUE(E_OK!=nglWaitEvent(eventHandle,5000));
   nglFreeSectionFilter(flt);
   printf("\tfreq=%d locakstate=%d \r\n",tp.frequency,lockstate);
   ASSERT_EQ(0,lockstate);
}

