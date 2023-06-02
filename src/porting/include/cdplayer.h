#ifndef __CDMEDIA_PLAYER_H__
#define __CDMEDIA_PLAYER_H__

#include <cdtypes.h>
#include <stdbool.h>

BEGIN_DECLS

typedef enum{
   MP_PREPARED,
   MP_STARTED,
   MP_PAUSED,
   MP_SEEKED,
   MP_BUFFERING,
   MP_ERROR,
   MP_END
}MPMESSAGE;
typedef void(*MP_CALLBACK)(HANDLE,MPMESSAGE msg,LONG param,void*userdata);

HANDLE MPOpen(const char*fname);
DWORD  MPPlay(HANDLE handle);
DWORD  MPStop(HANDLE handle);
DWORD  MPClose(HANDLE handle);
DWORD  MPResume(HANDLE handle);
DWORD  MPPause(HANDLE handle);

DWORD  MPGetDuration(HANDLE handle,double*mediatime);
DWORD  MPGetPosition(HANDLE handle,double*position);
DWORD  MPSeek(HANDLE handle,double seektime);
DWORD  MPSetCallback(HANDLE,MP_CALLBACK,void*userdata);
DWORD  MPSetVolume(HANDLE,int colume);
DWORD  MPSetWindow(HANDLE,int x,int y,int width,int height);
DWORD  MPRotate(HANDLE handle, int type);
int MPGetStatus(HANDLE handle);
int MPFlushScreen(bool enable);

END_DECLS

#endif

