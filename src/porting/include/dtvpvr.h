#ifndef __NGL_PVR_H__
#define __NGL_PVR_H__
#include<cdtypes.h>
#define PVR_MAX_AUDIO 8
#define PVR_MAX_NAME_LEN 512

BEGIN_DECLS

typedef struct{
    UINT recordMode;/*0--normal ,1--timeshifted*/
    UINT dmxid;
    USHORT video_pid;
    USHORT video_type;
    USHORT audio_pids[PVR_MAX_AUDIO];
    USHORT audio_types[PVR_MAX_AUDIO];
    USHORT ecm_pids[PVR_MAX_AUDIO];
    USHORT pcr_pid;
    USHORT pmt_pid;
    USHORT encrypt;
    char folderName[PVR_MAX_NAME_LEN];
}NGLPVR_RECORD_PARAM;

typedef enum{
   NGL_PVR_UPDATE,
   NGL_PVR_DISKFULL,
   NGL_PVR_SPEED_LOW,
   NGL_PVR_READ_FAIL,
   NGL_PVR_WRITE_FAIL,
   NGL_PVR_EVENT_MAX
}NGLPVR_EVENT;


typedef void(*PVR_CALLBACK)(DWORD handle,UINT event ,void*data);

DWORD nglPvrRecordOpen(const char*recorf_path,const NGLPVR_RECORD_PARAM*param);
DWORD nglPvrRecordClose(DWORD handle);
DWORD nglPvrRecordPause(DWORD handler);
DWORD nglPvrRecordResume(DWORD handler);
DWORD nglPvrRecordRegisterCallBack(DWORD handler,PVR_CALLBACK cbk,void*data);

DWORD nglPvrPlayerOpen(const char*pvrpath);
DWORD nglPvrPlayerPlay(DWORD handle);
DWORD nglPvrPlayerStop(DWORD handle);
DWORD nglPvrPlayerPause(DWORD handle);
DWORD nglPvrPlayerClose(DWORD handle);

END_DECLS

#endif
