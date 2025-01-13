/*
 * Copyright (c) 2023-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#include "os_esp.h"

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

    if(name == NULL) {
        LogError( ("thread name is required for xTaskCreate!\n") );
        return OSBadParameter;
    }

    if(pdPASS != xTaskCreate((void(*)(void*))func, name, stackSize, arg, 1, &pHandle->threadId))
	{
        LogError( ("xTaskCreate failed!\n") );
        return OSUnknown;
	}
    return OSSuccess;
}

OSError_t OS_ThreadDestroy( OSThreadHandle_t * pHandle )
{
    if (pHandle == NULL)
    {
        LogError( ("pHandle is Null.") );
        return OSBadParameter;
    }

    vTaskDelete(pHandle->threadId);
    LogDebug( ("vTaskDelete success, threadId=%lu.", pHandle->threadId) );
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
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
	if(NULL == mutex) 
    {
        LogError( ("xSemaphoreCreateMutex error.") );
        return OSUnknown;
	}
    pMutex->lock = mutex;
    LogDebug( ("xSemaphoreCreateMutex success.") );
    return OSSuccess;
}

OSError_t OS_MutexLock( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ( "pMutex is Null.") );
        return OSBadParameter;
    }
    if(xSemaphoreTake(pMutex->lock, portMAX_DELAY) != pdTRUE) 
    {
        LogError( ("xSemaphoreTake eorror.") );;
        return OSUnknown;
    }
    LogDebug( ("xSemaphoreTake success.") );
    return OSSuccess;
}

OSError_t OS_MutexTryLock( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ( "pMutex is Null.") );
        return OSBadParameter;
    }
    if(xSemaphoreTake(pMutex->lock, 0) != pdTRUE) 
    {
        LogError( ("xSemaphoreTake error.") );
        return OSUnknown;
    }
    LogDebug( ("xSemaphoreTake success.") );
    return OSSuccess;
}

OSError_t OS_MutexUnlock( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ( "pMutex is Null.") );
        return OSBadParameter;
    }
    if(xSemaphoreGive(pMutex->lock) != pdTRUE) 
    {
        LogError( ("xSemaphoreGive error.") );
        return OSUnknown;
    }
    LogDebug( ("xSemaphoreGive success.") );
    return OSSuccess;
}

OSError_t OS_MutexDestroy( OSMutex_t * pMutex )
{
    if(NULL == pMutex)
    {
        LogError( ("pMutex is Null.") );
        return OSBadParameter;
    }
    vSemaphoreDelete(pMutex->lock);
    LogDebug( ("vSemaphoreDelete success.") );
    return OSSuccess;
}

/*-----------------------------------------------------------*/

uint32_t OS_ClockGetTimeMs( void )
{
    int64_t timeMs = esp_timer_get_time() / 1000;
    return ( uint32_t ) timeMs;
}
void OS_ClockSleepMs( uint32_t sleepTimeMs )
{
    vTaskDelay( sleepTimeMs/portTICK_PERIOD_MS );
}

/*-----------------------------------------------------------*/

 