/*
 * Copyright (c) 2023-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Include logging header files and define logging macros in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define the LIBRARY_LOG_NAME and LIBRARY_LOG_LEVEL macros depending on
 * the logging configuration for DEMO.
 * 3. Include the header file "logging_stack.h", if logging is enabled for DEMO.
 */

#include "logging_levels.h"

/* Logging configuration for the Demo. */
#define LIBRARY_LOG_NAME    "DEMO_RTIO_TLS"
#define LIBRARY_LOG_LEVEL    LOG_DEBUG
#include "logging_stack.h"

/******** End of logging configuration ************/

/*-----------------------------------------------------------*/

#define SERVER_HOST "localhost"
#define SERVER_PORT 17017

#ifndef SERVER_HOST
    #error "Please define an RTIO server endpoint, SERVER_HOST, in demo_config.h."
#endif
#ifndef SERVER_PORT
    #error "Please define an RTIO server endpoint, SERVER_PORT, in demo_config.h."
#endif

#define SERVER_HOST_LENGTH         ( sizeof( SERVER_HOST ) - 1 )

#ifndef ROOT_CA_CERT_PATH
    #define ROOT_CA_CERT_PATH    "certificates/RTIORootCA.crt"
#endif


#define RTIO_COPOST_URI_NUM_MAX    ( 5U )
#define RTIO_OBGET_URI_NUM_MAX    ( 5U )
#define RTIO_DEVICE_SEND_RESP_NUM_MAX    ( 5U )


#endif /* ifndef DEMO_CONFIG_H */
