#include "stdafx.h"
#include "CWDllInjectServer.h"
#pragma warning( disable : 4127 )

using namespace std;

#include "_GenerateTmh.h"
#include "CWDllInjectServer.tmh"

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif



#define KERNEL32_MODULE_NAME                 L"kernel32.dll"

#define REMOTE_THREAD_SETUP_READY_TIMEOUT    ( 30 * 1000 )
#define REMOTE_THREAD_FREE_TIMEOUT           ( 20 * 1000 )



BOOL CDllInjectServer::RegisterDllInject( DllInjectServerUserCfg * aUserCfg )
{
    _ASSERT( NULL != aUserCfg );
    BOOL bRet = FALSE;

    do 
    {
        if ( ATOMIC_READ(m_bStarted) )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Please call RegisterDllInject() before StartMonitor()" );
            SetLastError( ERROR_ALREADY_RUNNING_LKG );
            break;
        }

        //At least one of the aDllPath should be set
        if ( 0 == aUserCfg->wstrDllPath32.length() && 0 == aUserCfg->wstrDllPath32.length() )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "At least one of the DllPath should be set" );
            SetLastError( ERROR_INVALID_PARAMETER );
            break;
        }

        if ( 0 < aUserCfg->wstrDllPath32.length() && FALSE == CWUtils::IsFileExist( aUserCfg->wstrDllPath32.c_str() ) )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "File not found. DllPath32=%ws, GetLastError()=%!WINERROR!" , aUserCfg->wstrDllPath32.c_str() , GetLastError() );
            break;
        }
        if ( 0 < aUserCfg->wstrDllPath64.length() && FALSE == CWUtils::IsFileExist( aUserCfg->wstrDllPath64.c_str() ) )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "File not found. DllPath64=%ws, GetLastError()=%!WINERROR!" , aUserCfg->wstrDllPath64.c_str() , GetLastError() );
            break;
        }

        m_UserCfg = *aUserCfg;
        bRet = TRUE;
    } while ( 0 );

    return bRet;
}

BOOL CDllInjectServer::UnregisterDllInject()
{
    BOOL bRet = FALSE;

    do 
    {
        if ( this->IsStarted() )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Please call StopMonitor() before UnregisterDllInject()" );
            SetLastError( ERROR_ALREADY_RUNNING_LKG );
            break;
        }

        bRet = TRUE;
    } while ( 0 );
    
    if ( TRUE == bRet )
    {
        m_UserCfg.wstrDllPath32.clear();
        m_UserCfg.wstrDllPath64.clear();
        m_UserCfg.pUserCtx = NULL;
        m_UserCfg.InjectedCbk = NULL;
        m_UserCfg.ScanCbk = NULL;
        m_UserCfg.UnInjectedCbk = NULL;
    }

    return bRet;
}

BOOL CDllInjectServer::StartMonitor( BOOL aCheckExistProcs )
{
    BOOL bRet = FALSE;

    if ( 0 == m_UserCfg.wstrDllPath32.length() && 0 == m_UserCfg.wstrDllPath64.length() )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Please call RegisterDllInject() before StartMonitor()" );
        goto exit;
    }

    if ( ATOMIC_READ(m_bStarted) )
    {
        DbgOut( WARN , DBG_DLL_INJECT_MGR , "Already started" );
        bRet = TRUE;
        goto exit;
    }

    //Load configuration from m_wstrCfgPath into m_CommonCfg
    if ( FALSE == this->ReloadCommonConfig() )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to load common configuration from registry" );
        goto exit;
    }

    if ( FALSE == this->CreateCommonHandles() )
    {
        goto exit;
    }   
    
    //Start JobCreator thread and worker thread if the configuration is enabled
    if ( m_CommonCfg.bEnabled )
    {
        //Create job events, creator thread and worker threads
        if ( FALSE == this->CreateJobThreads() )
        {
            goto exit;
        }
    }

    //All data are initialized
    bRet = TRUE;
    ATOMIC_ASSIGN( m_bStarted , TRUE );

    //Check existing processes
    if ( aCheckExistProcs && FALSE == this->ReloadClientStateTable() )
    {
        DbgOut( WARN , DBG_DLL_INJECT_MGR , "Failed to inject to some existing processes. GetLastError()=%!WINERROR!" , GetLastError() );
    }

exit :
    if ( FALSE == m_bStarted )
    {
        this->DestroyJobThreads();
        this->DestroyCommonHandles();
    }
    return bRet;
}

BOOL CDllInjectServer::StopMonitor()
{
    //Cleanup client state table entry
    EnterCriticalSection( &m_csClientStateTable );
    for ( map<DWORD , InjectClientInfo>::iterator it = m_mapClientStateTable.begin() ; it != m_mapClientStateTable.end() ; )
    {
        this->StopInject( it->first );
        m_mapClientStateTable.erase( it++ );
    }
    LeaveCriticalSection( &m_csClientStateTable );

    //Close all threads and handles we created
    this->DestroyJobThreads();
    this->DestroyCommonHandles();

    ATOMIC_ASSIGN( m_bStarted , FALSE );
    return TRUE;
}

