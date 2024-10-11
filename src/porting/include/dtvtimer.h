#ifndef __NGL_TIMER_H__
#define __NGL_TIMER_H__
#include<cdtypes.h>

BEGIN_DECLS

/**
This structure gives the date as the number of seconds elapsed since 00:00:00
on January 1, 1970 (UTC).
*/
typedef unsigned long NGL_TIME ;

/**
This structure specifies the various properties of the calendar time.
*/
typedef struct{
    INT uiSec;        /**<  Gives seconds after the minute [0, 59].     */
    INT uiMin;        /**<  Gives minutes after the hour [0, 59].       */
    INT uiHour;       /**<  Gives the hour of the day [0, 23].          */
    INT uiMonthDay;   /**<  Gives the day of the month [1, 31].         */
    INT uiMonth;      /**<  Gives the month of the year [0, 11].        */
    INT uiYear;       /**<  Gives the years since 1900.                 */
    INT uiWeekDay;    /**<  Gives days from Sunday [0,6].               */
    INT uiYearDay;    /**<  Gives the days since first January [0,365]. */
} NGL_TM;


/**
This structure gives information on the processor runtime.
*/
typedef struct{
    UINT uiMilliSec;  /**< Indicates the processor runtime, in milliseconds.   */
    UINT uiMicroSec;  /**< Indicates the remaining microseconds, if available. */
} NGL_RunTime ;

/**
This function retrieves the UTC time in the @ref tVA_Time format.
The date-time returned by @ref VA_GetTime must be a secure, tamperproof time and it cannot be
changed by the STB user. This absolute date-time should be retrieved by the STB either using
information in the broadcast signal (DVB- Time and Data Table or TDT) or using information
exchanged via the return channel (Network Time Protocol or NTP), for example.
@param pTime  Indicates the address where the UTC time value must be stored in
              the @ref tVA_Time format.
@b Comments @n The memory pointed by pTime is allocated and freed by Viaccess-Orca.
      If pTime is NULL the function must return immediately without any processing.
@see tVA_Time VA_TimeToTm VA_TmToTime
*/
void nglGetTime(NGL_TIME *pTime);
DWORD nglSetTime(NGL_TIME*ptime);

/**
This function translates a time value from the @ref tVA_Time format
to the @ref tVA_Tm format.
@param pTime
       Address of the variable that contains the time value in the @ref tVA_Time format.
@param pTm
       Address of the new time value in the @ref tVA_Tm format.
@b Comments @n The memory pointed by pTime and by pTm is allocated and freed by Viaccess-Orca.
      If pTime or pTm is NULL the function must return immediately without any processing.
@see tVA_Time tVA_Tm VA_TimeToTm
*/
void nglTimeToTm( const NGL_TIME *pTime, NGL_TM *pTm );


/**
This function translates a time value from the @ref tVA_Tm format to
the @ref tVA_Time format.
@param pTm
       Address of the variable that contains the time value in the @ref tVA_Tm format.
@param pTime
       Address of the new time value in the @ref tVA_Time format.

@b Comments @n The memory pointed by pTm and by pTime is allocated and freed by Viaccess-Orca.
      If pTm or pTime is NULL the function must return immediately without any processing.
      The @ref VA_TmToTime function ignores the specified contents of the uiWeekDay and
      uiYearDay members of the @ref tVA_Tm structure. It uses the values of the other
      components to determine the calendar time ; these components must not have unnormalized
      values outside their normal ranges (for example : uiMonth = 13).
      In this case @ref tVA_Time is set to (tVA_Time)(-1).
@see tVA_Time tVA_Tm VA_TimeToTm
*/
void nglTmToTime( const NGL_TM *pTm, NGL_TIME *pTime);


/**
This function indicates the processor runtime (in @ref tVA_RunTime format) starting from the
moment the STB is switched on. For more details refer to @ref tVA_RunTime.
@param pRunTime
       Indicates the address where the runtime value must be stored in the @ref tVA_RunTime
       format.
@b Comments @n The memory pointed by pRunTime is allocated and freed by Viaccess-Orca.
      If pRunTime is NULL the function must return immediately without any processing.
      If the STB manufacturer cannot reach a precision of less than a millisecond,
      the uiMicroSec field of the variable pointed by pRunTime must be set to 0.
*/
void nglGetRunTime( NGL_RunTime *pRunTime);

///////////////////////////timer///////////////////////
typedef void(*NGL_TIMER_CB)(DWORD timerid);

typedef enum
{
        EM_TIMERMODE_REPEAT,                    ///< ÖØ¸´Ä£Ê½Ñ¡Ïî£¬¼´Ã¿Ö¸¶¨Ê±¼ä´¥·¢Ò»´Î
        EM_TIMERMODE_ONESHOT,                   ///< µ¥´ÎÄ£Ê½Ñ¡Ïî£¬¼´½öÔÚÖ¸¶¨Ê±¼äºó´¥·¢Ò»´Î
        EM_TIMERMODE_MAX                            ///< Ä£Ê½Ñ¡Ïî½áÊø±ê¼Ç£¬ÎÞÐ§²ÎÊý
}NGL_TimerMode;

DWORD NGLCreateTimer(NGL_TimerMode emode, DWORD interval, NGL_TIMER_CB fnCB);
DWORD NGLDestroyTimer(DWORD timerid);
DWORD NGLStartTimer(DWORD timerid);
DWORD NGLStopTimer(DWORD timerid);

END_DECLS

#endif

