/*
 * Copyright (c) 2023-2025 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#include "core_rtio.h"
#include "core_rtio_serializer.h"
#include "backoff_algorithm.h"

#define RTIO_PING_SERIALIZE_BUFFER_SIZE ( 7U )
#define RTIO_NOTIFY_RESP_SERIALIZE_BUFFER_SIZE  ( 8U )

/*-----------------------------------------------------------*/
static uint32_t calculateElapsedTime( uint32_t later, uint32_t start )
{
    // LogDebug( ( "calculateElapsedTime: later=%u, start=%u.", later, start ) );
    return later - start;
}

uint32_t crc32Ieee( uint8_t* data, uint16_t length ) 
{
    uint32_t crc = 0xFFFFFFFF;
    uint8_t i = 0;
    while( length-- )
    {
        crc ^= *data++;
        for( i = 0; i < 8; ++i )
        {
            crc = ( crc >> 1 ) ^ ( ( crc & 1 ) ? 0xEDB88320 : 0 );
        }
    }
    return ~crc;
}
/*-----------------------------------------------------------*/
static uint16_t getNextHeaderId( RTIOContext_t* pContext )
{
    uint16_t headerId = 0;
    OS_MutexLock( pContext->pRollingHeaderIdLock );
    pContext->rollingHeaderId++;
    if( (uint16_t)0U == (uint16_t)( pContext->rollingHeaderId ) )
    {
        pContext->rollingHeaderId = 1;
    }
    headerId = pContext->rollingHeaderId;
    OS_MutexUnlock( pContext->pRollingHeaderIdLock );
    return headerId;
}

RTIOStatus_t transRestStatus( RTIORestStatus_t restStatus )
{
    RTIOStatus_t status = RTIOUnknown;
    switch( restStatus )
    {
    case RTIO_REST_STATUS_OK:
        status = RTIOSuccess;
        break;
    case RTIO_REST_STATUS_INTERNAL_SERVER_ERROR:
        status = RTIOInternelServerError;
        break;
    case RTIO_REST_STATUS_CONTINUE:
        status = RTIOContinue;
        break;
    case RTIO_REST_STATUS_TERMINATE:
        status = RTIOTerminate;
        break;
    case RTIO_REST_STATUS_NOT_FOUNT:
        status = RTIONotFound;
        break;
    case RTIO_REST_STATUS_BAD_REQUEST:
        status = RTIOBadRequest;
        break;
    case RTIO_REST_STATUS_METHOD_NOT_ALLOWED:
        status = RTIOMethodNotAllowed;
        break;
    case RTIO_REST_STATUS_TOO_MANY_REQUESTS:
        status = RTIOTooManyRequests;
        break;
    case RTIO_REST_STATUS_TOO_MANY_OBSERVERS:
        status = RTIOTooManyObservers;
        break;
    default:
        status = RTIOUnknown;
        break;
    }
    return status;
}
/*-----------------------------------------------------------*/
void connectStatus_ChangeWhenEventConnect( RTIOContext_t* pContext )
{
    assert( pContext != NULL );
    assert( pContext->pConnectionStatusLock != NULL );
    OS_MutexLock( pContext->pConnectionStatusLock );
    switch( pContext->connectStatus )
    {
    case RTIOConnectInit:
        pContext->connectStatus = RTIOConnecting;
        LogDebug( ( "Change status: RTIOConnectInit to RTIOConnecting." ) );
        break;
    case RTIOConnecting:
    case RTIOConnected:
    case RTIODisconnecting:
    case RTIODisconnected:
    default:
        LogWarn( ( "Change connect status: currentState=%u no this Event.", pContext->connectStatus ) );
        break;
    }
    OS_MutexUnlock( pContext->pConnectionStatusLock );
}
void connectStatus_ChangeWhenEventConnectSuccess( RTIOContext_t* pContext )
{
    assert( pContext != NULL );
    assert( pContext->pConnectionStatusLock != NULL );
    OS_MutexLock( pContext->pConnectionStatusLock );
    switch( pContext->connectStatus )
    {
    case RTIOConnecting:
        pContext->connectStatus = RTIOConnected;
        LogDebug( ( "Change status: RTIOConnecting to RTIOConnected." ) );
        break;
    case RTIOConnectInit:
    case RTIOConnected:
    case RTIODisconnecting:
    case RTIODisconnected:
    default:
        LogWarn( ( "Change connect status: currentState=%u no this Event.", pContext->connectStatus ) );
        break;
    }
    OS_MutexUnlock( pContext->pConnectionStatusLock );
}
void connectStatus_ChangeWhenEventReconnect( RTIOContext_t* pContext )
{
    assert( pContext != NULL );
    assert( pContext->pConnectionStatusLock != NULL );
    OS_MutexLock( pContext->pConnectionStatusLock );
    switch( pContext->connectStatus )
    {
    case RTIOConnecting:
        LogDebug( ( "Change status: RTIOConnecting to RTIOConnecting." ) );
        break;
    case RTIOConnected:
        pContext->connectStatus = RTIOConnecting;
        LogDebug( ( "Change status: RTIOConnected to RTIOConnecting." ) );
        break;
    case RTIOConnectInit:
    case RTIODisconnecting:
    case RTIODisconnected:
    default:
        LogWarn( ( "Change connect status: currentState=%u no this Event.", pContext->connectStatus ) );
        break;
    }
    OS_MutexUnlock( pContext->pConnectionStatusLock );
}

void connectStatus_ChangeWhenEventDisconnect( RTIOContext_t* pContext )
{
    assert( pContext != NULL );
    assert( pContext->pConnectionStatusLock != NULL );
    OS_MutexLock( pContext->pConnectionStatusLock );
    switch( pContext->connectStatus )
    {
    case RTIOConnecting:
        pContext->connectStatus = RTIODisconnecting;
        LogDebug( ( "Change status: RTIOConnecting to RTIODisconnecting." ) );
        break;
    case RTIOConnected:
        pContext->connectStatus = RTIODisconnecting;
        LogDebug( ( "Change status: RTIOConnected to RTIODisconnecting." ) );
        break;
    case RTIODisconnecting:
        LogDebug( ( "Change status: RTIODisconnecting to RTIODisconnecting." ) );
        break;
    case RTIOConnectInit:
    case RTIODisconnected:
    default:
        LogWarn( ( "Change connect status: currentState=%u no this Event.", pContext->connectStatus ) );
        break;
    }
    OS_MutexUnlock( pContext->pConnectionStatusLock );
}

void connectStatus_ChangeWhenEventDisconnectSuccess( RTIOContext_t* pContext )
{
    assert( pContext != NULL );
    assert( pContext->pConnectionStatusLock != NULL );
    OS_MutexLock( pContext->pConnectionStatusLock );
    switch( pContext->connectStatus )
    {
    case RTIODisconnecting:
        LogDebug( ( "Change status: RTIODisconnecting to RTIODisconnected." ) );
        break;
    case RTIOConnecting:
    case RTIOConnected:
    case RTIOConnectInit:
    case RTIODisconnected:
    default:
        LogWarn( ( "Change connect status: currentState=%u no this Event.", pContext->connectStatus ) );
        break;
    }
    OS_MutexUnlock( pContext->pConnectionStatusLock );
}



bool connectStatus_CheckStatus( RTIOContext_t* pContext, RTIOConnectStatus_t connectStatus )
{
    assert( pContext != NULL );
    assert( pContext->pConnectionStatusLock != NULL );

    bool right = false;
    OS_MutexLock( pContext->pConnectionStatusLock );
    if( pContext->connectStatus == connectStatus )
    {
        right = true;
    }
    OS_MutexUnlock( pContext->pConnectionStatusLock );
    return right;
}

RTIOConnectStatus_t connectStatus_GetStatus( RTIOContext_t* pContext )
{
    assert( pContext != NULL );
    assert( pContext->pConnectionStatusLock != NULL );

    RTIOConnectStatus_t connectStatus = RTIOConnectInit;

    OS_MutexLock( pContext->pConnectionStatusLock );
    connectStatus = pContext->connectStatus;
    OS_MutexUnlock( pContext->pConnectionStatusLock );
    return connectStatus;
}

/*-----------------------------------------------------------*/

static RTIOStatus_t sendMessageSafe( RTIOContext_t* pContext,
                                     const uint8_t* pBufferToSend,
                                     size_t bytesToSend )
{
    RTIOStatus_t status = RTIOSuccess;

    int32_t sendResult = 0;
    uint32_t startTimeMs;
    size_t  transBytes = 0;
    const uint8_t* pIndex = pBufferToSend;

    if( ( pContext == NULL ) ||
        ( pContext->transportInterface.send == NULL ) ||
        ( pBufferToSend == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, "
                    "pContext->transportInterface.send=%p, "
                    "pBufferToSend=%p.",
                    (void*)pContext,
                    (void*)pContext->transportInterface.send,
                    (void*)pBufferToSend ) );

        status = RTIOBadParameter;
    }

    startTimeMs = OS_ClockGetTimeMs();

    OS_MutexLock( pContext->pSendMessageLock );
    while( ( transBytes < bytesToSend ) )
    {
        sendResult = pContext->transportInterface.send( pContext->transportInterface.pNetworkContext,
                                                        pIndex,
                                                        bytesToSend - transBytes );
        if( sendResult > 0 )
        {
            if( sendResult > (int32_t)bytesToSend )
            {
                LogError( ( "Transport Send implementation error,"
                            "sendResult=%d bytesToSend=%d.",
                            (int)sendResult, (int)bytesToSend ) );
                status = RTIOTransportImplementError;
            }

            transBytes += sendResult;
            pIndex = &pIndex[ sendResult ];
            pContext->lastPacketTxTime = OS_ClockGetTimeMs(); // only for reset ping time, no mutext required

            LogDebug( ( "Bytes Sent=%d, Bytes Remaining=%d.",
                        (int)sendResult,
                        (int)( bytesToSend - transBytes ) ) );

        }
        else if( sendResult < 0 )
        {
            status = RTIOSendFailed;
            LogError( ( "Unable to send packet: Network Error, sendResult=%d.", (int)sendResult ) );
            break;
        }
        else
        {
            /* MISRA Empty body */
        }

        if( calculateElapsedTime( OS_ClockGetTimeMs(), startTimeMs ) >= RTIO_SEND_TIMEOUT_MS )
        {
            status = RTIOTimeout;
            LogError( ( "Unable to send packet: Timed out." ) );
            break;
        }
    }
    OS_MutexUnlock( pContext->pSendMessageLock );

    return status;
}

