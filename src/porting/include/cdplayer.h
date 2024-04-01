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
int MPPlay(HANDLE handle);
int MPStop(HANDLE handle);
int MPClose(HANDLE handle);
int MPResume(HANDLE handle);
int MPPause(HANDLE handle);

int MPGetDuration(HANDLE handle,double*mediatime);
int MPGetPosition(HANDLE handle,double*position);
int MPSeek(HANDLE handle,double seektime);
int MPSetCallback(HANDLE,MP_CALLBACK,void*userdata);
int MPSetVolume(HANDLE,int colume);
int MPSetWindow(HANDLE,int x,int y,int width,int height);
int MPRotate(HANDLE handle, int type);
int MPGetStatus(HANDLE handle);
int MPFlushScreen(bool enable);

END_DECLS

#endif