DWORD CDllInjectServer::SendData( DWORD aPid , CHAR * aReqBuf , DWORD aReqBufSize , CHAR * aRspBuf , DWORD * aRspBufSize )
{
    _ASSERT( aReqBuf && 0 < aReqBufSize );

    DWORD dwRet = ERROR_NOT_READY;
    if ( FALSE == this->IsStarted() )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Not started" );
        return dwRet;
    }


    EnterCriticalSection( &m_csClientStateTable );
    InjectClientInfo * pHookProcInfo = NULL;
    map<DWORD , InjectClientInfo>::iterator it = m_mapClientStateTable.find( aPid );
    if ( it != m_mapClientStateTable.end() )
    {
        pHookProcInfo = &it->second;
    }
    if ( WAIT_TIMEOUT == WaitForSingleObject( pHookProcInfo->hRemoteProc , 0 ) )
    {
        //Fill data and set data event
        EnterCriticalSection( &m_csSmL2R );

        ResetEvent( m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_RSP_OK] );
        DLL_INJECT_SERVER_SM_DATA_HEADER * pSmL2R = m_pPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE];
        DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ * pReq = (DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ *)&pSmL2R->pData;
        DbgOut( VERB , DBG_DLL_INJECT_MGR , "pSmL2R=0x%p, FIELD_OFFSET(DLL_INJECT_SERVER_SM_DATA_HEADER,pData)=0x%X, pRsp=0x%p" , pSmL2R , FIELD_OFFSET(DLL_INJECT_SERVER_SM_DATA_HEADER,pData) , pReq );
        
        pReq->dwReqSize = aReqBufSize;
        CopyMemory( &pReq->pReq , aReqBuf , pReq->dwReqSize );
        pSmL2R->dwDataSize = FIELD_OFFSET( DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ , pReq ) + pReq->dwReqSize;
        pSmL2R->uDataType = DLL_INJECT_SERVER_SM_DATA_TYPE_GENERAL_REQ;
        pSmL2R->pLocalCtx = (UINT64)pHookProcInfo;
        pSmL2R->hRemoteProc = (UINT64)pHookProcInfo->hRemoteProc; //Useless currently
        pSmL2R->dwRemotePid = aPid;         //Useless currently
        pSmL2R->dwStatus = ERROR_SUCCESS;   //Useless currently
        SetEvent( pHookProcInfo->hPerClientEvt[PER_CLIENT_EVT_INDEX_LOCAL_REQ] );

        HANDLE hEvtWaitReqOk[] = { m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] , pHookProcInfo->hRemoteProc , m_hPerServerEvt[PER_SERVER_EVT_INDEX_LOCAL_REQ_OK] };
        DWORD dwWaitReqOk = WaitForMultipleObjects( _countof(hEvtWaitReqOk) , hEvtWaitReqOk , FALSE , INFINITE );
        switch ( dwWaitReqOk )
        {
            case WAIT_OBJECT_0 :            //PER_SERVER_EVT_INDEX_STOP
            case WAIT_ABANDONED_0 :
            case WAIT_OBJECT_0 + 1 :        //hRemoteProc
            case WAIT_ABANDONED_0 + 1 :
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or remote process left. dwWaitReqOk=0x%08X" , dwWaitReqOk );
                dwRet = ERROR_SUCCESS;
                break;
            }
            case WAIT_OBJECT_0 + 2 :        //PER_SERVER_EVT_INDEX_LOCAL_REQ_OK
            case WAIT_ABANDONED_0 + 2 :
            {
                DbgOut( INFO , DBG_DLL_INJECT_MGR , "Remote thread has already got the request. dwWaitReqOk=0x%08X" , dwWaitReqOk );
                dwRet = pSmL2R->dwStatus;
                break;
            }
            default :
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Unexpected return value 0x%08X. GetLastError()=%!WINERROR!" , dwWaitReqOk , GetLastError() );
                dwRet = ERROR_INVALID_HANDLE_STATE;
                break;
            }
        }

        LeaveCriticalSection( &m_csSmL2R );

        if ( ERROR_SUCCESS != dwRet )
        {
            return dwRet;
        }




        //Wait for remote rsp event
        HANDLE hEvtWaitRsp[] = { m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] , pHookProcInfo->hRemoteProc , m_hPerServerEvt[PER_SERVER_EVT_INDEX_LOCAL_RSP] };
        DWORD dwWaitRsp = WaitForMultipleObjects( _countof(hEvtWaitRsp) , hEvtWaitRsp , FALSE , INFINITE );
        switch ( dwWaitRsp )
        {
            case WAIT_OBJECT_0 :            //PER_SERVER_EVT_INDEX_STOP
            case WAIT_ABANDONED_0 :
            case WAIT_OBJECT_0 + 1 :        //hRemoteProc
            case WAIT_ABANDONED_0 + 1 :
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Got stop event or remote process left. dwWaitReqOk=0x%08X" , dwWaitReqOk );
                dwRet = ERROR_SUCCESS;
                break;
            }
            case WAIT_OBJECT_0 + 2 :        //PER_SERVER_EVT_INDEX_LOCAL_RSP
            case WAIT_ABANDONED_0 + 2 :
            {
                DbgOut( INFO , DBG_DLL_INJECT_MGR , "Got remote thread response" );

                //Get data from share memory R2L
                DLL_INJECT_SERVER_SM_DATA_HEADER * pSmR2L = m_pPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL];
                _ASSERT( DLL_INJECT_SERVER_SM_DATA_TYPE_GENERAL_RSP == pSmL2R->uDataType );

                DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP * pRsp = (DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP *)&pSmR2L->pData;
                if ( pRsp->pRsp && pRsp->dwRspSize && aRspBuf && aRspBufSize )
                {
                    if ( pRsp->dwRspSize < *aRspBufSize )
                    {
                        CopyMemory( aRspBuf , &pRsp->pRsp , pRsp->dwRspSize );
                        *aRspBufSize = pRsp->dwRspSize;
                        dwRet = ERROR_SUCCESS;
                    }
                    else
                    {
                        CopyMemory( aRspBuf , &pRsp->pRsp , *aRspBufSize );
                        dwRet = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    DbgOut( INFO , DBG_DLL_INJECT_MGR , "Client response is %!HEXDUMP!" , WppHexDump((CONST UCHAR *)pRsp->pRsp,(ULONG)pRsp->dwRspSize) );
                }
                else
                {
                    dwRet = ERROR_SUCCESS;
                }            
                pSmR2L->dwStatus = dwRet;

                //Signal scan response received event to let worker thread release the share memory
                SetEvent( (HANDLE)pHookProcInfo->hPerClientEvt[PER_CLIENT_EVT_INDEX_LOCAL_RSP_OK] );
                break;
            }
            default :
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Unexpected return value 0x%X. GetLastError()=%!WINERROR!" , dwWaitRsp , GetLastError() );
                dwRet = ERROR_INVALID_HANDLE_STATE;
                break;
            }
        }
    }
    else
    {
        DbgOut( WARN , DBG_DLL_INJECT_MGR , "Remote process is down" );
    }

    LeaveCriticalSection( &m_csClientStateTable );
    return dwRet;
}

BOOL CDllInjectServer::OnProcessCreateTerminate( BOOL aCreate , DWORD aPid , CONST WCHAR * aProcPath , CONST WCHAR * aBaseName )
{
    UNREFERENCED_PARAMETER( aBaseName );
    if ( FALSE == m_CommonCfg.bEnabled )
    {
        return FALSE;
    }

    //If the process path match the config we loaded in Init(), add this pid to m_mapClientStateTable
    BOOL bRet = FALSE;
    if ( m_CommonCfg.wstrProcPath == aProcPath )
    {
        DbgOut( INFO , DBG_DLL_INJECT_MGR , "Process path match. aCreate=%d, dwPid=0x%04X, wzProcPath=%ws" , aCreate , aPid , aProcPath );
        if ( aCreate )
        {
            //Create client state table entry
            InjectClientInfo info;
            info.uInjectClientState = INJECT_CLIENT_STATE_PREPARING;
            info.wstrProcPath = aProcPath;
            ATOMIC_INC( info.lRefCnt );
            EnterCriticalSection( &m_csClientStateTable );
            std::map< DWORD , InjectClientInfo >::iterator it = m_mapClientStateTable.find( aPid );
            if ( it == m_mapClientStateTable.end() )
            {
                DbgOut( INFO , DBG_DLL_INJECT_MGR , "Add new entry to ClientStateTable. PID=0x%04X" , aPid );
                m_mapClientStateTable.insert( std::map< DWORD , InjectClientInfo >::value_type( aPid , info ) );
                bRet = TRUE;
            }
            else if ( WAIT_TIMEOUT != WaitForSingleObject(it->second.hRemoteProc , 0) )
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "PID exists in ClientStateTable but process is terminated. Skip it" );
            }
            else
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "PID exists but still receive process create event. Skip it" );
            }
            LeaveCriticalSection( &m_csClientStateTable );
        }
        else
        {
            //Cleanup client state table entry
            EnterCriticalSection( &m_csClientStateTable );
            std::map<DWORD , InjectClientInfo>::iterator it = m_mapClientStateTable.find( aPid );
            if ( it != m_mapClientStateTable.end() )
            {
                bRet = this->StopInject( aPid );
                if ( 0 == ATOMIC_DEC(it->second.lRefCnt) )
                {
                    m_mapClientStateTable.erase( it );
                }
                else
                {
                    DbgOut( WARN , DBG_DLL_INJECT_MGR , "Don't free the entry since someone is using it. lRetCnt=%ld" , ATOMIC_READ(it->second.lRefCnt) );
                }
            }
            LeaveCriticalSection( &m_csClientStateTable );
        }
    }

    return bRet;
}

BOOL CDllInjectServer::OnThreadCreateTerminate( BOOL aCreate , DWORD aPid , DWORD aTid )
{
    UNREFERENCED_PARAMETER( aCreate );
    UNREFERENCED_PARAMETER( aPid );
    UNREFERENCED_PARAMETER( aTid );
    return FALSE;
}

BOOL CDllInjectServer::OnImageLoaded( DWORD aPid , CONST WCHAR * aImagePath , CONST WCHAR * aBaseName )
{
    UNREFERENCED_PARAMETER( aBaseName );
    if ( FALSE == m_CommonCfg.bEnabled )
    {
        return FALSE;
    }

    //If all necessary DLLs are loaded, return TRUE
    BOOL bRet = FALSE;
    if ( 0 == _wcsicmp( KERNEL32_MODULE_NAME , aBaseName ) )
    {
        EnterCriticalSection( &m_csClientStateTable );
        std::map< DWORD , InjectClientInfo >::iterator it = m_mapClientStateTable.find( aPid );
        if ( it != m_mapClientStateTable.end() && INJECT_CLIENT_STATE_PREPARING == it->second.uInjectClientState &&
             WAIT_TIMEOUT != WaitForSingleObject(it->second.hRemoteProc , 0) )    //This prevent from double injection on the same process
        {
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "Image name match. dwPid=0x%04X, wzImagePath=%ws" , aPid , aImagePath );
            it->second.uInjectClientState = INJECT_CLIENT_STATE_CAN_INJECT;
            bRet = this->StartInject( aPid );
        }
        LeaveCriticalSection( &m_csClientStateTable );
    }

    return bRet;
}

