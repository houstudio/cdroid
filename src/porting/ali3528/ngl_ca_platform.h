#ifndef _NGL_CA_PLATFORM_H_
#define _NGL_CA_PLATFORM_H_
#include<aui_os.h>

#ifdef  __cplusplus
extern "C" {
#endif
#define MAX_FILTER_VALUE_CNT 8

typedef struct xFilter
{
	BYTE match[16];
	BYTE mask[16];
	BYTE negate[16];
	BYTE actuallen;
}MMFilter_S;

typedef enum
{
	SECTION_PRIORITY_LOW,
	SECTION_PRIORITY_MIDDLE,
	SECTION_PRIORITY_HIGH,
	SECTION_PRIORITY_UNKNOW
}SEC_PRI_E;

typedef struct __xRequestInfo
{
	DWORD dwPID;
	DWORD dwDemuxID;
	DWORD lTimeout;
	BOOL bCRCCheck;
	SEC_PRI_E enmPriority;
	void *pvAppData;
	MMFilter_S MMFilter[MAX_FILTER_VALUE_CNT];
	BYTE MMFilterCnt;
	DWORD dwReqID;
}RequestInfo;

typedef enum 
{
	SECTION_AVAIL,
	SECTION_COMPLETE,
	SECTION_REPEATED,
	SECTION_VERSION_CHANGED,
	SECTION_TIMEOUT
}SectionEventType;

typedef struct
{
	BYTE *m_pucDataBuf;
	int m_nDataLen;
}Section;

typedef void (*DMXSectionCallback_F)(SectionEventType eEvent, Section *psSection,void *pvUserData, DWORD hRequest);

typedef void (*Task_F)(void *pvParam);
typedef void (*SMCTaskCallback_F)(DWORD dwCardNumber, INT nStatus);
/**
@brief 定时器回调函数
*/
typedef void (*TimerCallback_F)(void *pvParam);

typedef enum
{
	NGL_SMC_IN,
	NGL_SMC_OUT,
	NGL_SMC_READY,
	NGL_SMC_MAX
}NGL_SMC_STATUS;


typedef struct{
	BOOL m_bIsReverseCard;
	DWORD dwSTBSoftVer;
}SmcConfig_S;

#define CS_OSP_SUCCESS (0)

typedef enum
{
	SECURED_NACHIPSET_NASECURED,
	SECURED_CHIPSET_SECURED,
	SECURED_CHIPSET_NASECURED,
	SECURED_CHIPSET_UNKNOW
}SecuredType_EM;

#define CS_SMC_SUCCRSS 0
#define CS_SMC_FAILURE (-1)
#define CS_SMC_NO_ANSWER (-2)

typedef enum
{
	TASKPRI_LOWEST,
	TASKPRI_LOW,
	TASKPRI_MIDDLE,
	TASKPRI_HIGHT,
	TASKPRI_HIGHTEST
}TaskPriority_E;

/**@brief 定时器选项*/
typedef enum
{
	EM_TIMERMODE_REPEAT,			///< 重复模式选项，即每指定时间触发一次
	EM_TIMERMODE_ONESHOT,			///< 单次模式选项，即仅在指定时间后触发一次
	EM_TIMERMODE_MAX			    ///< 模式选项结束标记，无效参数
}TimerMode_E;

typedef void (*NEWTMR_CB)(DWORD);

void NGLPrintf(const char *format, ...);


DWORD NGLCreateThread(char* name, DWORD priority, DWORD dwStackSize, Task_F fnThreadEntry, void *pvArg, DWORD *pdwThreadId);


DWORD NGLDestroyThread(DWORD dwThreadID);


DWORD NGLSuspendThread(DWORD dwThreadID);


DWORD NGLResumeThread(DWORD dwThreadID);


DWORD NGLCreateSemaphore(char* name, int initcount, int maxcount);


DWORD NGLDestorySemaphore(DWORD semid);


DWORD NGLWaitForSemaphore(DWORD semid, DWORD timeout);


DWORD NGLReleaseSemaphore(DWORD semid, DWORD howmany);


DWORD NGLCreateMsgQueue(char* name, int howmany, int sizepermag, int value);


DWORD NGLDestroyMsgQueue(DWORD msgid);


DWORD NGLSendMsg(DWORD msgid, const void* pvmag, int msgsize, DWORD timeout);
 

DWORD NGLReceiveMsg(DWORD msgid, const void* pvmag, DWORD msgsize, DWORD timeout);
 

void NGLSleep(DWORD milliseconds);


DWORD NGLNewStartTimer(char* name, TimerMode_E emode, DWORD interval, NEWTMR_CB fnCB);


void NGLNewStopTimer(DWORD* pTimerID);


DWORD NGLCreateTimer(TimerMode_E eTimeMode, TimerCallback_F fnCallback, void* pvArg, DWORD* pdwTimerId);


DWORD NGLDestroyTimer(DWORD timerid);


DWORD NGLStartTimer(DWORD timerid, DWORD milliseconds);


DWORD NGLStopTimer(DWORD timerid);


DWORD NGLCreateEvent(char* name, DWORD flag);


DWORD NGLDestroyEvent(DWORD evenid);


DWORD NGLResetEvent(DWORD evenid);


DWORD NGLSetEvent(DWORD evenid);


DWORD NGLWaitForSingleEvent(DWORD evenid, DWORD timeout);


DWORD NGLMalloc(DWORD dwSize);


DWORD NGLFree(void* paddr);


DWORD NGLGetTickCount(void);


DWORD NGLGetThreadId(void);


DWORD NGLWriteFlash(DWORD dwStartAddr, BYTE *pData, DWORD lDataLen);


DWORD NGLReadFlash(DWORD dwStartAddr, BYTE *pData, DWORD lDataLen);


DWORD NGLSMCConfig(DWORD index, int eProtocol, int eStandard, int eCaType);


void NGLSMCSetCardReaderResetDir(BOOL dir);

	
DWORD NGLSMCInit(void);


DWORD NGLSMCOpen(DWORD index, SMCTaskCallback_F fn);


DWORD NGLSMCClose(DWORD index);


DWORD NGLSMCDetectSmartCard(int index);


DWORD NGLSMCResetSmartCardEx(DWORD dwCardIndex, BYTE* aucAtr, DWORD* pdwAtrLen, BOOL bColdReset);


DWORD NGLSMCDataExchange(DWORD dwCardIndex, BYTE *pucWriteData, DWORD dwNumberToWrite, 
	BYTE *pucReadData, DWORD *pdwNumberRead, BYTE* pucStatusWord);


DWORD NGLSMCSend(DWORD dwCardIndex, BYTE* pucWriteData, 
	DWORD dwNumberToWrite, DWORD *pdwNumberWrite, DWORD dwTimeout);


DWORD NGLSMCReceive(DWORD dwCardIndex, BYTE* pucReadData, 
	DWORD dwNumberToRead, DWORD *pdwNumberRead, DWORD dwTimeout);


DWORD NGLSectionRequest(RequestInfo *pRequestInfo, DMXSectionCallback_F fnSectionCallback);


BOOL NGLSectionCancel(DWORD dwHandle);


void NGLProcessCW(BYTE* pucInputKey, BYTE cLength, BYTE* pucOutputKey, BOOL bClearCW, int param);


DWORD NGLGetSecuredType(SecuredType_EM *eType);


DWORD NGLSecureInit(void);


DWORD NGLLoadCWPK(BYTE* pucKey, DWORD nLen, int nn);

DWORD NGLMemcpy(void *dest, const void *src, UINT32  len);

DWORD NGLMemcmp(void *dest, const void *src, UINT32  len);

DWORD NGLMemset(void *dest, int c, UINT32  len);

#ifdef  __cplusplus
}
#endif

#endif //_CA_PLATFORM_H_

