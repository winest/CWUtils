#include "stdafx.h"
#include "CWDllInjectClient.h"


#include "_GenerateTmh.h"
#include "CWDllInjectClient.tmh"

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



BOOL CDllInjectClient::Connect( CONST WCHAR * aKey , PFN_DLL_INJECT_CLIENT_INIT_CBK aInitCbk , PFN_DLL_INJECT_CLIENT_DATA_RECV_CBK aDataCbk )
{
    DbgOut( VERB , DBG_DLL_INJECT_MGR , "Enter. aKey=%ws" , aKey );

    _ASSERT( aKey );
    BOOL bRet = FALSE;
    
    HANDLE hSmInit = NULL;
    DLL_INJECT_SERVER_SM_INIT * pSmInit = NULL;

    if ( this->IsConnected() )
    {
        if ( m_wstrKey == aKey )
        {
            DbgOut( WARN , DBG_DLL_INJECT_MGR , "Shared memory already connected. aKey=%ws" , aKey );
            SetLastError( ERROR_ALREADY_EXISTS );
            bRet = TRUE;
        }
        else
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Shared memory already used by others. m_wstrKey=%ws, aKey=%ws" , m_wstrKey.c_str() , aKey );
            SetLastError( ERROR_ADDRESS_ALREADY_ASSOCIATED );
        }
        goto exit;
    }

    //Get all necessary handles and configuration from named share memory
    hSmInit = OpenFileMappingW( FILE_MAP_READ | FILE_MAP_WRITE , FALSE , aKey );
    if ( ! hSmInit )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "OpenFileMappingW() with name %ws failed. GetLastError()=%!WINERROR!" , aKey , GetLastError() );
        goto exit;
    }
    pSmInit = (DLL_INJECT_SERVER_SM_INIT *)MapViewOfFile( hSmInit , FILE_MAP_ALL_ACCESS , 0 , 0 , 0 );
    if ( ! pSmInit )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "MapViewOfFile() with name %ws failed. GetLastError()=%!WINERROR!" , aKey , GetLastError() );
        goto exit;
    }

    //Shared memory can be opened. Get data from shared memory first and then validate the content later
    m_wstrKey = aKey;
    CopyMemory( &m_SmInit , pSmInit , sizeof(m_SmInit) );

    pSmInit->InitRsp.dwHookStatus = ERROR_APP_INIT_FAILURE;
    pSmInit->InitRsp.hModule = (UINT64)GetModuleHandleW( NULL );

    

    //Validate Local handles
    if ( NULL == pSmInit->InitReq.Local.pLocalCtx || NULL == pSmInit->InitReq.Local.hRemoteProc || NULL == pSmInit->InitReq.Local.pfnFreeLibrary )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "InitReq.Local data are invalid" );
        goto exit;
    }

    //Validate Remote handles
    if ( NULL == pSmInit->InitReq.Remote.hEvtInitRsp || NULL == pSmInit->InitReq.Remote.hDllInjectMgrAliveThread )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "InitReq.Remote data are invalid" );
        goto exit;
    }
    for ( size_t i = 0 ; i < _countof( pSmInit->InitReq.Remote.hPerServerSm ) ; i++ )
    {
        if ( NULL == pSmInit->InitReq.Remote.hPerServerSm[i] )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "InitReq.Remote.hPerServerSm are invalid" );
            goto exit;
        }
    }
    for ( size_t i = 0 ; i < _countof( pSmInit->InitReq.Remote.hPerServerMutex ) ; i++ )
    {
        if ( NULL == pSmInit->InitReq.Remote.hPerServerMutex[i] )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "InitReq.Remote.hPerServerMutex are invalid" );
            goto exit;
        }
    }
    for ( size_t i = 0 ; i < _countof( pSmInit->InitReq.Remote.hPerServerEvt ) ; i++ )
    {
        if ( NULL == pSmInit->InitReq.Remote.hPerServerEvt[i] )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "InitReq.Remote.hPerServerEvt are invalid" );
            goto exit;
        }
    }
    for ( size_t i = 0 ; i < _countof( pSmInit->InitReq.Remote.hPerClientEvt ) ; i++ )
    {
        if ( NULL == pSmInit->InitReq.Remote.hPerClientEvt[i] )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "InitReq.Remote.hPerClientEvt are invalid" );
            goto exit;
        }
    }

    //Get shared memory used to exchange data
    m_SmData[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] = (PDLL_INJECT_SERVER_SM_DATA_HEADER)MapViewOfFile( (HANDLE)pSmInit->InitReq.Remote.hPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] , FILE_MAP_ALL_ACCESS , 0 , 0 , 0 );
    if ( NULL == m_SmData[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "MapViewOfFile() with PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL failed. GetLastError()=%!WINERROR!" , GetLastError() );
        pSmInit->InitRsp.dwHookStatus = GetLastError();
        goto exit;
    }
    m_SmData[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] = (PDLL_INJECT_SERVER_SM_DATA_HEADER)MapViewOfFile( (HANDLE)pSmInit->InitReq.Remote.hPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] , FILE_MAP_ALL_ACCESS , 0 , 0 , 0 );
    if ( NULL == m_SmData[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "MapViewOfFile() with PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE failed. GetLastError()=%!WINERROR!" , GetLastError() );
        pSmInit->InitRsp.dwHookStatus = GetLastError();
        goto exit;
    }

    DbgOut( VERB , DBG_DLL_INJECT_MGR , "Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP]=0x%p, Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_REQ_OK]=0x%p, Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_RSP]=0x%p" ,
            (HANDLE)pSmInit->InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] , (HANDLE)pSmInit->InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_REQ_OK] , (HANDLE)pSmInit->InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_RSP] );

    //Create a thread to receive data from server
    if ( aDataCbk )
    {
        UINT uDataThreadId = 0;
        m_hDataRecvThread = (HANDLE)_beginthreadex( NULL , 0 , CDllInjectClient::DataRecvThread , this , 0 , &uDataThreadId );
        if ( NULL == m_hDataRecvThread )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "_beginthreadex() with for DataRecvThread failed. GetLastError()=%!WINERROR!" , GetLastError() );
            pSmInit->InitRsp.dwHookStatus = GetLastError();
            goto exit;
        }
        m_pfnDataRecv = aDataCbk;
    }
    



    pSmInit->InitRsp.dwHookStatus = ( aInitCbk ) ? aInitCbk( pSmInit->InitReq.wzServerDirPath , pSmInit->InitReq.wzClientCfgPath ) : ERROR_SUCCESS;
    if ( ERROR_SUCCESS == pSmInit->InitRsp.dwHookStatus )
    {
        ATOMIC_ASSIGN( m_bConnected , TRUE );
        bRet = TRUE;
    }
    