BOOL CDllInjectServer::StartInject( DWORD aPid )
{
    DbgOut( VERB , DBG_DLL_INJECT_MGR , "Enter. aPid=0x%04X" , aPid );

    BOOL bRet = FALSE;

    DWORD dwRet = ERROR_DLL_INIT_FAILED;
    HANDLE hInitRspEvent = NULL;
    LPVOID pRemoteBuffer = NULL;
    HANDLE hRemoteThread = NULL;
    HANDLE hSmInit = NULL;
    DLL_INJECT_SERVER_SM_INIT * pSmInit = NULL;
    map<DWORD , InjectClientInfo>::iterator it;

    //For the aPid entry in m_mapClientStateTable whose uInjectClientState is INJECT_CLIENT_STATE_CAN_INJECT
    //  1. Create a named share memory by using a specific GUID combined with process id
    //  2. Fill share memory all necessary handles by DuplicateHandle(event+mutex+share memory)
    //  3. Use VirtualAllocEx to allocate DLL name, fill DLL name with according to remote process's architecture(x86 or x64)
    //  4. CreateRemoteThread() and wait for remote thread's response
    //  5. Get ClientState in named share memory and update the ClientStateTable. Close handles if ClientState is failed

    if ( FALSE == this->IsStarted() )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "CDllInjectServer for rule %ws is not started. Skip it" , m_wstrRuleName.c_str() );
        goto exit;
    }

    EnterCriticalSection( &m_csClientStateTable );
    it = m_mapClientStateTable.find( aPid );
    do 
    {
        if ( it == m_mapClientStateTable.end() )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "PID not found in table. aPid=0x%04X" , aPid );
            break;
        }
        if ( INJECT_CLIENT_STATE_CAN_INJECT != it->second.uInjectClientState )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Client state is not INJECT_CLIENT_STATE_CAN_INJECT. aPid=0x%04X" , aPid );
            break;
        }

        //Open remote process
        it->second.hRemoteProc = OpenProcess( PROCESS_ALL_ACCESS , FALSE , it->first );
        if ( NULL == it->second.hRemoteProc )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "OpenProcess() faild. PID=%lu, GetLastError()=%!WINERROR!" , it->first , GetLastError() );
            dwRet = GetLastError();
            break;
        }

        //Select corresponding setting according to remote process's architecture
        CONST WCHAR * wzModulePath = NULL;
        LPTHREAD_START_ROUTINE lpLoadLibrary = NULL;
        LPTHREAD_START_ROUTINE lpFreeLibrary = NULL;
        if ( CWUtils::IsWin32Process( it->second.hRemoteProc ) )
        {
            wzModulePath = m_UserCfg.wstrDllPath32.c_str();
            lpLoadLibrary = (LPTHREAD_START_ROUTINE)m_uLoadLibraryW32;
            lpFreeLibrary = (LPTHREAD_START_ROUTINE)m_uFreeLibrary32;
        }
        else
        {
            wzModulePath = m_UserCfg.wstrDllPath64.c_str();
            lpLoadLibrary = (LPTHREAD_START_ROUTINE)m_uLoadLibraryW64;
            lpFreeLibrary = (LPTHREAD_START_ROUTINE)m_uFreeLibrary64;
        }

        //Create per-process initialization completion event
        hInitRspEvent = CreateEvent( NULL , FALSE , FALSE , NULL );
        if ( NULL == hInitRspEvent )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create initialization response event. GetLastError()=%!WINERROR!" , GetLastError() );
            dwRet = GetLastError();
            break;
        }            

        //Allocate memory in remote process
        SIZE_T nDataSize = ( wcslen(wzModulePath) + 1 ) * sizeof(WCHAR);
        pRemoteBuffer = VirtualAllocEx( it->second.hRemoteProc , NULL , nDataSize , MEM_RESERVE | MEM_COMMIT , PAGE_READWRITE );
        if ( NULL == pRemoteBuffer )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "VirtualAllocEx() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            dwRet = GetLastError();
            break;
        }
            
        //Copy wzModulePath to remote process's memory space
        if ( FALSE == WriteProcessMemory( it->second.hRemoteProc , pRemoteBuffer , (LPVOID)wzModulePath , nDataSize , NULL) ) 
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "WriteProcessMemory() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            dwRet = GetLastError();
            break;
        }

        //Prepare per-process share memory
        WCHAR wzSmPerClientess[MAX_PATH] = { 0 };
        GenerateShareMemoryName( wzSmPerClientess , it->first );

        SECURITY_ATTRIBUTES secAttr;
        SECURITY_DESCRIPTOR secDesc;
        ZeroMemory( &secAttr , sizeof(secAttr) );
        ZeroMemory( &secDesc , sizeof(secDesc) );
        InitializeSecurityDescriptor( &secDesc , SECURITY_DESCRIPTOR_REVISION );
        SetSecurityDescriptorDacl( &secDesc , TRUE , NULL , FALSE );
        secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        secAttr.lpSecurityDescriptor = &secDesc;
        secAttr.bInheritHandle = FALSE;
        hSmInit = CreateFileMappingW( INVALID_HANDLE_VALUE , &secAttr , PAGE_READWRITE , 0 , sizeof(DLL_INJECT_SERVER_SM_INIT) , wzSmPerClientess );
        if ( NULL == hSmInit )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "CreateFileMapping() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            dwRet = GetLastError();
            break;
        }

        pSmInit = (DLL_INJECT_SERVER_SM_INIT *)MapViewOfFile( hSmInit , FILE_MAP_ALL_ACCESS , 0 , 0 , 0 );
        if ( NULL == pSmInit )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "MapViewOfFile() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            dwRet = GetLastError();
            break;
        }

        //Fill per-process share memory
        ZeroMemory( pSmInit , sizeof(DLL_INJECT_SERVER_SM_INIT) );

        pSmInit->InitRsp.dwHookStatus = ERROR_OPEN_FAILED;    //Set initial response to error for the case remote process cannot open share memory

        pSmInit->InitReq.Local.hRemoteProc = (UINT64)it->second.hRemoteProc;
        pSmInit->InitReq.Local.pLocalCtx = (UINT64)&it->second;
        pSmInit->InitReq.Local.pfnFreeLibrary = (UINT64)lpFreeLibrary;
        wcsncpy_s( pSmInit->InitReq.wzServerDirPath , m_wstrModDir.c_str() , _TRUNCATE );
        wcsncpy_s( pSmInit->InitReq.wzClientCfgPath , m_CommonCfg.wstrClientCfgPath.c_str() , _TRUNCATE );

        it->second.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] = CreateEventW( NULL , FALSE , FALSE , NULL );
        it->second.hPerClientEvt[PER_CLIENT_EVT_INDEX_LOCAL_REQ] = CreateEventW( NULL , FALSE , FALSE , NULL );
        it->second.hPerClientEvt[PER_CLIENT_EVT_INDEX_LOCAL_RSP_OK] = CreateEventW( NULL , FALSE , FALSE , NULL );
        it->second.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_REQ_OK] = CreateEventW( NULL , FALSE , FALSE , NULL );
        it->second.hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_RSP] = CreateEventW( NULL , FALSE , FALSE , NULL );

        //Duplicate local handles to remote handles
        DuplicateHandle( GetCurrentProcess() , hInitRspEvent , it->second.hRemoteProc , (LPHANDLE)&pSmInit->InitReq.Remote.hEvtInitRsp , 0 , FALSE , DUPLICATE_SAME_ACCESS );
        DuplicateHandle( GetCurrentProcess() , m_hActiveThread , it->second.hRemoteProc , (LPHANDLE)&pSmInit->InitReq.Remote.hDllInjectMgrAliveThread , 0 , FALSE , DUPLICATE_SAME_ACCESS );

        for ( size_t i = 0 ; i < _countof( pSmInit->InitReq.Remote.hPerServerSm ) ; i++ )
        {
            DuplicateHandle( GetCurrentProcess() , m_hPerServerSm[i] , it->second.hRemoteProc , (LPHANDLE)&pSmInit->InitReq.Remote.hPerServerSm[i] , 0 , FALSE , DUPLICATE_SAME_ACCESS );
        }
        for ( size_t i = 0 ; i < _countof( pSmInit->InitReq.Remote.hPerServerMutex ) ; i++ )
        {
            DuplicateHandle( GetCurrentProcess() , m_hPerServerMutex[i] , it->second.hRemoteProc , (LPHANDLE)&pSmInit->InitReq.Remote.hPerServerMutex[i] , 0 , FALSE , DUPLICATE_SAME_ACCESS );
        }
        for ( size_t i = 0 ; i < _countof( pSmInit->InitReq.Remote.hPerServerEvt ) ; i++ )
        {
            DuplicateHandle( GetCurrentProcess() , m_hPerServerEvt[i] , it->second.hRemoteProc , (LPHANDLE)&pSmInit->InitReq.Remote.hPerServerEvt[i] , 0 , FALSE , DUPLICATE_SAME_ACCESS );
        }
        for ( size_t i = 0 ; i < _countof( pSmInit->InitReq.Remote.hPerClientEvt ) ; i++ )
        {
            DuplicateHandle( GetCurrentProcess() , it->second.hPerClientEvt[i] , it->second.hRemoteProc , (LPHANDLE)&pSmInit->InitReq.Remote.hPerClientEvt[i] , 0 , FALSE , DUPLICATE_SAME_ACCESS );
        }

        DbgOut( INFO , DBG_DLL_INJECT_MGR , "Ready to create remote LoadLibraryW thread. Remote PID=%lu" , it->first );
        hRemoteThread = CWUtils::TryCreateRemoteThread( it->second.hRemoteProc , NULL , 0 , lpLoadLibrary , pRemoteBuffer , 0 , NULL );
        if ( NULL == hRemoteThread )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create remote thread. Remote PID=%lu, GetLastError()=%!WINERROR!" , it->first , GetLastError() );
            dwRet = GetLastError();
            break;
        }

        DbgOut( INFO , DBG_DLL_INJECT_MGR , "Waiting for remote thread's initialization response for %d milli-seconds" , REMOTE_THREAD_SETUP_READY_TIMEOUT );
        DWORD dwWaitInitRsp = WaitForSingleObject( hInitRspEvent , REMOTE_THREAD_SETUP_READY_TIMEOUT );
        if ( WAIT_OBJECT_0 == dwWaitInitRsp )
        {
            //Update results
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "Get remote thread's initialization response. hRemoteDll=0x%p, dwHookStatus=%!WINERROR!" , (HMODULE)pSmInit->InitRsp.hModule , pSmInit->InitRsp.dwHookStatus );
            it->second.hRemoteDll = (HMODULE)pSmInit->InitRsp.hModule;
            dwRet = pSmInit->InitRsp.dwHookStatus; //ERROR_SUCCESS
        }
        else
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to get remote thread's initialization response. dwWaitInitRsp=%d. GetLastError()=%!WINERROR!. dwHookStatus=%!WINERROR!" , dwWaitInitRsp , GetLastError() , pSmInit->InitRsp.dwHookStatus );
            dwRet = GetLastError();
        }
    } while ( 0 );

    //Update ClientStateTable's status
    if ( ERROR_SUCCESS == dwRet && NULL != hRemoteThread )
    {
        DbgOut( INFO , DBG_DLL_INJECT_MGR , "Successfully injected to remote process. PID=0x%04X, hRemoteProc=0x%p" , it->first , it->second.hRemoteProc );
        it->second.uInjectClientState = INJECT_CLIENT_STATE_ALREADY_INJECTED;
        bRet = TRUE;
    }
    else if ( ERROR_NOT_SUPPORTED == dwRet || ERROR_RESOURCE_ENUM_USER_STOP == dwRet )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to inject to remote process. Need to update pattern. PID=0x%04X. dwRet=%!WINERROR!" , it->first , dwRet );
        it->second.uInjectClientState = INJECT_CLIENT_STATE_NEED_UPDATE_PATTERN;
    }
    else
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to inject to remote process. PID=0x%04X, dwRet()=%!WINERROR!" , it->first , dwRet );
        it->second.uInjectClientState = INJECT_CLIENT_STATE_INJECT_FAIL;
    }

    //Clean up
    if ( NULL != hRemoteThread )
    {
        CloseHandle( hRemoteThread );
    }
    if ( NULL != pSmInit && FALSE == UnmapViewOfFile( pSmInit ) )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "UnmapViewOfFile() failed. GetLastError()=%!WINERROR!" , GetLastError() );
    }
    if ( NULL != hSmInit )
    {
        CloseHandle( hSmInit );
    }
    if ( NULL != pRemoteBuffer && FALSE == VirtualFreeEx( it->second.hRemoteProc , pRemoteBuffer , 0 , MEM_RELEASE ) )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "VirtualFreeEx() failed. pRemoteBuffer=0x%p, GetLastError()=%!WINERROR!" , pRemoteBuffer , GetLastError() );
    }
    if ( NULL != hInitRspEvent )
    {
        CloseHandle( hInitRspEvent );
    }
    if ( FALSE == bRet )
    {
        it->second.ClearNonReusablePart();
    }

    LeaveCriticalSection( &m_csClientStateTable );
    if ( TRUE == bRet && NULL != m_UserCfg.InjectedCbk )
    {
        m_UserCfg.InjectedCbk( aPid , m_UserCfg.pUserCtx );
    }

