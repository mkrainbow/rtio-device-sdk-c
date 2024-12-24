/*
 * Copyright (c) 2023-2024 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

/* Standard includes. */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* POSIX includes. */
#include <unistd.h>

/* Include Demo Config as the first non-system header. */
#include "demo_config.h"

/* OS and Transport header. */
#include "os_posix.h"
#include "plaintext_posix.h"

/* RTIO API header. */
#include "core_rtio.h"


/* Fixed resources allocated for the RTIO context. */
RTIORamAllocationGlobal_t rtioFixedRAM = { 0 };
static RTIOContextFixedResource_t rtioFixedResource = RTIO_ResourceBuild( rtioFixedRAM );

/* Information about the RTIO server. */
static ServerInfo_t serverInfo = { SERVER_HOST, SERVER_HOST_LENGTH, SERVER_PORT };

/* Buffer receive data for application */
static uint8_t appBuffer[ RTIO_TRANSFER_FRAME_BUF_SIZE ];

static void postToServer( RTIOContext_t* pContext, RTIOFixedBuffer_t* pAppCoBuffer )
{
    RTIOStatus_t rtioStatus = RTIOUnknown;
    int i = 0;
    char* hello = "Hello, World!";
    uint16_t coPostRespLength = 0;

    LogInfo( ( "Post using function: postToServer" ) );
    for( ; i < 50; i++ )
    {
        rtioStatus = RTIO_CoPost( pContext, "/uri/example1", hello, strlen( hello ),
                                  pAppCoBuffer, &coPostRespLength, 5000 );

        if( rtioStatus != RTIOSuccess )
        {
            LogError( ( "Post failed status=%d.", rtioStatus ) );
        }
        else
        {
            LogInfo( ( "Post resp=%.*s.", coPostRespLength, pAppCoBuffer->pBuffer ) );
        }
        OS_ClockSleepMs( 1000U );
        
    }
}
static void postToServerWithDigest( RTIOContext_t* pContext, RTIOFixedBuffer_t* pAppCoBuffer )
{
    RTIOStatus_t rtioStatus = RTIOUnknown;
    int i = 0;
    char* hello = "Hello, World!";
    uint16_t coPostRespLength = 0;
    uint32_t uriDigest = 0;

    LogInfo( ( "Post using function: postToServerWithDigest" ) );

    RTIO_URIHash( "/uri/example1", &uriDigest );

    for( ; i < 50; i++ )
    {
        rtioStatus = RTIO_CoPostWithDigest( pContext, uriDigest, hello, strlen( hello ),
                                            pAppCoBuffer, &coPostRespLength, 5000 );

        if( rtioStatus != RTIOSuccess )
        {
            LogError( ( "Post failed status=%d.", rtioStatus ) );
        }
        else
        {
            LogInfo( ( "Post resp=%.*s.", coPostRespLength, pAppCoBuffer->pBuffer ) );
        }
        OS_ClockSleepMs( 1000U );
    }
}

int main()
{
    int32_t exitCode = EXIT_SUCCESS;
    RTIOStatus_t rtioStatus = RTIOUnknown;
    RTIOContext_t rtioContext = { 0 };
    PlaintextParams_t plaintextParams = { 0 };
    NetworkContext_t networkContext = { 0 };
    TransportInterface_t transport = { 0 };
    RTIODeviceInfo_t deviceInfo = { 0 };
    RTIOFixedBuffer_t appCoBuffer = { 0 };

    appCoBuffer.pBuffer = appBuffer;
    appCoBuffer.size = RTIO_TRANSFER_FRAME_BUF_SIZE;

    LogInfo( ( "RTIO Demo start... #2") );

    /* Ignore SIGPIPE. */
    signal( SIGPIPE, SIG_IGN ); 

    /* The network context for the transport layer interface. */
    networkContext.pParams = &plaintextParams;

    /* The transport layer interface used by the RTIO library. */
    transport.pNetworkContext = &networkContext;
    transport.connect = Plaintext_ConnectWithOption;
    transport.disconnect = Plaintext_Disconnect;
    transport.send = Plaintext_Send;
    transport.recv = Plaintext_Recv;

    /* Device infomation connect to RTIO server */
    deviceInfo.pDeviceId = "cfa09baa-4913-4ad7-a936-3e26f9671b10";
    deviceInfo.deviceIdLength = strlen( deviceInfo.pDeviceId );
    deviceInfo.pDeviceSecret = "mb6bgso4EChvyzA05thF9+He";
    deviceInfo.deviceSecretLength = strlen( deviceInfo.pDeviceSecret );

    LogInfo( ( "RTIO server=%.*s:%d.", (int)( serverInfo.hostNameLength ),
               serverInfo.pHostName, serverInfo.port ) );
    LogDebug( ( "The fixed RAM used by RTIO, size=%ld bytes.", sizeof( rtioFixedRAM ) ) );

    /* Attempt to connect to the RTIO server. If connection fails, retry after a timeout. */
    rtioStatus = RTIO_Connect( &rtioContext, &rtioFixedResource, &transport,
                               NULL /* No transport options */, &serverInfo, &deviceInfo );
    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_Connect failed." ) );
        exitCode = EXIT_FAILURE;
    }
    if( exitCode == EXIT_SUCCESS )
    {
        /* The device begins to provide services through URIs. */
        rtioStatus = RTIO_Serve( &rtioContext );
        if( rtioStatus != RTIOSuccess )
        {
            LogError( ( "RTIO_Serve status=%d.", rtioStatus ) );
            exitCode = EXIT_FAILURE;
        }
    }
    if( exitCode == EXIT_SUCCESS )
    {
        postToServer( &rtioContext, &appCoBuffer );
        postToServerWithDigest( &rtioContext, &appCoBuffer );
    }

    /* Disconnect from RTIO server. */
    rtioStatus = RTIO_Disconnect( &rtioContext );
    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_Disconnect status=%d.", rtioStatus ) );
        exitCode = EXIT_FAILURE;
    }

    LogInfo( ( "RTIO Demo exit exitCode=%d.", exitCode ) );
    return exitCode;
}