exit :
    if ( pSmInit && pSmInit->InitReq.Remote.hEvtInitRsp )
    {
        SetEvent( (HANDLE)pSmInit->InitReq.Remote.hEvtInitRsp );
        CloseHandle( (HANDLE)pSmInit->InitReq.Remote.hEvtInitRsp );
        pSmInit->InitReq.Remote.hEvtInitRsp = NULL;
        m_SmInit.InitReq.Remote.hEvtInitRsp = NULL;
    }

    if ( pSmInit && FALSE == UnmapViewOfFile( pSmInit ) )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "UnmapViewOfFile() pSmInit failed. GetLastError()=%!WINERROR!" , GetLastError() );
    }
    if ( hSmInit && FALSE == CloseHandle( hSmInit ) )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "CloseHandle() hSmInit failed. GetLastError()=%!WINERROR!" , GetLastError() );
    }

    if ( FALSE == bRet )
    {
        this->Disconnect();
    }
    return bRet;
}

BOOL CDllInjectClient::Disconnect()
{
    if ( m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] )
    {
        SetEvent( (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] );
    }

    if ( NULL != m_hDataRecvThread )
    {
        WaitForSingleObject( m_hDataRecvThread , INFINITE );
        CloseHandle( m_hDataRecvThread );
        m_hDataRecvThread = NULL;        
    }

    //Close all handles on m_SmInit
    if ( m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread )
    {
        CloseHandle( (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread );
        m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread = NULL;
    }

    for ( size_t i = 0 ; i < _countof( m_SmInit.InitReq.Remote.hPerServerSm ) ; i++ )
    {
        if ( m_SmData[i] )
        {
            UnmapViewOfFile( m_SmData[i] );
            m_SmData[i] = NULL;
        }
        if ( m_SmInit.InitReq.Remote.hPerServerSm[i] )
        {
            CloseHandle( (HANDLE)m_SmInit.InitReq.Remote.hPerServerSm[i] );
            m_SmInit.InitReq.Remote.hPerServerSm[i] = NULL;
        }
    }

    for ( size_t i = 0 ; i < _countof( m_SmInit.InitReq.Remote.hPerServerMutex ) ; i++ )
    {
        if ( m_SmInit.InitReq.Remote.hPerServerMutex[i] )
        {
            CloseHandle( (HANDLE)m_SmInit.InitReq.Remote.hPerServerMutex[i] );
            m_SmInit.InitReq.Remote.hPerServerMutex[i] = NULL;
        }
    }

    for ( size_t i = 0 ; i < _countof( m_SmInit.InitReq.Remote.hPerServerEvt ) ; i++ )
    {
        if ( m_SmInit.InitReq.Remote.hPerServerEvt[i] )
        {
            CloseHandle( (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[i] );
            m_SmInit.InitReq.Remote.hPerServerEvt[i] = NULL;
        }
    }

    for ( size_t i = 0 ; i < _countof( m_SmInit.InitReq.Remote.hPerClientEvt ) ; i++ )
    {
        if ( m_SmInit.InitReq.Remote.hPerClientEvt[i] )
        {
            CloseHandle( (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[i] );
            m_SmInit.InitReq.Remote.hPerClientEvt[i] = NULL;
        }
    }

    m_wstrKey.clear();
    return TRUE;
}

BOOL CDllInjectClient::SendData( CHAR * aReqBuf , DWORD aReqBufSize , CHAR * aRspBuf , DWORD * aRspBufSize )
{
    _ASSERT( m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread &&
             m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE] &&
             m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] &&
             m_SmInit.InitReq.Remote.hPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] &&
             m_SmInit.InitReq.Remote.hPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] &&
             m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] &&
             m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_REQ] &&
             m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] &&
             m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_REQ_OK] &&
             m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_RSP] &&
             m_SmInit.InitReq.Local.pLocalCtx &&
             m_SmData[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] &&
             m_SmData[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] );

    if ( FALSE == this->IsConnected() )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Not connected" );
        return FALSE;
    }

    BOOL bRet = FALSE;

    HANDLE hWaitInstance[] = { (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] ,
                               (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] ,
                               (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread ,
                               (HANDLE)m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE] };
    HANDLE hWaitSmR2L[] = { (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] ,
                            (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] ,
                            (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread ,
                            (HANDLE)m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] };
    HANDLE hWaitReqOk[] = { (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] ,
                            (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] ,
                            (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread ,
                            (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_REQ_OK] };
    HANDLE hWaitRsp[] = { (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] ,
                          (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] ,
                          (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread ,
                          (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_RSP] };
    
    DWORD dwWaitInstance = WaitForMultipleObjects( _countof(hWaitInstance) , hWaitInstance , FALSE , INFINITE );
    switch ( dwWaitInstance )
    {
        case WAIT_OBJECT_0 :                //PER_SERVER_EVT_INDEX_STOP
        case WAIT_OBJECT_0 + 1 :            //PER_CLIENT_EVT_INDEX_STOP
        case WAIT_OBJECT_0 + 2 :            //hDllInjectMgrAliveThread
            DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or local process left" );
            break;

        case WAIT_OBJECT_0 + 3 :            //PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE
        case WAIT_ABANDONED_0 + 3 :
        {
            DWORD dwWaitSmR2L = WaitForMultipleObjects( _countof(hWaitSmR2L) , hWaitSmR2L , FALSE , INFINITE );
            switch ( dwWaitSmR2L )
            {
                case WAIT_OBJECT_0 :            //PER_SERVER_EVT_INDEX_STOP
                case WAIT_OBJECT_0 + 1 :        //PER_CLIENT_EVT_INDEX_STOP
                case WAIT_OBJECT_0 + 2 :        //hDllInjectMgrAliveThread
                    DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or local process left" );
                    break;

                case WAIT_OBJECT_0 + 3 :        //PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L
                case WAIT_ABANDONED_0 + 3 :
                {
                    DbgOut( INFO , DBG_DLL_INJECT_MGR , "Filling data to share memory for scanning" );
                    //Fill pUserData into share memory R2L
                    DLL_INJECT_SERVER_SM_DATA_HEADER * pSmR2L = m_SmData[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL];
                    DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ * pReq = (DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ *)&pSmR2L->pData;

                    CopyMemory( &pReq->pReq , aReqBuf , aReqBufSize );
                    pReq->dwReqSize = aReqBufSize;
                    pSmR2L->dwDataSize = FIELD_OFFSET( DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ , pReq ) + pReq->dwReqSize;
                    pSmR2L->uDataType = DLL_INJECT_SERVER_SM_DATA_TYPE_GENERAL_REQ;
                    pSmR2L->pLocalCtx = m_SmInit.InitReq.Local.pLocalCtx;
                    pSmR2L->hRemoteProc = (UINT64)m_SmInit.InitReq.Local.hRemoteProc;
                    pSmR2L->dwRemotePid = GetCurrentProcessId();

                    //Signal scan request event
                    SetEvent( (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_REQ] );
                    break;
                }
                default :
                {
                    DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitSmR2L , GetLastError() );
                    break;
                }
            }
            ReleaseMutex( (HANDLE)m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] );
            break;
        }
        default :
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitInstance , GetLastError() );
            break;
        }
    }

    DWORD dwWaitReqOk = WaitForMultipleObjects( _countof(hWaitReqOk) , hWaitReqOk , FALSE , INFINITE );
    switch ( dwWaitReqOk )
    {
        case WAIT_OBJECT_0 :                //PER_SERVER_EVT_INDEX_STOP
        case WAIT_OBJECT_0 + 1 :            //PER_CLIENT_EVT_INDEX_STOP
        case WAIT_OBJECT_0 + 2 :            //hDllInjectMgrAliveThread
        {
            DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or local process left" );
            break;
        }
        case WAIT_OBJECT_0 + 3 :            //PER_CLIENT_EVT_INDEX_REMOTE_REQ_OK
        case WAIT_ABANDONED_0 + 3 :
        {
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "Got scan request received event" );
            break;
        }
        default :
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitReqOk , GetLastError() );
            break;
        }
    }
    ReleaseMutex( (HANDLE)m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE] );




    //Wait for scan response
    DWORD dwWaitRsp = WaitForMultipleObjects( _countof(hWaitRsp) , hWaitRsp , FALSE , INFINITE );
    switch ( dwWaitRsp )
    {
        case WAIT_OBJECT_0 :                //PER_SERVER_EVT_INDEX_STOP
        case WAIT_OBJECT_0 + 1 :            //PER_CLIENT_EVT_INDEX_STOP
        case WAIT_OBJECT_0 + 2 :            //hDllInjectMgrAliveThread
        {
            DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or local process left" );
            break;
        }
        case WAIT_OBJECT_0 + 3 :            //PER_CLIENT_EVT_INDEX_REMOTE_RSP
        case WAIT_ABANDONED_0 + 3 :
        {
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "Got scan response event" );
            
            //Get data from share memory L2R
            DLL_INJECT_SERVER_SM_DATA_HEADER * pSmL2R = m_SmData[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE];
            _ASSERT( DLL_INJECT_SERVER_SM_DATA_TYPE_GENERAL_RSP == pSmL2R->uDataType );

            DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP * pRsp = (DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP *)&pSmL2R->pData;
            if ( aRspBufSize )
            {
                CopyMemory( aRspBuf , &pRsp->pRsp , min(pRsp->dwRspSize , *aRspBufSize) );
                *aRspBufSize = pRsp->dwRspSize;
            }
            else
            {
                CopyMemory( aRspBuf , &pRsp->pRsp , pRsp->dwRspSize );
            }            
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "Scan result is %!HEXDUMP!" , WppHexDump((CONST UCHAR *)pRsp->pRsp,(ULONG)pRsp->dwRspSize) );

            //Signal scan response received event to let worker thread release the share memory
            SetEvent( (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_RSP_OK] );
            bRet = TRUE;
            break;
        }
        default :
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitReqOk , GetLastError() );
            break;
        }
    }

    return bRet;
}

