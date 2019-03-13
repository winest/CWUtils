#include "stdafx.h"
#include "CWFileMonitor.h"

#include <process.h>
#include <Shlwapi.h>
#include "CWGeneralUtils.h"
#include "CWFile.h"
using CWUtils::ShowDebugMsg;

#pragma comment( lib, "Shlwapi.lib" )

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif



BOOL CFileMonitorRequest::Init( CONST WCHAR * aMonitorPath, BOOL aRecursive, DWORD aFilter, DWORD aShareMemSize )
{
    _ASSERT( aMonitorPath );

    StopCheck( (ULONG_PTR)this );

    m_wstrMonitorPath = aMonitorPath;
    m_bRecursive = aRecursive;
    m_dwFilter = aFilter;
    m_vecShareMem.resize( aShareMemSize );
    m_vecShareMemBak.resize( aShareMemSize );
    ZeroMemory( &m_overlapped, sizeof( OVERLAPPED ) );
    m_overlapped.hEvent =
        this;    //This member is not used when completion routine exists, so we use it directly (See MSDN)

    m_hMonitorPath =
        CreateFileW( aMonitorPath, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                     OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL );
    if ( INVALID_HANDLE_VALUE == m_hMonitorPath )
        return FALSE;

    return TRUE;
}

VOID CFileMonitorRequest::StartCheck( ULONG_PTR aRequest )
{
    CFileMonitorRequest * req = (CFileMonitorRequest *)aRequest;
    _ASSERT( req->m_wstrMonitorPath.length() > 0 );

    if ( INVALID_HANDLE_VALUE == req->m_hMonitorPath )
    {
        req->m_hMonitorPath = CreateFileW( req->m_wstrMonitorPath.c_str(), FILE_LIST_DIRECTORY,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                                           FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL );
        if ( INVALID_HANDLE_VALUE == req->m_hMonitorPath )
            return;
    }
    req->CheckFileChange();
}

VOID CFileMonitorRequest::StopCheck( ULONG_PTR aRequest )
{
    CFileMonitorRequest * req = (CFileMonitorRequest *)aRequest;
    if ( INVALID_HANDLE_VALUE != req->m_hMonitorPath )
    {
        CancelIo( req->m_hMonitorPath );
        for ( int i = 0; i < MONITOR_THREAD_STOP_MAX_RETRY_COUNT; i++ )
        {
            if ( TRUE == HasOverlappedIoCompleted( &req->m_overlapped ) )
                break;
            else
                SleepEx( 100, TRUE );
        }
        CloseHandle( req->m_hMonitorPath );
        req->m_hMonitorPath = INVALID_HANDLE_VALUE;

        req->DecrementWorkCount( req->m_parent );
    }
}

VOID CFileMonitorRequest::PushToQueue( CFileMonitor * aMonitor, CWFileMonitorNotification & aNotification )
{
    aMonitor->m_queue.Push( aNotification );
}


VOID CFileMonitorRequest::DecrementWorkCount( CFileMonitor * aMonitor )
{
    InterlockedDecrement( &aMonitor->m_uTotalWork );
}

VOID CFileMonitorRequest::CheckFileChange()
{
    DWORD dwBytes = 0;
    if ( FALSE == ReadDirectoryChangesW( m_hMonitorPath, &m_vecShareMem[0], (DWORD)m_vecShareMem.size(), m_bRecursive,
                                         m_dwFilter, &dwBytes, &m_overlapped, CFileMonitorRequest::OnChangeComplete ) )
    {
        CWUtils::ShowDebugMsg();
    }
}

VOID CALLBACK CFileMonitorRequest::OnChangeComplete( DWORD aErrorCode,
                                                     DWORD aBytesTransfered,
                                                     LPOVERLAPPED aOverlapped )
{
    if ( ERROR_OPERATION_ABORTED == aErrorCode )
    {
        return;
    }
    else if ( 0 == aBytesTransfered )    // This might mean overflow
    {
        return;
    }
    else
    {
    }

    CFileMonitorRequest * req = (CFileMonitorRequest *)( aOverlapped->hEvent );

    //Backup share memory's content
    CopyMemory( &req->m_vecShareMemBak[0], &req->m_vecShareMem[0], aBytesTransfered );

    //Get the new (std::nothrow) read issued as fast as possible. The documentation says that the original OVERLAPPED structure will not be used
    //again once the completion routine is called
    req->CheckFileChange();

    req->ProcessNotification();
}

