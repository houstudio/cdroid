#ifndef __NGL_DSC_H__
#define __NGL_DSC_H__
#include<cdtypes.h>
BEGIN_DECLS

typedef enum{
    eDSC_ALGO_DVB_CSA, 
    eDSC_ALGO_AES_128_CBC,
    eDSC_ALGO_DES,
    eDSC_ALGO_DVB_CSA3_STANDARD_MODE, 
    eDSC_ALGO_DVB_CSA3_MINIMALLY_ENHANCED_MODE, 
    eDSC_ALGO_DVB_CSA3_FULLY_ENHANCED_MODE, 
    eDSC_ALGO_ENUM_LAST
}ScrambleAlgo;
typedef struct{
   ScrambleAlgo algo;
   UINT32 uiIVLength;
   const BYTE*pIV;
}NGLDSC_Param;

typedef enum{
   eCM_INACTIVE, 
   eCM_SESSION, 
   eCM_LOCKED
}NGLCipherMode;

typedef struct{
   NGLCipherMode eChipsetMode;
   UINT32 uiSessionKeyLength;
   const BYTE *pSessionKey;
   BOOL cwFlag;
}NGLSCHIP_ContentKey;

DWORD nglDscOpen(USHORT* pids,UINT cnt);
DWORD nglDscClose(DWORD dwDescrambleID);
DWORD nglDscSetParameters(DWORD dwDescrambleID,const NGLDSC_Param*param);
DWORD nglDscSetKeys(DWORD  dwDescrambleID, const BYTE* pbOddKey, UINT32 bOddLength,const BYTE *pbEvenKey, UINT32 bEvenLength);


DWORD nglGetCipherMode(NGLCipherMode*md);
DWORD nglSetCipherMode(NGLCipherMode md);
DWORD nglSetCipherSessionKey(const BYTE*key,UINT keylen);
DWORD nglGetChipID();

END_DECLS

#endif
