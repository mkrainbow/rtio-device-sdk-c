/*
 * Copyright (c) 2024-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* Include Demo Config as the first non-system header. */
#include "demo_config.h"

/* OS and Transport header. */
#include "os_esp.h"
#include "network_esp.h"

/* RTIO API header. */
#include "core_rtio.h"

extern const char root_cert_auth_start[] asm("_binary_RTIORootCA_crt_start");
extern const char root_cert_auth_end[]   asm("_binary_RTIORootCA_crt_end");

/* for wifi and led control */
extern void configure_led(void);
extern void blink_led(uint8_t state);
extern bool wifi_init_sta( void );

/* Fixed resources allocated for the RTIO context. */
RTIORamAllocationGlobal_t rtioFixedRAM = { 0 };
static RTIOContextFixedResource_t rtioFixedResource = RTIO_ResourceBuild( rtioFixedRAM );

/* Information about the RTIO server. */
static ServerInfo_t serverInfo = { SERVER_HOST, SERVER_HOST_LENGTH, SERVER_PORT };


/* URI handler. */
static RTIOStatus_t uriSwitch( uint8_t* pReqData, uint16_t reqLength,
                                RTIOFixedBuffer_t* pRespbuffer, uint16_t* respLength )
{
    LogInfo( ( "Handling the copost req with uri=/switch, pReqData=%.*s.", reqLength, pReqData ) );
    char* respErr = "error param";
    char* respOK = "ok";
    if( 0 == memcmp( pReqData, "on", reqLength ) )
    {
        blink_led(1);
        *respLength = strlen( respOK );
        memcpy( pRespbuffer->pBuffer, respOK,  *respLength );
    }
    else if( 0 == memcmp( pReqData, "off", reqLength ) )
    {
        blink_led(0);
        *respLength = strlen( respOK );
        memcpy( pRespbuffer->pBuffer, respOK,  *respLength );
    }
    else
    {
        *respLength = strlen( respErr );
        memcpy( pRespbuffer->pBuffer, respErr,  *respLength );
    }
    return RTIOSuccess;
}

// void serveFailedHandler( void )
// {
//     LogInfo( ( "RTIO serve failed, restart later") );
//     esp_restart();
// }

int demo_main()
{
    int32_t exitCode = EXIT_SUCCESS;
    RTIOStatus_t rtioStatus = RTIOUnknown;
    RTIOContext_t rtioContext = { 0 };
 
    NetworkContext_t networkContext = { 0 };
    TransportInterface_t transport = { 0 };
    RTIODeviceInfo_t deviceInfo = { 0 };


    LogInfo( ( "RTIO Demo start." ) );

    /* Configure the peripheral according to the LED type */
    configure_led();


    /* Certificate and SNI for TLS connection. */
    networkContext.pcServerRootCA = root_cert_auth_start;
    networkContext.pcServerRootCASize = root_cert_auth_end - root_cert_auth_start;
    networkContext.disableSni = true;
    // networkContext.sniHostName = SERVER_HOST;


    /* The transport layer interface used by the RTIO library. */
    transport.pNetworkContext = &networkContext;
    transport.connect = xTlsConnect;
    transport.disconnect = xTlsDisconnect;
    transport.send = espTlsTransportSend;
    transport.recv = espTlsTransportRecv;

    /* Device infomation connect to RTIO server */
    deviceInfo.pDeviceId = "cfa09baa-4913-4ad7-a936-3e26f9671b10";
    deviceInfo.deviceIdLength = strlen( deviceInfo.pDeviceId );
    deviceInfo.pDeviceSecret = "mb6bgso4EChvyzA05thF9+He";
    deviceInfo.deviceSecretLength = strlen( deviceInfo.pDeviceSecret );

    LogInfo( ( "RTIO server=%.*s:%d.", (int)( serverInfo.hostNameLength ),
               serverInfo.pHostName, serverInfo.port ) );
    LogDebug( ( "The fixed RAM used by RTIO, size=%d bytes.", (unsigned)sizeof( rtioFixedRAM ) ) );

    RTIO_SetHeartbeat( &rtioContext, 30000U );
    // RTIO_SetServeFailedHandler( &rtioContext, serveFailedHandler );

    /* Attempt to connect to the RTIO server. If connection fails, retry after a timeout. */
    rtioStatus = RTIO_Connect( &rtioContext, &rtioFixedResource,
                               &transport, NULL, &serverInfo, &deviceInfo );
    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_Connect failed." ) );
        exitCode = EXIT_FAILURE;
    }

    if( exitCode == EXIT_SUCCESS )
    {
        /* Register CoPostHandler for URI: /switch */
        rtioStatus = RTIO_RegisterCoPostHandler( &rtioContext, "/switch", uriSwitch );
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
            LogError( ( "RTIO_Serve status=%d.", (int)rtioStatus ) );
            exitCode = EXIT_FAILURE;
        }
    }

    if (exitCode == EXIT_SUCCESS)
    {
        /* Do other things. */
        // LogInfo( ( "Doing other things: Demonstrate sleeping for 600 seconds." ) );
        // OS_ClockSleepMs( 600000U );

        for( int i = 0; i < INT32_MAX; i++ )
        {
            LogInfo( ("Run times: %d, Minimum free heap size: %ld bytes.", i+1, esp_get_minimum_free_heap_size()) );
            OS_ClockSleepMs(60000U);
        }
    }

    /* Disconnect from RTIO server. */
    rtioStatus = RTIO_Disconnect( &rtioContext );
    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_Disconnect status=%d.", rtioStatus ) );
        exitCode = EXIT_FAILURE;
    }

    LogInfo( ( "RTIO Demo exit exitCode=%d.", (int)exitCode ) );
    return exitCode;
}


void app_main( void )
{
    int32_t exitCode = EXIT_SUCCESS;
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if( ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND )
    {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    if( wifi_init_sta() )
    {
        exitCode = demo_main();
        printf( "demo_main exitCode=%d.\n", (int)exitCode );
        fflush( stdout );
    }
}
