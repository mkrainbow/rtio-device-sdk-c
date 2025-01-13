/*
 * Copyright (c) 2024-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#include <assert.h>
#include "os_posix.h"

#include "posix_api_simple_test.h"

/*-----------------------------------------------------------*/
static void thread_1(void *params)
{
    (void)params;
    int i;
    for (i = 0; i < 10; i++)
    {
        LogDebug( ("Thread 1 is running=%d.", i) );
        OS_ClockSleepMs(1000);
    }
}
static void thread_2(void *params)
{
    (void)params;

    int i;
    for (i = 0; i < 10; i++)
    {
        LogDebug( ("Thread 2 is running=%d.", i) );
        OS_ClockSleepMs(1000);
    }
}
void test_OS_ThreadCreateAndDestroy()
{
    OSThreadHandle_t handle_1 = { 0 };
    OSThreadHandle_t handle_2 = { 0 };

    void * arg;
    uint32_t stackSize = 0;
    OSError_t result = OSUnknown;

    result = OS_ThreadCreate(&handle_1, thread_1, arg, "thread_1", stackSize);
    assert(result == OSSuccess);
    result = OS_ThreadCreate(&handle_2, thread_2, arg, "thread_2", stackSize);
    assert(result == OSSuccess);

    OS_ClockSleepMs(5000);

    result = OS_ThreadDestroy(&handle_1);
    assert(result == OSSuccess);
    result = OS_ThreadDestroy(&handle_2);
    assert(result == OSSuccess);
}

/*-----------------------------------------------------------*/
OSMutex_t mutext1 = { 0 };
OSThreadHandle_t tHandle11 = { 0 }, tHandle12 = { 0 };
static void thread11(void *params)
{
     OSError_t result = OSUnknown;
    (void)params;
    int i;
    for (i = 0; i < 10; i++)
    {
        LogDebug( ("Thread 11 is running=%d.", i) );
        result = OS_MutexLock(&mutext1);
        assert(result == OSSuccess);
        OS_ClockSleepMs(100);
        result = OS_MutexUnlock(&mutext1);
        assert(result == OSSuccess);
    }
}
static void thread12(void *params)
{
     OSError_t result = OSUnknown;
    (void)params;
    int i;
    for (i = 0; i < 3; i++)
    {
        LogDebug( ("Thread 12 is running=%d.", i) );
        result = OS_MutexLock(&mutext1);
        assert(result == OSSuccess);
        OS_ClockSleepMs(2000);
        result = OS_MutexUnlock(&mutext1);
        assert(result == OSSuccess);
    }
}
void test_OS_MutexCreateAndDestroy()
{
    OSError_t result = OSUnknown;
    result = OS_MutexCreate(&mutext1);
    assert(result == OSSuccess);
    result = OS_ThreadCreate(&tHandle11, thread11, NULL, "thread11", 0);
    assert(result == OSSuccess);
    result = OS_ThreadCreate(&tHandle12, thread12, NULL, "thread12", 0);
    assert(result == OSSuccess);

    OS_ClockSleepMs(10000);

    result = OS_ThreadDestroy(&tHandle11);
    assert(result == OSSuccess);
    result = OS_ThreadDestroy(&tHandle12);
    assert(result == OSSuccess);
    result = OS_MutexDestroy(&mutext1);
    assert(result == OSSuccess);
}

/*-----------------------------------------------------------*/
OSMutex_t mutext2 = { 0 };
OSThreadHandle_t tHandle21= { 0 }, tHandle22= { 0 };
static void thread21(void *params)
{
     OSError_t result = OSUnknown;
    (void)params;
    int i;
    for (i = 0; i < 1; i++)
    {
        LogDebug( ("Thread 21 is running=%d.", i) );
        result = OS_MutexLock(&mutext1);
        assert(result == OSSuccess);
        OS_ClockSleepMs(4000);
        result = OS_MutexUnlock(&mutext1);
        assert(result == OSSuccess);
    }
}
static void thread22(void *params)
{
     OSError_t result = OSUnknown;
    (void)params;
    int i;
    for (i = 0; i < 6; i++)
    {
        LogDebug( ("Thread 22 is running=%d.", i) );
        OS_ClockSleepMs(50);
        result = OS_MutexTryLock(&mutext1);
        LogDebug( ("Thread 22 is running=%d, OS_MutexTryLock ret=%d.", i, result) );
        assert ( (result == OSMutexNotAcquired)|| (result == OSSuccess) );
        OS_ClockSleepMs(1000);

        if(result == OSSuccess)
        {
            result = OS_MutexUnlock(&mutext1);
            assert(result == OSSuccess);
        }

    }
}
void test_OS_MutexTryLockTest()
{
    OSError_t result = OSUnknown;
    result = OS_MutexCreate(&mutext1);
    assert(result == OSSuccess);

    
    result = OS_ThreadCreate(&tHandle21, thread21, NULL, "thread21", 0);
    assert(result == OSSuccess);

    result = OS_ThreadCreate(&tHandle22, thread22, NULL, "thread22", 0);
    assert(result == OSSuccess);

    OS_ClockSleepMs(10000);

    result = OS_ThreadDestroy(&tHandle21);
    assert(result == OSSuccess);
    result = OS_ThreadDestroy(&tHandle22);
    assert(result == OSSuccess);
    result = OS_MutexDestroy(&mutext2);
    assert(result == OSSuccess);
}

/*-----------------------------------------------------------*/
int main( int argc,
          char ** argv )
{

    (void)argc;
    (void)argv;

    LogDebug(("Starting the POSIX API Simple Test."));

    // test_OS_ThreadCreateAndDestroy();
    // test_OS_MutexCreateAndDestroy();
    test_OS_MutexTryLockTest();

    return 0;
}