static RTIOStatus_t recvMessageSafe( RTIOContext_t* pContext,
                                     uint8_t* pBufferToRecv,
                                     uint32_t bytesToRecv )
{
    RTIOStatus_t status = RTIOSuccess;

    int32_t recvResult = 0;
    uint32_t startTimeMs;
    uint32_t transBytes = 0;
    uint8_t* pIndex = pBufferToRecv;

    if( ( pContext == NULL ) ||
        ( pContext->transportInterface.recv == NULL ) ||
        ( pBufferToRecv == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, "
                    "pContext->transportInterface.recv=%p, "
                    "pBufferToRecv=%p.",
                    (void*)pContext,
                    (void*)pContext->transportInterface.recv,
                    (void*)pBufferToRecv ) );

        status = RTIOBadParameter;
    }

    startTimeMs = OS_ClockGetTimeMs();

    OS_MutexLock( pContext->pRecvMessageLock );
    while( ( transBytes < (int32_t)bytesToRecv ) )
    {
        recvResult = pContext->transportInterface.recv( pContext->transportInterface.pNetworkContext,
                                                        pIndex,
                                                        bytesToRecv - transBytes );

        if( recvResult > 0 )
        {
            if( recvResult > (int32_t)bytesToRecv )
            {
                LogError( ( "Transport Recv implementation error, recvResult=%d bytesToRecv=%d.",
                            (int)recvResult, (int)bytesToRecv ) );
                status = RTIOTransportImplementError;
            }
            transBytes += recvResult;
            pIndex = &pIndex[ recvResult ];

            LogDebug( ( "Bytes Recv=%d, Bytes Remaining=%d.",
                        (int)recvResult, 
                        (int)( bytesToRecv - transBytes ) ) );
        }
        else if( recvResult < 0 )
        {
            status = RTIORecvFailed;
            LogError( ( "Unable to recv packet: Network Error, recvResult=%d.", (int)recvResult ) );
            break;
        }
        else
        {
            /* MISRA Empty body */
        }

        if( calculateElapsedTime( OS_ClockGetTimeMs(), startTimeMs ) >= RTIO_RECV_TIMEOUT_MS )
        {
            status = RTIOTimeout;
            break;
        }
    }
    OS_MutexUnlock( pContext->pRecvMessageLock );
    return status;
}

/**
 * @brief Connect to server.
 *
 * @param[in] pContext context pointer.
 *
 * @return RTIOSuccess，RTIOBadParameter, RTIOTransportFailed, RTIOVerifyFailedNeverRetry and other RTIOStatus_t.
 */
static RTIOStatus_t connectRTIOServer( RTIOContext_t* pContext )
{
    TransportStatus_t transportStatus = TransportUnknown;
    RTIOStatus_t status = RTIOSuccess;
    RTIOVerifyReq_t verifyReq = { 0 };
    RTIOVerifyResp_t verifyResp = { 0 };
    uint16_t dataLen = 0U;

    if( ( pContext == NULL ) ||
        ( pContext->transportInterface.connect == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, "
                    "transportInterface.connect=%p.",
                    (void*)pContext,
                    (void*)pContext->transportInterface.connect ) );

        return RTIOBadParameter;
    }

    if( pContext->transportInterface.send == NULL )
    {
        LogError( ( "Argument transportInterface.send is NULL." ) );
        return RTIOBadParameter;
    }
    if( pContext->transportInterface.recv == NULL )
    {
        LogError( ( "Argument transportInterface->recv is NULL." ) );
        return RTIOBadParameter;
    }
    if( pContext->transportInterface.pNetworkContext == NULL )
    {
        LogError( ( "Argument transportInterface->pNetworkContext is NULL." ) );
        return RTIOBadParameter;
    }
    if( pContext->pServerInfo == NULL )
    {
        LogError( ( "Argument pServerInfo is NULL." ) );
        return RTIOBadParameter;
    }

    transportStatus = pContext->transportInterface.connect(
        pContext->transportInterface.pNetworkContext,
        pContext->pTransportOption,
        pContext->pServerInfo );

    if( transportStatus != TransportSuccess )
    {
        LogError( ( "Transport connect error, transportStatus=%d.", transportStatus ) );
        status = RTIOTransportFailed;
    }

    if( RTIOSuccess == status )
    {
        verifyReq.header.version = RTIO_PROTOCAL_VERSION;
        verifyReq.header.type = RTIO_TYPE_DEVICE_VERIFY_REQ;
        verifyReq.header.id = getNextHeaderId( pContext );
        verifyReq.pDeviceId = pContext->pDeviceInfo->pDeviceId;
        verifyReq.pDeviceSecret = pContext->pDeviceInfo->pDeviceSecret;
        verifyReq.header.bodyLen = pContext->pDeviceInfo->deviceIdLength + pContext->pDeviceInfo->deviceSecretLength + 2;
        verifyReq.capLevel = RTIO_CAP_CURRENT_LEVEL;
        status = RTIO_SerializeVerifyReq( &verifyReq, &pContext->networkOutgoingBuffer, &dataLen );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to serialize verify request." ) );
        }
        else
        {
            status = sendMessageSafe( pContext, pContext->networkOutgoingBuffer.pBuffer, dataLen );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to send verify request, status=%d.", status ) );
            }
        }
    }

    if( RTIOSuccess == status )
    {
        status = recvMessageSafe( pContext, pContext->networkIncommingBuffer.pBuffer, RTIO_PROTOCAL_HEADER_LEN );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to recv verify response, status=%d.", status ) );
        }
        else
        {
            status = RTIO_DeserializeVerifyResp( &pContext->networkIncommingBuffer, &verifyResp );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to deserial response, status=%d.", status ) );
            }
            else
            {
                if( verifyResp.header.code == REMOTECODE_SUCCESS )
                {
                    LogInfo( ( "Device verification successful." ) );
                    status = RTIOSuccess;
                }
                else
                {
                    LogError( ( "Failed to verify device, code=%u.", verifyResp.header.code ) );
                    status = RTIOVerifyFailedNeverRetry;
                }
            }
        }
    }

    if( RTIOSuccess != status )
    {
        pContext->transportInterface.disconnect( pContext->transportInterface.pNetworkContext );
    }
    return status;
}
/*-----------------------------------------------------------*/

static RTIOStatus_t deviceSendRespList_Add( rtioDeviceSendRespList_t* pRespList,
                                            uint16_t headerId,
                                            RTIOFixedBuffer_t* pRespBuffer,
                                            uint16_t* pIndex )
{
    RTIOStatus_t status = RTIOSuccess;


    OS_MutexLock( pRespList->pLock );

    *pIndex = 0;
    for( ; *pIndex < pRespList->size; ( *pIndex )++ )
    {
        if( pRespList->pList[ *pIndex ].headerId == 0 )
        {
            pRespList->pList[ *pIndex ].headerId = headerId;
            pRespList->pList[ *pIndex ].pFixedBuffer = pRespBuffer;
            pRespList->pList[ *pIndex ].arrived = false;
            pRespList->pList[ *pIndex ].timestampMs = OS_ClockGetTimeMs();
            break;
        }
    }
    if( *pIndex == pRespList->size )
    {
        status = RTIOListFull;
    }

    OS_MutexUnlock( pRespList->pLock );

    if( status == RTIOSuccess )
    {
        LogDebug( ( "Add: headerId=%u, index=%u.", headerId, *pIndex ) );
    }

    return status;
}

