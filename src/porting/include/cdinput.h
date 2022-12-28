#ifndef __CDROID_INPUT_H__
#define __CDROID_INPUT_H__

#include <cdtypes.h>


#define  KEY_VOLUP   KEY_VOLUMEUP
#define  KEY_VOLDOWN KEY_VOLUMEDOWN
#define  KEY_CHUP    KEY_F11
#define  KEY_CHDOWN  KEY_F12

BEGIN_DECLS

#define SOURCECLASS_BUTTON   0x01 //for Keyboard
#define SOURCECLASS_POINTER  0x02 //for Touch,Mouse
#define SOURCECLASS_POSITION 0x04 
#define SOURCECLASS_JOYSTICK 0x08
typedef struct{
    unsigned long tv_sec;
    unsigned int tv_usec;
    unsigned short type;
    unsigned short code;
    unsigned int value;
    int device;
}INPUTEVENT;//reference to linux input_event

#define MAX_DEVICE_NAME 64
#define INJECTDEV_KEY   0x101080
#define INJECTDEV_TOUCH 0x101081
#define INJECTDEV_MOUSE 0x101082
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

