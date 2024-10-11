#ifndef __CD_MSGQ_H__
#define __CD_MSGQ_H__
#include <cdtypes.h>

BEGIN_DECLS

HANDLE nglMsgQCreate(int howmany, int sizepermag);
int32_t nglMsgQDestroy(HANDLE msgq);
int32_t nglMsgQSend(HANDLE msgq, const void* pvmag, uint32_t msgsize, uint32_t timeout);
int32_t nglMsgQReceive(HANDLE msgq,void* pvmag, uint32_t msgsize, uint32_t timeout);
int32_t nglMsgQGetCount(HANDLE msgq,uint32_t*count);
END_DECLS

#endif
