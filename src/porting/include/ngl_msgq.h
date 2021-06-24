#ifndef __CD_MSGQ_H__
#define __CD_MSGQ_H__
#include <cdtypes.h>

BEGIN_DECLS

HANDLE nglMsgQCreate(int howmany, int sizepermag);
DWORD nglMsgQDestroy(HANDLE msgq);
DWORD nglMsgQSend(HANDLE msgq, const void* pvmag, int msgsize, DWORD timeout);
DWORD nglMsgQReceive(HANDLE msgq,void* pvmag, DWORD msgsize, DWORD timeout);
DWORD nglMsgQGetCount(HANDLE msgq,UINT*count);
END_DECLS

#endif