VOID CFileMonitorRequest::ProcessNotification()
{
    BYTE * pBase = (BYTE *)&m_vecShareMemBak[0];
    for ( ;; )
    {
        FILE_NOTIFY_INFORMATION * fni = (FILE_NOTIFY_INFORMATION *)pBase;
        //Handle the rename issue here to avoid the case only receiving a C:\old.txt and only receiving D:\new.txt
        switch ( fni->Action )
        {
            case FILE_ACTION_ADDED:
            case FILE_ACTION_REMOVED:
            case FILE_ACTION_MODIFIED:
            {
                CWFileMonitorNotification notification;
                notification.dwEvent = GetNotifyEvent( fni );
                notification.wstrSrcPath = GetNotifyPath( fni );
                PushToQueue( m_parent, notification );    //Push the notification to queue for later processed
                break;
            }
            case FILE_ACTION_RENAMED_OLD_NAME:
            {
                if (
                    0 !=
                    fni->NextEntryOffset )    //We assume RENAMED_OLD always immediately followed by the RENAME_new (std::nothrow) event
                {
                    CWFileMonitorNotification notification;
                    notification.dwEvent = GetNotifyEvent( fni );
                    notification.wstrSrcPath = GetNotifyPath( fni );

                    pBase += fni->NextEntryOffset;
                    fni = (FILE_NOTIFY_INFORMATION *)pBase;
                    if ( FILE_ACTION_RENAMED_NEW_NAME == fni->Action )
                    {
                        notification.wstrDstPath = GetNotifyPath( fni );
                        PushToQueue( m_parent, notification );    //Push the notification to queue for later processing
                    }
                    else
                    {
                        _ASSERT( TRUE );
                    }
                }
                break;
            }
            default:    //Ignore the case only a single RENAMED_new (std::nothrow) event is triggered
                break;
        }

        if ( 0 == fni->NextEntryOffset )
            break;
        pBase += fni->NextEntryOffset;
    }
}

CWFileMonitorEvent CFileMonitorRequest::GetNotifyEvent( CONST FILE_NOTIFY_INFORMATION * aNotifyInfo )
{
    _ASSERT( aNotifyInfo );
    static const struct ACTION_MAP
    {
        DWORD dwOldAct;
        CWFileMonitorEvent dwNewAct;
    } mapAct[] = { { FILE_ACTION_ADDED, MY_FILE_MONITOR_EVT_LOCAL_ADD },
                   { FILE_ACTION_REMOVED, MY_FILE_MONITOR_EVT_LOCAL_REMOVE },
                   { FILE_ACTION_RENAMED_OLD_NAME, MY_FILE_MONITOR_EVT_LOCAL_RENAME },
                   { FILE_ACTION_RENAMED_NEW_NAME, MY_FILE_MONITOR_EVT_LOCAL_RENAME },
                   { FILE_ACTION_MODIFIED, MY_FILE_MONITOR_EVT_LOCAL_MODIFY } };
    for ( int i = 0; i < _countof( mapAct ); i++ )
    {
        if ( mapAct[i].dwOldAct == aNotifyInfo->Action )
            return mapAct[i].dwNewAct;
    }

    return MY_FILE_MONITOR_EVT_NONE;
}


wstring CFileMonitorRequest::GetNotifyPath( CONST FILE_NOTIFY_INFORMATION * aNotifyInfo )
{
    _ASSERT( aNotifyInfo );
    wstring wstrFullPath( aNotifyInfo->FileName, aNotifyInfo->FileNameLength / sizeof( WCHAR ) );

    //Handle a trailing backslash, such as for a root directory
    if ( m_wstrMonitorPath.length() > 0 && m_wstrMonitorPath[m_wstrMonitorPath.length() - 1] != L'\\' )
        wstrFullPath = ( m_wstrMonitorPath + L"\\" ) + wstrFullPath;
    else
        wstrFullPath = m_wstrMonitorPath + wstrFullPath;

    //Handle the case it's a 8.3 short filename. Convert it to long name
    WCHAR * wzFilename = PathFindFileNameW( wstrFullPath.c_str() );
    if ( wcslen( wzFilename ) <= 12 && wcschr( wzFilename, L'~' ) )
    {
        // Convert to the long filename form. Unfortunately, this does not work for deletions, so it's an imperfect fix
        WCHAR wzBuf[MAX_PATH];
        if ( GetLongPathNameW( wstrFullPath.c_str(), wzBuf, _countof( wzBuf ) ) > 0 )
            wstrFullPath = wzBuf;
    }

    return wstrFullPath;
}






//============================================================================
//============================== CFileMonitor ==============================
//============================================================================

BOOL CFileMonitor::NormalizeParam( CONST WCHAR * aMonitorPath,
                                   BOOL & aRecursive,
                                   DWORD & aFilter,
                                   DWORD & aShareMemSize )
{
    BOOL bRet = FALSE;

    if ( NULL == aMonitorPath )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
    }
    else
    {
        aRecursive = ( FALSE != aRecursive );
        aFilter &= 0x000001FF;                             //FILE_NOTIFY_CHANGE_SECURITY
        aShareMemSize = max( 4 * 1024, aShareMemSize );    //Share memory should have at least 4KB size

        HANDLE hPath =
            CreateFileW( aMonitorPath, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                         NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL );
        if ( INVALID_HANDLE_VALUE != hPath )
        {
            CloseHandle( hPath );
            bRet = TRUE;
        }
    }
    return bRet;
}

BOOL CFileMonitor::CreateMonitorDispatcherThread()
{
    if ( NULL == m_hEventStop )
    {
        m_hEventStop = CreateEventW( NULL, TRUE, FALSE, NULL );
        if ( NULL == m_hEventStop )
        {
            return FALSE;
        }
    }

    if ( NULL == m_hThreadMonitor )
    {
        InterlockedIncrement( &m_uTotalThread );
        m_hThreadMonitor =
            (HANDLE)_beginthreadex( NULL, 0, CFileMonitor::MonitorThread, static_cast<VOID *>( this ), NULL, NULL );
        if ( (HANDLE)1 >= m_hThreadMonitor )
        {
            InterlockedDecrement( &m_uTotalThread );
            CloseHandle( m_hEventStop );
            return FALSE;
        }
    }

    if ( NULL == m_hThreadDispatcher )
    {
        InterlockedIncrement( &m_uTotalThread );
        m_hThreadDispatcher =
            (HANDLE)_beginthreadex( NULL, 0, CFileMonitor::DispatcherThread, static_cast<VOID *>( this ), NULL, NULL );
        if ( (HANDLE)1 >= m_hThreadDispatcher )
        {
            InterlockedDecrement( &m_uTotalThread );
            SetEvent( m_hEventStop );
            WaitForSingleObject( m_hThreadMonitor, INFINITE );
            CloseHandle( m_hEventStop );
            return FALSE;
        }
    }

    return TRUE;
}

VOID CFileMonitor::SetFileChangedProc( MyMonitorFileProc aFileChangedProc )
{
    _ASSERT( aFileChangedProc );
    InterlockedExchangePointer( (VOID **)&m_fnOnFileChanged, aFileChangedProc );    //Ensure it's an atomic operation
}

BOOL CFileMonitor::Start( CONST WCHAR * aMonitorPath, BOOL aRecursive, DWORD aFilter, DWORD aShareMemSize )
{
    _ASSERT( m_fnOnFileChanged );

    BOOL bRet = FALSE;

    if ( FALSE == NormalizeParam( aMonitorPath, aRecursive, aFilter, aShareMemSize ) )
    {
        goto exit;
    }
    else if ( FALSE == CreateMonitorDispatcherThread() )
    {
        goto exit;
    }
    else
    {
    }

    CFileMonitorRequest * req = new ( std::nothrow ) CFileMonitorRequest( this );
    if ( NULL == req )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        goto exit;
    }
    else if ( FALSE == req->Init( aMonitorPath, aRecursive, aFilter, aShareMemSize ) )
    {
        goto exit;
    }
    else
    {
        InterlockedIncrement( &m_uTotalWork );
        m_lsRequest.push_back( req );
        QueueUserAPC( CFileMonitorRequest::StartCheck, m_hThreadMonitor, (ULONG_PTR)req );
        bRet = TRUE;
    }

