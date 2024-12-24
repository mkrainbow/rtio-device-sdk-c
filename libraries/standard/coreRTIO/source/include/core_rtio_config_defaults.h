/*
 * Copyright (c) 2023-2024 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef CORE_RTIO_CONFIG_DEFAULTS_
#define CORE_RTIO_CONFIG_DEFAULTS_

#ifdef __cplusplus
extern "C"
{
#endif

/*-----------------------------------------------------------*/
/* Currently, only support 512 bytes. */
#ifndef RTIO_TRANSFER_FRAME_BUF_SIZE 
#define RTIO_TRANSFER_FRAME_BUF_SIZE ( 512U )
#endif

/*-----------------------------------------------------------*/
#ifndef RTIO_OBGET_OBSERVATIONS_MAX 
#define RTIO_OBGET_OBSERVATIONS_MAX = ( 256U ) 
#endif

/*-----------------------------------------------------------*/

#ifndef RTIO_SEND_TIMEOUT_MS
#define RTIO_SEND_TIMEOUT_MS    ( 20000U )
#endif

#ifndef RTIO_RECV_TIMEOUT_MS
#define RTIO_RECV_TIMEOUT_MS    ( 20000U )
#endif

/*-----------------------------------------------------------*/

#ifndef RTIO_THREAD_INCOMMING_STACK_SIZE
#define RTIO_THREAD_INCOMMING_STACK_SIZE ( 4096U )
#endif

#ifndef RTIO_THREAD_KEEPALIVE_STACK_SIZE
#define RTIO_THREAD_KEEPALIVE_STACK_SIZE ( 8192U )
#endif

/*-----------------------------------------------------------*/

#define RTIO_PING_INTERVAL_MS_DEFAULT ( 300000U )
#define RTIO_PING_INTERVAL_MS_MIN ( 30000U )
#define RTIO_PING_INTERVAL_MS_MAX ( 4320000U ) /* 12 Hours*/

/* The initial heartbeat interval. */
/* Heartbeat interval can be modified by RTIO_SetHeartbeat(). */
#ifndef RTIO_PING_INTERVAL_MS_INIT
#define RTIO_PING_INTERVAL_MS_INIT RTIO_PING_INTERVAL_MS_DEFAULT
#endif

#ifndef RTIO_PING_TIMEOUT_MS
#define RTIO_PING_TIMEOUT_MS ( 5000U )
#endif

/*-----------------------------------------------------------*/

/* The maximum number of retries for connecting to server. */
#ifndef RTIO_RETRY_MAX_ATTEMPTS
#define RTIO_RETRY_MAX_ATTEMPTS            ( 5U )
#endif
/* The maximum back-off delay (in milliseconds) for retrying connection to server. */
#ifndef RTIO_RETRY_MAX_BACKOFF_DELAY_MS
#define RTIO_RETRY_MAX_BACKOFF_DELAY_MS    ( 5000U )
#endif
/* The base back-off delay (in milliseconds) to use for connection retry attempts. */
#ifndef RTIO_RETRY_BACKOFF_BASE_MS
#define RTIO_RETRY_BACKOFF_BASE_MS         ( 3000U )
#endif

/*-----------------------------------------------------------*/

#ifndef RTIO_OBSERVA_NOTIFY_TIMEOUT_MS
#define RTIO_OBSERVA_NOTIFY_TIMEOUT_MS  ( 5000U ) 
#endif



#ifndef LogError
#define LogError(message)
#endif

#ifndef LogWarn
#define LogWarn(message)
#endif

#ifndef LogInfo
#define LogInfo(message)
#endif

#ifndef LogDebug
#define LogDebug(message)
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef CORE_RTIO_CONFIG_DEFAULTS_ */