static RTIOStatus_t deviceSendRespList_Delete( rtioDeviceSendRespList_t* pRespList, uint16_t index )
{
    if( ( pRespList == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pList=%p, index=%u.", (void*)pRespList, index ) );
        return RTIOBadParameter;
    }

    if( index >= pRespList->size )
    {
        LogWarn( ( "Invalid index=%u, pRespList->size=%u, ignore.", index, pRespList->size) );
        return RTIOSuccess;
    }

    OS_MutexLock( pRespList->pLock );
    pRespList->pList[ index ].headerId = 0;
    pRespList->pList[ index ].code = 0;
    pRespList->pList[ index ].respLength = 0;
    pRespList->pList[ index ].pFixedBuffer = NULL;
    pRespList->pList[ index ].arrived = false;
    pRespList->pList[ index ].timestampMs = 0;
    OS_MutexUnlock( pRespList->pLock );

    return RTIOSuccess;
}

static RTIOStatus_t deviceSendRespList_Wait( const rtioDeviceSendRespList_t* pRespList, uint16_t index, uint16_t timeoutMs )
{
    if( ( pRespList == NULL ) || ( index >= pRespList->size ) )
    {
        LogError( ( "Argument cannot be NULL: pList=%p, index=%u.", (void*)pRespList, index ) );
        return RTIOBadParameter;
    }

    // wait for resp arrived or timeout, no lock required:
    // because every deviceSendRespListWait read a different index
    // only incomming thread trigger the arrived flag
    while( pRespList->pList[ index ].arrived == false )
    {
        OS_ClockSleepMs( 50U );
        if( calculateElapsedTime( OS_ClockGetTimeMs(), pRespList->pList[ index ].timestampMs )
            >= timeoutMs )
        {
            return RTIOTimeout; /* TODO: replace with RTIOWaitRespTimeout, #49 */
        }
    }
    return RTIOSuccess;
}

static RTIOStatus_t deviceSendRespList_Find( const rtioDeviceSendRespList_t* pRespList, uint16_t headerId, uint16_t* pIndex )
{
    if( ( pRespList == NULL ) || ( pIndex == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pList=%p, pIndex=%p.", (void*)pRespList, (void*)pIndex ) );
        return RTIOBadParameter;
    }

    *pIndex = 0;
    for( ; *pIndex < pRespList->size; ( *pIndex )++ )
    {
        if( pRespList->pList[ *pIndex ].headerId == headerId )
        {
            break;
        }
    }
    if( *pIndex == pRespList->size )
    {
        *pIndex = pRespList->size;
        return RTIONotFound;
    }

    return RTIOSuccess;
}
static RTIOStatus_t deviceSendRespList_Ready( const rtioDeviceSendRespList_t* pRespList, uint16_t index )
{
    if( ( pRespList == NULL ) || ( index >= pRespList->size ) )
    {
        LogError( ( "Argument cannot be NULL: pList=%p, index=%u.", (void*)pRespList, index ) );
        return RTIOBadParameter;
    }
    // no lock required:
    // because every deviceSendRespListArrived trigger different arrived flag
    rtioDeviceSendResp_t* pResp = &( pRespList->pList[ index ] );
    pResp->arrived = true;
    return RTIOSuccess;
}

static RTIOStatus_t deviceSendRespList_GetResp( const rtioDeviceSendRespList_t* pRespList, uint16_t index, rtioDeviceSendResp_t** ppResp )
{
    if( ( pRespList == NULL ) || ( index >= pRespList->size ) )
    {
        LogError( ( "Argument cannot be NULL: pList=%p, index=%u.", (void*)pRespList, index ) );
        return RTIOBadParameter;
    }
    *ppResp = &( pRespList->pList[ index ] );
    return RTIOSuccess;
}

static RTIOStatus_t handleCoPostRequest( RTIOContext_t* pContext, RTIOCoReq_t* pReq )
{
    RTIOStatus_t status = RTIOUnknown;
    RTIOCoResp_t resp = { 0 };
    uint16_t serianlizeLength = 0;

    if( pReq == NULL )
    {
        LogError( ( "Argument cannot be NULL: pReq=%p.", (void*)pReq ) );
        return RTIOBadParameter;
    }

    RTIOCoPostHandler_t handler = { 0 };
    int i = 0;
    for( ; i < pContext->coPostInfoList.size; i++ )
    {
        if( pContext->coPostInfoList.pList[ i ].uri == pReq->uri )
        {
            handler = pContext->coPostInfoList.pList[ i ].handler;
            break;
        }
    }


    resp.headerId = pReq->headerId;
    resp.method = pReq->method;

    if( i == pContext->coPostInfoList.size )
    {
        LogWarn( ( "Handler not found, uri=%u.", (unsigned)pReq->uri ) );
        resp.code = RTIO_REST_STATUS_NOT_FOUNT;
    }
    else
    {
        if( NULL != handler )
        {
            status = handler( pReq->pData, pReq->dataLength,
                              &( pContext->serverSendRespBuffer ), &resp.dataLength );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to handler, status=%d.", status ) );
                resp.code = RTIO_REST_STATUS_INTERNAL_SERVER_ERROR;
            }
            else
            {
                resp.pData = pContext->serverSendRespBuffer.pBuffer;
                resp.code = RTIO_REST_STATUS_OK;
            }
        }
        else
        {
            LogError( ( "Handler is NULL, index=%d handler=%p.", i, (void*)handler ) );
            resp.code = RTIO_REST_STATUS_NOT_FOUNT;
        }
    }

    OS_MutexLock( pContext->pNetworkOutgoingBufferLock );
    status = RTIO_SerializeCoResp_OverServerSendResp( &resp, &( pContext->networkOutgoingBuffer ), &serianlizeLength );
    if( status != RTIOSuccess )
    {
        LogError( ( "Failed to SerializeCoResp, status=%d.", status ) );
    }
    else
    {
        status = sendMessageSafe( pContext, pContext->networkOutgoingBuffer.pBuffer, serianlizeLength );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to send CoResp, status=%d.", status ) );
        }
        else
        {
            status = RTIOSuccess;
        }
    }
    OS_MutexUnlock( pContext->pNetworkOutgoingBufferLock );

    return status;

}

static RTIOStatus_t handleObEstabRequest( RTIOContext_t* pContext, RTIOObEstabReq_t* pReq )
{
    RTIOStatus_t status = RTIOUnknown;
    RTIOObEstabResp_t resp = { 0 };
    uint16_t serianlizeLength = 0;

    if( pReq == NULL )
    {
        LogError( ( "Argument cannot be NULL: pReq=%p.", (void*)pReq ) );
        return RTIOBadParameter;
    }

    RTIOObGetHandler_t handler = { 0 };
    int i = 0;
    for( ; i < pContext->obGetInfoList.size; i++ )
    {
        if( pContext->obGetInfoList.pList[ i ].uri == pReq->uri )
        {
            handler = pContext->obGetInfoList.pList[ i ].handler;
            break;
        }
    }

    resp.headerId = pReq->headerId;
    resp.method = pReq->method;
    resp.obId = pReq->obId;

    if( i == pContext->obGetInfoList.size )
    {
        LogError( ( "Handler not found, uri=%u.", (unsigned)pReq->uri ) );
        resp.code = RTIO_REST_STATUS_NOT_FOUNT;
    }
    else
    {
        if( NULL != handler )
        {
            status = handler( pReq->pData, pReq->dataLength, pReq->obId );
            if( status != RTIOSuccess )
            {
                if( status == RTIOListFull )
                {
                    LogWarn( ( "This uri too many observers, uri=%u , status=%u.", (unsigned)pReq->uri, (unsigned)status ) );
                    resp.code = RTIO_REST_STATUS_TOO_MANY_OBSERVERS;
                }
                else
                {
                    LogError( ( "Failed to handler, status=%d.", status ) );
                    resp.code = RTIO_REST_STATUS_INTERNAL_SERVER_ERROR;
                }
            }
            else
            {
                resp.code = RTIO_REST_STATUS_CONTINUE;
            }
        }
        else
        {
            LogError( ( "Handler is NULL, index=%d handler=%p.", i, (void*)handler ) );
            resp.code = RTIO_REST_STATUS_NOT_FOUNT;
        }
    }

    OS_MutexLock( pContext->pNetworkOutgoingBufferLock );
    status = RTIO_SerializeObEstabResp_OverServerSendResp( &resp, &( pContext->networkOutgoingBuffer ), &serianlizeLength );
    if( status != RTIOSuccess )
    {
        LogError( ( "Failed to SerializeObResp, status=%d.", status ) );
    }
    else
    {
        status = sendMessageSafe( pContext, pContext->networkOutgoingBuffer.pBuffer, serianlizeLength );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to send ObResp, status=%d.", status ) );
        }
        else
        {
            status = RTIOSuccess;
        }
    }
    OS_MutexUnlock( pContext->pNetworkOutgoingBufferLock );
    return status;
}


static RTIOStatus_t handleDevicePingResp( RTIOContext_t* pContext, RTIOHeader_t* pHeader )
{
    rtioDeviceSendResp_t* pResp = NULL;
    uint16_t index = 0;
    RTIOStatus_t  status;

    if( pContext == NULL || pHeader == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, pHeader=%p.", (void*)pContext, (void*)pHeader ) );
        return RTIOBadParameter;
    }

    status = recvMessageSafe( pContext, pContext->networkIncommingBuffer.pBuffer, pHeader->bodyLen );
    if( status != RTIOSuccess )
    {
        LogError( ( "Failed to recv body, status=%d.", status ) );
    }
    else
    {
        status = deviceSendRespList_Find( &( pContext->deviceSendRespList ), pHeader->id, &index );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to deviceSendRespListFind, status=%d, hearderId=%u.", status, pHeader->id ) );
        }
        else
        {

            status = deviceSendRespList_GetResp( &( pContext->deviceSendRespList ), index, &pResp );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to deviceSendRespListFind, status=%d.", status ) );
            }
            else
            {
                status = RTIO_DeSerializeDevicePingResp( pHeader,
                                                         &( pContext->networkIncommingBuffer ),
                                                         pResp );
                if( status != RTIOSuccess )
                {
                    LogError( ( "Failed to DeSerializeDeviceSendResp, status=%d.", status ) );
                }
                else
                {
                    status = deviceSendRespList_Ready( &( pContext->deviceSendRespList ), index );
                    if( status != RTIOSuccess )
                    {
                        LogError( ( "Failed to deviceSendRespListArrived, status=%d.", status ) );
                    }
                }
            }
        }
    }
    return status;
}

static RTIOStatus_t handleDeviceSendResp( RTIOContext_t* pContext, RTIOHeader_t* pHeader )
{
    rtioDeviceSendResp_t* pResp = NULL;
    uint16_t index = 0;
    RTIOStatus_t  status;

    if( pContext == NULL || pHeader == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, pHeader=%p.", (void*)pContext, (void*)pHeader ) );
        return RTIOBadParameter;
    }

    status = recvMessageSafe( pContext, pContext->networkIncommingBuffer.pBuffer, pHeader->bodyLen );
    if( status != RTIOSuccess )
    {
        LogError( ( "Failed to recv body, status=%d.", status ) );
    }
    else
    {
        status = deviceSendRespList_Find( &( pContext->deviceSendRespList ), pHeader->id, &index );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to deviceSendRespListFind, status=%d.", status ) );
        }
        else
        {
            status = deviceSendRespList_GetResp( &( pContext->deviceSendRespList ), index, &pResp );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to deviceSendRespListFind, status=%d.", status ) );
            }
            else
            {
                status = RTIO_DeSerializeDeviceSendResp( pHeader,
                                                         &( pContext->networkIncommingBuffer ),
                                                         pResp );
                if( status != RTIOSuccess )
                {
                    LogError( ( "Failed to DeSerializeDeviceSendResp, status=%d.", status ) );
                }
                else
                {
                    status = deviceSendRespList_Ready( &( pContext->deviceSendRespList ), index );
                    if( status != RTIOSuccess )
                    {
                        LogError( ( "Failed to deviceSendRespListArrived, status=%d.", status ) );
                    }
                }
            }
        }
    }
    return status;
}