exit :
    return bRet;
}

BOOL CDllInjectServer::StopInject( DWORD aPid )
{
    DbgOut( VERB , DBG_DLL_INJECT_MGR , "Enter. aPid=0x%04X" , aPid );
    BOOL bRet = FALSE;

    //Update ClientStateTable, set all process's state to INJECT_CLIENT_STATE_CAN_INJECT if the state was INJECT_CLIENT_STATE_ALREADY_INJECTED

    EnterCriticalSection( &m_csClientStateTable );
    map<DWORD , InjectClientInfo>::iterator it = m_mapClientStateTable.find( aPid );
    do 
    {
        if ( it == m_mapClientStateTable.end() )
        {
            DbgOut( WARN , DBG_DLL_INJECT_MGR , "PID not found in table. aPid=0x%04X" , aPid );
            bRet = TRUE;
            break;
        }
        if ( INJECT_CLIENT_STATE_ALREADY_INJECTED != it->second.uInjectClientState )
        {
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "Process is not injected. aPid=0x%04X, uInjectClientState=%lu" , aPid , it->second.uInjectClientState );
            bRet = TRUE;
            break;
        }
        
        if ( WAIT_TIMEOUT == WaitForSingleObject(it->second.hRemoteProc , 0) )
        {
            //Instead of using CWUtils::TryCreateRemoteThread() with FreeLibrary, we would like to set a remote quit event to avoid loader lock
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "Ready to set remote quit event. Remote aPid=0x%04X" , aPid );
            SetEvent( it->second.hPerClientEvt[PER_CLIENT_EVT_INDEX_STOP] );
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "Reset running entry PID=0x%04X with state=%d. GetLastError()=%!WINERROR!" , 
                                                 it->first , it->second.uInjectClientState , GetLastError() );
            it->second.ClearNonReusablePart();
            it->second.uInjectClientState = INJECT_CLIENT_STATE_CAN_INJECT;
        }
        else
        {
            DbgOut( WARN , DBG_DLL_INJECT_MGR , "Remote process is already terminated" );
        }

        bRet = TRUE;
    } while ( 0 );

    LeaveCriticalSection( &m_csClientStateTable );
    if ( TRUE == bRet && NULL != m_UserCfg.UnInjectedCbk )
    {
        m_UserCfg.UnInjectedCbk( aPid , m_UserCfg.pUserCtx );
    }

    DbgOut( VERB , DBG_DLL_INJECT_MGR , "StopHook() Leave. bRet=%d" , bRet );
    return bRet;
}



BOOL CDllInjectServer::GetConfig( DLL_INJECT_SERVER_CFG_TYPE aCfgType , IN OUT VOID * pData , IN OUT UINT * pDataSize  )
{
    _ASSERT( pDataSize );
    BOOL bRet = FALSE;
    //If *uDataSize is not enough, fill necessary size and return TMPX_INSUFFICIENT_BUFFER
    //Else, fill configuration according to aCfgType to pData and return TMPX_SUCCESS;

    switch ( aCfgType )
    {
        case DLL_INJECT_SERVER_CFG_IS_ENABLED :
        {
            if ( sizeof(BOOL) != *pDataSize )
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Parameter size mismatched. *pDataSize=%lu" , *pDataSize );
                *pDataSize = sizeof(BOOL);
                goto exit;
            }
            *((BOOL *)pData) = m_CommonCfg.bEnabled;
            break;
        }
        default :
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Unexpected aCfgType=%d" , aCfgType );
            goto exit;
        }
    }

    bRet = TRUE;

