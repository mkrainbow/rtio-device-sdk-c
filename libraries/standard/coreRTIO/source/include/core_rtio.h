/*
 * Copyright (c) 2023-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef CORE_RTIO_H
#define CORE_RTIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h> 
#include <stdlib.h>

#ifndef RTIO_DO_NOT_USE_CUSTOM_CONFIG
#include "core_rtio_config.h"
#endif
#include "core_rtio_config_defaults.h"

#include "transport_interface.h"
#include "os_interface.h"

#define RTIO_LIBRARY_VERSION "v0.0.1"

    /*-----------------------------------------------------------*/

   /* RTIO status codes, indicating the result of APIs. */
    typedef enum RTIOStatus
    {
        /* Base Code. */
        RTIOUnknown = -1,
        RTIOSuccess = 0,
        RTIOBadParameter = 1,
        RTIONoMemory = 2,
        RTIOMutexFailure = 3,
        RTIOTimeout = 4,
        /* About Network. */
        RTIOConnectFailedNeverRetry = 10,
        RTIOVerifyFailedNeverRetry = 11,
        RTIOTransportFailed = 12,
        RTIODisconnectFailed = 13,
        RTIOSendFailed = 14,
        RTIORecvFailed = 15,
        RTIOTransportImplementError = 16,
        /* About OS. */
        RTIOThreadCreateFailed = 20,
        RTIOThreadDestroyFailed = 21,
        /* About Protocal Implement. */
        RTIOProtocalFailed = 30,
        RTIOListFull = 31,
        RTIONotFound = 32, /* Not fount headerid from RespList*/
        /* About REST-Like Layer. */
        RTIOInternelServerError = 61,
        RTIOBadRequest = 62,
        RTIOMethodNotAllowed = 63,
        RTIOTooManyRequests = 64,
        RTIOTooManyObservers = 65,
        RTIOContinue = 66,
        RTIOTerminate = 67,
    } RTIOStatus_t;

    /* RTIO connect status. */
    enum RTIOConnectStatus
    {
        RTIOConnectInit = 0,
        RTIOConnecting = 1,
        RTIOConnected = 2,
        RTIODisconnecting = 3,
        RTIODisconnected = 4,
    };
    typedef uint8_t RTIOConnectStatus_t;

    /* A fixed buffer, containing a pointer to the buffer and its size. */
    typedef struct RTIOFixedBuffer
    {
        uint8_t* pBuffer;
        uint16_t size;
    } RTIOFixedBuffer_t;

    /*-----------------------------------------------------------*/

    /* Device information, including device ID and secret. */
    typedef struct RTIODeviceInfo
    {
        const char* pDeviceId;;
        uint16_t deviceIdLength;
        const char* pDeviceSecret;
        uint16_t deviceSecretLength;
    } RTIODeviceInfo_t;

    /*-----------------------------------------------------------*/

    typedef RTIOStatus_t( *RTIOCoPostHandler_t )( uint8_t* pReqData, uint16_t reqLength,
                                                  RTIOFixedBuffer_t* pRespbuffer, uint16_t* respLength );

    typedef struct RTIOCoPostUri
    {
        uint32_t uri;
        RTIOCoPostHandler_t handler;
    } RTIOCoPostUri_t;

    typedef struct RTIOCoPostUriList
    {
        RTIOCoPostUri_t* pList;
        uint16_t size;
    } RTIOCoPostUriList_t;

    /*-----------------------------------------------------------*/

    typedef RTIOStatus_t( *RTIOObGetHandler_t )( uint8_t* pReqData, uint16_t reqLength, uint16_t obId );

    typedef struct RTIOObGetUri
    {
        uint32_t uri;
        RTIOObGetHandler_t handler;
    } RTIOObGetUri_t;

    typedef struct RTIOObGetUriList
    {
        RTIOObGetUri_t* pList;
        uint16_t size;
    } RTIOObGetUriList_t;

    /*-----------------------------------------------------------*/

    /* Defined for internal use in RTIO */

    typedef struct rtioDeviceSendResp
    {
        uint16_t headerId;
        uint16_t respLength;
        RTIOFixedBuffer_t* pFixedBuffer; /* Copy incomming data in incommingProcess. */
        uint32_t timestampMs;
        bool arrived;
        uint8_t code; /* RTIORemoteCode_t */
    } rtioDeviceSendResp_t;

    typedef struct rtioDeviceSendRespList
    {
        rtioDeviceSendResp_t* pList;
        uint16_t size;
        OSMutex_t* pLock;
    } rtioDeviceSendRespList_t;

    uint32_t crc32Ieee( uint8_t* data, uint16_t length );

    /*-----------------------------------------------------------*/
    typedef void( *RTIOServeFailedHandler_t )( void );

    /*-----------------------------------------------------------*/

    /* Context for a RTIO connection. */
    typedef struct RTIOContext
    {
        TransportInterface_t transportInterface;
        const TransportOption_t* pTransportOption;
        const ServerInfo_t* pServerInfo;
        const RTIODeviceInfo_t* pDeviceInfo;
        RTIOServeFailedHandler_t serveFailedHandler;
        uint32_t heartbeatMs;
        uint32_t lastPacketTxTime;
        RTIOFixedBuffer_t networkIncommingBuffer; /* single-thread read, do not need lock. */
        RTIOFixedBuffer_t networkOutgoingBuffer;  /* multi-thread write, need lock. */
        OSMutex_t* pNetworkOutgoingBufferLock;    /* lock for write buffer. */
        RTIOFixedBuffer_t serverSendRespBuffer;
        RTIOCoPostUriList_t coPostInfoList;
        RTIOObGetUriList_t obGetInfoList;
        rtioDeviceSendRespList_t deviceSendRespList;
        uint16_t rollingHeaderId;
        OSMutex_t* pRollingHeaderIdLock;
        OSMutex_t* pSendMessageLock;
        OSMutex_t* pRecvMessageLock;
        OSThreadHandle_t* pThreadIncomming;
        OSThreadHandle_t* pThreadKeepAlive;
        RTIOConnectStatus_t connectStatus;
        OSMutex_t* pConnectionStatusLock;
        bool serviceDone;
    } RTIOContext_t;

#define RTIORamAllocationGlobal_t struct RTIORamAllocation \
    { \
        uint8_t buffer1[ RTIO_TRANSFER_FRAME_BUF_SIZE ]; \
        uint8_t buffer2[ RTIO_TRANSFER_FRAME_BUF_SIZE ]; \
        uint8_t buffer3[ RTIO_TRANSFER_FRAME_BUF_SIZE ]; \
        OSThreadHandle_t threads[2]; \
        OSMutex_t locks[6]; \
        RTIOCoPostUri_t coPostInfoList[ RTIO_COPOST_URI_NUM_MAX ]; \
        RTIOObGetUri_t obGetInfoList[ RTIO_OBGET_URI_NUM_MAX ] ; \
        rtioDeviceSendResp_t deviceSendRespList[ RTIO_DEVICE_SEND_RESP_NUM_MAX ]; \
    }

#define RTIO_ResourceBuild(ram) \
    { \
        .networkIncommingBuffer = {ram.buffer1, RTIO_TRANSFER_FRAME_BUF_SIZE}, \
        .networkOutgoingBuffer = {ram.buffer2, RTIO_TRANSFER_FRAME_BUF_SIZE}, \
        .serverSendRespBuffer = {ram.buffer3, RTIO_TRANSFER_FRAME_BUF_SIZE}, \
        .pThreadIncomming = &ram.threads[0], \
        .pThreadKeepAlive = &ram.threads[1], \
        .pRollingHeaderIdLock = &ram.locks[0], \
        .pSendMessageLock = &ram.locks[1], \
        .pRecvMessageLock = &ram.locks[2], \
        .pConnectionStatusLock = &ram.locks[3], \
        .pNetworkOutgoingBufferLock = &ram.locks[4], \
        .coPostUriList = {ram.coPostInfoList, RTIO_COPOST_URI_NUM_MAX}, \
        .obGetUriList = {ram.obGetInfoList, RTIO_OBGET_URI_NUM_MAX}, \
        .deviceSendRespList =  {ram.deviceSendRespList, RTIO_DEVICE_SEND_RESP_NUM_MAX, &ram.locks[5]}, \
    }

    /* Fixed resources for the RTIO connection's context. */
    typedef struct RTIOContextFixedResource
    {
        RTIOFixedBuffer_t networkIncommingBuffer;
        RTIOFixedBuffer_t networkOutgoingBuffer;
        RTIOFixedBuffer_t serverSendRespBuffer;
        OSThreadHandle_t* pThreadIncomming;
        OSThreadHandle_t* pThreadKeepAlive;
        OSMutex_t* pRollingHeaderIdLock;
        OSMutex_t* pSendMessageLock;
        OSMutex_t* pRecvMessageLock;
        OSMutex_t* pNetworkOutgoingBufferLock;
        OSMutex_t* pConnectionStatusLock;
        RTIOCoPostUriList_t coPostUriList;
        RTIOObGetUriList_t obGetUriList;
        rtioDeviceSendRespList_t deviceSendRespList;

    } RTIOContextFixedResource_t;

    /*-----------------------------------------------------------*/

    /* Connects to a remote RTIO service with the specified resources and options. */
    RTIOStatus_t RTIO_Connect( RTIOContext_t* pContext,
                               const RTIOContextFixedResource_t* pFixedResource,
                               const TransportInterface_t* pTransportInterface,
                               const TransportOption_t* pTransportOptions,
                               const ServerInfo_t* pServerInfo,
                               const RTIODeviceInfo_t* pDeviceInfo );

     /* Disconnects from the RTIO service. */
    RTIOStatus_t RTIO_Disconnect( RTIOContext_t* pContext );

    /* Registers a handler for "constrained-post" request on the specified URI. */
    RTIOStatus_t RTIO_RegisterCoPostHandler( const RTIOContext_t* pContext,
                                             const char* pUri, RTIOCoPostHandler_t handler );

    /* Registers a handler for "observe-get" request on the specified URI. */
    RTIOStatus_t RTIO_RegisterObGetHandler( const RTIOContext_t* pContext,
                                            const char* pUri, RTIOObGetHandler_t handler );

    /* Notifies the RTIO service with buffer data for the specified observer ID. */
    RTIOStatus_t RTIO_ObNotify( RTIOContext_t* pContext, uint8_t* pData, uint16_t Length,
                                uint16_t obId, uint32_t timeoutMs );

    /* Terminates the notification for the specified observer ID. */
    RTIOStatus_t RTIO_ObNotifyTerminate( RTIOContext_t* pContext,
                                         uint16_t obId,
                                         uint32_t timeoutMs );

    /* Sets the heartbeat interval for the RTIO service. */
    RTIOStatus_t RTIO_SetHeartbeat( RTIOContext_t* pContext, uint32_t heartbeatMs );

    /* Sets a handler to be invoked when the service fails. */
    RTIOStatus_t RTIO_SetServeFailedHandler( RTIOContext_t* pContext, RTIOServeFailedHandler_t handler );

    /* Serve with the given RTIO context in the background. */
    RTIOStatus_t RTIO_Serve( RTIOContext_t* pContext );

    /*-----------------------------------------------------------*/

    /* Computes the URI hash and stores the result in pDigest. */
    RTIOStatus_t RTIO_URIHash( const char* pUri, uint32_t* pDigest );

    /* Sends a "constrained-post" request to the device-service's URI with the specified data. */
    RTIOStatus_t RTIO_CoPost( RTIOContext_t* pContext, const char* pUri,
                              uint8_t* pReqData, uint16_t reqLength,
                              RTIOFixedBuffer_t* pRespbuffer, uint16_t* respLength,
                              uint32_t timeoutMs );
        
    /* Sends a "constrained-post" request using a precomputed URI hash with the specified data. */
    RTIOStatus_t RTIO_CoPostWithDigest( RTIOContext_t* pContext, uint32_t uri,
                                        uint8_t* pReqData, uint16_t reqLength,
                                        RTIOFixedBuffer_t* pRespbuffer, uint16_t* respLength,
                                        uint32_t timeoutMs );


    /*-----------------------------------------------------------*/

    /* Observers list. */
    typedef struct RTIO_ObList
    {
        uint16_t* pArray;
        OSMutex_t* pLock;
        uint16_t arraySize;
        uint16_t obNumber;
    } RTIO_ObList_t;

    /* Initializes the given observer list. */
    RTIOStatus_t RTIO_ObListInit( RTIO_ObList_t* pObList );

    /* Adds an observer ID to the given list. */
    RTIOStatus_t RTIO_ObListAdd( RTIO_ObList_t* pObList, uint16_t obId );

    /* Retrieves the number of observers in the list, not real-time. */
    uint16_t RTIO_ObListGetObNumberNotRealtime( RTIO_ObList_t* pObList );

    /* Notifies all observers in the list with the given data. */
    RTIOStatus_t RTIO_ObListNotifyAll( RTIOContext_t* pContext,
                                       RTIO_ObList_t* pObList,
                                       uint8_t* pData, uint16_t dataLength );

    /* Deinitializes the given observer list. */
    RTIOStatus_t RTIO_ObListDeInit( RTIO_ObList_t* pObList );

    /*-----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* ifndef CORE_RTIO_H */