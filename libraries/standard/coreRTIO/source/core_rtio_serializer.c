/*
 * Copyright (c) 2023-2024 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#include "core_rtio_serializer.h"

uint16_t capLevelToSize[ RTIO_CAP_LEVELS ] = { 512 }; // { 512, 1024, 2048, 4096 }, Currently only support 512

RTIOStatus_t RTIO_SerializeHeader( const RTIOHeader_t* pHeader,
                                   const RTIOFixedBuffer_t* pFixedBuffer )
{
    if( ( pHeader == NULL ) || ( pFixedBuffer == NULL ) || pFixedBuffer->size < RTIO_PROTOCAL_HEADER_LEN )
    {
        LogError( ( "Argument cannot be NULL: pHeader=%p, pFixedBuffer=%p, pFixedBuffer->size=%u.",
                    (void*)pHeader,
                    (void*)pFixedBuffer,
                    pFixedBuffer->size ) );
        return RTIOBadParameter;
    }

    pFixedBuffer->pBuffer[ 0 ] = ( ( pHeader->type << 4 ) & 0xF0 ) + ( ( pHeader->version << 3 ) & 0x80 ) + ( pHeader->code & 0x07 );
    pFixedBuffer->pBuffer[ 1 ] = ( pHeader->id >> 8 ) & 0xFF;
    pFixedBuffer->pBuffer[ 2 ] = ( pHeader->id ) & 0xFF;
    pFixedBuffer->pBuffer[ 3 ] = ( pHeader->bodyLen >> 8 ) & 0xFF;
    pFixedBuffer->pBuffer[ 4 ] = ( pHeader->bodyLen ) & 0xFF;
    return RTIOSuccess;
}

RTIOStatus_t RTIO_DeserializeHeader( const RTIOFixedBuffer_t* pFixedBuffer,
                                     RTIOHeader_t* pHeader )
{
    if( ( pHeader == NULL ) || ( pFixedBuffer == NULL ) || pFixedBuffer->size < RTIO_PROTOCAL_HEADER_LEN )
    {
        LogError( ( "Argument cannot be NULL: pHeader=%p, pFixedBuffer=%p, pFixedBuffer->size=%u.",
                    (void*)pHeader,
                    (void*)pFixedBuffer,
                    pFixedBuffer->size ) );
        return RTIOBadParameter;
    }

    pHeader->type = ( pFixedBuffer->pBuffer[ 0 ] >> 4 ) & 0x0F;
    pHeader->version = ( pFixedBuffer->pBuffer[ 0 ] >> 3 ) & 0x01;
    pHeader->code = pFixedBuffer->pBuffer[ 0 ] & 0x07;
    pHeader->id = (uint16_t)( pFixedBuffer->pBuffer[ 1 ] << 8 ) + (uint16_t)( pFixedBuffer->pBuffer[ 2 ] );
    pHeader->bodyLen = (uint16_t)( pFixedBuffer->pBuffer[ 3 ] << 8 ) + (uint16_t)( pFixedBuffer->pBuffer[ 4 ] );

    if( pHeader->bodyLen > capLevelToSize[ RTIO_CAP_LEVEL_MAX ] )
    {
        LogError( ( "Body length exceeds the maximum size: pHeader->bodyLen=%u, capLevelToSize[RTIO_CAP_LEVEL_MAX]=%u.",
                    pHeader->bodyLen,
                    capLevelToSize[ RTIO_CAP_LEVEL_MAX ] ) );
        return RTIOProtocalFailed;
    }
    return RTIOSuccess;
}

RTIOStatus_t RTIO_SerializeVerifyReq( RTIOVerifyReq_t* pReq,
                                      const RTIOFixedBuffer_t* pFixedBuffer,
                                      uint16_t* dataLen )
{
    RTIOStatus_t status = RTIOUnknown;
    uint8_t deviceIdLen = strlen( pReq->pDeviceId );
    uint8_t secretLen = strlen( pReq->pDeviceSecret );

    if( ( pReq == NULL ) || ( pFixedBuffer == NULL ) ||
        pFixedBuffer->size < ( pReq->header.bodyLen + RTIO_PROTOCAL_HEADER_LEN ) )
    {
        LogError( ( "Argument cannot be NULL: pReq=%p, pFixedBuffer=%p, pFixedBuffer->size=%u, totalLen=%u.",
                    (void*)pReq,
                    (void*)pFixedBuffer,
                    pFixedBuffer->size,
                    ( pReq->header.bodyLen + RTIO_PROTOCAL_HEADER_LEN ) ) );
        return RTIOBadParameter;
    }

    status = RTIO_SerializeHeader( &( pReq->header ), pFixedBuffer );
    if( status != RTIOSuccess )
    {
        return status;
    }
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN ] = ( pReq->capLevel << 6 ) & 0xc0;
    memcpy( pFixedBuffer->pBuffer + RTIO_PROTOCAL_HEADER_LEN + 1, pReq->pDeviceId, deviceIdLen );
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + deviceIdLen + 1 ] = ':';
    memcpy( pFixedBuffer->pBuffer + RTIO_PROTOCAL_HEADER_LEN + deviceIdLen + 2, pReq->pDeviceSecret, secretLen );
    *dataLen = ( pReq->header.bodyLen + RTIO_PROTOCAL_HEADER_LEN );
    return RTIOSuccess;
}

RTIOStatus_t RTIO_DeserializeVerifyResp( const RTIOFixedBuffer_t* pFixedBuffer,
                                         RTIOVerifyResp_t* pResp )
{
    if( ( pResp == NULL ) || ( pFixedBuffer == NULL ) || pFixedBuffer->size < RTIO_PROTOCAL_HEADER_LEN )
    {
        LogError( ( "Argument cannot be NULL: pHeader=%p, pFixedBuffer=%p, pFixedBuffer->size=%u.",
                    (void*)pResp,
                    (void*)pFixedBuffer,
                    pFixedBuffer->size ) );
        return RTIOBadParameter;
    }
    return RTIO_DeserializeHeader( pFixedBuffer, &( pResp->header ) );

}

RTIOStatus_t RTIO_SerializePingReq( const RTIOPingReq_t* pReq,
                                    const RTIOFixedBuffer_t* pFixedBuffer,
                                    uint16_t* dataLength )
{

    if( ( pReq == NULL ) || ( pFixedBuffer == NULL ) || pFixedBuffer->size < ( pReq->header.bodyLen + RTIO_PROTOCAL_HEADER_LEN ) )
    {
        LogError( ( "Argument cannot be NULL: pReq=%p, pFixedBuffer=%p, pFixedBuffer->size=%u, totalLen=%u.",
                    (void*)pReq,
                    (void*)pFixedBuffer,
                    pFixedBuffer->size,
                    ( pReq->header.bodyLen + RTIO_PROTOCAL_HEADER_LEN ) ) );
        return RTIOBadParameter;
    }

    RTIOStatus_t status = RTIOSuccess;
    status = RTIO_SerializeHeader( &( pReq->header ), pFixedBuffer );
    if( status != RTIOSuccess )
    {
        return status;
    }
    if( pReq->timeout > 0 )
    {
        pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN ] = ( pReq->timeout >> 8 ) & 0xFF;
        pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 1 ] = ( pReq->timeout ) & 0xFF;
        *dataLength = ( pReq->header.bodyLen + RTIO_PROTOCAL_HEADER_LEN );
    }
    else
    {
        *dataLength = RTIO_PROTOCAL_HEADER_LEN;
    }

    return RTIOSuccess;
}

/*-----------------------------------------------------------*/

