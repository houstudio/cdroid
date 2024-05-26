#ifndef __MISC_API_H__
#define __MISC_API_H__
#include <cdtypes.h>
BEGIN_DECLS

int SYSInit();
int SYSSuspend();
int SYSGetSerialNo(char*sn,int max_size);
END_DECLS

#endif
