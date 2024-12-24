# SDK APIs

You can also refer  [core_rtio.h](../libraries/standard/coreRTIO/source/include/core_rtio.h), as it provides more detailed descriptions.

```c
    /* Context for a RTIO connection. */
    typedef struct RTIOContext {} RTIOContext_t;

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
    typedef struct RTIO_ObList {} RTIO_ObList_t;

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

```
