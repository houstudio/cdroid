#ifndef __NGL_TUNER_H__
#define __NGL_TUNER_H__
#include<cdtypes.h>

BEGIN_DECLS

typedef enum{
    NGL_NIM_NOT_DEFINED,
    NGL_NIM_QAM16,
    NGL_NIM_QAM32,
    NGL_NIM_QAM64,
    NGL_NIM_QAM128,
    NGL_NIM_QAM256
}NGLQamMode;

typedef enum{
    NGL_NIM_BANDWIDTH_AUTO,
    NGL_NIM_BANDWIDTH_6_MHZ,
    NGL_NIM_BANDWIDTH_7_MHZ,
    NGL_NIM_BANDWIDTH_8_MHZ
}NGLBandWidth;

typedef enum{
    NGL_NIM_FEC_AUTO,
    NGL_NIM_FEC_1_2,/*Value to specify the fraction @b 1/2*/
    NGL_NIM_FEC_2_3,/*Value to specify the fraction @b 2/3*/
    NGL_NIM_FEC_3_4,/*Value to specify the fraction @b 3/4*/
    NGL_NIM_FEC_5_6,/*Value to specify the fraction @b 5/6*/
    NGL_NIM_FEC_7_8,
    NGL_NIM_FEC_8_9,
    NGL_NIM_FEC_3_5,
    NGL_NIM_FEC_4_5,
    NGL_NIM_FEC_9_10
}NGLNimFEC;

typedef enum{
    NGL_NIM_GUARD_INTER_AUTO,
    NGL_NIM_GUARD_INTER_1_4,
    NGL_NIM_GUARD_INTER_1_8,
    NGL_NIM_GUARD_INTER_1_16,
    NGL_NIM_GUARD_INTER_1_32
}NGLGuardInter;

typedef enum{
    NGL_NIM_FFT_MODE_AUTO,
    NGL_NIM_FFT_MODE_2K,
    NGL_NIM_FFT_MODE_4K,
    NGL_NIM_FFT_MODE_8K
}NGLFFTMode;

typedef enum{
    NGL_NIM_MODUL_AUTO,
    NGL_NIM_MODUL_DQPSK,
    NGL_NIM_MODUL_QPSK,
    NGL_NIM_MODUL_16QAM,
    NGL_NIM_MODUL_64QAM
}NGLModulation;

typedef enum{
    NGL_NIM_POLAR_HORIZONTAL = 0,/*Horizontal Polarization (18V)*/
    NGL_NIM_POLAR_VERTICAL=1,/*Vertical Polarization (13V)*/
    NGL_NIM_CPOLAR_LEFT = 2,/* Left Hand Polarization (18V)*/
    NGL_NIM_CPOLAR_RIGHT=3/*Right Hand Polarization (13V)*/
}NGLNimPolar;

typedef enum{
   NGL_22K_OFF=0,
   NGL_22K_ON=1
}NGLNim22K;

typedef enum{
   NGL_LNB_OFF=0,
   NGL_LNB_ON=1
}NGLNimLNB;

typedef struct{ 
    UINT symbol_rate;
    NGLQamMode modulation;
    NGLBandWidth bandwidth;
}NGLCableParam;

typedef struct{
    NGLBandWidth bandwidth;
    NGLNimFEC fec; 
    NGLGuardInter guard_interval;
    NGLFFTMode fft;
    NGLModulation modulation;
}NGLTerristrialParam;

typedef struct{
   UINT lnbfreq;
   UINT symbol_rate;
   NGLModulation modulation;
   USHORT position;
   NGLNimFEC fec;    
   NGLNimPolar polar;
   USHORT direction;//0 weast 1-east
}NGLSattliteParam;

typedef enum{
  DELIVERY_C,
  DELIVERY_S,
  DELIVERY_S2,
  DELIVERY_T,
  DELIVERY_ATSC
}NGLDelieryType;

typedef struct{
    NGLDelieryType delivery_type;
    UINT frequency;
    USHORT reserved;
    USHORT tpid;
    union{
        NGLCableParam c;
        NGLTerristrialParam t;
        NGLSattliteParam s;
    }u;
}NGLTunerParam;

typedef struct{
    UINT frequency;
    INT strength;
    INT quality;
    INT ber;/*Bit Error Ratio (BER)*/
    INT cnr;/*Carrier-to-Noise Ratio*/
    INT mer;/*Modulation Error Ratio (MER)*/
    INT locked;      
}NGLTunerState;

typedef void(*NGLTunerCallBack)(INT tuneridx,INT lockedState,void*param);
typedef void(*NGLTunerAutoScan)(INT tuneridx,NGLTunerParam*,int count,void*userdata);
DWORD nglTunerInit();
DWORD nglTunerLock(INT tuneridx,NGLTunerParam*param);
DWORD nglTunerRegisteCBK(INT tuneridx,NGLTunerCallBack cbk,void*param);
DWORD nglTunerUnRegisteCBK(INT tuneridx,NGLTunerCallBack cbk);
DWORD nglTunerGetState(INT tuneridx,NGLTunerState*state);

DWORD nglTunerAutoScan(INT tuneridx,const NGLTunerParam*start,INT endfreq,NGLTunerAutoScan cbk,void*userdata);
DWORD nglTunerSetLNB(int tuneridx,int polarity);
//k22 0 OFF 1-ON
DWORD nglTunerSet22K(int tuneridx,INT k22);

DWORD nglSendDiseqcCommand(int tuneridx,BYTE*Command,BYTE cmdlen,BYTE diseqc_version);

END_DECLS

#endif

