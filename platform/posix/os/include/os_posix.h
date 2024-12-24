/*
 * Copyright (c) 2023-2024 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */
#ifndef OS_POSIX_H_
#define OS_POSIX_H_

#ifdef POSIX_THREAD_HEADER
    #include POSIX_THREAD_HEADER
#else
    #include <pthread.h>
#endif



/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Logging related header files are required to be included in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define LIBRARY_LOG_NAME and  LIBRARY_LOG_LEVEL.
 * 3. Include the header file "logging_stack.h".
 */

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Logging configuration for the transport interface implementation which uses
 * Sockets. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "OS_POSIX"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif

#include "logging_stack.h"

/************ End of logging configuration ****************/


#ifdef __cplusplus
    extern "C" {
#endif

struct OSThreadHandle
{
    pthread_t threadId;
};

struct OSMutex
{
    pthread_mutex_t lock;
};


#include "os_interface.h"


#ifdef __cplusplus
    }
#endif


#endif /* ifndef OS_POSIX_H_ */