exit :
    return bRet;
}


BOOL CDllInjectServer::ChangeConfig( DLL_INJECT_SERVER_CFG_TYPE aCfgType , IN VOID * pData , IN UINT uDataSize )
{
    DbgOut( VERB , DBG_DLL_INJECT_MGR, "ChangeConfig() Enter. aCfgType=%lu" , aCfgType );

    BOOL bRet = FALSE;
    //If *uDataSize is not correct, fill necessary size and return TMPX_INSUFFICIENT_BUFFER
    //Else, change configuration according to aCfgType and pData and return TMPX_SUCCESS;
    switch ( aCfgType )
    {
        case DLL_INJECT_SERVER_CFG_ENABLE_DISABLE :
        {
            //For DLL_INJECT_SERVER_CFG_ENABLE_DISABLE, call StartHook() or StopHook() accordingly
            if ( sizeof(BOOL) != uDataSize )
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Parameter size mismatched. uDataSize=%lu" , uDataSize );
                SetLastError( ERROR_INVALID_PARAMETER );
                goto exit;
            }
            BOOL bEnabled = ( *((BOOL *)pData) ) ? TRUE : FALSE;
            DbgOut( INFO , DBG_DLL_INJECT_MGR , "ChangeConfig() with DLL_INJECT_SERVER_CFG_ENABLE_DISABLE from %d to %d" , m_CommonCfg.bEnabled , bEnabled );

            if ( FALSE == m_CommonCfg.bEnabled && TRUE == bEnabled )
            {
                EnterCriticalSection( &m_csClientStateTable );
                m_CommonCfg.bEnabled = TRUE;
                if ( FALSE == this->StartMonitor( TRUE ) )
                {
                    DbgOut( ERRO , DBG_DLL_INJECT_MGR , "StartMonitor() failed. GetLastError()=%!WINERROR!" , GetLastError() );
                }
                LeaveCriticalSection( &m_csClientStateTable );
            }
            else if ( TRUE == m_CommonCfg.bEnabled && FALSE == bEnabled )
            {
                EnterCriticalSection( &m_csClientStateTable );
                if ( FALSE == this->StopMonitor() )
                {
                    DbgOut( ERRO , DBG_DLL_INJECT_MGR , "StartMonitor() failed. GetLastError()=%!WINERROR!" , GetLastError() );
                }
                m_CommonCfg.bEnabled = FALSE;
                LeaveCriticalSection( &m_csClientStateTable );
            }
            else
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Config is not changed. Skip it" );
            }
            break;
        }
        default :
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Unexpected aCfgType=%d" , aCfgType );
            SetLastError( ERROR_INVALID_PARAMETER );
            goto exit;
        }
    }

    bRet = TRUE;

exit :
    DbgOut( VERB , DBG_DLL_INJECT_MGR, "ChangeConfig() leave. bRet=%d" , bRet );
    return bRet;
}



BOOL CDllInjectServer::ReloadCommonConfig()
{
    BOOL bRet = FALSE;

    do 
    {
        map<wstring , wstring> mapKeyVal;
        if ( FALSE == CWUtils::GetIniSectionValues( m_wstrCfgPath.c_str() , m_wstrRuleName.c_str() , mapKeyVal ) || 0 == mapKeyVal.size() )
        {
            DbgOut( WARN , DBG_DLL_INJECT_MGR , "Config value is empty. CfgPath=%ws, RuleName=%ws" , m_wstrCfgPath.c_str() , m_wstrRuleName.c_str() );
            break;
        }

        for ( map<wstring,wstring>::iterator itMap = mapKeyVal.begin() ; itMap != mapKeyVal.end() ; itMap++ )
        {
            if ( itMap->first == L"Enabled" )
            {
                m_CommonCfg.bEnabled = wcstoul(itMap->second.c_str() , NULL , 10) ? TRUE : DLL_INJECT_SERVER_ENABLE_DEFAULT;
            }
            else if ( itMap->first == L"WorkerCnt" )
            {
                m_CommonCfg.ulWorkerCnt = max( DLL_INJECT_SERVER_WORKER_COUNT_MIN , min(DLL_INJECT_SERVER_WORKER_COUNT_MAX , wcstoul(itMap->second.c_str() , NULL , 10)) );
            }
            else if ( itMap->first == L"ProcPath" )
            {
                m_CommonCfg.wstrProcPath = itMap->second;
            }
            else if ( itMap->first == L"ClientCfgPath" )
            {
                m_CommonCfg.wstrClientCfgPath = itMap->second;
            }
            else
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Unknown data \"%ws=%ws\" under rule \"%ws\"" , itMap->first.c_str() , itMap->second.c_str() , m_wstrRuleName.c_str() );
            }
        }

        if ( 0 == m_CommonCfg.wstrProcPath.length() )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "ProcPath is empty under rule \"%ws\"" , m_wstrRuleName.c_str() );
            break;
        }

        DbgOut( INFO , DBG_DLL_INJECT_MGR , "Rule %ws. Enabled=%d, ulWorkerCnt=%lu, wstrClientCfgPath=%ws, wstrClientCfgPath=%ws" , 
                m_wstrRuleName.c_str() , m_CommonCfg.bEnabled , m_CommonCfg.ulWorkerCnt , m_CommonCfg.wstrProcPath.c_str() , m_CommonCfg.wstrClientCfgPath.c_str() );
        bRet = TRUE;
    } while ( 0 );

    return bRet;
}



BOOL CDllInjectServer::ReloadClientStateTable()
{
    BOOL bRet = FALSE;
    DWORD dwRetSize;
    DWORD dwPid[4096];
    HMODULE hMod[4096];
    DWORD dwPidCount;

    DbgOut( INFO , DBG_DLL_INJECT_MGR , "ReloadClientStateTable() Enter");

    CWUtils::AdjustSelfPrivilege( SE_DEBUG_NAME , TRUE );

    if ( ! EnumProcesses( dwPid , sizeof(dwPid) , &dwRetSize ) )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "EnumProcesses() failed. GetLastError()=%!WINERROR!" , GetLastError() );
        goto exit;
    }
    dwPidCount = dwRetSize / sizeof(DWORD);

    EnterCriticalSection( &m_csClientStateTable );
    _ASSERT( 0 == m_mapClientStateTable.size() );
    WCHAR wzTmpPath[MAX_PATH];
    for ( DWORD i = 0 ; i < dwPidCount ; i++ )
    {
        //Get a handle to the process, may failed if access denied
        HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE , dwPid[i] );
        if ( NULL == hProcess )
        {
            continue;
        }

        if ( GetModuleFileNameExW( hProcess , NULL , wzTmpPath , _countof(wzTmpPath) ) &&
             EnumProcessModulesEx( hProcess , hMod , sizeof(hMod) , &dwRetSize , LIST_MODULES_ALL ) )
        {
            DWORD dwModCount = dwRetSize / sizeof(DWORD);
            this->OnProcessCreateTerminate( TRUE , dwPid[i] , wzTmpPath , CWUtils::GetPathBaseNameW(wzTmpPath) );

            for ( DWORD j = 0 ; j < dwModCount ; j++ )
            {
                if ( GetModuleFileNameExW( hProcess , hMod[j] , wzTmpPath , _countof(wzTmpPath) ) )
                {
                    this->OnImageLoaded( dwPid[i] , wzTmpPath , CWUtils::GetPathBaseNameW(wzTmpPath) );
                }
            }
        }
        CloseHandle( hProcess );
    }
    LeaveCriticalSection( &m_csClientStateTable );
    bRet = TRUE;

exit :
    DbgOut( INFO , DBG_DLL_INJECT_MGR , "ReloadClientStateTable() Leave. bRet=%d" , bRet );
    return bRet;
}