static RTIOStatus_t handleCoPost( RTIOContext_t* pContext, RTIOHeader_t* pHeader )
{
    RTIOCoReq_t req = { 0 };
    RTIOStatus_t  status = RTIO_DeSerializeCoReqNoCopy( pHeader->id,
                                                        pContext->networkIncommingBuffer.pBuffer, 
                                                        pHeader->bodyLen, &req );
    if( status != RTIOSuccess )
    {
        LogError( ( "Failed to SerializeCoReq, status=%d.", status ) );
    }
    else
    {
        status = handleCoPostRequest( pContext, &req );
    }
    return status;
}
static RTIOStatus_t handleObGet( RTIOContext_t* pContext, RTIOHeader_t* pHeader )
{
    RTIOObEstabReq_t req = { 0 };
    RTIOStatus_t status = RTIO_DeSerializeObEstabReqNoCopy( pHeader->id,
                                                            pContext->networkIncommingBuffer.pBuffer,
                                                            pHeader->bodyLen, &req );
    if( status != RTIOSuccess )
    {
        LogError( ( "Failed to SerializeObReq, status=%d.", status ) );
    }
    else
    {
        status = handleObEstabRequest( pContext, &req );
    }

    return status;
}
static RTIOStatus_t handleServerSendReqest( RTIOContext_t* pContext, RTIOHeader_t* pHeader )
{
    if( pContext == NULL || pHeader == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, pHeader=%p.", (void*)pContext, (void*)pHeader ) );
        return RTIOBadParameter;
    }

    RTIOStatus_t  status = recvMessageSafe( pContext, pContext->networkIncommingBuffer.pBuffer, pHeader->bodyLen );
    if( status != RTIOSuccess )
    {
        LogError( ( "Failed to recv body, status=%d.", status ) );
    }
    else
    {
        RTIORestMethod_t method;
        status = RTIO_DeSerializeRestMethod( pContext->networkIncommingBuffer.pBuffer, pHeader->bodyLen, &method );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to DeSerializeMethod, status=%d.", status ) );
        }
        else
        {
            switch( method )
            {
            case RTIO_REST_COPOST:
                status = handleCoPost( pContext, pHeader );
                break;
            case RTIO_REST_OBGET:
                status = handleObGet( pContext, pHeader );
                break;
            default:
                break;
            }
        }
    }
    return status;
}

static RTIOStatus_t incommingHeaderHandle( RTIOContext_t* pContext, RTIOHeader_t* pHeader )
{
    RTIOStatus_t status = RTIOSuccess;
    LogDebug( ( "Incomming Header: headerId=%u, type=%u, version=%u, bodyLen=%u, code=%u.",
                pHeader->id, pHeader->type, pHeader->version, pHeader->bodyLen, pHeader->code ) );

    switch( pHeader->type )
    {
    case RTIO_TYPE_DEVICE_PING_RESP:
        status = handleDevicePingResp( pContext, pHeader );
        break;
    case RTIO_TYPE_DEVICE_SEND_RESP:
        status = handleDeviceSendResp( pContext, pHeader );
        break;
    case RTIO_TYPE_SERVER_SEND_REQ:
        status = handleServerSendReqest( pContext, pHeader );
        break;
    default:
        LogError( ( "Incomming type error: type=%u.", pHeader->type ) );
        status = RTIOBadParameter;
        break;
    }
    return status;
}

static void incommingProccess( void* pContext )
{
    RTIOStatus_t status = RTIOUnknown;
    RTIOFixedBuffer_t* pRecvBuffer = NULL;
    RTIOHeader_t header = { 0 };
    OSError_t osRet = OSUnknown;

    LogInfo( ( "Incomming proccess started." ) );

    if( pContext == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p.", (void*)pContext ) );
        return;
    }

    pRecvBuffer = &( ( (RTIOContext_t*)pContext )->networkIncommingBuffer );
    while( false == ( (RTIOContext_t*)pContext )->serviceDone )
    {
        status = recvMessageSafe( pContext, pRecvBuffer->pBuffer, RTIO_PROTOCAL_HEADER_LEN );

        if( RTIOSuccess == status )
        {
            status = RTIO_DeserializeHeader( pRecvBuffer, &header );
            if( RTIOSuccess == status )
            {
                status = incommingHeaderHandle( pContext, &header );
                if( status != RTIOSuccess )
                {
                    LogError( ( "Header handle Failed, status=%d.", status ) );
                }
            }
            else
            {
                LogError( ( "Failed to DeserializeHeader, status=%d.", status ) );
            }
        }
        else if( RTIOTimeout == status ||
                 RTIONotFound == status /* Due to a timeout, headerid not found. */ )
        {
            continue;
        }
        else
        {
            /* MISRA else. */
        }

        if( RTIOSendFailed == status ||
            RTIORecvFailed == status ||
            RTIOTransportFailed == status ||
            RTIOProtocalFailed == status )
        {
            LogError( ( "Session bad when read, status=%d, will reconnet later.", status ) );
            connectStatus_ChangeWhenEventReconnect( pContext );
            bool ok = false;
            while( !( (RTIOContext_t*)pContext )->serviceDone && !ok )
            {
                ok = connectStatus_CheckStatus( pContext, RTIOConnected );
                if( !ok )
                {
                    OS_ClockSleepMs( 200U );
                }
            }
        }
        else
        {
            if( RTIOSuccess != status )
            {
                LogError( ( "Unrecoverable error, status=%d.", status ) );
                break;
            }
        }
    }

    LogInfo( ( "Incomming proccess stopped with status=%d.",  status) );

    if ( status != RTIOSuccess && 
         ( (RTIOContext_t*)pContext )->serveFailedHandler != NULL )
    {
        ( (RTIOContext_t*)pContext )->serveFailedHandler();
    }
    
    osRet =  OS_ThreadDestroy( ( (RTIOContext_t*)pContext )->pThreadIncomming );
    if( osRet != OSSuccess )
    {
        LogError( ( "Failed to destroy pThreadIncomming, ret=%d.", osRet ) );
    }
    // else
    // {
    //     ( (RTIOContext_t*)pContext )->pThreadIncomming = NULL;
    // }
}

RTIOStatus_t ping( RTIOContext_t* pContext, uint32_t heartbeatMs, uint32_t timeoutMs )
{
    static uint8_t pingSerializeBuffer[ RTIO_PING_SERIALIZE_BUFFER_SIZE ];
    RTIOFixedBuffer_t serializeBuffer = { 0 };
    RTIOStatus_t status = RTIOSuccess;
    RTIOPingReq_t pingReq = { 0 };
    uint16_t respIndex = UINT16_MAX;
    uint16_t serianlizeLength = 0;
    rtioDeviceSendResp_t* pDeviceSendResp = NULL;
    RTIORemoteCode_t pingRespCode = REMOTECODE_UNKOWN_ERR;

    if( pContext == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p.", (void*)pContext ) );
        status = RTIOBadParameter;
    }

    LogDebug( ( "Ping started, heartbeatMs=%u, timeoutMs=%u.",
                (unsigned)heartbeatMs, (unsigned)timeoutMs ) );

    serializeBuffer.pBuffer = pingSerializeBuffer;
    serializeBuffer.size = RTIO_PING_SERIALIZE_BUFFER_SIZE;

    /* Add ping request to deviceSendRespList. */
    if( status == RTIOSuccess )
    {
        if( heartbeatMs == 0
            || heartbeatMs == RTIO_PING_INTERVAL_MS_DEFAULT )
        {
            pingReq.header.bodyLen = 0; /* Using default value. */
            heartbeatMs = 0;
        }
        else
        {
            pingReq.header.bodyLen = 2;
        }

        pingReq.header.id = getNextHeaderId( pContext );
        pingReq.header.type = RTIO_TYPE_DEVICE_PING_REQ;
        pingReq.header.version = RTIO_PROTOCAL_VERSION;
        pingReq.timeout = heartbeatMs / 1000;

        status = deviceSendRespList_Add( &( pContext->deviceSendRespList ),
                                         pingReq.header.id,
                                         &serializeBuffer,
                                         &respIndex );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to deviceSendRespList_Add, status=%d.", status ) );
        }
    }

    OS_MutexLock( pContext->pNetworkOutgoingBufferLock );

    /* Send ping request. */
    if( status == RTIOSuccess )
    {
        status = RTIO_SerializePingReq( &pingReq, &serializeBuffer, &serianlizeLength );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to SerializePingReq, status=%d.", status ) );
        }
        else
        {
            status = sendMessageSafe( pContext, serializeBuffer.pBuffer, serianlizeLength );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to send ping request, status=%d.", status ) );
            }
        }
    }

    OS_MutexUnlock( pContext->pNetworkOutgoingBufferLock );

    /* Wait for ping response. */
    if( status == RTIOSuccess )
    {
        status = deviceSendRespList_Wait( &( pContext->deviceSendRespList ), respIndex, timeoutMs );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to wait PingResp, status=%d, respIndex=%d.", status, respIndex ) );
        }
        else
        {
            status = deviceSendRespList_GetResp( &( pContext->deviceSendRespList ), respIndex, &pDeviceSendResp );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to deviceSendRespList_GetResp, status=%d.", status ) );
            }
            else
            {
                pingRespCode = pDeviceSendResp->code;
            }
        }
    }

    if( deviceSendRespList_Delete( &( pContext->deviceSendRespList ), respIndex ) != RTIOSuccess )
    {
        LogError( ( "Failed to deviceSendRespListDelete." ) );
    }

    /* Check ping response. */
    if( status == RTIOSuccess )
    {
        if( pingRespCode != REMOTECODE_SUCCESS )
        {
            LogError( ( "Failed to ping, pingRespCode=%u.", pingRespCode ) );
        }
    }

    return status;
}

/*-----------------------------------------------------------*/
/**
 * @brief connect to server with backoff algorithm
 *
 * @param[in] pContext context pointer with pContext->connectStatus, pContext->serviceDone.
 * @param[out] pContext context pointer with pContext->connectStatus.
 * @return RTIOSuccess, RTIOVerifyFailedNeverRetry, RTIOConnectFailedNeverRetry and others in RTIOStatus_t.
 */