UINT CALLBACK CDllInjectClient::DataRecvThread( VOID * pThis )
{
    CDllInjectClient * pSelf = (CDllInjectClient *)pThis;
    UINT uRet = pSelf->DoDataRecv();
    if ( ERROR_SUCCESS != uRet )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "DataRecv exit abnormally. Try to set stop event. uRet=%!WINERROR!. GetLastError()=%!WINERROR!" , uRet , GetLastError() );
        SetEvent( pSelf->GetClientQuitEvt() );
    }
    return uRet;
}

DWORD CDllInjectClient::DoDataRecv()
{
    _ASSERT( m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread &&
             m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE] &&
             m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] &&
             m_SmInit.InitReq.Remote.hPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] &&
             m_SmInit.InitReq.Remote.hPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] &&
             m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] &&
             m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_LOCAL_REQ_OK] &&
             m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_LOCAL_RSP] &&
             m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] &&
             m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_LOCAL_REQ] &&
             m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_LOCAL_RSP_OK] &&
             m_SmInit.InitReq.Local.pLocalCtx &&
             m_SmData[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] &&
             m_SmData[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] &&
             NULL != m_pfnDataRecv );

    BOOL bStop = FALSE;
    DWORD dwRet = ERROR_NOT_READY;
    HANDLE hEvtWaitData[] = { (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] , 
                              (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] , 
                              (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread ,
                              (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_LOCAL_REQ] };
    HANDLE hWaitInstance[] = { (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] ,
                               (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] ,
                               (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread ,
                               (HANDLE)m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE] };
    HANDLE hWaitSmR2L[] = { (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] ,
                            (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] ,
                            (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread ,
                            (HANDLE)m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] };
    HANDLE hWaitRspOk[] = { (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] ,
                            (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] ,
                            (HANDLE)m_SmInit.InitReq.Remote.hDllInjectMgrAliveThread ,
                            (HANDLE)m_SmInit.InitReq.Remote.hPerClientEvt[PER_CLIENT_EVT_INDEX_LOCAL_RSP_OK] };

    CHAR * pReqCopy = NULL , * pRspCopy = NULL;
    DWORD dwReqSizeCopy = 0 , dwRspSizeCopy = 0;
    while ( ! bStop )
    {
        //Listen for data event
        DWORD dwWaitData = WaitForMultipleObjects( _countof(hEvtWaitData) , hEvtWaitData , FALSE , INFINITE );
        switch ( dwWaitData )
        {
            case WAIT_OBJECT_0 :                //PER_SERVER_EVT_INDEX_STOP
            case WAIT_ABANDONED_0 :
            case WAIT_OBJECT_0 + 1 :            //PER_CLIENT_EVT_INDEX_STOP
            case WAIT_ABANDONED_0 + 1 :
            case WAIT_OBJECT_0 + 2 :            //hDllInjectMgrAliveThread
            case WAIT_ABANDONED_0 + 2 :
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or local process left" );
                dwRet = ERROR_SUCCESS;
                bStop = TRUE;
                break;
            }
            case WAIT_OBJECT_0 + 3 :        //PER_CLIENT_EVT_INDEX_LOCAL_REQ
            {
                DbgOut( INFO , DBG_DLL_INJECT_MGR , "Get data from share memory" );
                DLL_INJECT_SERVER_SM_DATA_HEADER * pSmL2R = m_SmData[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE];
                DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ * pReq = (DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ *)&pSmL2R->pData;

                dwReqSizeCopy = pReq->dwReqSize;
                if ( NULL != pReq->pReq && 0 < dwReqSizeCopy )
                {
                    pReqCopy = new (std::nothrow) CHAR[dwReqSizeCopy];
                    if ( NULL == pReqCopy )
                    {
                        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to allocate memory for pReqCopy. dwReqSizeCopy=%u" , dwReqSizeCopy );
                        dwRet = ERROR_OUTOFMEMORY;
                    }
                    else
                    {
                        CopyMemory( pReqCopy , &pReq->pReq , dwReqSizeCopy );
                        dwRet = ERROR_SUCCESS;
                    }
                }
                else
                {
                    dwRet = ERROR_NO_DATA;
                }
                pSmL2R->dwStatus = dwRet;
                SetEvent( (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_LOCAL_REQ_OK] );
                break;
            }
            default :
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitData , GetLastError() );
                dwRet = ERROR_INVALID_HANDLE_STATE;
                bStop = TRUE;
                break;
            }
        }
        if ( bStop )
        {
            break;
        }


        //Call data recv callback
        if ( ERROR_SUCCESS == dwRet )
        {
            dwRet = m_pfnDataRecv( pReqCopy , dwReqSizeCopy , &pRspCopy , &dwRspSizeCopy );
            delete [] pReqCopy;
            pReqCopy = NULL;
            dwReqSizeCopy = 0;
        }
        else
        {
            continue;
        }
        


        //Get lock and write data back to shared memory
        DWORD dwWaitInstance = WaitForMultipleObjects( _countof(hWaitInstance) , hWaitInstance , FALSE , INFINITE );
        switch ( dwWaitInstance )
        {
            case WAIT_OBJECT_0 :                //PER_SERVER_EVT_INDEX_STOP
            case WAIT_ABANDONED_0 :
            case WAIT_OBJECT_0 + 1 :            //PER_CLIENT_EVT_INDEX_STOP
            case WAIT_ABANDONED_0 + 1 :
            case WAIT_OBJECT_0 + 2 :            //hDllInjectMgrAliveThread
            case WAIT_ABANDONED_0 + 2 :
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or local process left" );
                dwRet = ERROR_SUCCESS;
                bStop = TRUE;
                break;
            }
            case WAIT_OBJECT_0 + 3 :            //PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE
            case WAIT_ABANDONED_0 + 3 :
            {
                DWORD dwWaitSmR2L = WaitForMultipleObjects( _countof(hWaitSmR2L) , hWaitSmR2L , FALSE , INFINITE );
                switch ( dwWaitSmR2L )
                {
                    case WAIT_OBJECT_0 :                //PER_SERVER_EVT_INDEX_STOP
                    case WAIT_ABANDONED_0 :
                    case WAIT_OBJECT_0 + 1 :            //PER_CLIENT_EVT_INDEX_STOP
                    case WAIT_ABANDONED_0 + 1 :
                    case WAIT_OBJECT_0 + 2 :            //hDllInjectMgrAliveThread
                    case WAIT_ABANDONED_0 + 2 :
                    {
                        DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or local process left" );
                        dwRet = ERROR_SUCCESS;
                        bStop = TRUE;
                        break;
                    }
                    case WAIT_OBJECT_0 + 3 :        //PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L
                    case WAIT_ABANDONED_0 + 3 :
                    {
                        DbgOut( INFO , DBG_DLL_INJECT_MGR , "Filling data to share memory for scanning" );
                        DLL_INJECT_SERVER_SM_DATA_HEADER * pSmR2L = m_SmData[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL];
                        DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP * pRsp = (DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP *)&pSmR2L->pData;

                        if ( NULL != pRspCopy && 0 < dwRspSizeCopy )
                        {
                            CopyMemory( &pRsp->pRsp , pRspCopy , dwRspSizeCopy );
                        }
                        pRsp->dwRspSize = dwRspSizeCopy;
                        pSmR2L->dwDataSize = FIELD_OFFSET( DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP , pRsp ) + pRsp->dwRspSize;
                        pSmR2L->uDataType = DLL_INJECT_SERVER_SM_DATA_TYPE_GENERAL_RSP;
                        pSmR2L->pLocalCtx = m_SmInit.InitReq.Local.pLocalCtx;
                        pSmR2L->hRemoteProc = (UINT64)m_SmInit.InitReq.Local.hRemoteProc;
                        pSmR2L->dwRemotePid = GetCurrentProcessId();
                        pSmR2L->dwStatus = dwRet;

                        //Signal data response event
                        SetEvent( (HANDLE)m_SmInit.InitReq.Remote.hPerServerEvt[PER_SERVER_EVT_INDEX_LOCAL_RSP] );



                        
                        //Waiting for response ok event
                        DWORD dwWaitRspOk = WaitForMultipleObjects( _countof(hWaitRspOk) , hWaitRspOk , FALSE , INFINITE );
                        switch ( dwWaitRspOk )
                        {
                            case WAIT_OBJECT_0 :                //PER_SERVER_EVT_INDEX_STOP
                            case WAIT_ABANDONED_0 :
                            case WAIT_OBJECT_0 + 1 :            //PER_CLIENT_EVT_INDEX_STOP
                            case WAIT_ABANDONED_0 + 1 :
                            case WAIT_OBJECT_0 + 2 :            //hDllInjectMgrAliveThread
                            case WAIT_ABANDONED_0 + 2 :
                            {
                                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or local process left" );
                                dwRet = ERROR_SUCCESS;
                                bStop = TRUE;
                                break;
                            }
                            case WAIT_OBJECT_0 + 3 :            //PER_CLIENT_EVT_INDEX_LOCAL_RSP_OK
                            case WAIT_ABANDONED_0 + 3 :
                            {
                                DbgOut( INFO , DBG_DLL_INJECT_MGR , "Get local rsp ok event" );
                                dwRet = ERROR_SUCCESS;
                                break;
                            }
                            default :
                            {
                                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitRspOk , GetLastError() );
                                dwRet = ERROR_INVALID_HANDLE_STATE;
                                bStop = TRUE;
                                break;
                            }
                        }
                        break;
                    }
                    default :
                    {
                        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitSmR2L , GetLastError() );
                        dwRet = ERROR_INVALID_HANDLE_STATE;
                        bStop = TRUE;
                        break;
                    }
                }
                ReleaseMutex( (HANDLE)m_SmInit.InitReq.Remote.hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] );
                break;
            }
            default :
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitInstance , GetLastError() );
                dwRet = ERROR_INVALID_HANDLE_STATE;
                bStop = TRUE;
                break;
            }
        }
        if ( bStop )
        {
            break;
        }
        if ( pRspCopy && dwRspSizeCopy )
        {
            delete [] pRspCopy;
            pRspCopy = NULL;
            dwRspSizeCopy = 0;
        }
    }

    if ( pReqCopy )
    {
        delete [] pReqCopy;
    }
    if ( pRspCopy )
    {
        delete [] pRspCopy;
    }

    return dwRet;
}






#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils