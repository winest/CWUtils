#include "stdafx.h"
#include "CWService.h"
#include <algorithm>

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

//This API is not exported
BOOL _GetWindowsDir( wstring & aWinDir )
{
    BOOL bRet = FALSE;
    UINT uReturnSize = 0;
    WCHAR wzWinDir[MAX_PATH] = { 0 };

    uReturnSize = ::GetSystemWindowsDirectoryW( (LPWSTR)&wzWinDir, _countof( wzWinDir ) );
    if ( _countof( wzWinDir ) <= uReturnSize )
    {
        WCHAR * wzWinDirEx = new ( std::nothrow ) WCHAR[uReturnSize + 1];
        if ( NULL != wzWinDirEx )
        {
            wzWinDirEx[uReturnSize] = L'\0';
            uReturnSize = ::GetSystemWindowsDirectoryW( wzWinDirEx, uReturnSize );
            if ( 0 != uReturnSize )
            {
                aWinDir = wzWinDirEx;
                bRet = TRUE;
            }
            delete[] wzWinDirEx;
            wzWinDirEx = NULL;
        }
    }
    else
    {
        aWinDir = wzWinDir;
        bRet = TRUE;
    }

    if ( TRUE == bRet && L'\\' != aWinDir[aWinDir.length() - 1] )
    {
        aWinDir.push_back( L'\\' );
    }
    return bRet;
}



BOOL _WaitForServiceState( SC_HANDLE aService, DWORD aDesiredState, DWORD aTimeout )
{
    BOOL bRet = TRUE;
    SERVICE_STATUS svcStatus = { 0 };
    BOOL bFirstTime = TRUE;
    DWORD dwLastState = 0, dwLastCheckPoint = 0;
    DWORD dwExpireTime = GetTickCount() + aTimeout;

    for ( ;; )
    {
        bRet = ::QueryServiceStatus( aService, &svcStatus );
        if ( FALSE == bRet )
        {
            break;
        }

        //Reaches the desired state
        if ( aDesiredState == svcStatus.dwCurrentState )
        {
            break;
        }

        //Timeout triggered
        if ( ( INFINITE != aTimeout ) && ( dwExpireTime < GetTickCount() ) )
        {
            SetLastError( ERROR_TIMEOUT );
            bRet = FALSE;
            break;
        }

        if ( bFirstTime )
        {
            bFirstTime = FALSE;
            dwLastState = svcStatus.dwCurrentState;
            dwLastCheckPoint = svcStatus.dwCheckPoint;
        }
        else
        {
            if ( dwLastState != svcStatus.dwCurrentState )
            {
                dwLastState = svcStatus.dwCurrentState;
                dwLastCheckPoint = svcStatus.dwCheckPoint;
            }
            else if ( svcStatus.dwCheckPoint >= dwLastCheckPoint )
            {
                dwLastCheckPoint = svcStatus.dwCheckPoint;
            }
            else
            {
                bRet = FALSE;
                break;
            }
        }

        //Wait the specified period of time
        DWORD dwWaitHint = svcStatus.dwWaitHint / 10;    //Poll 1/10 of the wait hint
        dwWaitHint = std::min( 10 * 1000, std::max( 1000, static_cast<int>( dwWaitHint ) ) );    //Sleep 1~10 seconds
        Sleep( dwWaitHint );
    }

    return bRet;
}


BOOL IsScmLocked( UINT aRetryTimes, DWORD aRetryWaitTime )
{
    BOOL bLocked = TRUE;
    size_t dwTimes = 0;

    SC_HANDLE hSvcMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_LOCK );
    if ( NULL == hSvcMgr )
    {
        //DbgOut( ERRO , DBG_UTILS , "OpenSCManagerW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
        goto exit;
    }

    for ( dwTimes = 0; dwTimes < aRetryTimes; dwTimes++ )
    {
        SC_LOCK lockSc = LockServiceDatabase( hSvcMgr );
        if ( lockSc )
        {
            //DbgOut( INFO , DBG_UTILS , "LockServiceDatabase() OK" );
            UnlockServiceDatabase( lockSc );
            bLocked = FALSE;
            break;
        }
        else
        {
            //DbgOut( ERRO , DBG_UTILS , "LockServiceDatabase() Failed" );
        }

        DWORD dwLastErr = GetLastError();
        if ( ERROR_SERVICE_DATABASE_LOCKED == dwLastErr )
        {
            //DbgOut( WARN , DBG_UTILS , "Service database is locked" );
        }
        else
        {
            //DbgOut( ERRO , DBG_UTILS , "LockServiceDatabase() failed. GetLastError()=%!WINERROR!" , dwLastErr );
            break;
        }

        Sleep( aRetryWaitTime );
    }

exit:
    if ( NULL != hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return bLocked;
}



