#ifndef __BASIC_TYPE_H__
#define __BASIC_TYPE_H__
#include <cderrors.h>
#include <stdint.h>
#ifdef  __cplusplus
#define BEGIN_DECLS extern "C" {
#define END_DECLS }
#else
#define BEGIN_DECLS
#define END_DECLS
#endif

#ifndef __HANDLE
#define __HANDLE
typedef void* HANDLE;
#endif

#ifndef __BOOL 
typedef int BOOL;
#define FALSE 0
#define TRUE 1
#define __BOOL
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#endif
