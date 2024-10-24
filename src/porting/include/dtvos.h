/******************************************************************************
 
    Copyright
    This code is strictly confidential and the receiver is obliged to use it 
    exclusively for his or her own purposes. No part of Viaccess code may be 
    reproduced or transmitted in any form or by any means, electronic or 
    mechanical, including photocopying, recording, or by any information storage 
    and retrieval system, without permission in writing from Viaccess. 
    The information in this code is subject to change without notice. Viaccess 
    does not warrant that this code is error free. If you find any problems 
    with this code or wish to make comments, please report them to Viaccess.
 
    Trademarks 
    Viaccess is a registered trademark of Viaccess ?in France and/or other 
    countries. All other product and company names mentioned herein are the 
    trademarks of their respective owners.
    Viaccess may hold patents, patent applications, trademarks, copyrights 
    or other intellectual property rights over the code hereafter. Unless 
    expressly specified otherwise in a Viaccess written license agreement, the
    delivery of this code does not imply the concession of any license over 
    these patents, trademarks, copyrights or other intellectual property.
 
******************************************************************************/

/*
    $Revision: 5833 $
    $Date: 2007-05-10 17:56:04 +0200 (jeu., 10 mai 2007) $
*/



#ifndef __CD_OS_H__
#define __CD_OS_H__

#include "cdtypes.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void* OSHANDLE;
/**
 @ingroup std_drivers
 */

/**
 @defgroup osDriver VA_OS API
 @brief The functions described in this section provide mechanisms for the Viaccess-Orca ACS to manage
 memory, synchronize access to a resource between threads, process data from and to a communication
 port, and retrieve the UTC time and the processor runtime.
* @{
*/

/**
* @defgroup osConst VA_OS Constants
  @brief The VA_OS driver uses the following constants:

    In the `va_errors.h` header file:
    - @ref kVA_OK
    - @ref kVA_INVALID_PARAMETER
    - @ref kVA_ILLEGAL_HANDLE
    - @ref kVA_TIMEOUT

    In the va_os.h header file:
    - @ref kVA_WAIT_FOREVER
    - @ref kVA_NO_WAIT
    - @ref kVA_NBMAX_PERIODIC_CALLS

    In the `va_setup.h` header file:
    - @ref kVA_SETUP_SEMAPHORE_SIZE_IN_int32_t
    - @ref kVA_SETUP_MUTEX_SIZE_IN_DWORD
* @{
*/

#define kVA_NBMAX_PERIODIC_CALLS            1            /**< Nb max periodic calls */
#define kVA_WAIT_FOREVER                    0xFFFFFFFF   /**< Wait forever */
#define kVA_NO_WAIT                         0            /**< No wait */


void*nglMalloc(uint32_t uiSize);
/**
This function allocates memory blocks.
@param uiSize Size of allocated memory in bytes.
@return a pointer to the allocated space or NULL if the available memory is insufficient.

@b Comments @n
    This function must be thread-safe. This means that it can be called by two threads 
    at the same time without generating an error.
    This function must be mapped to the standard libc equivalent function.
@see VA_Free
*/
void *nglAlloc ( uint32_t uiSize);
void *nglRealloc(void*ptr,uint32_t newsize);
/**
The VA_Free function frees the memory block previously allocated by VA_Alloc.
@param ptr Indicates the address of the memory block to free.
@b Comments @n This function must be thread-safe. This means that it can be called by two threads 
      at the same time without generating an error.
      If ptr is NULL the function must return immediately without any processing.
      This function must be mapped to the standard libc equivalent function.
@see VA_Alloc
*/
void nglFree( void *ptr );

/**
Viaccess-Orca uses this function to print formatted output to a unique external communication port 
(preferably a serial port).
@param pFormat This is the format of the string to be printed. It has the same syntax as printf.
@return the number of printed characters.
@b Comments @n This function must be thread-safe.
*/
int32_t nglPrintf(const char *pFormat, ...);
int32_t nglPrintData(const char*label,const uint8_t*data,int len);

/**
This function initializes a semaphore. Semaphores are commonly used to count and limit the number 
of threads that access a shared resource.

@param pSemaphore
       Points to the semaphore to initialize.

       @param uiValue
       Indicates the initial value of the semaphore's counter. It is greater than or equal to 
       zero. It gives the initial number of available resources. These resources are acquired and 
       released using the respective functions @ref VA_AcquireSemaphore and
       @ref VA_ReleaseSemaphore.

@retval kVA_OK                 If the semaphore is successfully initialized.
@retval kVA_INVALID_PARAMETER  If pSemaphore is set to NULL.
@b Comments @n This function does not allocate memory for the semaphore's data structure. 
      It only initializes the structure's fields. 
@see VA_DeleteSemaphore VA_AcquireSemaphore VA_ReleaseSemaphore
*/

typedef OSHANDLE NGLSemaphore;
typedef OSHANDLE NGLMutex;

int32_t nglCreateSemaphore(NGLSemaphore * const pSemaphore, uint32_t uiValue);

/**
This function deletes a semaphore that was previously initialized by 
@ref VA_InitializeSemaphore.

@param pSemaphore Points to the semaphore to be deleted.

@retval kVA_OK                If the semaphore was successfully deleted.
@retval kVA_INVALID_PARAMETER If pSemaphore does not correspond to an initialized semaphore.

@b Comments @n This function does not free the memory addressed by pSemaphore. For related information, 

@see VA_InitializeSemaphore VA_AcquireSemaphore VA_ReleaseSemaphore
*/
int32_t nglDeleteSemaphore(NGLSemaphore pSemaphore);

/**
A thread calls the function VA_AcquireSemaphore to acquire a semaphore previously initialized 
by @ref VA_InitializeSemaphore. If no resource is available, the thread is suspended. As soon as a
resource is made available, the waiting thread is released and continues its execution.

@param pSemaphore
       Pointer to the semaphore to acquire.
@param uiDuration
       The value of this parameter is @ref kVA_WAIT_FOREVER. The function waits indefinitely 
       and exits only when another thread releases the semaphore.

@retval kVA_OK
        If the semaphore was successfully acquired.
@retval kVA_INVALID_PARAMETER
        Either if pSemaphore does not correspond to an initialized semaphore or if `uiDuration` does
        not fulfill the constraints detailed above.
@see @ref VA_InitializeSemaphore @ref VA_DeleteSemaphore @ref VA_ReleaseSemaphore
*/
int32_t nglAcquireSemaphore(NGLSemaphore pSemaphore, uint32_t uiDuration);


/**
This function releases the semaphore initialized by @ref VA_InitializeSemaphore. 
This mechanism enables a thread, waiting for the semaphore to be resumed.
@param pSemaphore Points to the semaphore to be released.
@retval kVA_OK
        If the semaphore is successfully released.
@retval kVA_INVALID_PARAMETER
        If pSemaphore does not correspond to an initialized semaphore.
@see VA_InitializeSemaphore VA_DeleteSemaphore VA_AcquireSemaphore
*/
int32_t nglReleaseSemaphore(NGLSemaphore pSemaphore);

/**
This function initializes a mutex. A mutex is a synchronization object that is created so that 
multiple program threads can take turns in sharing the same resource. It ensures that only one 
thread at a time can modify data or have access to a resource .

@param pMutex Points to the mutex to initialize.

@retval kVA_OK                If the initialization of the mutex is successful.
@retval kVA_INVALID_PARAMETER If pMutex is set to NULL.

@b Comments @n this function does not allocate memory for the mutex's data structure. 
      It only initializes the structure's fields.
@see VA_DeleteMutex VA_LockMutex VA_UnlockMutex
*/
int32_t nglCreateMutex(NGLMutex * const pMutex);


/**
This function deletes a mutex that was previously initialized by the 
@ref VA_InitializeMutex function.
@param pMutex Points to the mutex to delete.
@retval kVA_OK
        If mutex is successfully deleted.
@retval kVA_INVALID_PARAMETER
        If pMutex does not correspond to an initialized mutex.
@b Comments @n This function does not free the memory addressed by pMutex.
@see VA_InitializeMutex VA_LockMutex VA_UnlockMutex
*/
int32_t nglDeleteMutex (NGLMutex pMutex);


/**
This function locks a mutex. If the mutex is currently locked by another thread, the caller is 
blocked until the owner unlocks it. The thread owns the mutex when the @ref VA_LockMutex call
is successful. At that point only this thread is permitted to access the shared resource.
Mutexes are recursive. The OS records the number of times one thread locks the mutex. 
The same number of @ref VA_UnlockMutex operations must be performed before the mutex is 
unlocked.
@param pMutex Points to the mutex to lock.
@retval kVA_OK                If the mutex is successfully locked.
@retval kVA_INVALID_PARAMETER If pMutex does not correspond to an initialized mutex.
@see VA_InitializeMutex VA_DeleteMutex VA_UnlockMutex
*/
int32_t nglLockMutex (NGLMutex pMutex);


/**
This function unlocks a mutex if the calling thread is the owner. If the @ref VA_LockMutex 
function was called several times by the thread, the @ref VA_UnlockMutex function must be 
called the same number of times to free the mutex for other threads.
@param pMutex Points to the mutex to unlock.
@retval kVA_OK                If the mutex was successfully unlocked.
@retval kVA_INVALID_PARAMETER If pMutex does not correspond to an initialized mutex.
@see VA_InitializeMutex VA_DeleteMutex VA_LockMutex
*/
int32_t nglUnlockMutex (NGLMutex pMutex);


typedef void (*THREAD_PROC)(void*p);
void nglCreateThread(OSHANDLE*threadid,int p,int stacksize,THREAD_PROC proc,void*param);
void nglDeleteThread(OSHANDLE threadid);
/**
This function suspends the execution of the current thread for a specified amount 
of time.
@param uiDuration This value specifies the duration of the suspended execution in ms. 
                  If uiDuration is equal to 0, the calling thread is moved to the end of 
                  the queue by the OS scheduler and a new thread gets to run.
*/
void nglSleep( uint32_t uiDuration );

OSHANDLE nglCreateEvent(bool state, bool autoreset);//autoreset=true WaitEvent will resetstate to false

int32_t nglDestroyEvent(OSHANDLE evenid);

int32_t nglResetEvent(OSHANDLE evenid);

int32_t nglSetEvent(OSHANDLE evenid);

int32_t nglWaitEvent(OSHANDLE evenid, uint32_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* _NGL_OS_H_ */

