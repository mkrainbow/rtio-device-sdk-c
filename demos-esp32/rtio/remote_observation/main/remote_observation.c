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

#include "driver/gpio.h"

/* Include Demo Config as the first non-system header. */
#include "demo_config.h"

/* OS and Transport header. */
#include "os_esp.h"
#include "network_esp.h"

/* RTIO API header. */
#include "core_rtio.h"

/* ROOT CA address. */
extern const char root_cert_auth_start[] asm("_binary_RTIORootCA_crt_start");
extern const char root_cert_auth_end[]   asm("_binary_RTIORootCA_crt_end");

/* ESP WIFI and LED control. */
extern void configure_led(void);
extern void blink_led(uint8_t state);
extern bool wifi_init_sta( void );

/* Fixed resources allocated for the RTIO context. */
RTIORamAllocationGlobal_t rtioFixedRAM = { 0 };
static RTIOContextFixedResource_t rtioFixedResource = RTIO_ResourceBuild( rtioFixedRAM );

/* Information about the RTIO server. */
static ServerInfo_t serverInfo = { SERVER_HOST, SERVER_HOST_LENGTH, SERVER_PORT };

/* Observer List Init. */
#define GPIO_OBSERVER_NUM_MAX    ( 5U ) /* MAX 5 Observers. */
static uint16_t pGPIOObserverList[ GPIO_OBSERVER_NUM_MAX ] = { 0 };
static OSMutex_t GPIOObserverListLock = { 0 };
RTIO_ObList_t GPIOObList = { pGPIOObserverList,
                                &GPIOObserverListLock,
                                GPIO_OBSERVER_NUM_MAX,
                                0U };  

/* URI handlers. */

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

static RTIOStatus_t uriGPIO( uint8_t* pReqData, uint16_t reqLength, uint16_t obId )
{
    RTIOStatus_t status = RTIOSuccess;
    LogInfo( ( "Handling the obget req with uri=/gpio, obId=%d, pReqData=%.*s.", obId, reqLength, pReqData ) );

    status = RTIO_ObListAdd(&GPIOObList, obId); 
    if (status == RTIOSuccess)
    {
        LogInfo(("Add obId=%d to GPIOObList.", obId));
    }
    else
    {
        LogWarn(("Add obId=%d to GPIOObList Error, status=%d.", obId, status));
    }
    return status;
}

static bool isPoweroffKeyPressed()
{
    if( 0 == gpio_get_level(POWEROFF_GPIO) )
    {
        OS_ClockSleepMs( 10U );
        if( 0 == gpio_get_level(POWEROFF_GPIO) )
        {
            LogInfo( ( "Poweroff key pressed." ) );
            return true;
        }
    }
    return false;
}

static bool isObservingKeyPressed()
{
    if( 0 == gpio_get_level(OBSERVING_GPIO) )
    {
        OS_ClockSleepMs( 10U );
        if( 0 == gpio_get_level(OBSERVING_GPIO) )
        {
            return true;
        }
    }
    return false;
}

static RTIOStatus_t notifyRainbowObservers( RTIOContext_t* pContext )
{
    RTIOStatus_t rtioStatus = RTIOSuccess;
    uint16_t obNumber = 0, obNumberPre = UINT16_MAX;
    uint8_t buf[ DEMO_NOTIFY_DATALEN_MAX ] = { 0 };
    uint16_t len =  { 0 };
    uint16_t sequence = 0;
    bool observingKeyStatus = false, observingKeyOldStatus = false;

    rtioStatus = RTIO_ObListInit( &GPIOObList );

    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_ObListInit fxailed status=%d.", rtioStatus ) );
        return rtioStatus;
    }

    while( !isPoweroffKeyPressed() )
    {
        obNumber = RTIO_ObListGetObNumberNotRealtime( &GPIOObList );
        observingKeyStatus = isObservingKeyPressed();
        if( observingKeyOldStatus != observingKeyStatus )
        {
            observingKeyOldStatus = observingKeyStatus;
            blink_led( (uint8_t)(( observingKeyStatus == true ? 1 : 0))  );
            if( obNumber > 0 )
            {
                len = snprintf( (char*)buf, DEMO_NOTIFY_DATALEN_MAX, "seq=%u, GPIO=[%d]",
                                sequence++, ( observingKeyStatus == true ? 0 : 1 ) );
                RTIO_ObListNotifyAll( pContext, &GPIOObList, buf, len );
            }
        }
        /* For log obNumber when changed. */
        if( obNumberPre != obNumber )
        {
            if( obNumber > obNumberPre ) 
            {
                len = snprintf( (char*)buf, DEMO_NOTIFY_DATALEN_MAX, "seq=%u, GPIO=[%d]",
                                sequence++, ( observingKeyStatus == true ? 0 : 1 ) );
                RTIO_ObListNotifyAll( pContext, &GPIOObList, buf, len );
            }
            obNumberPre = obNumber;
            LogDebug( ( "The uri=/gpio, obNumber=%u.", obNumber ) );
        }

        OS_ClockSleepMs( 20U );
    }

    rtioStatus = RTIO_ObListDeInit( &GPIOObList );
    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_ObListInit failed status=%d.", rtioStatus ) );
    }

    return rtioStatus;
}

