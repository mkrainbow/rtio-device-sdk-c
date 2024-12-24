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

#ifndef PLAINTEXT_POSIX_H_
#define PLAINTEXT_POSIX_H_

#include "logging_levels.h"

#ifndef LIBRARY_LOG_NAME
#define LIBRARY_LOG_NAME     "Transport_Plaintext_Sockets"
#endif
#ifndef LIBRARY_LOG_LEVEL
#define LIBRARY_LOG_LEVEL    LOG_DEBUG
#endif

#include "logging_stack.h"


#ifdef __cplusplus
extern "C" {
#endif


#include "sockets_posix.h"
#include "transport_interface.h"

    typedef struct PlaintextParams
    {
        int32_t socketDescriptor;
    } PlaintextParams_t;

    struct NetworkContext
    {
        PlaintextParams_t* pParams;
    };

    struct TransportOption
    {
    };


    TransportStatus_t Plaintext_Connect( NetworkContext_t* pNetworkContext,
                                         const ServerInfo_t* pServerInfo );
    TransportStatus_t Plaintext_ConnectWithOption( NetworkContext_t* pNetworkContext,
                                                   const TransportOption_t* pTransportOptions,
                                                   const ServerInfo_t* pServerInfo );

    TransportStatus_t Plaintext_Disconnect( NetworkContext_t* pNetworkContext );


    int32_t Plaintext_Recv( NetworkContext_t* pNetworkContext,
                            void* pBuffer,
                            size_t bytesToRecv );


    int32_t Plaintext_Send( NetworkContext_t* pNetworkContext,
                            const void* pBuffer,
                            size_t bytesToSend );


#ifdef __cplusplus
}
#endif


#endif /* ifndef PLAINTEXT_POSIX_H_ */