BOOL CDllInjectServer::CreateCommonHandles()
{
#ifdef _DEBUG
    for ( size_t i = 0 ; i < _countof(m_hPerServerEvt) ; i++ )
    {
        _ASSERT( NULL == m_hPerServerEvt[i] );
    }
    for ( size_t i = 0 ; i < _countof(m_hPerServerMutex) ; i++ )
    {
        _ASSERT( NULL == m_hPerServerMutex[i] );
    }
    for ( size_t i = 0 ; i < _countof(m_hPerServerSm) ; i++ )
    {
        _ASSERT( NULL == m_hPerServerSm[i] );
    }
    for ( size_t i = 0 ; i < _countof(m_pPerServerSm) ; i++ )
    {
        _ASSERT( NULL == m_pPerServerSm[i] );
    }
#endif

    BOOL bRet = FALSE;

    m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] = CreateEvent( NULL , TRUE , FALSE , NULL );
    m_hPerServerEvt[PER_SERVER_EVT_INDEX_LOCAL_REQ_OK] = CreateEvent( NULL , TRUE , FALSE , NULL );
    m_hPerServerEvt[PER_SERVER_EVT_INDEX_LOCAL_RSP] = CreateEvent( NULL , TRUE , FALSE , NULL );
    m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_REQ] = CreateEvent( NULL , TRUE , FALSE , NULL );
    m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_RSP_OK] = CreateEvent( NULL , TRUE , FALSE , NULL );
    for ( size_t i = 0 ; i < _countof(m_hPerServerEvt) ; i++ )
    {
        if ( NULL == m_hPerServerEvt[i] )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create Per-server events. GetLastError()=%!WINERROR!" , GetLastError() );
            goto exit;
        }
    }

    m_hPerServerMutex[PER_SERVER_MUTEX_INDEX_REMOTE_INSTANCE] = CreateMutex( NULL , FALSE , NULL );
    m_hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] = CreateMutex( NULL , FALSE , NULL );
    for ( size_t i = 0 ; i < _countof(m_hPerServerMutex) ; i++ )
    {
        if ( NULL == m_hPerServerMutex[i] )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create Per-server mutexes. GetLastError()=%!WINERROR!" , GetLastError() );
            goto exit;
        }
    }
 
    m_hPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] = CreateFileMapping( INVALID_HANDLE_VALUE , NULL , PAGE_READWRITE , 0 , DLL_INJECT_SERVER_SM_DATA_HEADER_MAX_SIZE , NULL );
    m_hPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] = CreateFileMapping( INVALID_HANDLE_VALUE , NULL , PAGE_READWRITE , 0 , DLL_INJECT_SERVER_SM_DATA_HEADER_MAX_SIZE , NULL );
    for ( size_t i = 0 ; i < _countof(m_hPerServerSm) ; i++ )
    {
        if ( NULL == m_hPerServerSm[i] )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create Per-server share memory. GetLastError()=%!WINERROR!" , GetLastError() );
            goto exit;
        }
    }

    m_pPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] = (DLL_INJECT_SERVER_SM_DATA_HEADER *)MapViewOfFile( m_hPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] , FILE_MAP_ALL_ACCESS , 0 , 0 , 0 );
    m_pPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] = (DLL_INJECT_SERVER_SM_DATA_HEADER *)MapViewOfFile( m_hPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] , FILE_MAP_ALL_ACCESS , 0 , 0 , 0 );
    for ( size_t i = 0 ; i < _countof(m_pPerServerSm) ; i++ )
    {
        if ( NULL == m_pPerServerSm[i] )
        {
            DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to mapping Per-server share memory. GetLastError()=%!WINERROR!" , GetLastError() );
            goto exit;
        }
    }

    bRet = TRUE;

exit :
    if ( FALSE == bRet )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "CreateCommonHandles() failed" );
        this->DestroyCommonHandles();
    }
    return bRet;
}

BOOL CDllInjectServer::DestroyCommonHandles()
{
    for ( size_t i = 0 ; i < _countof(m_pPerServerSm) ; i++ )
    {
        if( NULL != m_pPerServerSm[i] )
        {
            UnmapViewOfFile( m_pPerServerSm[i] );
            m_pPerServerSm[i] = NULL;
        }
    }

    for ( size_t i = 0 ; i < _countof(m_hPerServerSm) ; i++ )
    {
        if( NULL != m_hPerServerSm[i] )
        {
            CloseHandle( m_hPerServerSm[i] );
            m_hPerServerSm[i] = NULL;
        }
    }

    for ( size_t i = 0 ; i < _countof(m_hPerServerMutex) ; i++ )
    {
        if( NULL != m_hPerServerMutex[i] )
        {
            CloseHandle( m_hPerServerMutex[i] );
            m_hPerServerMutex[i] = NULL;
        }
    }

    for ( size_t i = 0 ; i < _countof(m_hPerServerEvt) ; i++ )
    {
        if( NULL != m_hPerServerEvt[i] )
        {
            CloseHandle( m_hPerServerEvt[i] );
            m_hPerServerEvt[i] = NULL;
        }
    }
    return TRUE;
}



BOOL CDllInjectServer::CreateJobThreads()
{
    _ASSERT( m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] && 0 == m_JobQueue.size() &&
             NULL == m_hEvtNewJob && NULL == m_hJobCreatorThread && NULL == m_hJobWorkerThreads );

    BOOL bRet = FALSE;

    EnterCriticalSection( &m_csJobs );

    //Create new job event
    m_hEvtNewJob = CreateEventW( NULL , FALSE , FALSE , NULL );
    if ( NULL == m_hEvtNewJob )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create new job event. GetLastError()=%!WINERROR!" , GetLastError() );
        goto exit;
    }

    //Create JobCreator thread
    UINT uCreatorTid = 0;
    InterlockedIncrement( &m_lJobThreadCnt );
    m_hJobCreatorThread = (HANDLE)_beginthreadex( NULL , 0 , CDllInjectServer::JobCreatorThread , this , 0 , &uCreatorTid );
    if ( NULL == m_hJobCreatorThread )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create JobCreator thread. GetLastError()=%!WINERROR!" , GetLastError() );
        InterlockedDecrement( &m_lJobThreadCnt );
        goto exit;
    }
    else
    {
        DbgOut( INFO , DBG_DLL_INJECT_MGR , "JobCreator thread created. Handle=0x%p, tid=0x%04X" , m_hJobCreatorThread , uCreatorTid );
    }

    //Create JobWorker threads
    m_hJobWorkerThreads = new (std::nothrow) HANDLE[m_CommonCfg.ulWorkerCnt];
    if ( NULL == m_hJobWorkerThreads )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create JobWorker thread handles. GetLastError()=%!WINERROR!" , GetLastError() );
        goto exit;
    }
    else
    {
        for ( ULONG i = 0 ; i < m_CommonCfg.ulWorkerCnt ; i++ )
        {
            UINT uWorkerTid = 0;
            InterlockedIncrement( &m_lJobThreadCnt );
            m_hJobWorkerThreads[i] = (HANDLE)_beginthreadex( NULL , 0 , CDllInjectServer::JobWorkerThread , this , 0 , &uWorkerTid );
            if ( NULL == m_hJobWorkerThreads[i] )
            {
                InterlockedDecrement( &m_lJobThreadCnt );
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Failed to create JobWorker thread. GetLastError()=%!WINERROR!" , GetLastError() );
                goto exit;
            }
            else
            {
                DbgOut( INFO , DBG_DLL_INJECT_MGR , "JobWorker thread created. Handle=0x%p, tid=0x%04X" , m_hJobWorkerThreads[i] , uWorkerTid );
            }
        }
    }
    bRet = TRUE;

exit :
    if ( FALSE == bRet )
    {
        this->DestroyJobThreads();
    }
    LeaveCriticalSection( &m_csJobs );
    DbgOut( VERB , DBG_DLL_INJECT_MGR , "CreateJobThreads() Leave. bRet=%d" , bRet );
    return bRet;
}

BOOL CDllInjectServer::DestroyJobThreads()
{
    EnterCriticalSection( &m_csJobs );

    if ( NULL != m_hEvtNewJob )
    {
        CloseHandle( m_hEvtNewJob );
        m_hEvtNewJob = NULL;
    }

    if ( NULL != m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] )
    {
        SetEvent( m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] );
        while ( 0 != m_lJobThreadCnt )
        {
            DbgOut( WARN , DBG_DLL_INJECT_MGR , "Waiting for all job threads leaving. Current job thread count=%lu" , m_lJobThreadCnt );
            Sleep( 100 );
        }
        ResetEvent( m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] );
    }
    
    if ( NULL != m_hJobCreatorThread )
    {
        WaitForSingleObject( m_hJobCreatorThread , INFINITE );
        CloseHandle( m_hJobCreatorThread );
        m_hJobCreatorThread = NULL;        
    }

    if ( NULL != m_hJobWorkerThreads )
    {
        WaitForMultipleObjects( m_CommonCfg.ulWorkerCnt , m_hJobWorkerThreads , TRUE , INFINITE );
        for ( ULONG i = 0 ; i < m_CommonCfg.ulWorkerCnt ; i++ )
        {
            CloseHandle( m_hJobWorkerThreads[i] );
        }
        delete [] m_hJobWorkerThreads;
        m_hJobWorkerThreads = NULL;
    }

    LeaveCriticalSection( &m_csJobs );
    return TRUE;
}




