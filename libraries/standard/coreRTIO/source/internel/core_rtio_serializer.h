/*
 * Copyright (c) 2023-2024 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef CORE_RTIO_SERIALIZER_H
#define CORE_RTIO_SERIALIZER_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "core_rtio.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define RTIO_PROTOCAL_VERSION ( 0U )
#define RTIO_PROTOCAL_HEADER_LEN  ( 5U )


#if RTIO_TRANSFER_FRAME_BUF_SIZE == (512U)
#define RTIO_CAP_CURRENT_LEVEL ( 0U )
#elif RTIO_TRANSFER_FRAME_BUF_SIZE == (1024U)
#define RTIO_CAP_CURRENT_LEVEL ( 1U )
#elif RTIO_TRANSFER_FRAME_BUF_SIZE == (2048U)
#define RTIO_CAP_CURRENT_LEVEL ( 2U )
#elif RTIO_TRANSFER_FRAME_BUF_SIZE == (4096U)
#define RTIO_CAP_CURRENT_LEVEL ( 3U )
#else    
#define RTIO_CAP_CURRENT_LEVEL ( 0U )      
#endif

/* Currently only support 1. */
#define RTIO_CAP_LEVELS ( 1U ) 
#define RTIO_CAP_LEVEL_MAX ( RTIO_CAP_LEVELS -1 )

#define RTIO_URI_STRING_LENGTH_MIN ( 5U )
#define RTIO_URI_STRING_LENGTH_MAX ( 128U )

#define RTIO_REST_HEADER_LENGTH_MIN ( 1U )
#define RTIO_REST_HEADER_LENGTH_CO_REQ ( 5U )
#define RTIO_REST_HEADER_LENGTH_CO_RESP ( 1U )
#define RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_REQ ( 7U )
#define RTIO_REST_HEADER_LENGTH_OBGET_ESTAB_RESP ( 3U )
#define RTIO_REST_HEADER_LENGTH_OBGET_NOTIFY_REQ ( 3U )
#define RTIO_REST_HEADER_LENGTH_OBGET_NOTIFY_RESP ( 3U )


    /*-----------------------------------------------------------*/
    enum RTIORemoteCode
    {
        REMOTECODE_UNKOWN_ERR = 0,
        REMOTECODE_SUCCESS = 0x01,
        REMOTECODE_TYPE_ERR = 0x02,
        REMOTECODE_VERIFY_FAIL = 0x03,
        REMOTECODE_PARA_INVALID = 0x04,
        REMOTECODE_LENGTH_ERR = 0x05,
    };
    typedef uint8_t RTIORemoteCode_t;


    enum RTIOMessageType
    {
        RTIO_TYPE_UNKNOWN_TYPE = 0,
        RTIO_TYPE_DEVICE_VERIFY_REQ = 1,
        RTIO_TYPE_DEVICE_VERIFY_RESP = 2,
        RTIO_TYPE_DEVICE_PING_REQ = 3,
        RTIO_TYPE_DEVICE_PING_RESP = 4,
        RTIO_TYPE_DEVICE_SEND_REQ = 5,
        RTIO_TYPE_DEVICE_SEND_RESP = 6,
        RTIO_TYPE_SERVER_SEND_REQ = 7,
        RTIO_TYPE_SERVER_SEND_RESP = 8,
    };
    typedef uint8_t RTIOMessageType_t;

    enum RTIORestMethod
    {
        RTIO_REST_COGET = 1,
        RTIO_REST_COPOST = 2,
        RTIO_REST_OBGET = 3,
    };
    typedef uint8_t RTIORestMethod_t;

    typedef struct RTIOHeader
    {
        uint8_t version;
        RTIOMessageType_t type;
        uint16_t id;
        uint16_t bodyLen;
        RTIORemoteCode_t code;
    } RTIOHeader_t;

    typedef struct RTIOVerifyReq
    {
        RTIOHeader_t header;
        uint8_t capLevel;
        const char* pDeviceId;
        const char* pDeviceSecret;
    } RTIOVerifyReq_t;

    typedef struct RTIOVerifyResp
    {
        RTIOHeader_t header;
    } RTIOVerifyResp_t;


    typedef struct RTIOPingReq
    {
        RTIOHeader_t header;
        uint16_t timeout;
    } RTIOPingReq_t;
    typedef struct RTIOPingResp
    {
        RTIOHeader_t header;
    } RTIOPingResp_t;



    RTIOStatus_t RTIO_SerializeHeader( const RTIOHeader_t* pHeader,
                                       const RTIOFixedBuffer_t* pFixedBuffer );

    RTIOStatus_t RTIO_DeserializeHeader( const RTIOFixedBuffer_t* pFixedBuffer,
                                         RTIOHeader_t* pHeader );

    RTIOStatus_t RTIO_SerializeVerifyReq( RTIOVerifyReq_t* pReq,
                                          const RTIOFixedBuffer_t* pFixedBuffer,
                                          uint16_t* dataLength );

    RTIOStatus_t RTIO_DeserializeVerifyResp( const RTIOFixedBuffer_t* pFixedBuffer,
                                             RTIOVerifyResp_t* pResp );

    RTIOStatus_t RTIO_SerializePingReq( const RTIOPingReq_t* pReq,
                                        const RTIOFixedBuffer_t* pFixedBuffer,
                                        uint16_t* dataLength );
    RTIOStatus_t RTIO_DeSerializeDevicePingResp( const RTIOHeader_t* pHeader,
                                                 const RTIOFixedBuffer_t* pIncommingBuffer,
                                                 rtioDeviceSendResp_t* pResp );
    RTIOStatus_t RTIO_DeSerializeDeviceSendResp( const RTIOHeader_t* pHeader,
                                                 const RTIOFixedBuffer_t* pIncommingBuffer,
                                                 rtioDeviceSendResp_t* pResp );

    /*-----------------------------------------------------------*/

    enum RTIORestStatus
    {
        RTIO_REST_STATUS_UNKNOWN = 0,
        RTIO_REST_STATUS_INTERNAL_SERVER_ERROR = 1, /* TODO: add device internal error or server internal error, #51 */ 
        RTIO_REST_STATUS_OK = 2,
        RTIO_REST_STATUS_CONTINUE = 3,
        RTIO_REST_STATUS_TERMINATE = 4,
        RTIO_REST_STATUS_NOT_FOUNT = 5,
        RTIO_REST_STATUS_BAD_REQUEST = 6,
        RTIO_REST_STATUS_METHOD_NOT_ALLOWED = 7,
        RTIO_REST_STATUS_TOO_MANY_REQUESTS = 8,
        RTIO_REST_STATUS_TOO_MANY_OBSERVERS = 9,
    };
    typedef uint8_t RTIORestStatus_t;


    typedef struct RTIOCoReq
    {
        uint16_t headerId; // redundant for low level message
        RTIORestMethod_t method;
        uint32_t uri;
        uint8_t* pData;
        uint16_t dataLength;
    } RTIOCoReq_t;


    typedef struct RTIOCoResp
    {
        uint16_t headerId; // redundant for low level message
        RTIORestMethod_t method;
        RTIORestStatus_t code;
        uint8_t* pData;
        uint16_t dataLength;
    } RTIOCoResp_t;

    typedef struct RTIOObEstabReq
    {
        uint16_t headerId; // redundant for low level message
        RTIORestMethod_t method;
        uint16_t obId;
        uint32_t uri;
        uint8_t* pData;
        uint16_t dataLength;
    } RTIOObEstabReq_t;
    typedef struct RTIOObEstabResp
    {
        uint16_t headerId; // redundant for low level message
        RTIORestMethod_t method;
        RTIORestStatus_t code;
        uint16_t obId;
    } RTIOObEstabResp_t;

    typedef struct RTIOObNotifyReq
    {
        uint16_t headerId; // redundant for low level message
        RTIORestMethod_t method;
        RTIORestStatus_t code;
        uint16_t obId;
        uint8_t* pData;
        uint16_t dataLength;
    } RTIOObNotifyReq_t;
    typedef struct RTIOObGetNotifyResp
    {
        uint16_t headerId; // redundant for low level message
        RTIORestMethod_t method;
        RTIORestStatus_t code;
        uint16_t obId;
    } RTIOObNotifyResp_t;



    RTIOStatus_t RTIO_DeSerializeCoResp_FromDeviceSendResp( const rtioDeviceSendResp_t* pDeviceSendResp,
                                                            RTIOCoResp_t* pCoResp );

    RTIOStatus_t RTIO_DeSerializeCoReqNoCopy( const uint16_t headerId,
                                              const uint8_t* pData, uint16_t dataLength,
                                              RTIOCoReq_t* pReq );

    RTIOStatus_t RTIO_DeSerializeObEstabReqNoCopy( const uint16_t headerId,
                                                   const uint8_t* pData, uint16_t dataLength,
                                                   RTIOObEstabReq_t* pReq );

    RTIOStatus_t RTIO_DeSerializeCoResp( const uint16_t headerId,
                                         const uint8_t* pData, uint16_t dataLength,
                                         RTIOCoResp_t* pResp );

    RTIOStatus_t RTIO_SerializeCoReq_OverDeviceSendReq( const RTIOCoReq_t* pReq,
                                                        const RTIOFixedBuffer_t* pFixedBuffer,
                                                        uint16_t* dataLength );

    RTIOStatus_t RTIO_SerializeCoResp_OverServerSendResp( const RTIOCoResp_t* pResp,
                                                          const RTIOFixedBuffer_t* pFixedBuffer,
                                                          uint16_t* length );
    RTIOStatus_t RTIO_SerializeObEstabResp_OverServerSendResp( const RTIOObEstabResp_t* pResp,
                                                               const RTIOFixedBuffer_t* pFixedBuffer,
                                                               uint16_t* length );

    RTIOStatus_t RTIO_SerializeObNotifyReq_OverDeviceSendReq( const RTIOObNotifyReq_t* pReq,
                                                              const RTIOFixedBuffer_t* pFixedBuffer,
                                                              uint16_t* dataLength );

    RTIOStatus_t RTIO_DeSerializeObNotifyResp_FromDeviceSendResp( const rtioDeviceSendResp_t* pDeviceSendResp,
                                                                  RTIOObNotifyResp_t* pObResp );

    RTIOStatus_t RTIO_DeSerializeRestMethod( const uint8_t* buf, uint16_t length,
                                             RTIORestMethod_t* method );

#ifdef __cplusplus
}
#endif

#endif /* ifndef CORE_RTIO_SERIALIZER_H */
