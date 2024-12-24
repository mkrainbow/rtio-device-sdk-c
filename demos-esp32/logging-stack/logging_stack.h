/*
 * AWS IoT Device SDK for Embedded C 202211.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/*
 * Modifications:
 * - Add the LOG_WITH_MORE_INFO option to include additional information in the log output, 
 *   such as color, timestamp, function name, etc.
 * Author: mkrainbow.com
 * Date of Modification: 2024-12-24
 */


#ifndef LOGGING_STACK_H_
#define LOGGING_STACK_H_

/* Include header for logging level macros. */
#include "logging_levels.h"

/* Standard Include. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_timer.h"


/* Check if LIBRARY_LOG_NAME macro has been defined. */
#if !defined( LIBRARY_LOG_NAME )
    #error "Please define LIBRARY_LOG_NAME for the library."
#endif

#define FILENAME               ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )

#if !defined( LOG_WITH_MORE_INFO )
    #define LOG_WITH_MORE_INFO 
#endif

#if defined( LOG_WITH_MORE_INFO )
    #define LOG_METADATA_FORMAT    "[%s] [\033[0;32m%d\033[0m] [\033[0;34m%s:%d %s\033[0m] " 
    #define LOG_METADATA_ARGS LIBRARY_LOG_NAME,(int)(esp_timer_get_time()/1000),FILENAME,__LINE__,__FUNCTION__
    #define LOG_METADATA_LEVEL_ERROR "\033[0;31m[ERROR]\033[0m "
    #define LOG_METADATA_LEVEL_WARN "\033[0;33m[WARN]\033[0m "
    #define LOG_METADATA_LEVEL_INFO "\033[0;32m[INFO]\033[0m "
    #define LOG_METADATA_LEVEL_DEBUG "[DEBUG] "
#else
    #define LOG_METADATA_FORMAT    "[%s] [%s:%d] " 
    #define LOG_METADATA_ARGS LIBRARY_LOG_NAME,FILENAME,__LINE__
    #define LOG_METADATA_LEVEL_ERROR "[ERROR] "
    #define LOG_METADATA_LEVEL_WARN "[WARN] "
    #define LOG_METADATA_LEVEL_INFO "[INFO] "
    #define LOG_METADATA_LEVEL_DEBUG "[DEBUG] "
#endif


#if !defined( DISABLE_LOGGING )
    #define SdkLog( string )    printf string
#else
    #define SdkLog( string )
#endif

/* Check that LIBRARY_LOG_LEVEL is defined and has a valid value. */
#if !defined( LIBRARY_LOG_LEVEL ) ||       \
    ( ( LIBRARY_LOG_LEVEL != LOG_NONE ) && \
    ( LIBRARY_LOG_LEVEL != LOG_ERROR ) &&  \
    ( LIBRARY_LOG_LEVEL != LOG_WARN ) &&   \
    ( LIBRARY_LOG_LEVEL != LOG_INFO ) &&   \
    ( LIBRARY_LOG_LEVEL != LOG_DEBUG )     \
    )
    #error "Please define LIBRARY_LOG_LEVEL as either LOG_NONE, LOG_ERROR, LOG_WARN, LOG_INFO, or LOG_DEBUG."
#else
    #if LIBRARY_LOG_LEVEL == LOG_DEBUG
        /* All log level messages will logged. */
        #define LogError( message )    SdkLog( ( LOG_METADATA_LEVEL_ERROR LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogWarn( message )     SdkLog( ( LOG_METADATA_LEVEL_WARN LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogInfo( message )     SdkLog( ( LOG_METADATA_LEVEL_INFO LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogDebug( message )    SdkLog( ( LOG_METADATA_LEVEL_DEBUG LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )

    #elif LIBRARY_LOG_LEVEL == LOG_INFO
        /* Only INFO, WARNING and ERROR messages will be logged. */
        #define LogError( message )    SdkLog( ( LOG_METADATA_LEVEL_ERROR LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogWarn( message )     SdkLog( ( LOG_METADATA_LEVEL_WARN LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogInfo( message )     SdkLog( ( LOG_METADATA_LEVEL_INFO LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogDebug( message )

    #elif LIBRARY_LOG_LEVEL == LOG_WARN
        /* Only WARNING and ERROR messages will be logged.*/
        #define LogError( message )    SdkLog( ( LOG_METADATA_LEVEL_ERROR LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogWarn( message )     SdkLog( ( LOG_METADATA_LEVEL_WARN LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogInfo( message )
        #define LogDebug( message )

    #elif LIBRARY_LOG_LEVEL == LOG_ERROR
        /* Only ERROR messages will be logged. */
        #define LogError( message )    SdkLog( (LOG_METADATA_LEVEL_ERROR LOG_METADATA_FORMAT, LOG_METADATA_ARGS ) ); SdkLog( message ); SdkLog( ( "\r\n" ) )
        #define LogWarn( message )
        #define LogInfo( message )
        #define LogDebug( message )

    #else /* if LIBRARY_LOG_LEVEL == LOG_ERROR */

        #define LogError( message )
        #define LogWarn( message )
        #define LogInfo( message )
        #define LogDebug( message )

    #endif /* if LIBRARY_LOG_LEVEL == LOG_ERROR */
#endif /* if !defined( LIBRARY_LOG_LEVEL ) || ( ( LIBRARY_LOG_LEVEL != LOG_NONE ) && ( LIBRARY_LOG_LEVEL != LOG_ERROR ) && ( LIBRARY_LOG_LEVEL != LOG_WARN ) && ( LIBRARY_LOG_LEVEL != LOG_INFO ) && ( LIBRARY_LOG_LEVEL != LOG_DEBUG ) ) */



#endif /* ifndef LOGGING_STACK_H_ */