UINT CALLBACK CDllInjectServer::JobCreatorThread( VOID * pThis )
{
    CDllInjectServer * pSelf = (CDllInjectServer *)pThis;
    UINT uRet = pSelf->DoJobCreator();
    if ( ERROR_SUCCESS != uRet )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "JobCreator exit abnormally. Try to set stop event. uRet=%!WINERROR!. GetLastError()=%!WINERROR!" , uRet , GetLastError() );
        SetEvent( pSelf->m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] );
    }
    InterlockedDecrement( &pSelf->m_lJobThreadCnt );
    return uRet;
}


DWORD CDllInjectServer::DoJobCreator()
{
    _ASSERT( m_hEvtNewJob && m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] && m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_REQ] &&
             m_hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] && m_pPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL] );

    DWORD dwRet = ERROR_NOT_READY;
    BOOL bLoop = TRUE;
    HANDLE hEvtWaitReq[] = { m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] , m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_REQ] };
    HANDLE hEvtWaitSmR2L[] = { m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] , m_hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] };

    while ( bLoop )
    {
        DWORD dwWaitReq = WaitForMultipleObjects( _countof(hEvtWaitReq) , hEvtWaitReq , FALSE , INFINITE );
        switch ( dwWaitReq )
        {
            case WAIT_OBJECT_0 :        //PER_SERVER_EVT_INDEX_STOP
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Stop event triggered. Leaving creator thread" );
                dwRet = ERROR_SUCCESS;
                bLoop = FALSE;
                break;
            }
            case WAIT_OBJECT_0 + 1 :    //PER_SERVER_EVT_INDEX_REMOTE_REQ
            {
                DbgOut( INFO , DBG_DLL_INJECT_MGR , "Receive remote handler's request event" );                
                DWORD dwWaitSmR2L = WaitForMultipleObjects( _countof(hEvtWaitSmR2L) , hEvtWaitSmR2L , FALSE , INFINITE );
                switch ( dwWaitSmR2L )
                {
                    case WAIT_OBJECT_0 :        //PER_SERVER_EVT_INDEX_STOP
                    {
                        DbgOut( WARN , DBG_DLL_INJECT_MGR , "Stop event triggered. Releasing share memory mutex" );
                        dwRet = ERROR_SUCCESS;
                        bLoop = FALSE;
                        break;
                    }
                    case WAIT_OBJECT_0 + 1 :    //PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L
                    {
                        DbgOut( VERB , DBG_DLL_INJECT_MGR , "Get share memory mutex" );

                        //Reset request event to prevent double scanning for the case the remote process crashed
                        ResetEvent( m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_REQ] );

                        DLL_INJECT_SERVER_SM_DATA_HEADER * pSmR2L = m_pPerServerSm[PER_SERVER_SM_INDEX_REMOTE_TO_LOCAL];
                        _ASSERT( DLL_INJECT_SERVER_SM_DATA_TYPE_GENERAL_REQ == pSmR2L->uDataType );

                        InjectClientInfo * pHookProcInfo = (InjectClientInfo *)pSmR2L->pLocalCtx;
                        _ASSERT( pHookProcInfo );

                        DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ * pReq = (DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ *)&pSmR2L->pData;
                        DbgOut( VERB , DBG_DLL_INJECT_MGR , "pSmR2L=0x%p, FIELD_OFFSET(DLL_INJECT_SERVER_SM_DATA_HEADER,pData)=0x%X, pReq=0x%p" , pSmR2L , FIELD_OFFSET(DLL_INJECT_SERVER_SM_DATA_HEADER,pData) , pReq );

                        
                        //Check whether the remote process still exist
                        DWORD dwProcExist = WaitForSingleObject( (HANDLE)pSmR2L->hRemoteProc , 0 );
                        if ( WAIT_TIMEOUT == dwProcExist )
                        {
                            DWORD dwTotalSize = FIELD_OFFSET( InjectJob , pJobData ) + pSmR2L->dwDataSize;
                            InjectJob * pJob = (InjectJob *) new (std::nothrow) BYTE[dwTotalSize];
                            if ( NULL != pJob )
                            {
                                DbgOut( INFO , DBG_DLL_INJECT_MGR , "New scan request job from process handle 0x%p. pData=0x%p, dwDataSize=%u" , (HANDLE)pSmR2L->hRemoteProc , (VOID *)pSmR2L->pData , pSmR2L->dwDataSize );
                                pJob->dwOwnerPid = pSmR2L->dwRemotePid;
                                pJob->hOwnerProc = (HANDLE)pSmR2L->hRemoteProc;
                                pJob->pOwnerCtx = (VOID *)pHookProcInfo;
                                pJob->uJobType = INJECT_JOB_TYPE_SCAN_REQ;
                                pJob->dwJobDataSize = pSmR2L->dwDataSize;
                                CopyMemory( &pJob->pJobData , &pSmR2L->pData , pSmR2L->dwDataSize );
                                PushToJobQueue( pJob );
                                SetEvent( pHookProcInfo->hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_REQ_OK] );
                            }
                        }
                        else
                        {
                            DbgOut( VERB , DBG_DLL_INJECT_MGR , "dwProcExist=%lu. GetLastError()=%!WINERROR!" , dwProcExist , GetLastError() );
                            DbgOut( WARN , DBG_DLL_INJECT_MGR , "Remote thread crashed. Skip this request" );
                        }
                        break;
                    }
                    case WAIT_ABANDONED_0 :         //PER_SERVER_EVT_INDEX_STOP
                    {
                        DbgOut( WARN , DBG_DLL_INJECT_MGR , "Stop event abandoned. Leaving creator thread" );
                        dwRet = ERROR_INVALID_HANDLE;
                        bLoop = FALSE;
                        break;
                    }
                    case WAIT_ABANDONED_0 + 1 :     //PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L
                    {
                        DbgOut( WARN , DBG_DLL_INJECT_MGR , "One of the remote threads crashed. Releasing the mutex" );
                        break;
                    }
                    default :
                    {
                        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Unexpected return value 0x%X. GetLastError()=%!WINERROR!" , dwWaitSmR2L , GetLastError() );
                        dwRet = ERROR_INVALID_HANDLE_STATE;
                        bLoop = FALSE;
                        break;
                    }
                }
                ReleaseMutex( m_hPerServerMutex[PER_SERVER_MUTEX_INDEX_SHAREM_MEM_R2L] );
                break;
            }
            case WAIT_ABANDONED_0 :        //PER_SERVER_EVT_INDEX_STOP
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Stop event abandoned. Leaving creator thread" );
                dwRet = ERROR_INVALID_HANDLE;
                bLoop = FALSE;
                break;
            }
            default :
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Unexpected return value 0x%X. GetLastError()=%!WINERROR!" , dwWaitReq , GetLastError() );
                dwRet = ERROR_CANCELLED;
                bLoop = FALSE;
                break;
            }
        }
    }
    return dwRet;
}


