#ifndef __DMX_READER_H__
#define __DMX_READER_H__
//#include<cdtypes.h>
BEGIN_DECLS

typedef void (*ON_TS_RECEIVED)(BYTE*tsdata,int len,void*userdata);
int get_ts_pid(BYTE*tspack);
int get_ts_unit_start(BYTE*tspack);
int get_ts_continue_count(BYTE*tspack);
int get_ts_payload(BYTE*tspack,BYTE*data,int issection);
int get_section_length(BYTE*sec);
void  StartTSReceiver(ON_TS_RECEIVED cbk,void*data);

END_DECLS
#endif