BOOL IsServiceAccessible( CONST WCHAR * aServiceName, DWORD aDesiredAccess )
{
    BOOL bRet = FALSE;
    SC_HANDLE hSvcMgr = NULL, hSvc = NULL;
    do
    {
        hSvcMgr = OpenSCManagerW( NULL, NULL, GENERIC_READ );
        if ( NULL == hSvcMgr )
        {
            break;
        }

        hSvc = OpenServiceW( hSvcMgr, aServiceName, aDesiredAccess );
        if ( NULL == hSvc )
        {
            break;
        }

        bRet = TRUE;
    } while ( 0 );

    if ( NULL != hSvc )
    {
        CloseServiceHandle( hSvc );
    }
    if ( NULL != hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return bRet;
}

BOOL GetServiceBinaryPath( CONST WCHAR * aServiceName, wstring & aFullPath )
{
    BOOL bRet = FALSE;
    SC_HANDLE hSvcMgr = NULL, hSvc = NULL;
    LPQUERY_SERVICE_CONFIGW pSvcCfg = NULL;
    DWORD dwBytesNeeded, cbBufSize, dwError;

    wstring wstrWinDir;
    if ( !_GetWindowsDir( wstrWinDir ) )
    {
        //DbgOut( ERRO , DBG_UTILS , "Cannot get windows directory, use C:\\Windows\\ as default. GetLastError()=%!WINERROR!" , GetLastError() );
        wstrWinDir = L"C:\\Windows\\";
    }

    do
    {
        CONST struct
        {
            wstring wstrOldPrefix;
            wstring wstrNewPrefix;
        } stPrefixMap[] = { { L"\\SystemRoot\\", wstrWinDir }, { L"%SystemRoot%\\", wstrWinDir } };
        hSvcMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if ( NULL == hSvcMgr )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenSCManagerW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        hSvc = OpenServiceW( hSvcMgr, aServiceName, SERVICE_QUERY_CONFIG );
        if ( NULL == hSvc )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenServiceW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        if ( !QueryServiceConfigW( hSvc, NULL, 0, &dwBytesNeeded ) )
        {
            dwError = GetLastError();
            if ( ERROR_INSUFFICIENT_BUFFER == dwError )
            {
                cbBufSize = dwBytesNeeded;
                pSvcCfg = (LPQUERY_SERVICE_CONFIGW)LocalAlloc( LMEM_FIXED, cbBufSize );
                if ( NULL == pSvcCfg )
                {
                    //DbgOut( ERRO , DBG_UTILS , "LocalAlloc() failed. GetLastError()=%!WINERROR!" , dwError );
                    break;
                }
            }
            else
            {
                //DbgOut( ERRO , DBG_UTILS , "QueryServiceConfigW() failed. GetLastError()=%!WINERROR!" , dwError );
                break;
            }
        }

        if ( !QueryServiceConfigW( hSvc, pSvcCfg, cbBufSize, &dwBytesNeeded ) )
        {
            //DbgOut( ERRO , DBG_UTILS , "QueryServiceConfigW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        if ( NULL == pSvcCfg->lpBinaryPathName || L'\0' == pSvcCfg->lpBinaryPathName[0] )
        {
            //DbgOut( ERRO , DBG_UTILS , "Binary path is empty" );
            break;
        }

        aFullPath = pSvcCfg->lpBinaryPathName;
        bRet = TRUE;

        //Handle the case ImagePath is started with "\\SystemRoot\\" or "%SystemRoot%"
        for ( size_t i = 0; i < _countof( stPrefixMap ); i++ )
        {
            if ( 0 == _wcsnicmp( pSvcCfg->lpBinaryPathName, stPrefixMap[i].wstrOldPrefix.c_str(),
                                 stPrefixMap[i].wstrOldPrefix.length() ) )
            {
                aFullPath.replace( 0, stPrefixMap[i].wstrOldPrefix.length(), stPrefixMap[i].wstrNewPrefix );
                break;
            }
        }

        //DbgOut( VERB , DBG_UTILS , "Service binary path=%ws" , wstrFullPath.c_str() );
    } while ( 0 );

    if ( NULL != pSvcCfg )
    {
        LocalFree( pSvcCfg );
    }
    if ( NULL != hSvc )
    {
        CloseServiceHandle( hSvc );
    }
    if ( NULL != hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return bRet;
}

BOOL InstallService( CONST WCHAR * aServicePath, CONST WCHAR * aServiceName, DWORD aServiceType, DWORD aStartType )
{
    BOOL bRet = FALSE;
    SC_HANDLE hSvcMgr = NULL, hSvc = NULL;

    do
    {
        hSvcMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
        if ( NULL == hSvcMgr )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenSCManagerW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        hSvc = CreateServiceW( hSvcMgr, aServiceName, aServicePath, SERVICE_ALL_ACCESS, aServiceType, aStartType,
                               SERVICE_ERROR_NORMAL, aServicePath, NULL, NULL, NULL, NULL, NULL );
        if ( NULL == hSvc )
        {
            //DbgOut( ERRO , DBG_UTILS , "CreateServiceW() failed. aServiceName=%ws" , aServiceName );
            break;
        }
        bRet = TRUE;
    } while ( 0 );


    if ( NULL != hSvc )
    {
        CloseServiceHandle( hSvc );
    }
    if ( NULL != hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return bRet;
}

BOOL UninstallService( CONST WCHAR * aServiceName )
{
    BOOL bRet = FALSE;
    SC_HANDLE hSvcMgr = NULL, hSvc = NULL;

    do
    {
        hSvcMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_CREATE_SERVICE );
        if ( NULL == hSvcMgr )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenSCManagerW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        hSvc = OpenServiceW( hSvcMgr, aServiceName, DELETE );
        if ( NULL == hSvc )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenServiceW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        bRet = DeleteService( hSvc );
    } while ( 0 );

    if ( NULL != hSvc )
    {
        CloseServiceHandle( hSvc );
    }
    if ( NULL != hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return bRet;
}



BOOL GetServiceCurrentState( IN CONST WCHAR * aServiceName, OUT DWORD * aCurrentState )
{
    BOOL bRet = FALSE;
    SC_HANDLE hSvcMgr = NULL, hSvc = NULL;

    do
    {
        hSvcMgr = OpenSCManagerW( NULL, NULL, GENERIC_READ );
        if ( NULL == hSvcMgr )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenSCManagerW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        hSvc = OpenServiceW( hSvcMgr, aServiceName, GENERIC_READ );
        if ( NULL == hSvc )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenServiceW() failed. aServiceName=%ws" , aServiceName );
            break;
        }

        SERVICE_STATUS SrvStatus = { 0 };
        if ( !QueryServiceStatus( hSvc, &SrvStatus ) )
        {
            //DbgOut( ERRO , DBG_UTILS , "QueryServiceStatus() failed" );
            break;
        }

        *aCurrentState = SrvStatus.dwCurrentState;
        bRet = TRUE;
    } while ( 0 );


    if ( NULL != hSvc )
    {
        CloseServiceHandle( hSvc );
    }
    if ( NULL != hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return bRet;
}

BOOL StartServiceByName( CONST WCHAR * aServiceName, DWORD aTimeout )
{
    BOOL bRet = FALSE;
    SC_HANDLE hSvcMgr = NULL, hSvc = NULL;

    if ( NULL == aServiceName )
    {
        //DbgOut( ERRO , DBG_UTILS , "aServiceName is NULL" );
        goto exit;
    }

    hSvcMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( NULL == hSvcMgr )
    {
        //DbgOut( ERRO , DBG_UTILS , "OpenSCManagerW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
        goto exit;
    }

    hSvc = OpenServiceW( hSvcMgr, aServiceName, SERVICE_ALL_ACCESS );
    if ( NULL == hSvc )
    {
        //DbgOut( ERRO , DBG_UTILS , "OpenServiceW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
        goto exit;
    }

    if ( FALSE != StartServiceW( hSvc, 0, NULL ) )
    {
        if ( TRUE == _WaitForServiceState( hSvc, SERVICE_RUNNING, aTimeout ) )
        {
            bRet = TRUE;
        }
        else
        {
            SetLastError( ERROR_SERVICE_REQUEST_TIMEOUT );
        }
    }
    else
    {
        bRet = ( ERROR_SERVICE_ALREADY_RUNNING == GetLastError() ) ? TRUE : FALSE;
    }

exit:
    if ( hSvc )
    {
        CloseServiceHandle( hSvc );
    }
    if ( hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return bRet;
}

BOOL StopServiceByName( CONST WCHAR * aServiceName, DWORD aTimeout )
{
    BOOL bRet = FALSE;
    SC_HANDLE hSvcMgr = NULL, hSvc = NULL;
    SERVICE_STATUS svcStatus;

    if ( NULL == aServiceName )
    {
        //DbgOut( ERRO , DBG_UTILS , "aServiceName is NULL" );
        goto exit;
    }

    hSvcMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( NULL == hSvcMgr )
    {
        //DbgOut( ERRO , DBG_UTILS , "OpenSCManagerW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
        goto exit;
    }

    hSvc = OpenServiceW( hSvcMgr, aServiceName, SERVICE_ALL_ACCESS );
    if ( NULL == hSvc )
    {
        //DbgOut( ERRO , DBG_UTILS , "OpenServiceW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
        goto exit;
    }

    for ( ;; )
    {
        if ( FALSE != ControlService( hSvc, SERVICE_CONTROL_STOP, &svcStatus ) )
        {
            if ( TRUE == _WaitForServiceState( hSvc, SERVICE_STOPPED, aTimeout ) )
            {
                bRet = TRUE;
            }
            else
            {
                SetLastError( ERROR_SERVICE_REQUEST_TIMEOUT );
                break;
            }
        }
        else
        {
            bRet = ( ERROR_SERVICE_NOT_ACTIVE == GetLastError() ) ? TRUE : FALSE;
            break;
        }
    }

exit:
    if ( hSvc )
    {
        CloseServiceHandle( hSvc );
    }
    if ( hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return bRet;
}



BOOL ChangeServiceStartType( CONST WCHAR * aServiceName, DWORD aStartType, DWORD * aLastStartType )
{
    BOOL bRet = FALSE;
    SC_HANDLE hSvcMgr = NULL, hSvc = NULL;
    SC_LOCK lockSvcDb = NULL;
    QUERY_SERVICE_CONFIGW * pLastConfig = NULL;

    do
    {
        hSvcMgr = OpenSCManagerW( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if ( NULL == hSvcMgr )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenSCManagerW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        //Need to acquire database lock before changing config
        lockSvcDb = LockServiceDatabase( hSvcMgr );
        if ( NULL == lockSvcDb )
        {
            //DbgOut( ERRO , DBG_UTILS , "LockServiceDatabase() failed. GetLastError()=%!WINERROR!" , GetLastError() );

            //Allocate a buffer to get details about the lock
            DWORD dwBytesNeeded = sizeof( QUERY_SERVICE_LOCK_STATUSW ) + MAX_PATH;
            LPQUERY_SERVICE_LOCK_STATUSW lpQslsBuf;
            lpQslsBuf = (LPQUERY_SERVICE_LOCK_STATUSW)LocalAlloc( LPTR, dwBytesNeeded );
            if ( NULL != lpQslsBuf )
            {
                if ( FALSE != QueryServiceLockStatusW( hSvcMgr, lpQslsBuf, dwBytesNeeded, &dwBytesNeeded ) )
                {
                    //DbgOut( ERRO , DBG_UTILS , "fIsLocked=%lu, lpLockOwner=%ws, dwLockDuration=%lu" ,
                    //        lpQslsBuf->fIsLocked , lpQslsBuf->lpLockOwner , lpQslsBuf->dwLockDuration );
                }
                LocalFree( lpQslsBuf );
            }
            break;
        }

        //Open a handle to the service
        DWORD dwAccess =
            ( aLastStartType == NULL ) ? SERVICE_CHANGE_CONFIG : ( SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG );
        hSvc = OpenServiceW( hSvcMgr, aServiceName, dwAccess );
        if ( NULL == hSvc )
        {
            //DbgOut( ERRO , DBG_UTILS , "OpenServiceW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        //Backup last service config if dwLastStartType is not NULL
        if ( NULL != aLastStartType )
        {
            DWORD dwLastConfigSize = 0;
            if ( FALSE == QueryServiceConfigW( hSvc, NULL, 0, &dwLastConfigSize ) &&
                 ERROR_INSUFFICIENT_BUFFER == GetLastError() )
            {
                pLastConfig = (QUERY_SERVICE_CONFIGW *)LocalAlloc( LMEM_FIXED, dwLastConfigSize );
                if ( NULL != pLastConfig )
                {
                    if ( TRUE == QueryServiceConfigW( hSvc, pLastConfig, dwLastConfigSize, &dwLastConfigSize ) )
                    {
                        *aLastStartType = pLastConfig->dwStartType;
                    }
                    else
                    {
                        //DbgOut( ERRO , DBG_UTILS , "QueryServiceConfigW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
                        break;
                    }
                }
                else
                {
                    //DbgOut( ERRO , DBG_UTILS , "LocalAlloc() failed for lpLastConfig. GetLastError()=%!WINERROR!" , GetLastError() );
                    break;
                }
            }
            else
            {
                //DbgOut( ERRO , DBG_UTILS , "QueryServiceConfigW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
                break;
            }
        }

        //Make the changes
        if ( !ChangeServiceConfigW( hSvc, SERVICE_NO_CHANGE, aStartType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL,
                                    NULL, NULL, NULL ) )
        {
            //DbgOut( ERRO , DBG_UTILS , "ChangeServiceConfigW() failed. GetLastError()=%!WINERROR!" , GetLastError() );
            break;
        }

        bRet = TRUE;
    } while ( 0 );



    if ( NULL != pLastConfig )
    {
        LocalFree( pLastConfig );
    }
    if ( NULL != hSvc )
    {
        CloseServiceHandle( hSvc );
    }
    if ( NULL != lockSvcDb )
    {
        UnlockServiceDatabase( lockSvcDb );
    }
    if ( NULL != hSvcMgr )
    {
        CloseServiceHandle( hSvcMgr );
    }
    return TRUE;
}



#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