UINT CALLBACK CDllInjectServer::JobWorkerThread( VOID * pThis )
{
    CDllInjectServer * pSelf = (CDllInjectServer *)pThis;
    UINT uRet = pSelf->DoJobWorker();
    if ( ERROR_SUCCESS != uRet )
    {
        DbgOut( ERRO , DBG_DLL_INJECT_MGR , "JobWorker exit abnormally. Try to set stop event. uRet=%!WINERROR!. GetLastError()=%!WINERROR!" , uRet , GetLastError() );
        SetEvent( pSelf->m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] );
    }
    InterlockedDecrement( &pSelf->m_lJobThreadCnt );
    return uRet;
}
DWORD CDllInjectServer::DoJobWorker()
{
    _ASSERT( m_hEvtNewJob && m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] &&
             m_pPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE] && m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_RSP_OK] );

    DWORD dwRet = ERROR_NOT_READY;
    BOOL bLoop = TRUE;
    HANDLE hEvtWaitJob[] = { m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] , m_hEvtNewJob };

    while ( bLoop )
    {
        DWORD dwWaitJob = WaitForMultipleObjects( _countof(hEvtWaitJob) , hEvtWaitJob , FALSE , INFINITE );
        switch ( dwWaitJob )
        {
            case WAIT_OBJECT_0 :        //PER_SERVER_EVT_INDEX_STOP
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Stop event triggered. Leaving worker thread" );
                dwRet = ERROR_SUCCESS;
                bLoop = FALSE;
                break;
            }
            case WAIT_OBJECT_0 + 1 :    //m_hEvtNewJob
            {
                InjectJob * pJob = NULL;
                while ( TRUE == PopFromJobQueue( &pJob ) )
                {
                    _ASSERT( pJob && pJob->hOwnerProc && pJob->pOwnerCtx && INJECT_JOB_TYPE_SCAN_REQ == pJob->uJobType );

                    do 
                    {
                        //Only scan if the remote thread still exist
                        if ( WAIT_TIMEOUT != WaitForSingleObject( pJob->hOwnerProc , 0 ) )
                        {
                            break;
                        }

                        InjectClientInfo * pHookProcInfo = (InjectClientInfo *)pJob->pOwnerCtx;
                        DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ * pReq = (DLL_INJECT_SERVER_SM_DATA_GENERAL_REQ *)pJob->pJobData;

                        //Call registered ScanCbk
                        DbgOut( INFO , DBG_DLL_INJECT_MGR , "Ready to scan. data=%!HEXDUMP!" , WppHexDump( (CONST UCHAR *)pReq->pReq , pReq->dwReqSize ) );
                        DWORD dwScanResult = ERROR_SUCCESS;
                        if ( NULL != m_UserCfg.ScanCbk )
                        {
                            dwScanResult = m_UserCfg.ScanCbk( pJob->dwOwnerPid , m_UserCfg.pUserCtx , (CHAR *)pReq->pReq , pReq->dwReqSize );
                        }
                        DbgOut( INFO , DBG_DLL_INJECT_MGR , "Scan result is %lu" , dwScanResult );
                        
                        //Put scan result to SmL2R and notify remote process
                        EnterCriticalSection( &m_csSmL2R );
                        if ( WAIT_TIMEOUT == WaitForSingleObject( pJob->hOwnerProc , 0 ) )
                        {
                            ResetEvent( m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_RSP_OK] );
                            DLL_INJECT_SERVER_SM_DATA_HEADER * pSmL2R = m_pPerServerSm[PER_SERVER_SM_INDEX_LOCAL_TO_REMOTE];
                            DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP * pRsp = (DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP *)&pSmL2R->pData;
                            DbgOut( VERB , DBG_DLL_INJECT_MGR , "pSmL2R=0x%p, FIELD_OFFSET(DLL_INJECT_SERVER_SM_DATA_HEADER,pData)=0x%X, pRsp=0x%p" , pSmL2R , FIELD_OFFSET(DLL_INJECT_SERVER_SM_DATA_HEADER,pData) , pRsp );
                            
                            pRsp->dwRspSize = sizeof(dwScanResult);
                            CopyMemory( &pRsp->pRsp , &dwScanResult , pRsp->dwRspSize );
                            pSmL2R->dwDataSize = FIELD_OFFSET( DLL_INJECT_SERVER_SM_DATA_GENERAL_RSP , pRsp ) + pRsp->dwRspSize;
                            pSmL2R->uDataType = DLL_INJECT_SERVER_SM_DATA_TYPE_GENERAL_RSP;
                            pSmL2R->pLocalCtx = (UINT64)pHookProcInfo;
                            pSmL2R->hRemoteProc = (UINT64)pJob->hOwnerProc; //Useless currently
                            pSmL2R->dwRemotePid = pJob->dwOwnerPid;         //Useless currently
                            SetEvent( pHookProcInfo->hPerClientEvt[PER_CLIENT_EVT_INDEX_REMOTE_RSP] );

                            HANDLE hEvtWaitRspOk[] = { m_hPerServerEvt[PER_SERVER_EVT_INDEX_STOP] , pJob->hOwnerProc , m_hPerServerEvt[PER_SERVER_EVT_INDEX_REMOTE_RSP_OK] };
                            DWORD dwWaitRsp = WaitForMultipleObjects( _countof(hEvtWaitRspOk) , hEvtWaitRspOk , FALSE , INFINITE );
                            switch ( dwWaitRsp )
                            {
                                case WAIT_OBJECT_0 :            //PER_SERVER_EVT_INDEX_STOP
                                {
                                    DbgOut( WARN , DBG_DLL_INJECT_MGR , "Stop event triggered. Leaving worker thread" );
                                    dwRet = ERROR_SUCCESS;
                                    bLoop = FALSE;
                                    break;
                                }
                                case WAIT_OBJECT_0 + 1 :        //hOwnerProc
                                {
                                    DbgOut( WARN , DBG_DLL_INJECT_MGR , "One of the remote threads crashed. Skip this response" );
                                    break;
                                }
                                case WAIT_OBJECT_0 + 2 :        //PER_SERVER_EVT_INDEX_REMOTE_RSP_OK
                                {
                                    DbgOut( INFO , DBG_DLL_INJECT_MGR , "Remote thread has already got the response. release the critical section" );
                                    break;
                                }
                                case WAIT_ABANDONED_0 :        //PER_SERVER_EVT_INDEX_STOP
                                {
                                    DbgOut( WARN , DBG_DLL_INJECT_MGR , "Stop event abandoned. Leaving worker thread" );
                                    dwRet = ERROR_INVALID_HANDLE;
                                    bLoop = FALSE;
                                    break;
                                }
                                case WAIT_ABANDONED_0 + 1 :     //hOwnerProc
                                case WAIT_ABANDONED_0 + 2 :     //PER_SERVER_EVT_INDEX_REMOTE_RSP_OK
                                {
                                    DbgOut( WARN , DBG_DLL_INJECT_MGR , "One of the remote threads crashed. Skip this response" );
                                    break;
                                }
                                default :
                                {
                                    DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Unexpected return value 0x%X. GetLastError()=%!WINERROR!" , dwWaitRsp , GetLastError() );
                                    dwRet = ERROR_INVALID_HANDLE_STATE;
                                    bLoop = FALSE;
                                    break;
                                }
                            }
                        }
                        LeaveCriticalSection( &m_csSmL2R );
                    } while ( 0 );

                    delete pJob;
                }
                break;
            }
            case WAIT_ABANDONED_0 :        //PER_SERVER_EVT_INDEX_STOP
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "Stop event abandoned. Leaving worker thread" );
                dwRet = ERROR_INVALID_HANDLE;
                bLoop = FALSE;
                break;
            }
            case WAIT_ABANDONED_0 + 1 :    //m_hEvtNewJob
            {
                DbgOut( WARN , DBG_DLL_INJECT_MGR , "New job event abandoned. Leaving worker thread" );
                dwRet = ERROR_INVALID_HANDLE;
                bLoop = FALSE;
                break;
            }
            default :
            {
                DbgOut( ERRO , DBG_DLL_INJECT_MGR , "Unexpected return value 0x%X. GetLastError()=%!WINERROR!" , dwWaitJob , GetLastError() );
                dwRet = ERROR_CANCELLED;
                bLoop = FALSE;
                break;
            }
        }
    }
    return dwRet;
}


BOOL CDllInjectServer::PushToJobQueue( InjectJob * pJob )
{
    _ASSERT( pJob && m_hEvtNewJob );

    EnterCriticalSection( &m_csJobs );
    m_JobQueue.push_back( pJob );
    SetEvent( m_hEvtNewJob );
    LeaveCriticalSection( &m_csJobs );
    return TRUE;
}

BOOL CDllInjectServer::PopFromJobQueue( InjectJob ** pJob )
{
    _ASSERT( pJob );

    BOOL bRet = FALSE;
    EnterCriticalSection( &m_csJobs );
    if ( ! m_JobQueue.empty() )
    {
        *pJob = m_JobQueue.front();
        m_JobQueue.pop_front();
        bRet = TRUE;
    }
    LeaveCriticalSection( &m_csJobs );
    return bRet;
}


#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils