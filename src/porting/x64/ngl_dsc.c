#include <ngl_dsc.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <string.h>

#define INVALID_PID 0x1FFFF
typedef struct {
    int key_len;
} NGLDSC;
#define NUM_DSCS 8
static NGLDSC nglDSCS[NUM_DSCS];
#define CHECK(p) {if(p<nglDSCS||p>=&nglDSCS[NUM_DSCS])return E_INVALID_PARA;}
DWORD nglDscInit() {
    LOGD("");
    memset(nglDSCS,0,sizeof(nglDSCS));
}

DWORD nglDscOpen(USHORT *pids,UINT cnt) {
    int i;
    return (DWORD)0;
}

DWORD nglDscClose(DWORD dwDescrambleID ) {
}

DWORD nglDscSetParameters(DWORD dwStbStreamHandle,const NGLDSC_Param *param ) {
    return E_OK;
}


DWORD nglDscSetKeys(DWORD dwStbDescrHandle,const BYTE  *pOddKey,UINT32 uiOddKeyLength,
                    const BYTE  *pEvenKey,UINT32 uiEvenKeyLength) {
    return E_OK;
}

#define CHIPSET_OTP_ADDR (0)
#define KEY_OTP_ADDR	(0x03 * 4)
#define KL_KEY_OTP_SET	(1 << 23)

DWORD nglGetCipherMode(NGLCipherMode*md) {
    return E_OK;
}

DWORD nglSetCipherMode(NGLCipherMode md) {
    return E_OK;
}

DWORD nglSetCipherSessionKey(const BYTE*pSessionKey,UINT uiSessionKeyLength) {
    return E_OK;
}

DWORD nglGetChipID() {
    DWORD chipid=0000;
    return E_OK;
}