RTIOStatus_t RTIO_DeSerializeRestMethod( const uint8_t* buf, uint16_t length,
                                         RTIORestMethod_t* method )
{
    if( ( method == NULL ) || ( buf == NULL ) || length < RTIO_REST_HEADER_LENGTH_MIN )
    {
        LogError( ( "Argument cannot be NULL: method=%p, buf=%p, length=%u.",
                    (void*)method,
                    (void*)buf,
                    length ) );
        return RTIOBadParameter;
    }
    *method = ( buf[ 0 ] >> 4 ) & 0x0F;
    return RTIOSuccess;
}
RTIOStatus_t RTIO_DeSerializeCoReqNoCopy( const uint16_t headerId,
                                          const uint8_t* pData, uint16_t dataLength,
                                          RTIOCoReq_t* pReq )
{
    if( ( pData == NULL ) || ( pReq == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pData=%p, pReq=%p.",
                    (void*)pData,
                    (void*)pReq ) );
        return RTIOBadParameter;
    }
    if( headerId == 0 )
    {
        LogError( ( "Header id cannot be 0." ) );
        return RTIOProtocalFailed;
    }
    if( dataLength < RTIO_REST_HEADER_LENGTH_CO_REQ )
    {
        LogError( ( "Data length is not enough: dataLength=%u, RTIO_REST_HEADER_LENGTH_CO_REQ=%u.",
                    dataLength,
                    RTIO_REST_HEADER_LENGTH_CO_REQ ) );
        return RTIOBadParameter;
    }
    pReq->headerId = headerId;
    pReq->method = ( pData[ 0 ] >> 4 ) & 0x0F;
    pReq->uri = (uint32_t)( pData[ 1 ] << 24 ) + (uint32_t)( pData[ 2 ] << 16 ) + (uint32_t)( pData[ 3 ] << 8 ) + (uint32_t)( pData[ 4 ] );
    pReq->pData = (uint8_t*)pData + RTIO_REST_HEADER_LENGTH_CO_REQ;
    pReq->dataLength = dataLength - RTIO_REST_HEADER_LENGTH_CO_REQ;
    return RTIOSuccess;
}