static RTIOStatus_t connectWithBackoffAlgorithm( RTIOContext_t* pContext )
{
    BackoffAlgorithmStatus_t retryStatus = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t backoffContext;
    uint16_t nextRetryBackoff = 0, retryTimes = 0;
    RTIOStatus_t status = RTIOSuccess;

    BackoffAlgorithm_InitializeParams( &backoffContext,
                                       RTIO_RETRY_BACKOFF_BASE_MS,
                                       RTIO_RETRY_MAX_BACKOFF_DELAY_MS,
                                       RTIO_RETRY_MAX_ATTEMPTS );
    do
    {
        status = connectRTIOServer( pContext );
        LogDebug( ( "Connect to Server retryTimes=%u, status=%d.", retryTimes, status ) );

        if( RTIOTransportFailed == status || RTIORecvFailed== status )
        {
            retryStatus = BackoffAlgorithm_GetNextBackoff( &backoffContext, rand(), &nextRetryBackoff );
            LogDebug( ( "Reconnect nextRetryBackoff=%u, retryStatus=%d.", nextRetryBackoff, retryStatus ) );
            retryTimes++;
            if( retryStatus == BackoffAlgorithmRetriesExhausted )
            {
                LogInfo( ( "Reconect stoped, retryTimes exceeded, retryStatus=%d.", BackoffAlgorithmRetriesExhausted ) );
                status = RTIOConnectFailedNeverRetry;
                break;
            }
            OS_ClockSleepMs( nextRetryBackoff );
        }
        else if( RTIOSuccess == status )
        {
            LogDebug( ( "Connect retryTimes=%u success.", retryTimes ) );
            break;
        }
        else if( RTIOVerifyFailedNeverRetry == status )
        {
            LogError( ( "Connect VerifyFailed, please check deviceId or devieSecret, status=%u.", status ) );
            break;
        }
        else /* Other error code */
        {
            LogError( ( "Connect Failed, status=%u.", status ) );
            break;
        }
    } while( 1 );

    return status;
}

static RTIOStatus_t reconnectWithBackoffAlgorithm( RTIOContext_t* pContext )
{
    BackoffAlgorithmStatus_t retryStatus = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t backoffContext;
    uint16_t nextRetryBackoff = 0, retryTimes = 0;
    RTIOStatus_t status = RTIOSuccess;


    status = pContext->transportInterface.disconnect( pContext->transportInterface.pNetworkContext );
    LogInfo( ( "Disconnect old connection, status=%d.", status ) );


    BackoffAlgorithm_InitializeParams( &backoffContext,
                                       RTIO_RETRY_BACKOFF_BASE_MS,
                                       RTIO_RETRY_MAX_BACKOFF_DELAY_MS,
                                       RTIO_RETRY_MAX_ATTEMPTS );
    do
    {
        status = connectRTIOServer( pContext );
        LogDebug( ( "Connect retryTimes=%u, status=%d.", retryTimes, status ) );


        if( RTIOTransportFailed == status  || RTIORecvFailed== status )
        {
            retryStatus = BackoffAlgorithm_GetNextBackoff( &backoffContext, rand(), &nextRetryBackoff );
            LogDebug( ( "Reconnect nextRetryBackoff=%u, retryStatus=%d.", nextRetryBackoff, retryStatus ) );
            retryTimes++;
            if( retryStatus == BackoffAlgorithmRetriesExhausted )
            {
                LogInfo( ( "Reconect stoped, retryTimes exceeded, retryStatus=%d.", BackoffAlgorithmRetriesExhausted ) );
                status = RTIOConnectFailedNeverRetry;
                break;
            }
            else if( pContext->serviceDone )
            {
                LogInfo( ( "Reconect stoped, serviceDone=%d.", pContext->serviceDone ) );
                status = RTIOConnectFailedNeverRetry;
                break;
            }
            OS_ClockSleepMs( nextRetryBackoff );
        }
        else if( RTIOSuccess == status )
        {
            LogDebug( ( "Connect retryTimes=%u success.", retryTimes ) );
            break;
        }
        else if( RTIOVerifyFailedNeverRetry == status )
        {
            LogError( ( "Connect VerifyFailed, please check deviceId or devieSecret, status=%u.", status ) );
            break;
        }
        else /* other error code */
        {
            LogError( ( "Connect Failed, status=%u.", status ) );
            break;
        }
    } while( 1 );

    return status;
}

static void keepAliveProccess( void* pContext )
{
    RTIOContext_t* pRTIOContext = (RTIOContext_t*)pContext;
    RTIOStatus_t status = RTIOSuccess;
    uint32_t currentTimeMs = 0, elapsedTimeMs = 0;
    RTIOConnectStatus_t currentStatus = RTIOConnectInit;
    OSError_t osRet = OSUnknown;

    if( pContext == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p.", (void*)pContext ) );
        return;
    }
    LogInfo( ( "Keep alive proccess started, heartbeatMs=%u.", (unsigned)pRTIOContext->heartbeatMs ) );
    
    while( ( (RTIOContext_t*)pContext )->serviceDone == false )
    {
        currentStatus = connectStatus_GetStatus( pContext );

        if( RTIOConnected == currentStatus )
        {
            currentTimeMs = OS_ClockGetTimeMs();
            elapsedTimeMs = calculateElapsedTime( currentTimeMs, pRTIOContext->lastPacketTxTime );
            // LogDebug( ( "Check: current=%u, last=%u, elapsed=%u, heartbeat=%u.",
            //             (unsigned)currentTimeMs, (unsigned)pRTIOContext->lastPacketTxTime, 
            //             (unsigned)elapsedTimeMs, (unsigned)pRTIOContext->heartbeatMs ) );
            if( elapsedTimeMs >= pRTIOContext->heartbeatMs )
            {
                status = ping( pContext, pRTIOContext->heartbeatMs, RTIO_PING_TIMEOUT_MS );
            }
            
            if( status == RTIOSuccess )
            {
                /* Check interval When Ping Success: 1900ms + (base)100ms = 2 Second. */
                OS_ClockSleepMs( 1900U ); 
            }
            else
            {
                LogError( ( "Failed to ping, status=%d.", status ) );
                if (RTIOSendFailed == status ||
                    RTIORecvFailed == status ||
                    RTIOTransportFailed == status ||
                    RTIOProtocalFailed == status ||
                    RTIOTimeout == status)
                {
                    LogError( ( "Session bad when ping, status=%d, will reconnet later.", status ) );
                    connectStatus_ChangeWhenEventReconnect(pContext);
                }
            }
        }
        else if( RTIOConnecting == currentStatus )
        {
            status = reconnectWithBackoffAlgorithm( pContext );
            if( RTIOSuccess == status )
            {
                connectStatus_ChangeWhenEventConnectSuccess( pContext );
            }
            else
            {
                LogError( ( "Reconnect stoped, status=%d.", status ) );
                connectStatus_ChangeWhenEventDisconnect( pContext );
                break;
            }
        }
        else
        {
            /* MISRA else. */
        }
        /* Base interval: 100ms. */
        OS_ClockSleepMs( 100U ); 
    }


    LogInfo( ( "KeepAlive proccess stopped with status=%d.", status ) );
    if ( status != RTIOSuccess && 
         ( (RTIOContext_t*)pContext )->serveFailedHandler != NULL )
    {
        ( (RTIOContext_t*)pContext )->serveFailedHandler();
    }

    osRet =  OS_ThreadDestroy( ( (RTIOContext_t*)pContext )->pThreadKeepAlive );
    if( osRet != OSSuccess )
    {
        LogError( ( "Failed to destroy pThreadKeepAlive, ret=%d.", osRet ) );
    }
    // else
    // {
    //     ( (RTIOContext_t*)pContext )->pThreadKeepAlive = NULL;
    // }
   
}


/*-----------------------------------------------------------*/

