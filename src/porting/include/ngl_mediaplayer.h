#ifndef __NGL_MEDIA_PLAYER_H__
#define __NGL_MEDIA_PLAYER_H__
#include <cdtypes.h>

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

HANDLE nglMPOpen(const char*fname);
DWORD nglMPPlay(HANDLE handle);
DWORD nglMPStop(HANDLE handle);
DWORD nglMPClose(HANDLE handle);
DWORD nglMPResume(HANDLE handle);
DWORD nglMPPause(HANDLE handle);

DWORD nglMPGetTime(HANDLE handle,UINT*curtime,UINT*timems);
DWORD nglMPSeek(HANDLE handle,UINT timems);
DWORD nglMPSetCallback(HANDLE,MP_CALLBACK,void*userdata);
END_DECLS

#endif