RTIOStatus_t RTIO_DeSerializeObEstabReqNoCopy( const uint16_t headerId,
                                               const uint8_t* pData, uint16_t dataLength,
                                               RTIOObEstabReq_t* pReq )
{
    if( ( pData == NULL ) || ( pReq == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pData=%p, pReq=%p.",
                    (void*)pData,
                    (void*)pReq ) );
        return RTIOBadParameter;
    }
    if( headerId == 0 )
    {
        LogError( ( "Header id cannot be 0." ) );
        return RTIOProtocalFailed;
    }
    if( dataLength < RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_REQ )
    {
        LogError( ( "Data length is not enough: dataLength=%u, RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_REQ=%u.",
                    dataLength,
                    RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_REQ ) );
        return RTIOProtocalFailed;
    }
    pReq->headerId = headerId;
    pReq->method = ( pData[ 0 ] >> 4 ) & 0x0F;
    pReq->obId = (uint16_t)( pData[ 1 ] << 8 ) + (uint16_t)( pData[ 2 ] );
    pReq->uri = (uint32_t)( pData[ 3 ] << 24 ) + (uint32_t)( pData[ 4 ] << 16 ) + (uint32_t)( pData[ 5 ] << 8 ) + (uint32_t)( pData[ 6 ] );
    pReq->pData = (uint8_t*)pData + RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_REQ;
    pReq->dataLength = dataLength - RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_REQ;
    return RTIOSuccess;
}


RTIOStatus_t RTIO_DeSerializeDevicePingResp( const RTIOHeader_t* pHeader,
                                             const RTIOFixedBuffer_t* pIncommingBuffer,
                                             rtioDeviceSendResp_t* pResp )
{

    if( ( pHeader == NULL ) || ( pResp == NULL ) || ( pIncommingBuffer == NULL )
        || ( pIncommingBuffer->pBuffer == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pHeader=%p, pResp=%p, pIncommingBuffer=%p, pIncommingBuffer->pBuffer=%p.",
                    (void*)pHeader,
                    (void*)pResp,
                    (void*)pIncommingBuffer,
                    (void*)pIncommingBuffer->pBuffer ) );
        return RTIOBadParameter;
    }

    if( pHeader->id == 0 )
    {
        LogError( ( "Header id cannot be 0." ) );
        return RTIOProtocalFailed;
    }

    pResp->headerId = pHeader->id;
    pResp->code = pHeader->code;
    pResp->respLength = pHeader->bodyLen;
    return RTIOSuccess;
}

RTIOStatus_t RTIO_DeSerializeDeviceSendResp( const RTIOHeader_t* pHeader,
                                             const RTIOFixedBuffer_t* pIncommingBuffer,
                                             rtioDeviceSendResp_t* pResp )
{

    if( ( pHeader == NULL ) || ( pResp == NULL ) || ( pIncommingBuffer == NULL )
        || ( pIncommingBuffer->pBuffer == NULL ) || ( pHeader->bodyLen < RTIO_REST_HEADER_LENGTH_MIN ) )
    {
        LogError( ( "Argument cannot be NULL: pHeader=%p, pResp=%p, pIncommingBuffer=%p, pIncommingBuffer->pBuffer=%p, pHeader->bodyLen=%u.",
                    (void*)pHeader,
                    (void*)pResp,
                    (void*)pIncommingBuffer,
                    (void*)pIncommingBuffer->pBuffer,
                    pHeader->bodyLen ) );
        return RTIOBadParameter;
    }

    if( pHeader->id == 0 )
    {
        LogError( ( "Header id cannot be 0." ) );
        return RTIOProtocalFailed;
    }

    pResp->headerId = pHeader->id;
    pResp->code = pHeader->code;
    pResp->respLength = pHeader->bodyLen;
    memcpy( pResp->pFixedBuffer->pBuffer, pIncommingBuffer->pBuffer, pResp->respLength );
    return RTIOSuccess;
}