RTIOStatus_t rtioInit( RTIOContext_t* pContext,
                       const RTIOContextFixedResource_t* pFixedResource,
                       const TransportInterface_t* pTransportInterface,
                       const TransportOption_t* pTransportOption,
                       const ServerInfo_t* pServerInfo,
                       const RTIODeviceInfo_t* pDeviceInfo )
{
    /* check pContext */
    if( pContext == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p.", (void*)pContext ) );
        return RTIOBadParameter;
    }
    /* check pFixedResource  */
    if( pFixedResource == NULL )
    {
        LogError( ( "Argument cannot be NULL: pFixedResource=%p.", (void*)pFixedResource ) );
        return RTIOBadParameter;
    }

    if( pFixedResource->networkIncommingBuffer.pBuffer == NULL )
    {
        LogError( ( "Argument cannot be NULL: networkIncommingBuffer=%p.", (void*)pFixedResource->networkIncommingBuffer.pBuffer ) );
        return RTIOBadParameter;
    }
    if( pFixedResource->networkOutgoingBuffer.pBuffer == NULL )
    {
        LogError( ( "Argument cannot be NULL: networkOutgoingBuffer=%p.", (void*)pFixedResource->networkOutgoingBuffer.pBuffer ) );
        return RTIOBadParameter;
    }
    if( pFixedResource->serverSendRespBuffer.pBuffer == NULL )
    {
        LogError( ( "Argument cannot be NULL: serverSendRespBuffer=%p.", (void*)pFixedResource->serverSendRespBuffer.pBuffer ) );
        return RTIOBadParameter;
    }

    if( pFixedResource->pThreadIncomming == NULL )
    {
        LogError( ( "Argument cannot be NULL: pThreadIncomming=%p.", (void*)pFixedResource->pThreadIncomming ) );
        return RTIOBadParameter;
    }

    if( pFixedResource->pThreadKeepAlive == NULL )
    {
        LogError( ( "Argument cannot be NULL: pThreadKeepAlive=%p.", (void*)pFixedResource->pThreadKeepAlive ) );
        return RTIOBadParameter;
    }
    if( pFixedResource->pRollingHeaderIdLock == NULL )
    {
        LogError( ( "Argument cannot be NULL: pRollingHeaderIdLock=%p.", (void*)pFixedResource->pRollingHeaderIdLock ) );
    }
    if( pFixedResource->pSendMessageLock == NULL )
    {
        LogError( ( "Argument cannot be NULL: pSendMessageLock=%p.", (void*)pFixedResource->pSendMessageLock ) );
    }
    if( pFixedResource->pRecvMessageLock == NULL )
    {
        LogError( ( "Argument cannot be NULL: pRecvMessageLock=%p.", (void*)pFixedResource->pRecvMessageLock ) );
    }
    if( pFixedResource->pConnectionStatusLock == NULL )
    {
        LogError( ( "Argument cannot be NULL: pConnectionStatusLock=%p.", (void*)pFixedResource->pConnectionStatusLock ) );
    }
    if( pFixedResource->pNetworkOutgoingBufferLock == NULL )
    {
        LogError( ( "Argument cannot be NULL: pNetworkOutgoingBufferLock=%p.", (void*)pFixedResource->pNetworkOutgoingBufferLock ) );
    }
    if( pFixedResource->coPostUriList.pList == NULL )
    {
        LogError( ( "Argument cannot be NULL: pCoPostUriList=%p.", (void*)pFixedResource->coPostUriList.pList ) );
    }
    if( pFixedResource->obGetUriList.pList == NULL )
    {
        LogError( ( "Argument cannot be NULL: pObGetUriList=%p.", (void*)pFixedResource->obGetUriList.pList ) );
    }
    if( pFixedResource->deviceSendRespList.pList == NULL )
    {
        LogError( ( "Argument cannot be NULL: pDeviceSendRespList=%p.", (void*)pFixedResource->deviceSendRespList.pList ) );
    }
    /* check pTransportInterface  */
    if( pTransportInterface == NULL )
    {
        LogError( ( "Argument cannot be NULL: pTransportInterface=%p.", (void*)pTransportInterface ) );
        return RTIOBadParameter;
    }
    if( pTransportInterface->pNetworkContext == NULL )
    {
        LogError( ( "Argument cannot be NULL: pTransportInterface->pNetworkContext=%p.", (void*)pTransportInterface->pNetworkContext ) );
        return RTIOBadParameter;
    }
    if( pTransportInterface->disconnect == NULL )
    {
        LogError( ( "Invalid parameter: pTransportInterface->disconnect is NULL" ) );
        return RTIOBadParameter;
    }
    if( pTransportInterface->connect == NULL )
    {
        LogError( ( "Invalid parameter: pTransportInterface->connect is NULL" ) );
        return RTIOBadParameter;
    }
    if( pTransportInterface->recv == NULL )
    {
        LogError( ( "Invalid parameter: pTransportInterface->recv is NULL" ) );
        return RTIOBadParameter;
    }
    if( pTransportInterface->send == NULL )
    {
        LogError( ( "Invalid parameter: pTransportInterface->send is NULL" ) );
        return RTIOBadParameter;
    }
    /* check pTransportOptions, tls required */
    if( pTransportOption == NULL )
    {
        LogWarn( ( "Invalid parameter: pTransportOption is NULL" ) );
    }
    /* check pServerInfo and  pDeviceInfo */
    if( pServerInfo == NULL )
    {
        LogError( ( "Invalid parameter: pServerInfo is NULL" ) );
        return RTIOBadParameter;
    }
    if( pDeviceInfo == NULL )
    {
        LogError( ( "Invalid parameter: pDeviceInfo is NULL" ) );
        return RTIOBadParameter;
    }

    pContext->networkIncommingBuffer = pFixedResource->networkIncommingBuffer;
    pContext->networkOutgoingBuffer = pFixedResource->networkOutgoingBuffer;
    pContext->serverSendRespBuffer = pFixedResource->serverSendRespBuffer;
    pContext->coPostInfoList = pFixedResource->coPostUriList;
    pContext->obGetInfoList = pFixedResource->obGetUriList;
    pContext->deviceSendRespList = pFixedResource->deviceSendRespList;
    pContext->pRollingHeaderIdLock = pFixedResource->pRollingHeaderIdLock;
    pContext->pSendMessageLock = pFixedResource->pSendMessageLock;
    pContext->pRecvMessageLock = pFixedResource->pRecvMessageLock;
    pContext->pNetworkOutgoingBufferLock = pFixedResource->pNetworkOutgoingBufferLock;
    pContext->pThreadIncomming = pFixedResource->pThreadIncomming;
    pContext->pThreadKeepAlive = pFixedResource->pThreadKeepAlive;
    pContext->connectStatus = RTIOConnectInit;
    pContext->pConnectionStatusLock = pFixedResource->pConnectionStatusLock;
    if( pContext->heartbeatMs != 0 )
    {
        LogInfo( ( "Heartbeat interval is not default, heartbeatMs=%u.", (unsigned)pContext->heartbeatMs ) );
    }
    else
    {
        pContext->heartbeatMs = RTIO_PING_INTERVAL_MS_INIT;
    }
    pContext->transportInterface = *pTransportInterface;
    pContext->pTransportOption = pTransportOption;
    pContext->pServerInfo = pServerInfo;
    pContext->pDeviceInfo = pDeviceInfo;

    if( OS_MutexCreate( pContext->pRollingHeaderIdLock ) != OSSuccess )
    {
        LogError( ( "Failed to create rollingHeaderIdLock." ) );
        return RTIOMutexFailure;
    }
    if( OS_MutexCreate( pContext->pSendMessageLock ) != OSSuccess )
    {
        LogError( ( "Failed to create sendMessageLock." ) );
        return RTIOMutexFailure;
    }
    if( OS_MutexCreate( pContext->pRecvMessageLock ) != OSSuccess )
    {
        LogError( ( "Failed to create recvMessageLock." ) );
        return RTIOMutexFailure;
    }
    if( OS_MutexCreate( pContext->pNetworkOutgoingBufferLock ) != OSSuccess )
    {
        LogError( ( "Failed to create networkOutgoingBufferLock." ) );
        return RTIOMutexFailure;
    }
    if( OS_MutexCreate( pContext->deviceSendRespList.pLock ) != OSSuccess )
    {
        LogError( ( "Failed to create deviceSendRespListLock." ) );
        return RTIOMutexFailure;
    }
    if( OS_MutexCreate( pContext->pConnectionStatusLock ) != OSSuccess )
    {
        LogError( ( "Failed to create pConnectionStatusLock." ) );
        return RTIOMutexFailure;
    }

    srand( (int)OS_ClockGetTimeMs() );

    return RTIOSuccess;
}

RTIOStatus_t RTIO_Connect( RTIOContext_t* pContext,
                           const RTIOContextFixedResource_t* pFixedResource,
                           const TransportInterface_t* pTransportInterface,
                           const TransportOption_t* pTransportOption,
                           const ServerInfo_t* pServerInfo,
                           const RTIODeviceInfo_t* pDeviceInfo )
{
    RTIOStatus_t status = RTIOUnknown;

    status = rtioInit( pContext, pFixedResource,
                       pTransportInterface, pTransportOption, pServerInfo, pDeviceInfo );
    if( RTIOSuccess != status )
    {
        return status;
    }

    connectStatus_ChangeWhenEventConnect( pContext );

    status = connectWithBackoffAlgorithm( pContext );
    if( RTIOSuccess == status )
    {
        connectStatus_ChangeWhenEventConnectSuccess( pContext );
    }
    else
    {
        LogError( ( "Reconnect stoped, status=%d.", status ) );
        connectStatus_ChangeWhenEventDisconnect( pContext );
    }
    return status;
}

RTIOStatus_t RTIO_Disconnect( RTIOContext_t* pContext )
{
    RTIOStatus_t status = RTIOSuccess;
    TransportStatus_t transportStatus = TransportUnknown;

    if( ( pContext == NULL ) ||
        ( pContext->transportInterface.disconnect == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, "
                    "transportInterface.disconnect=%p.",
                    (void*)pContext,
                    (void*)pContext->transportInterface.disconnect ) );

        return RTIOBadParameter;
    }

    LogInfo( ( "Disconnect..." ) );

    /* thread destroy */
    pContext->serviceDone = true;
    LogInfo( ( "To exit gracefully, wait for seconds=%d for related threads to terminate.", (RTIO_SEND_TIMEOUT_MS + 1000U)/1000 ) );
    OS_ClockSleepMs( RTIO_SEND_TIMEOUT_MS + 1000U );
 
    /* transport destroy */
    connectStatus_ChangeWhenEventDisconnect( pContext );
    transportStatus = pContext->transportInterface.disconnect( pContext->transportInterface.pNetworkContext );
    if( transportStatus == TransportSuccess )
    {
        connectStatus_ChangeWhenEventDisconnectSuccess( pContext );
    }
    else
    {
        LogError( ( "Failed to disconnect" ) );
        status = RTIODisconnectFailed;
    }

    /* mutex destroy */
    if( OS_MutexDestroy( pContext->pRollingHeaderIdLock ) != OSSuccess )
    {
        LogError( ( "Failed to destroy rollingHeaderIdLock." ) );
    }
    if( OS_MutexDestroy( pContext->pSendMessageLock ) != OSSuccess )
    {
        LogError( ( "Failed to destroy pSendMessageLock." ) );
    }
    if( OS_MutexDestroy( pContext->pRecvMessageLock ) != OSSuccess )
    {
        LogError( ( "Failed to destroy pRecvMessageLock." ) );
    }
    if( OS_MutexDestroy( pContext->pNetworkOutgoingBufferLock ) != OSSuccess )
    {
        LogError( ( "Failed to destroy pNetworkOutgoingBufferLock." ) );
    }
    if( OS_MutexDestroy( pContext->deviceSendRespList.pLock ) != OSSuccess )
    {
        LogError( ( "Failed to destroy deviceSendRespList.pLock." ) );
    }
    if( OS_MutexDestroy( pContext->pConnectionStatusLock ) != OSSuccess )
    {
        LogError( ( "Failed to destroy pConnectionStatusLock." ) );
    }
    return status;
}

