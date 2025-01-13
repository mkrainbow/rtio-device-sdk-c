/*
 * Copyright (c) 2023-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef CORE_RTIO_CONFIG_H_
#define CORE_RTIO_CONFIG_H_

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Include logging header files and define logging macros in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define the LIBRARY_LOG_NAME and LIBRARY_LOG_LEVEL macros depending on
 * the logging configuration for RTIO.
 * 3. Include the header file "logging_stack.h", if logging is enabled for RTIO.
 */

#include "logging_levels.h"

 /* Logging configuration for the RTIO library. */
#ifndef LIBRARY_LOG_NAME
#define LIBRARY_LOG_NAME    "RTIO"
#endif

#ifndef LIBRARY_LOG_LEVEL
#define LIBRARY_LOG_LEVEL    LOG_DEBUG
#endif

#include "logging_stack.h"

/******** End of logging configuration ************/

/* Set the initial heartbeat interval to 60 seconds. */
/* Heartbeat interval can be modified by RTIO_SetHeartbeat(). */
/* The definition can be deleted and the default Heartbeat interval 300 seconds. */
#define RTIO_PING_INTERVAL_MS_INIT ( 60000U )

/*  Unlimited number of retry attempts. */
#define RTIO_RETRY_MAX_ATTEMPTS  ( UINT32_MAX )

#endif /* ifndef CORE_RTIO_CONFIG_H_ */