RTIOStatus_t RTIO_DeSerializeCoResp_FromDeviceSendResp( const rtioDeviceSendResp_t* pDeviceSendResp,
                                                        RTIOCoResp_t* pCoResp )
{
    if( ( pDeviceSendResp == NULL ) || pDeviceSendResp->pFixedBuffer == NULL || ( pCoResp == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pDeviceSendResp=%p, pDeviceSendResp->pFixedBuffer=%p, pCoResp=%p.",
                    (void*)pDeviceSendResp,
                    (void*)pDeviceSendResp->pFixedBuffer,
                    (void*)pCoResp ) );
        return RTIOBadParameter;
    }

    if( ( pDeviceSendResp->pFixedBuffer->pBuffer[ 0 ] >> 4 ) != RTIO_REST_COGET &&
        ( pDeviceSendResp->pFixedBuffer->pBuffer[ 0 ] >> 4 ) != RTIO_REST_COPOST )
    {
        LogError( ( "Method is not RTIO_REST_COPOST: pDeviceSendResp->pFixedBuffer->pBuffer[0] >> 4=%u.",
                    ( pDeviceSendResp->pFixedBuffer->pBuffer[ 0 ] >> 4 ) ) );
        return RTIOProtocalFailed;
    }
    pCoResp->headerId = pDeviceSendResp->headerId;
    pCoResp->code = pDeviceSendResp->pFixedBuffer->pBuffer[ 0 ] & 0x0F;
    pCoResp->pData = &( pDeviceSendResp->pFixedBuffer->pBuffer[ RTIO_REST_HEADER_LENGTH_CO_RESP ] );
    pCoResp->dataLength = pDeviceSendResp->respLength - RTIO_REST_HEADER_LENGTH_CO_RESP;
    return RTIOSuccess;
}
RTIOStatus_t RTIO_SerializeCoReq_OverDeviceSendReq( const RTIOCoReq_t* pReq,
                                                    const RTIOFixedBuffer_t* pFixedBuffer,
                                                    uint16_t* dataLength )
{
    RTIOStatus_t status = RTIOSuccess;

    if( ( pReq == NULL ) || ( pFixedBuffer == NULL ) || ( pFixedBuffer->pBuffer == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pReq=%p, pFixedBuffer=%p, pFixedBuffer->pBuffer=%p.",
                    (void*)pReq,
                    (void*)pFixedBuffer,
                    (void*)pFixedBuffer->pBuffer ) );
        return RTIOBadParameter;
    }

    if( RTIO_PROTOCAL_HEADER_LEN + RTIO_REST_HEADER_LENGTH_CO_REQ + pReq->dataLength > pFixedBuffer->size )
    {
        LogError( ( "Buffer size is not enough: RTIO_PROTOCAL_HEADER_LEN=%u, RTIO_REST_HEADER_LENGTH_CO_REQ=%u, pReq->dataLength=%u, bufsize=%u.",
                    RTIO_PROTOCAL_HEADER_LEN,
                    RTIO_REST_HEADER_LENGTH_CO_REQ,
                    pReq->dataLength,
                    pFixedBuffer->size ) );
        return RTIOBadParameter;
    }

    // serialize coReq
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 0 ] = ( pReq->method << 4 ) & 0xF0;
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 1 ] = ( pReq->uri >> 24 ) & 0xFF;
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 2 ] = ( pReq->uri >> 16 ) & 0xFF;
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 3 ] = ( pReq->uri >> 8 ) & 0xFF;
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 4 ] = ( pReq->uri ) & 0xFF;
    memcpy( &pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + RTIO_REST_HEADER_LENGTH_CO_REQ ],
            pReq->pData, pReq->dataLength );

    // serialize header
    RTIOHeader_t Header = { 0 };
    Header.id = pReq->headerId;
    Header.type = RTIO_TYPE_DEVICE_SEND_REQ;
    Header.version = RTIO_PROTOCAL_VERSION;
    Header.code = REMOTECODE_SUCCESS;
    Header.bodyLen = RTIO_REST_HEADER_LENGTH_CO_REQ + pReq->dataLength;

    status = RTIO_SerializeHeader( &Header, pFixedBuffer );
    if( status != RTIOSuccess )
    {
        return status;
    }
    *dataLength = Header.bodyLen + RTIO_PROTOCAL_HEADER_LEN;
    return status;
}
RTIOStatus_t RTIO_SerializeCoResp_OverServerSendResp( const RTIOCoResp_t* pResp,
                                                      const RTIOFixedBuffer_t* pFixedBuffer,
                                                      uint16_t* length )
{
    if( ( pResp == NULL ) || ( pFixedBuffer == NULL ) || ( pFixedBuffer->pBuffer == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pResp=%p, pFixedBuffer=%p, pFixedBuffer->pBuffer=%p.",
                    (void*)pResp,
                    (void*)pFixedBuffer,
                    (void*)pFixedBuffer->pBuffer ) );
        return RTIOBadParameter;
    }

    if( RTIO_PROTOCAL_HEADER_LEN + RTIO_REST_HEADER_LENGTH_CO_RESP + pResp->dataLength > pFixedBuffer->size )
    {
        LogError( ( "Buffer size is not enough: RTIO_PROTOCAL_HEADER_LEN=%u, RTIO_REST_HEADER_LENGTH_CO_RESP=%u, pResp->dataLength=%u, bufsize=%u.",
                    RTIO_PROTOCAL_HEADER_LEN,
                    RTIO_REST_HEADER_LENGTH_CO_RESP,
                    pResp->dataLength,
                    pFixedBuffer->size ) );
        return RTIOBadParameter;
    }

    RTIOStatus_t status = RTIOSuccess;

    // serialize coResp
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN ] = ( ( pResp->method << 4 ) & 0xF0 )
        + ( (uint8_t)( pResp->code ) & 0x0F );
    memcpy( &pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + RTIO_REST_HEADER_LENGTH_CO_RESP ],
            pResp->pData, pResp->dataLength );

    // serialize header
    RTIOHeader_t Header = { 0 };
    Header.id = pResp->headerId;
    Header.type = RTIO_TYPE_SERVER_SEND_RESP;
    Header.version = RTIO_PROTOCAL_VERSION;
    Header.code = REMOTECODE_SUCCESS;
    Header.bodyLen = RTIO_REST_HEADER_LENGTH_CO_RESP + pResp->dataLength;
    status = RTIO_SerializeHeader( &Header, pFixedBuffer );
    if( status != RTIOSuccess )
    {
        return status;
    }
    *length = Header.bodyLen + RTIO_PROTOCAL_HEADER_LEN;
    return status;
}