RTIOStatus_t RTIO_RegisterCoPostHandler( const RTIOContext_t* pContext,
                                         const char* pUri, RTIOCoPostHandler_t handler )
{
    uint16_t i = 0;
    uint32_t uri = 0;
    uint16_t length = 0;
    if( ( pContext == NULL ) || ( handler == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, handler=%p.", (void*)pContext, (void*)handler ) );
        return RTIOBadParameter;
    }
    if( pUri == NULL )
    {
        LogError( ( "Argument cannot be NULL: pUri=%p.", (void*)pUri ) );
        return RTIOBadParameter;
    }
    length = (uint16_t)strlen( pUri );
    if( ( length < RTIO_URI_STRING_LENGTH_MIN ) || ( length > RTIO_URI_STRING_LENGTH_MAX ) )
    {
        LogError( ( "Argument error: uri length=%u.", length ) );
        return RTIOBadParameter;
    }

    uri = crc32Ieee( (uint8_t*)pUri, length );
    LogInfo( ( "Register pUri=%s, digest=%u, digestHex=0x%x.", pUri, (unsigned)uri, (unsigned)uri ) );

    for( i = 0; i < pContext->coPostInfoList.size; ++i )
    {
        LogDebug( ( "List[%d].uri=%u.", i, (unsigned)pContext->coPostInfoList.pList[ i ].uri ) );
        if( pContext->coPostInfoList.pList[ i ].uri == 0 )
        {
            pContext->coPostInfoList.pList[ i ].uri = uri;
            pContext->coPostInfoList.pList[ i ].handler = handler;
            break;
        }
    }
    if( i == pContext->coPostInfoList.size )
    {
        return RTIOListFull;
    }
    return RTIOSuccess;
}

RTIOStatus_t RTIO_ObNotify( RTIOContext_t* pContext,
                            uint8_t* pData, uint16_t Length,
                            uint16_t obId,
                            uint32_t timeoutMs )
{
    uint8_t notifyRespSerializeBuffer[ RTIO_NOTIFY_RESP_SERIALIZE_BUFFER_SIZE ];
    RTIOFixedBuffer_t serializeBuffer = { 0 };
    uint16_t serianlizeLength = 0;
    RTIOStatus_t status = RTIOSuccess;
    uint16_t respIndex = UINT16_MAX;
    rtioDeviceSendResp_t* pDeviceSendResp = NULL;
    RTIOObNotifyReq_t req = { 0 };
    RTIOObNotifyResp_t resp = { 0 };

    if( pContext == NULL || pData == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, pData=%p.", (void*)pContext, (void*)pData ) );
        status = RTIOBadParameter;
    }

    serializeBuffer.pBuffer = notifyRespSerializeBuffer;
    serializeBuffer.size = RTIO_NOTIFY_RESP_SERIALIZE_BUFFER_SIZE;

    if( status == RTIOSuccess )
    {
        req.headerId = getNextHeaderId( pContext );
        req.method = RTIO_REST_OBGET;
        req.code = RTIO_REST_STATUS_CONTINUE;
        req.obId = obId;
        req.pData = pData;
        req.dataLength = Length;

        status = deviceSendRespList_Add( &( pContext->deviceSendRespList ),
                                         req.headerId,
                                         &serializeBuffer,
                                         &respIndex );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to deviceSendRespListAdd, status=%d, headerid=%u.", status, req.headerId) );
        }
    }

    OS_MutexLock( pContext->pNetworkOutgoingBufferLock );

    if( status == RTIOSuccess )
    {
        status = RTIO_SerializeObNotifyReq_OverDeviceSendReq( &req, &( pContext->networkOutgoingBuffer ), &serianlizeLength );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to SerializeCoReq, status=%d.", status ) );
        }
        else
        {
            status = sendMessageSafe( pContext, pContext->networkOutgoingBuffer.pBuffer, serianlizeLength );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to send CoReq, status=%d.", status ) );
            }
        }
    }

    OS_MutexUnlock( pContext->pNetworkOutgoingBufferLock );

    if( status == RTIOSuccess )
    {
        status = deviceSendRespList_Wait( &( pContext->deviceSendRespList ), respIndex, timeoutMs );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to wait ObNotifyResp, status=%d.", status ) );
        }
        else
        {
            status = deviceSendRespList_GetResp( &( pContext->deviceSendRespList ), respIndex, &pDeviceSendResp );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to deviceSendRespList_GetResp, status=%d.", status ) );
            }
            else
            {
                status = RTIO_DeSerializeObNotifyResp_FromDeviceSendResp( pDeviceSendResp, &resp );
                if( status != RTIOSuccess )
                {
                    LogError( ( "Failed to RTIO_DeSerializeObNotifyResp_FromDeviceSendResp, status=%d.", status ) );
                }
            }
        }
    }

    if( deviceSendRespList_Delete( &( pContext->deviceSendRespList ), respIndex ) != RTIOSuccess )
    {
        LogError( ( "Failed to deviceSendRespListDelete." ) );
    }

    status = transRestStatus( resp.code );
    return status;
}

RTIOStatus_t RTIO_ObNotifyTerminate( RTIOContext_t* pContext,
                                     uint16_t obId,
                                     uint32_t timeoutMs )
{
    uint8_t notifyRespSerializeBuffer[ RTIO_NOTIFY_RESP_SERIALIZE_BUFFER_SIZE ];
    RTIOFixedBuffer_t serializeBuffer = { 0 };
    uint16_t serianlizeLength = 0;
    RTIOStatus_t status = RTIOSuccess;
    uint16_t respIndex = UINT16_MAX;
    rtioDeviceSendResp_t* pDeviceSendResp = NULL;
    RTIOObNotifyReq_t req = { 0 };
    RTIOObNotifyResp_t resp = { 0 };

    if( pContext == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p.", (void*)pContext ) );
        status = RTIOBadParameter;
    }

    serializeBuffer.pBuffer = notifyRespSerializeBuffer;
    serializeBuffer.size = RTIO_NOTIFY_RESP_SERIALIZE_BUFFER_SIZE;

    if( status == RTIOSuccess )
    {
        req.headerId = getNextHeaderId( pContext );
        req.method = RTIO_REST_OBGET;
        req.code = RTIO_REST_STATUS_TERMINATE;
        req.obId = obId;

        status = deviceSendRespList_Add( &( pContext->deviceSendRespList ),
                                         req.headerId,
                                         &serializeBuffer,
                                         &respIndex );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to deviceSendRespListAdd, status=%d.", status ) );
        }
    }

    // lock networkOutgoingBuffer
    OS_MutexLock( pContext->pNetworkOutgoingBufferLock );

    if( status == RTIOSuccess )
    {
        status = RTIO_SerializeObNotifyReq_OverDeviceSendReq( &req, &( pContext->networkOutgoingBuffer ), &serianlizeLength );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to SerializeCoReq, status=%d.", status ) );
        }
        else
        {
            status = sendMessageSafe( pContext, pContext->networkOutgoingBuffer.pBuffer, serianlizeLength );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to send CoReq, status=%d.", status ) );
            }
        }
    }

    // unlock networkOutgoingBuffer
    OS_MutexUnlock( pContext->pNetworkOutgoingBufferLock );

    if( status == RTIOSuccess )
    {
        status = deviceSendRespList_Wait( &( pContext->deviceSendRespList ), respIndex, timeoutMs );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to wait ObNotifyResp, status=%d.", status ) );
        }
        else
        {
            status = deviceSendRespList_GetResp( &( pContext->deviceSendRespList ), respIndex, &pDeviceSendResp );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to deviceSendRespList_GetResp, status=%d.", status ) );
            }
            else
            {
                status = RTIO_DeSerializeObNotifyResp_FromDeviceSendResp( pDeviceSendResp, &resp );
                if( status != RTIOSuccess )
                {
                    LogError( ( "Failed to RTIO_DeSerializeObNotifyResp_FromDeviceSendResp, status=%d.", status ) );
                }
            }
        }
    }

    if( deviceSendRespList_Delete( &( pContext->deviceSendRespList ), respIndex ) != RTIOSuccess )
    {
        LogError( ( "Failed to deviceSendRespListDelete." ) );
    }

    status = transRestStatus( resp.code );
    return status;
}

RTIOStatus_t RTIO_RegisterObGetHandler( const RTIOContext_t* pContext,
                                            const char* pUri, RTIOObGetHandler_t handler )
{
    uint16_t i = 0;
    uint32_t uri = 0;
    uint16_t length = 0;
    if( ( pContext == NULL ) || ( handler == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, handler=%p.", (void*)pContext, (void*)handler ) );
        return RTIOBadParameter;
    }
    if( pUri == NULL )
    {
        LogError( ( "Argument cannot be NULL: pUri=%p.", (void*)pUri ) );
        return RTIOBadParameter;
    }
    length = (uint16_t)strlen( pUri );
    if( ( length < RTIO_URI_STRING_LENGTH_MIN ) || ( length > RTIO_URI_STRING_LENGTH_MAX ) )
    {
        LogError( ( "Argument error: uri length=%u.", length ) );
        return RTIOBadParameter;
    }

    uri = crc32Ieee( (uint8_t*)pUri, length );
    LogInfo( ( "Register pUri=%s, digest=%u, digestHex=0x%x.", pUri, (unsigned)uri, (unsigned)uri ) );

    for( i = 0; i < pContext->obGetInfoList.size; ++i )
    {
        if( pContext->obGetInfoList.pList[ i ].uri == 0 )
        {
            LogDebug( ( "ObList[%d].uri=%u, ready for digest=%u.", i, (unsigned)pContext->obGetInfoList.pList[ i ].uri, (unsigned)uri ) );
            pContext->obGetInfoList.pList[ i ].uri = uri;
            pContext->obGetInfoList.pList[ i ].handler = handler;
            break;
        }
        else
        {
            LogDebug( ( "ObList[%d].uri=%u.", i, (unsigned)pContext->obGetInfoList.pList[ i ].uri ) );
        }
    }
    if( i == pContext->obGetInfoList.size )
    {
        return RTIOListFull;
    }

    return RTIOSuccess;
}


RTIOStatus_t RTIO_Serve( RTIOContext_t* pContext )
{
    RTIOStatus_t status = RTIOSuccess;
    OSError_t ret = OSUnknown;

    if( ( pContext == NULL ) || ( pContext->pThreadIncomming == NULL ) || ( pContext->pThreadKeepAlive == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, pThreadIncomming=%p, pThreadKeepAlive=%p.",
                    (void*)pContext, (void*)pContext->pThreadIncomming, (void*)pContext->pThreadKeepAlive ) );
        status = RTIOBadParameter;
    }

    pContext->serviceDone = false;

    if( status == RTIOSuccess )
    {
        ret = OS_ThreadCreate( pContext->pThreadIncomming, incommingProccess,
                               (void*)pContext, "IncommingProccess", RTIO_THREAD_INCOMMING_STACK_SIZE );
        if( ret != OSSuccess )
        {
            LogError( ( "Failed to create pThreadIncomming, ret=%d.", ret ) );
            status = RTIOThreadCreateFailed;
        }
    }
    if( status == RTIOSuccess )
    {
        ret = OS_ThreadCreate( pContext->pThreadKeepAlive, keepAliveProccess,
                               (void*)pContext, "KeepAliveProccess", RTIO_THREAD_KEEPALIVE_STACK_SIZE );
        if( ret != OSSuccess )
        {
            LogError( ( "Failed to create pThreadKeepAlive, ret=%d.", ret ) );
            status = RTIOThreadCreateFailed;
        }
    }

    return status;
}

