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
    uint16_t bustype;
    uint16_t vendor;
    uint16_t product;
    uint16_t version;
    uint8_t uniqueId[64];
    uint8_t keyBitMask[0x60];/*(KEY_CNT(0x300)+7)/8*/
    uint8_t absBitMask[8];   /*(ABS_CNT(0x40)+7)/8*/
    uint8_t relBitMask[4];   /*(REL_CNT(0x10)+7)/8*/
    uint8_t ledBitMask[4];   /*(LED_CNT(0x10)+7)/8*/
    uint8_t swBitMask [4];   /*(SW_CNT(0x11)+7)/8*/
    uint8_t propBitMask[4];  /*(INPUT_PROP_CNT(0x20)+7)/8*/
    uint8_t ffBitMask [16];  /*(FF_CNT(0x80)+7)/8*/
    INPUTAXISINFO axis[64/*ABS_CNT*/];
    char name[MAX_DEVICE_NAME];
}INPUTDEVICEINFO;

int32_t InputInit();
int32_t InputGetEvents(INPUTEVENT*events,uint32_t maxevent,uint32_t timeout);
int32_t InputInjectEvents(const INPUTEVENT*events,uint32_t count,uint32_t timeout);
int32_t InputGetDeviceInfo(int device,INPUTDEVICEINFO*);

END_DECLS

#endif

