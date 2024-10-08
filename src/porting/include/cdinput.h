#ifndef __CDROID_INPUT_H__
#define __CDROID_INPUT_H__

#include <cdtypes.h>

BEGIN_DECLS

typedef struct{
    unsigned long tv_sec;
    long int tv_usec;
    uint16_t type;
    uint16_t code;
    uint32_t value;
    int32_t device;
}INPUTEVENT;//reference to linux input_event

#define MAX_DEVICE_NAME 64
#define INJECTDEV_KEY   0x101080
#define INJECTDEV_TOUCH 0x101081
#define INJECTDEV_MOUSE 0x101082

#ifndef EV_ADD
   #define EV_ADD          0xFE/*used by INPUTEVENT's type, for device add*/
   #define EV_REMOVE       0xFF/*used by INPUTEVENT's type, for device remove*/
#endif

typedef struct{
    int axis;
    int minimum;
    int maximum;
    int fuzz;
    int flat;
    int resolution;
}INPUTAXISINFO;

typedef struct{
    USHORT bustype;
    USHORT vendor;
    USHORT product;
    USHORT version;
    BYTE uniqueId[64];
    BYTE keyBitMask[128];
    BYTE absBitMask[8];
    BYTE relBitMask[4];
    BYTE swBitMask [4];
    BYTE ledBitMask[4];
    BYTE propBitMask[4];
    BYTE ffBitMask [16];
    INPUTAXISINFO axis[64/*ABS_CNT*/];
    char name[MAX_DEVICE_NAME];
}INPUTDEVICEINFO;

INT InputInit();
INT InputGetEvents(INPUTEVENT*events,UINT maxevent,DWORD timeout);
INT InputInjectEvents(const INPUTEVENT*events,UINT count,DWORD timeout);
INT InputGetDeviceInfo(int device,INPUTDEVICEINFO*);

END_DECLS

#endif