RTIOStatus_t RTIO_SerializeObEstabResp_OverServerSendResp( const RTIOObEstabResp_t* pResp,
                                                           const RTIOFixedBuffer_t* pFixedBuffer,
                                                           uint16_t* length )
{
    RTIOStatus_t status = RTIOSuccess;

    if( ( pResp == NULL ) || ( pFixedBuffer == NULL ) || ( pFixedBuffer->pBuffer == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pResp=%p, pFixedBuffer=%p, pFixedBuffer->pBuffer=%p.",
                    (void*)pResp,
                    (void*)pFixedBuffer,
                    (void*)pFixedBuffer->pBuffer ) );
        return RTIOBadParameter;
    }

    if( RTIO_PROTOCAL_HEADER_LEN + RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_RESP > pFixedBuffer->size )
    {
        LogError( ( "Buffer size is not enough: RTIO_PROTOCAL_HEADER_LEN=%u, "
                    "RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_RESP=%u, "
                    "bufsize=%u.",
                    RTIO_PROTOCAL_HEADER_LEN,
                    RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_RESP,
                    pFixedBuffer->size ) );
        return RTIOBadParameter;
    }

    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 0 ] = ( ( pResp->method << 4 ) & 0xF0 )
        + ( (uint8_t)( pResp->code ) & 0x0F );
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 1 ] = ( pResp->obId >> 8 ) & 0xFF;
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 2 ] = ( pResp->obId ) & 0xFF;

    RTIOHeader_t Header = { 0 };
    Header.id = pResp->headerId;
    Header.type = RTIO_TYPE_SERVER_SEND_RESP;
    Header.version = RTIO_PROTOCAL_VERSION;
    Header.code = REMOTECODE_SUCCESS;
    Header.bodyLen = RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_RESP;
    status = RTIO_SerializeHeader( &Header, pFixedBuffer );
    if( status != RTIOSuccess )
    {
        return status;
    }
    *length = Header.bodyLen + RTIO_PROTOCAL_HEADER_LEN;
    return status;
}

RTIOStatus_t RTIO_SerializeObNotifyReq_OverDeviceSendReq( const RTIOObNotifyReq_t* pReq,
                                                          const RTIOFixedBuffer_t* pFixedBuffer,
                                                          uint16_t* dataLength )
{
    RTIOStatus_t status = RTIOSuccess;
    RTIOHeader_t Header = { 0 };

    if( ( pReq == NULL ) || ( pFixedBuffer == NULL ) || ( pFixedBuffer->pBuffer == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pReq=%p, pFixedBuffer=%p, pFixedBuffer->pBuffer=%p.",
                    (void*)pReq,
                    (void*)pFixedBuffer,
                    (void*)pFixedBuffer->pBuffer ) );
        return RTIOBadParameter;
    }

    if( RTIO_PROTOCAL_HEADER_LEN + RTIO_REST_HEADER_LENGTH_OBGET_NOTIFY_REQ + pReq->dataLength > pFixedBuffer->size )
    {
        LogError( ( "Buffer size is not enough: RTIO_PROTOCAL_HEADER_LEN=%u, "
                    "RTIO_REST_HEADER_LENGTH_OBGET_NOTIFY_REQ=%u, pReq->dataLength=%u, bufsize=%u.",
                    RTIO_PROTOCAL_HEADER_LEN,
                    RTIO_REST_HEADER_LENGTH_OBGET_NOTIFY_REQ,
                    pReq->dataLength,
                    pFixedBuffer->size ) );
        return RTIOBadParameter;
    }

    // serialize obGetNotifyReq
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 0 ] = ( ( pReq->method << 4 ) & 0xF0 ) + ( pReq->code & 0x0F );
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 1 ] = (uint8_t)( ( pReq->obId >> 8 ) & 0xFF );
    pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + 2 ] = (uint8_t)( pReq->obId & 0xFF );

    if( pReq->dataLength > 0 && pReq->pData != NULL )
    {
        memcpy( &pFixedBuffer->pBuffer[ RTIO_PROTOCAL_HEADER_LEN + RTIO_REST_HEADER_LENGTH_OBGET_NOTIFY_REQ ],
                pReq->pData, pReq->dataLength );
    }

    // serialize header
    Header.id = pReq->headerId;
    Header.type = RTIO_TYPE_DEVICE_SEND_REQ;
    Header.version = RTIO_PROTOCAL_VERSION;
    Header.code = REMOTECODE_SUCCESS;
    Header.bodyLen = RTIO_REST_HEADER_LENGTH_OBGET_NOTIFY_REQ + pReq->dataLength;
    status = RTIO_SerializeHeader( &Header, pFixedBuffer );
    if( status != RTIOSuccess )
    {
        return status;
    }

    *dataLength = Header.bodyLen + RTIO_PROTOCAL_HEADER_LEN;
    return status;
}

RTIOStatus_t RTIO_DeSerializeObNotifyResp_FromDeviceSendResp( const rtioDeviceSendResp_t* pDeviceSendResp,
                                                              RTIOObNotifyResp_t* pObResp )
{
    if( ( pDeviceSendResp == NULL ) || pDeviceSendResp->pFixedBuffer == NULL || ( pObResp == NULL ) )
    {
        LogError( ( "Argument cannot be NULL: pDeviceSendResp=%p, pDeviceSendResp->pFixedBuffer=%p, pObResp=%p.",
                    (void*)pDeviceSendResp,
                    (void*)pDeviceSendResp->pFixedBuffer,
                    (void*)pObResp ) );
        return RTIOBadParameter;
    }

    if( ( pDeviceSendResp->pFixedBuffer->pBuffer[ 0 ] >> 4 ) != RTIO_REST_OBGET )
    {
        LogError( ( "Method is not RTIO_REST_OBGET: pDeviceSendResp->pFixedBuffer->pBuffer[0] >> 4=%u.",
                    ( pDeviceSendResp->pFixedBuffer->pBuffer[ 0 ] >> 4 ) ) );
        return RTIOBadParameter;
    }
    pObResp->headerId = pDeviceSendResp->headerId;
    pObResp->method = ( pDeviceSendResp->pFixedBuffer->pBuffer[ 0 ] >> 4 ) & 0x0F;
    pObResp->code = pDeviceSendResp->pFixedBuffer->pBuffer[ 0 ] & 0x0F;
    pObResp->obId = ( pDeviceSendResp->pFixedBuffer->pBuffer[ 1 ] << 8 ) + ( pDeviceSendResp->pFixedBuffer->pBuffer[ 2 ] );
    return RTIOSuccess;
}