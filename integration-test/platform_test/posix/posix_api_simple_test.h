/*
 * Copyright (c) 2023-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef POSIX_API_SIMPLE_TEST_H
#define POSIX_API_SIMPLE_TEST_H

#ifdef __cplusplus
extern "C"
{
#endif


/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Logging config definition and header files inclusion are required in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define the LIBRARY_LOG_NAME and LIBRARY_LOG_LEVEL macros depending on
 * the logging configuration for HTTP.
 * 3. Include the header file "logging_stack.h", if logging is enabled for HTTP.
 */

#include "logging_levels.h"

/* Logging configuration for the HTTP library. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME    "POSIX_API_SIMPLE_TEST"
#endif

#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_DEBUG
#endif

#include "logging_stack.h"

/************ End of logging configuration ****************/

#ifdef __cplusplus
}
#endif

#endif /* ifndef POSIX_API_SIMPLE_TEST_H */