RTIOStatus_t RTIO_SetHeartbeat( RTIOContext_t* pContext, uint32_t heartbeatMs )
{
    if( pContext == NULL )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p.", (void*)pContext ) );
        return RTIOBadParameter;
    }

    if( heartbeatMs < RTIO_PING_INTERVAL_MS_MIN ||
        heartbeatMs > RTIO_PING_INTERVAL_MS_MAX )
    {
        LogError( ( "Argument Error: heartbeatMs=%u.", (unsigned)heartbeatMs ) );
        return RTIOBadParameter;
    }
    else
    {
        pContext->heartbeatMs = heartbeatMs;
    }

    return RTIOSuccess;
}

RTIOStatus_t RTIO_SetServeFailedHandler( RTIOContext_t* pContext, RTIOServeFailedHandler_t handler )
{
    if( ( pContext == NULL ) || ( handler == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, handler=%p.", (void*)pContext, (void*)handler ) );
        return RTIOBadParameter;
    }

    pContext->serveFailedHandler = handler;
    return RTIOSuccess;
}

RTIOStatus_t RTIO_CoPost( RTIOContext_t* pContext, const char* pUri,
                          uint8_t* pReqData, uint16_t reqLength,
                          RTIOFixedBuffer_t* pRespbuffer, uint16_t* respLength,
                          uint32_t timeoutMs )
{
    if( pUri == NULL )
    {
        LogError( ( "Argument cannot be NULL: pUri=%p.", (void*)pUri ) );
        return RTIOBadParameter;
    }

    uint32_t uri = 0;
    uri = crc32Ieee( (uint8_t*)pUri, strlen( pUri ) );
    LogDebug( ( "Post pUri=%s, digest=%u, digestHex=0x%x.", pUri, (unsigned)uri, (unsigned)uri ) );

    return RTIO_CoPostWithDigest( pContext, uri, pReqData, reqLength,
                                  pRespbuffer, respLength, timeoutMs );

}

RTIOStatus_t RTIO_URIHash( const char* pUri, uint32_t* pDigest )
{
    uint16_t length = 0;
    if( ( pUri == NULL ) || ( pDigest == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pUri=%p, pDigest=%p.",
                    (void*)pUri, pDigest ) );
        return RTIOBadParameter;
    }

    length = (uint16_t)strlen( pUri );

    if( ( length < RTIO_URI_STRING_LENGTH_MIN ) || ( length > RTIO_URI_STRING_LENGTH_MAX ) )
    {
        LogError( ( "Argument error: uri length=%u.", length ) );
        return RTIOBadParameter;
    }

    *pDigest = crc32Ieee( (uint8_t*)pUri, length );
    return RTIOSuccess;

}

RTIOStatus_t RTIO_CoPostWithDigest( RTIOContext_t* pContext, uint32_t uri,
                          uint8_t* pReqData, uint16_t reqLength,
                          RTIOFixedBuffer_t* pRespbuffer, uint16_t* respLength,
                          uint32_t timeoutMs )
{
    RTIOStatus_t status = RTIOSuccess;
    uint16_t serianlizeLength = 0;
    uint16_t respIndex = UINT16_MAX;
    rtioDeviceSendResp_t* pDeviceSendResp = NULL;
    RTIOCoReq_t coReq = { 0 };
    RTIOCoResp_t coResp = { 0 };

    if( ( pContext == NULL ) || ( pReqData == NULL ) || ( pRespbuffer == NULL ) || ( respLength == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pContext=%p, pReqData=%p, pRespbuffer=%p, respLength=%p.",
                    (void*)pContext,
                    (void*)pReqData,
                    (void*)pRespbuffer,
                    (void*)respLength ) );
        status = RTIOBadParameter;
    }

    if( status == RTIOSuccess )
    {
        coReq.headerId = getNextHeaderId( pContext );
        coReq.uri = uri;
        coReq.method = RTIO_REST_COPOST;
        coReq.dataLength = reqLength;
        coReq.pData = pReqData;
        LogInfo( ( "Post, uri=%u, reqLength=%u, headerId=%u, timeoutMs=%u.",
                   (unsigned)uri, reqLength, coReq.headerId, (unsigned)timeoutMs ) );
        status = deviceSendRespList_Add( &( pContext->deviceSendRespList ),
                                         coReq.headerId, pRespbuffer, &respIndex );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to addDeviceSendRespEvent, status=%d.", status ) );
        }
    }

    // lock networkOutgoingBuffer
    OS_MutexLock( pContext->pNetworkOutgoingBufferLock );

    if( status == RTIOSuccess )
    {
        status = RTIO_SerializeCoReq_OverDeviceSendReq( &coReq, &( pContext->networkOutgoingBuffer ), &serianlizeLength );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to SerializeCoReq, status=%d.", status ) );
        }
        else
        {
            status = sendMessageSafe( pContext, pContext->networkOutgoingBuffer.pBuffer, serianlizeLength );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to send CoReq, status=%d.", status ) );
            }
        }
    }

    // unlock networkOutgoingBuffer
    OS_MutexUnlock( pContext->pNetworkOutgoingBufferLock );

    if( status == RTIOSuccess )
    {
        status = deviceSendRespList_Wait( &( pContext->deviceSendRespList ), respIndex, timeoutMs );
        if( status != RTIOSuccess )
        {
            LogError( ( "Failed to wait CoResp, status=%d.", status ) );
        }
        else
        {
            status = deviceSendRespList_GetResp( &( pContext->deviceSendRespList ), respIndex, &pDeviceSendResp );
            if( status != RTIOSuccess )
            {
                LogError( ( "Failed to deviceSendRespList_GetResp, status=%d.", status ) );
            }
            else
            {
                status = RTIO_DeSerializeCoResp_FromDeviceSendResp( pDeviceSendResp, &coResp );
                if( status != RTIOSuccess )
                {
                    LogError( ( "Failed to RTIO_DeSerializeCoResp_FromDeviceSendResp, status=%d.", status ) );
                }
            }
        }
    }

    if( deviceSendRespList_Delete( &( pContext->deviceSendRespList ), respIndex ) != RTIOSuccess )
    {
        LogError( ( "Failed to deviceSendRespListDelete." ) );
    }

    *respLength = coResp.dataLength;
    status = transRestStatus( coResp.code );
    return status;
}


/*-----------------------------------------------------------*/

RTIOStatus_t RTIO_ObListInit( RTIO_ObList_t* pObList )
{

    if( NULL == pObList ||
        NULL == pObList->pArray ||
        NULL == pObList->pLock ||
        0 == pObList->arraySize )
    {
        LogError( ( "Bad parameter." ) );
        return RTIOBadParameter;
    }

    if( OS_MutexCreate( pObList->pLock ) != OSSuccess )
    {
        LogError( ( "OS_MutexCreate Failed." ) );
        return RTIOMutexFailure;
    }

    pObList->obNumber = 0;
    return RTIOSuccess;
}

RTIOStatus_t RTIO_ObListAdd( RTIO_ObList_t* pObList, uint16_t obId )
{
    RTIOStatus_t status = RTIOSuccess;
    int i = 0;
    bool emptyItemFound = false;

    if( NULL == pObList || 0 == obId )
    {
        LogError( ( "Bad parameter." ) );
        return RTIOBadParameter;
    }

    for( i = 0; i < pObList->arraySize; i++ )
    {
        OS_MutexLock( pObList->pLock );
        if( 0 == pObList->pArray[ i ] )
        {
            emptyItemFound = true;
            pObList->pArray[ i ] = obId;
            pObList->obNumber++;
        }
        OS_MutexUnlock( pObList->pLock );

        if( emptyItemFound )
        {
            break;
        }
    }
    if( i == pObList->arraySize )
    {
        status = RTIOListFull;
    }

    return status;
}

uint16_t RTIO_ObListGetObNumberNotRealtime( RTIO_ObList_t* pObList )
{
    if( NULL == pObList )
    {
        LogError( ( "Bad parameter, pObList=NULL." ) );
        return 0;
    }
    // LogDebug( ( "RTIO_ObList, obNumber=%u.", pObList->obNumber ) );
    return pObList->obNumber;
}

RTIOStatus_t RTIO_ObListNotifyAll( RTIOContext_t* pContext,
                                   RTIO_ObList_t* pObList,
                                   uint8_t* pData, uint16_t dataLength )
{
    RTIOStatus_t notifyStatus = RTIOSuccess;
    if( NULL == pContext || NULL == pObList || NULL == pData )
    {
        LogError( ( "Bad parameter." ) );
        return RTIOBadParameter;
    }
    LogDebug( ( "Notify all, dataLength=%u, NOTIFY_TIMEOUT=%u.",
                dataLength, RTIO_OBSERVA_NOTIFY_TIMEOUT_MS ) );

    int i = 0;
    for( ; i < pObList->arraySize; i++ )
    {
        
        if( pObList->pArray[ i ] != 0 )
        {
            notifyStatus = RTIO_ObNotify( pContext, pData, dataLength,
                                          pObList->pArray[ i ],
                                          RTIO_OBSERVA_NOTIFY_TIMEOUT_MS );

            if( notifyStatus != RTIOContinue )
            {
                if( notifyStatus == RTIOTimeout )
                {
                    LogError( ( "RTIO_ObNotify Timeout obId=%d.", pObList->pArray[ i ] ) );
                }
                else if( notifyStatus == RTIOTerminate )
                {
                    LogInfo( ( "RTIO_ObNotify Terminate obId=%d.", pObList->pArray[ i ] ) );
                    OS_MutexLock( pObList->pLock );
                    pObList->pArray[ i ] = 0;
                    if( pObList->obNumber > 0 )
                    {
                        pObList->obNumber--;
                    }
                    OS_MutexUnlock( pObList->pLock );
                }
            }
            else
            {
                LogDebug( ( "RTIO_ObNotify Continue obId=%d.", pObList->pArray[ i ] ) );
            }
        }
        
    }

    return RTIOSuccess;
}

RTIOStatus_t RTIO_ObListDeInit( RTIO_ObList_t* pObList )
{
    if( NULL == pObList )
    {
        LogError( ( "Bad parameter." ) );
        return RTIOBadParameter;
    }

    memset( pObList->pArray, 0, pObList->arraySize * sizeof( uint16_t ) );
    pObList->arraySize = 0;
    pObList->obNumber = 0;
    if( OS_MutexDestroy( pObList->pLock ) != OSSuccess )
    {
        LogError( ( "OS_MutexDestroy Failed." ) );
        return RTIOMutexFailure;
    }
    return RTIOSuccess;
}

/*-----------------------------------------------------------*/