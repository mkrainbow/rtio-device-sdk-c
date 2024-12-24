/*
 * Copyright (c) 2023-2024 mkrainbow.com.
 *
 * Licensed under MIT.
 * See the LICENSE for detail or copy at https://opensource.org/license/MIT.
 */

#ifndef TRANSPORT_INTERFACE_H_
#define TRANSPORT_INTERFACE_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
    extern "C" {
#endif

struct NetworkContext;
typedef struct NetworkContext NetworkContext_t;

struct ServerInfo;
typedef struct ServerInfo ServerInfo_t;

struct TransportOption;
typedef struct TransportOption TransportOption_t;


typedef enum TransportStatus
{
    TransportUnknown = -1,
    TransportSuccess = 0,
    TransportInvalidParameter = 1,
    TransportInsufficientMemory = 2,
    TransportCredentialsInvalid = 3,
    TransportInternalError = 4,
    TransportDnsFailure = 5,
    TransportConnectFailure = 6,
    TransportDisconnectFailure = 7
} TransportStatus_t;

typedef TransportStatus_t (*TransportConnect_t)(NetworkContext_t *pNetworkContext,
                                     const TransportOption_t *pTransportOptions,
                                     const ServerInfo_t *pServerInfo);

typedef TransportStatus_t (*TransportDisconnect_t)( NetworkContext_t *pNetworkContext);
           

typedef int32_t ( * TransportRecv_t )( NetworkContext_t * pNetworkContext,
                                       void * pBuffer,
                                       size_t bytesToRecv );

typedef int32_t ( * TransportSend_t )( NetworkContext_t * pNetworkContext,
                                       const void * pBuffer,
                                       size_t bytesToSend );
                       
typedef struct TransportInterface
{
    TransportRecv_t recv;               
    TransportSend_t send;                       
    TransportConnect_t connect;         
    TransportDisconnect_t disconnect;   
    NetworkContext_t * pNetworkContext; 
} TransportInterface_t;

#ifdef __cplusplus
    }
#endif


#endif /* ifndef TRANSPORT_INTERFACE_H_ */
