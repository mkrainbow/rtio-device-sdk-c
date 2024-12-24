/*
*
*  Copyright 2021 Espressif Systems (Shanghai) CO LTD
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
/*
 * Modifications:
 * - Modify the returned Status and other elements to adapt to the RTIO transmission interface.
 * Author: mkrainbow.com
 * Date of Modification: 2024-12-24
 */

#ifndef ESP_TLS_TRANSPORT_H
#define ESP_TLS_TRANSPORT_H

/**************************************************/
/******* DO NOT CHANGE the following order ********/
/**************************************************/

/* Logging related header files are required to be included in the following order:
 * 1. Include the header file "logging_levels.h".
 * 2. Define LIBRARY_LOG_NAME and  LIBRARY_LOG_LEVEL.
 * 3. Include the header file "logging_stack.h".
 */

/* Include header that defines log levels. */
#include "logging_levels.h"

/* Logging configuration for the Sockets. */
#ifndef LIBRARY_LOG_NAME
    #define LIBRARY_LOG_NAME     "ESP_TLS"
#endif
#ifndef LIBRARY_LOG_LEVEL
    #define LIBRARY_LOG_LEVEL    LOG_INFO
#endif

#include "logging_stack.h"

/************ End of logging configuration ****************/

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "transport_interface.h"
#include "esp_tls.h"

#include "transport_interface.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */




typedef struct Timeouts
{
    uint16_t connectionTimeoutMs;
    uint16_t sendTimeoutMs;
    uint16_t recvTimeoutMs;
} Timeouts_t;

struct NetworkContext
{
    SemaphoreHandle_t xTlsContextSemaphore;
    esp_tls_t* pxTls;
    const char *pcHostname;          /**< @brief Server host name. */
    int xPort;                       /**< @brief Server port in host-order. */
    const char *pcServerRootCA;      /**< @brief Trusted server root certificate bytes. */
    uint32_t pcServerRootCASize;     /**< @brief Number of trusted server root certificate bytes. */
    const char *pcClientCert;        /**< @brief Client certificate bytes. */
    uint32_t pcClientCertSize;       /**< @brief Number of client certificate bytes. */
    const char *pcClientKey;         /**< @brief Client certificate's private key bytes. */
    uint32_t pcClientKeySize;        /**< @brief Number of client certificate's private key bytes. */
    bool use_secure_element;         /**< @brief Boolean representing the use of secure element
                                                 for the TLS connection. */
    void *ds_data;                   /**< @brief Pointer for digital signature peripheral context */
    const char ** pAlpnProtos;

    /**
    * @brief Disable server name indication (SNI) for a TLS session.
    */
    BaseType_t disableSni;
};

struct ServerInfo
{
    const char * pHostName; /**< @brief Server host name. */
    size_t hostNameLength;  /**< @brief Length of the server host name. */
    uint16_t port;          /**< @brief Server port in host-order. */
};

TransportStatus_t xTlsConnect( NetworkContext_t* pxNetworkContext,
                                  const TransportOption_t* pTransportOptions,
                                  const ServerInfo_t* pServerInfo );

TransportStatus_t xTlsDisconnect( NetworkContext_t* pxNetworkContext );

int32_t espTlsTransportSend( NetworkContext_t* pxNetworkContext,
    const void* pvData, size_t uxDataLen );

int32_t espTlsTransportRecv( NetworkContext_t* pxNetworkContext,
    void* pvData, size_t uxDataLen );

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ESP_TLS_TRANSPORT_H */