void serveFailedHandler( void )
{
    LogInfo( ( "RTIO serve failed, restart later") );
    esp_restart();
}

int demo_main()
{
    int32_t exitCode = EXIT_SUCCESS;
    RTIOStatus_t rtioStatus = RTIOUnknown;
    RTIOContext_t rtioContext = { 0 };
 
    NetworkContext_t networkContext = { 0 };
    TransportInterface_t transport = { 0 };
    RTIODeviceInfo_t deviceInfo = { 0 };

    LogInfo( ( "RTIO Demo start." ) );
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
    deviceInfo.pDeviceId = "cfa09baa-4913-4ad7-a936-0e26f9671b06";
    deviceInfo.deviceIdLength = strlen( deviceInfo.pDeviceId );
    deviceInfo.pDeviceSecret = "mb6bgso4EChvyzA05thforESP32";
    deviceInfo.deviceSecretLength = strlen( deviceInfo.pDeviceSecret );

    LogInfo( ( "RTIO server=%.*s:%d.", (int)( serverInfo.hostNameLength ),
               serverInfo.pHostName, serverInfo.port ) );
    LogDebug( ( "The fixed RAM used by RTIO, size=%d bytes.", (unsigned)sizeof( rtioFixedRAM ) ) );

    RTIO_SetHeartbeat( &rtioContext, 30000U );
    RTIO_SetServeFailedHandler( &rtioContext, serveFailedHandler );

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
        rtioStatus = RTIO_RegisterObGetHandler( &rtioContext, "/gpio", uriGPIO );
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
            LogError( ( "RTIO_Serve status=%d.", (int)rtioStatus ) );
            exitCode = EXIT_FAILURE;
        }
    }

    if (exitCode == EXIT_SUCCESS)
    {
        /* Blink for ready. */
        blink_led(0);
        OS_ClockSleepMs( 200U );
        blink_led(1);
        OS_ClockSleepMs( 200U );
        blink_led(0);

        /* Do other things. */
        LogInfo( ( "Doing other things: notify observers." ) );
        notifyRainbowObservers( &rtioContext );
    }

    /* Blink for poweroff. */
    blink_led(0);
    OS_ClockSleepMs( 200U );
    blink_led(1);
    OS_ClockSleepMs( 200U );
    blink_led(0);
    OS_ClockSleepMs( 200U );
    blink_led(1);

    /* Disconnect from RTIO server. */
    rtioStatus = RTIO_Disconnect( &rtioContext );
    if( rtioStatus != RTIOSuccess )
    {
        LogError( ( "RTIO_Disconnect status=%d.", rtioStatus ) );
        exitCode = EXIT_FAILURE;
    }

    /* Blink for poweroff, led off. */
    blink_led(0);

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

    /* Configure the peripheral. */
    configure_led();
    /* Blink for init. */
    blink_led( 1 );

    if( wifi_init_sta() )
    {
        gpio_set_pull_mode(OBSERVING_GPIO, GPIO_PULLUP_ONLY);

        exitCode = demo_main();
        printf( "demo_main exitCode=%d.\n", (int)exitCode );
        fflush( stdout );
    }
}