exit:
    return bRet;
}

BOOL CFileMonitor::Stop()
{
    if ( m_uTotalWork > 0 )
    {
        for ( list<CFileMonitorRequest *>::iterator it = m_lsRequest.begin(); it != m_lsRequest.end(); it++ )
        {
            //Use an APC to ensure that all previous APCs queued by ReadDirectoryChangesW() are processed
            QueueUserAPC( CFileMonitorRequest::StopCheck, m_hThreadMonitor, (ULONG_PTR)*it );
        }

        for ( INT nRetryCount = 0; nRetryCount < MONITOR_THREAD_STOP_MAX_RETRY_COUNT; nRetryCount++ )
        {
            if ( 0 == m_uTotalWork )
                break;

            Sleep( 100 );
        }

        //After all works are stopped, remove the request
        for ( list<CFileMonitorRequest *>::iterator it = m_lsRequest.begin(); it != m_lsRequest.end(); )
        {
            delete *it;
            m_lsRequest.erase( it++ );
        }
    }

    if ( NULL != m_hEventStop )
    {
        SetEvent( m_hEventStop );
        for ( INT nRetryCount = 0; nRetryCount < MONITOR_THREAD_STOP_MAX_RETRY_COUNT; nRetryCount++ )
        {
            if ( 0 == m_uTotalThread )
                break;

            Sleep( 100 );
        }
        CloseHandle( m_hEventStop );
        m_hEventStop = NULL;
    }

    if ( NULL != m_hThreadMonitor )
    {
        CloseHandle( m_hThreadMonitor );
    }

    if ( NULL != m_hThreadDispatcher )
    {
        CloseHandle( m_hThreadDispatcher );
    }

    if ( 0 != m_uTotalWork || 0 != m_uTotalThread )
    {
        SetLastError( ERROR_CANT_TERMINATE_SELF );
        return FALSE;
    }
    else
    {
        InterlockedExchangePointer( (VOID **)&m_fnOnFileChanged, NULL );    //Ensure it's an atomic operation
        return TRUE;
    }
}

UINT CALLBACK CFileMonitor::MonitorThread( VOID * aArgs )
{
    CFileMonitor * monitor = (CFileMonitor *)aArgs;
    while ( WAIT_IO_COMPLETION == WaitForSingleObjectEx( monitor->m_hEventStop, INFINITE, TRUE ) )
        ;

    InterlockedDecrement( &monitor->m_uTotalThread );
    _endthreadex( TRUE );
    return TRUE;
}

UINT CALLBACK CFileMonitor::DispatcherThread( VOID * aArgs )
{
    CFileMonitor * monitor = (CFileMonitor *)aArgs;
    HANDLE hEvents[] = { monitor->m_hEventStop, monitor->m_queue.GetEvent() };
    BOOL bRun = TRUE;
    DWORD dwRet = 0;

    while ( bRun )
    {
        dwRet = WaitForMultipleObjects( _countof( hEvents ), hEvents, FALSE, INFINITE );
        switch ( dwRet )
        {
            case WAIT_OBJECT_0:    //hEventStop is triggered
            {
                bRun = FALSE;
                break;
            }
            case WAIT_OBJECT_0 + 1:    //Data queue is not empty, handle data
            {
                CWFileMonitorNotification notification = monitor->m_queue.Pop();
                monitor->m_fnOnFileChanged( notification.dwEvent, notification.wstrSrcPath, notification.wstrDstPath );
                break;
            }
            default:
            {
                bRun = FALSE;
                break;
            }
        }
    }

    InterlockedDecrement( &monitor->m_uTotalThread );
    _endthreadex( dwRet );
    return dwRet;
}

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils