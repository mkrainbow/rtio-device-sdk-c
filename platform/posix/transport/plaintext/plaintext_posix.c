/*
 * AWS IoT Device SDK for Embedded C 202211.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/*
 * Modifications:
 * - Modify the returned Status and other elements to adapt to the RTIO transmission interface.
 * Author: mkrainbow.com
 * Date of Modification: 2024-12-24
 */

#include <assert.h>
#include <string.h>

#include <errno.h>
#include <sys/socket.h>
#include <poll.h>

#include "plaintext_posix.h"

#define TRANSPORT_SEND_RECV_TIMEOUT_MS   ( 10000U )

static void logTransportError( int32_t errorNumber )
{
    
    ( void ) errorNumber;

    LogError( ( "A transport error occurred: %s.", strerror( errorNumber ) ) );
}

static TransportStatus_t convertToTransportStatus(SocketStatus_t socketStatus)
{
    TransportStatus_t status = TransportUnknown;
    switch (socketStatus)
    {
    case SOCKETS_SUCCESS:
        status = TransportSuccess;
        break;
    case SOCKETS_INVALID_PARAMETER:
        status = TransportInvalidParameter;
        break;
    case SOCKETS_INSUFFICIENT_MEMORY:
        status = TransportInsufficientMemory;
        break;
    case SOCKETS_DNS_FAILURE:
        status = TransportDnsFailure;
        break;
    case SOCKETS_CONNECT_FAILURE:
        status = TransportConnectFailure;
        break;
    default:
        LogError(
            ("Unexpected status received from socket wrapper: Socket status = %u",
             socketStatus));
        break;
    }
    return status;
}


TransportStatus_t Plaintext_Connect( NetworkContext_t * pNetworkContext,
                                  const ServerInfo_t * pServerInfo )
{
    SocketStatus_t returnStatus = SOCKETS_SUCCESS;
    PlaintextParams_t * pPlaintextParams = NULL;

    if( ( pNetworkContext == NULL ) || ( pNetworkContext->pParams == NULL ) )
    {
        LogError( ( "Parameter check failed: pNetworkContext is NULL." ) );
        returnStatus = SOCKETS_INVALID_PARAMETER;
    }
    else
    {
        pPlaintextParams = pNetworkContext->pParams;
        returnStatus = Sockets_Connect( &pPlaintextParams->socketDescriptor,
                                        pServerInfo,
                                        TRANSPORT_SEND_RECV_TIMEOUT_MS,
                                        TRANSPORT_SEND_RECV_TIMEOUT_MS );
    }

    return convertToTransportStatus( returnStatus );
}

TransportStatus_t Plaintext_ConnectWithOption(NetworkContext_t *pNetworkContext,
                                              const TransportOption_t *pTransportOptions,
                                              const ServerInfo_t *pServerInfo)
{
    (void)pTransportOptions;
    return Plaintext_Connect(pNetworkContext, pServerInfo);
}


TransportStatus_t Plaintext_Disconnect( NetworkContext_t * pNetworkContext )
{
    SocketStatus_t returnStatus = SOCKETS_SUCCESS;
    PlaintextParams_t * pPlaintextParams = NULL;

    
    if( ( pNetworkContext == NULL ) || ( pNetworkContext->pParams == NULL ) )
    {
        LogError( ( "Parameter check failed: pNetworkContext is NULL." ) );
        returnStatus = TransportInvalidParameter;
    }
    else
    {
        pPlaintextParams = pNetworkContext->pParams;
        returnStatus = Sockets_Disconnect( pPlaintextParams->socketDescriptor );
    }

    return  convertToTransportStatus( returnStatus );;
}



int32_t Plaintext_Recv( NetworkContext_t * pNetworkContext,
                        void * pBuffer,
                        size_t bytesToRecv )
{
    PlaintextParams_t * pPlaintextParams = NULL;
    int32_t bytesReceived = -1, pollStatus = 1;
    struct pollfd pollFds;

    assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    assert( pBuffer != NULL );
    assert( bytesToRecv > 0 );

    
    pPlaintextParams = pNetworkContext->pParams;

    
    pollFds.events = POLLIN | POLLPRI;
    pollFds.revents = 0;
    
    pollFds.fd = pPlaintextParams->socketDescriptor;

    
    pollStatus = poll( &pollFds, 1, 0 );

    if( pollStatus > 0 )
    {
        
        bytesReceived = ( int32_t ) recv( pPlaintextParams->socketDescriptor,
                                          pBuffer,
                                          bytesToRecv,
                                          0 );
    }
    else if( pollStatus < 0 )
    {
        
        bytesReceived = -1;
    }
    else
    {
        
        bytesReceived = 0;
    }

    
    if( ( pollStatus > 0 ) && ( bytesReceived == 0 ) )
    {
        
        bytesReceived = -1;
    }
    else if( bytesReceived < 0 )
    {
        logTransportError( errno );
    }
    else
    {
        
    }

    return bytesReceived;
}



int32_t Plaintext_Send( NetworkContext_t * pNetworkContext,
                        const void * pBuffer,
                        size_t bytesToSend )
{
    PlaintextParams_t * pPlaintextParams = NULL;
    int32_t bytesSent = -1, pollStatus = -1;
    struct pollfd pollFds;

    assert( pNetworkContext != NULL && pNetworkContext->pParams != NULL );
    assert( pBuffer != NULL );
    assert( bytesToSend > 0 );

    
    pPlaintextParams = pNetworkContext->pParams;

    
    pollFds.events = POLLOUT;
    pollFds.revents = 0;
    
    pollFds.fd = pPlaintextParams->socketDescriptor;

    
    pollStatus = poll( &pollFds, 1, 0 );

    if( pollStatus > 0 )
    {
        
        bytesSent = ( int32_t ) send( pPlaintextParams->socketDescriptor,
                                      pBuffer,
                                      bytesToSend,
                                      0 );
    }
    else if( pollStatus < 0 )
    {
        
        bytesSent = -1;
    }
    else
    {
        
        bytesSent = 0;
    }

    if( ( pollStatus > 0 ) && ( bytesSent == 0 ) )
    {
        
        bytesSent = -1;
    }
    else if( bytesSent < 0 )
    {
        logTransportError( errno );
    }
    else
    {
        
    }

    return bytesSent;
}

