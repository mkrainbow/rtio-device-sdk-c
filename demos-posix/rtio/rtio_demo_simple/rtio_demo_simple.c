/*
 * Copyright (c) 2024-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

/* Standard includes. */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

/* URI handler. */
static RTIOStatus_t uriRainbow( uint8_t* pReqData, uint16_t reqLength,
                                   RTIOFixedBuffer_t* pRespbuffer, uint16_t* respLength )
{
    LogInfo( ( "Handling the copost req with uri=/rainbow, pReqData=%.*s.", reqLength, pReqData ) );
    const char* world = "World!";
    *respLength = strlen( world );
    memcpy( pRespbuffer->pBuffer, world, *respLength);
    return RTIOSuccess;
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
        /* Register CoPostHandler for URI: /rainbow, URI-digest: 0xd5a7abdb */
        rtioStatus = RTIO_RegisterCoPostHandler( &rtioContext, "/rainbow", uriRainbow );
        if( rtioStatus != RTIOSuccess )
        {
            LogError( ( "Register copost handler failed status=%d.", rtioStatus ) );
            exitCode = EXIT_FAILURE;
        }
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
        /* Do other things. */
        LogInfo( ( "Doing other things: Demonstrate sleeping for 600 seconds." ) );
        OS_ClockSleepMs( 600000U );
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