/*
 * Copyright (c) 2024-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef OS__INTERFACE_H_
#define OS__INTERFACE_H_

/* Standard includes. */
#include <stdint.h>

#ifdef __cplusplus
    extern "C" {
#endif



typedef enum {	
    OSUnknown = -1,
    OSSuccess = 0,
    OSBadParameter = 1,
    OSMutexNotAcquired  = 10,
} OSError_t;

struct OSThreadHandle;
typedef struct OSThreadHandle OSThreadHandle_t;

OSError_t OS_ThreadCreate( OSThreadHandle_t * pHandle, 
                            void (*func)(void *), 
                            void * arg, 
                            const char * name, 
                            uint32_t stackSize); // stackSize for constrained systems

OSError_t OS_ThreadDestroy( OSThreadHandle_t * pHandle);


struct OSMutex;
typedef struct OSMutex OSMutex_t;

OSError_t OS_MutexCreate( OSMutex_t * pMutex );
OSError_t OS_MutexLock( OSMutex_t * pMutex );
OSError_t OS_MutexTryLock( OSMutex_t * pMutex );
OSError_t OS_MutexUnlock( OSMutex_t * pMutex );
OSError_t OS_MutexDestroy( OSMutex_t * pMutex );

uint32_t OS_ClockGetTimeMs( void );
void OS_ClockSleepMs( uint32_t sleepTimeMs );


#ifdef __cplusplus
    }
#endif

#endif /* ifndef OS__INTERFACE_H_ */