/*
 * Copyright (c) 2023-2024 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#include <errno.h>
#include "os_posix.h"

/*-----------------------------------------------------------*/


OSError_t OS_ThreadCreate( OSThreadHandle_t * pHandle, 
                            void (*func)(void *), 
                            void * arg, 
                            const char * name, 
                            uint32_t stackSize)
{

    if(NULL == pHandle)
    {
        LogError( ( "pHandle is Null.") );
        return OSBadParameter;
    }  

    if(0 == pthread_create( &pHandle->threadId, NULL, (void *)func, arg) )
    {
        LogDebug( ("pthread_create success, threadId=%lu.", pHandle->threadId) );
        return OSSuccess;
    }
    return OSUnknown;
}

OSError_t OS_ThreadDestroy( OSThreadHandle_t * pHandle )
{
    if (pHandle == NULL)
    {
        LogError( ("pHandle is Null.") );
        return OSBadParameter;
    }

    if( 0 == pHandle->threadId )
    {
        LogWarn( ("pHandle->threadId is 0, the thread was not created.") );
        return OSBadParameter;
    }

    int ret = pthread_cancel( *((pthread_t *)pHandle->threadId) );
    if (0 != ret) 
    {
        LogError( ("pthread_cancel threadId=%lu., ret=%d.",  pHandle->threadId, ret) );
        return OSUnknown;
    } 
    pthread_join( *((pthread_t *)pHandle->threadId), NULL );
    LogDebug( ("pthread_cancel success, threadId=%lu.", pHandle->threadId) );
    return OSSuccess;
}
/*-----------------------------------------------------------*/



OSError_t OS_MutexCreate( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ( "pMutex is Null.") );
        return OSBadParameter;
    }
    int ret = pthread_mutex_init( &(pMutex->lock), NULL );
	if(0 != ret) 
    {
        LogError( ("pthread_mutex_init lock=%p, ret=%d.", &(pMutex->lock), ret) );
        return OSUnknown;
	}
    LogDebug( ("pthread_mutex_init success, lock=%p.", &(pMutex->lock)) );
    return OSSuccess;
}

OSError_t OS_MutexLock( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ( "pMutex is Null.") );
        return OSBadParameter;
    }
    int ret = pthread_mutex_lock( &(pMutex->lock) );
    if(0 != ret) 
    {
        LogError( ("pthread_mutex_lock lock=%p, ret=%d.", &(pMutex->lock), ret) );;
        return OSUnknown;
    }
    LogDebug( ("pthread_mutex_lock success, lock=%p.",&(pMutex->lock)) );
    return OSSuccess;
}
OSError_t OS_MutexTryLock( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ( "pMutex is Null.") );
        return OSBadParameter;
    }
    int ret = pthread_mutex_trylock( &(pMutex->lock) );
    if(0 != ret) 
    {
        if(EBUSY == ret)
        {
            return OSMutexNotAcquired;
        }
        LogError( ("pthread_mutex_trylock lock=%p, ret=%d.", &(pMutex->lock), ret) );
        return OSUnknown;
    }
    LogDebug( ("pthread_mutex_trylock success, lock=%p.",&(pMutex->lock)) );
    return OSSuccess;
}
OSError_t OS_MutexUnlock( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ( "pMutex is Null.") );
        return OSBadParameter;
    }
    int ret = pthread_mutex_unlock( &(pMutex->lock) );
    if(0 != ret) 
    {
        LogError( ("pthread_mutex_unlock lock=%p, ret=%d.", &(pMutex->lock), ret) );
        return OSUnknown;
    }
    LogDebug( ("pthread_mutex_unlock success, lock=%p.",&(pMutex->lock)) );
    return OSSuccess;
}
OSError_t OS_MutexDestroy( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ("pMutex is Null.") );
        return OSBadParameter;
    }
    int ret = pthread_mutex_destroy( &(pMutex->lock) );
    if(0 != ret) 
    {
        LogError( ("pthread_mutex_destroy lock=%p, ret=%d.", &(pMutex->lock), ret) );
        return OSUnknown;
    }
    LogDebug( ("pthread_mutex_destroy success, lock=%p.",&(pMutex->lock)) );
    return OSSuccess;
}

/*-----------------------------------------------------------*/
uint32_t OS_ClockGetTimeMs( void )
{
    return Clock_GetTimeMs();
}
void OS_ClockSleepMs( uint32_t sleepTimeMs )
{
    Clock_SleepMs( sleepTimeMs );
}

/*-----------------------------------------------------------*/

 