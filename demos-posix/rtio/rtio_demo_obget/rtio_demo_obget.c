/*
 * Copyright (c) 2023-2025 mkrainbow.com.
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

/* Stop Signal and Flag for Demo. */
#include <signal.h>
bool gDemoStopFlag = false;

/* Fixed resources allocated for the RTIO context. */
RTIORamAllocationGlobal_t rtioFixedRAM = { 0 };
static RTIOContextFixedResource_t rtioFixedResource = RTIO_ResourceBuild( rtioFixedRAM );

/* Information about the RTIO server. */
static ServerInfo_t serverInfo = { SERVER_HOST, SERVER_HOST_LENGTH, SERVER_PORT };

/* Observer List Init. */
#define RAINBOW_OBSERVER_NUM_MAX    ( 5U ) /* MAX 5 Observers. */
static uint16_t pRainbowObserverList[ RAINBOW_OBSERVER_NUM_MAX ] = { 0 };
static OSMutex_t rainbowObserverListLock = { 0 };
RTIO_ObList_t rainbowObList = { pRainbowObserverList,
                                &rainbowObserverListLock,
                                RAINBOW_OBSERVER_NUM_MAX,
                                0U };
                       
/* URI handler. */
static RTIOStatus_t uriRainbow( uint8_t* pReqData, uint16_t reqLength, uint16_t obId )
{
    RTIOStatus_t status = RTIOSuccess;
    LogInfo( ( "Handling the obget req with uri=/rainbow, obId=%d, pReqData=%.*s.", obId, reqLength, pReqData ) );

    status = RTIO_ObListAdd(&rainbowObList, obId); 
    if (status == RTIOSuccess)
    {
        LogInfo(("Add obId=%d to rainbowObList.", obId));
    }
    else
    {
        LogWarn(("Add obId=%d to rainbowObList Error, status=%d.", obId, status));
    }
    return status;
}

static RTIOStatus_t notifyRainbowObservers( RTIOContext_t* pContext )
{
    RTIOStatus_t rtioStatus = RTIOSuccess;
    uint16_t obNumber = 0, obNumberPre = UINT16_MAX;
    char buf[ DEMO_RAINBOW_NOTIFY_DATALEN_MAX ] = { 0 };
    uint16_t len =  { 0 };
    uint16_t coPostRespLength = 0;
    uint16_t sequence = 0;

    rtioStatus = RTIO_ObListInit( &rainbowObList );

    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_ObListInit fxailed status=%d.", rtioStatus ) );
        return rtioStatus;
    }

    while( !gDemoStopFlag )
    {
        obNumber = RTIO_ObListGetObNumberNotRealtime( &rainbowObList );

        if( obNumber > 0 )
        {
            len = snprintf( buf, DEMO_RAINBOW_NOTIFY_DATALEN_MAX, "World! %d", sequence++ );
            RTIO_ObListNotifyAll( pContext, &rainbowObList, buf, len );
        }
        /* For log obNumber when changed. */
        if( obNumberPre != obNumber )
        {
            obNumberPre = obNumber;
            LogDebug( ( "The uri=/rainbow, obNumber=%u.", obNumber ) );
        }

        OS_ClockSleepMs( 1000U );
    }

    rtioStatus = RTIO_ObListDeInit( &rainbowObList );
    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_ObListInit failed status=%d.", rtioStatus ) );
    }

    return rtioStatus;
}

void demoStop( int )
{
    LogInfo( ( "Caught SIGINT (Ctrl+C)! Exiting...." ) );
    gDemoStopFlag = true;
}

void serveFailedHandler( void )
{
    LogInfo( ( "RTIO serve failed, Demo stop later") );
    gDemoStopFlag = true;
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
        /* This step is optional. If the SDK fails, it can callback the application, which can either restart or exit. */
        rtioStatus = RTIO_SetServeFailedHandler( &rtioContext, serveFailedHandler );
        if( rtioStatus != RTIOSuccess )
        {
            LogError( ( "RTIO_RegisterObGetHandler failed status=%d.", rtioStatus ) );
            exitCode = EXIT_FAILURE;
        }
    }
    if( exitCode == EXIT_SUCCESS )
    {
        rtioStatus = RTIO_RegisterObGetHandler( &rtioContext, "/rainbow", uriRainbow );
        if( rtioStatus != RTIOSuccess )
        {
            LogError( ( "RTIO_RegisterObGetHandler failed status=%d.", rtioStatus ) );
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
        LogInfo( ( "Doing other things: notify observers." ) );
        signal(SIGINT, demoStop);
        notifyRainbowObservers( &rtioContext );
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