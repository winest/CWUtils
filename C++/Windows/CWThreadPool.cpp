#include "stdafx.h"
#include "CWThreadPool.h"
#include <process.h>
#include <crtdbg.h>

namespace CWUtils
{

#define THREAD_POOL_IOCP_COMPLETION_KEY  ERROR_CANCELLED

BOOL CThreadPoolIOCP::Start( UINT aWorkerCnt , PFN_THREAD_POOL_IOCP_WORKER_THREAD aWorkerCbk , VOID * aWorkerArgs )
{
    _ASSERT( NULL == m_hPort && aWorkerCbk );

    if ( 0 == aWorkerCnt )
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo( &sysInfo );
        aWorkerCnt = sysInfo.dwNumberOfProcessors;
    }
    aWorkerCnt = min( 32 , max( 1 , aWorkerCnt ) );

    BOOL bRet = FALSE;

    m_hPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE , NULL , 0 , aWorkerCnt );
    if ( NULL == m_hPort )
    {
        goto exit;
    }
    m_pfnWorkerCbk = aWorkerCbk;
    m_pWorkerArgs = aWorkerArgs;

    //The real thread count should be larger since IoCompletionPort only check the maximal active thread count when there is a new request
    //Therefore, there is some possibility that the real active thread count is larger than the aWorkerCnt we specified
    for ( DWORD i = 0 ; i < ( aWorkerCnt * 2 ) ; i++ )
    {
        InterlockedIncrement( &m_lThreadCnt );
        HANDLE hWorkerThread = (HANDLE)_beginthreadex( NULL , 0 , CThreadPoolIOCP::IOCPWorkerThread , this , 0 , NULL );
        if ( NULL == hWorkerThread )
        {
            InterlockedDecrement( &m_lThreadCnt );
            goto exit;
        }
        CloseHandle( hWorkerThread );
    }
    bRet = TRUE;
                
exit :
    if ( FALSE == bRet )
    {
        Stop();
    }
    return bRet;
}

BOOL CThreadPoolIOCP::Stop()
{
    if ( NULL != m_hPort )
    {
        PostQueuedCompletionStatus( m_hPort , 0 , THREAD_POOL_IOCP_COMPLETION_KEY , NULL );
        CloseHandle( m_hPort );
        m_hPort = NULL;
    }

    while ( 0 < m_lThreadCnt )
    {
        Sleep( 100 );
    }

    m_pfnWorkerCbk = NULL;
    m_pWorkerArgs = NULL;

    return TRUE;
}

BOOL CThreadPoolIOCP::WaitAllJobs( DWORD aWaitMilliSec )
{
    BOOL bRet = FALSE;
    LARGE_INTEGER liFreq , liCurrTime , liEndTime;
    QueryPerformanceFrequency( &liFreq );
    QueryPerformanceCounter( &liCurrTime );
    liEndTime.QuadPart = liCurrTime.QuadPart + aWaitMilliSec * ( liFreq.QuadPart / 1000 );
    while ( liCurrTime.QuadPart < liEndTime.QuadPart )
    {
        if ( 0 == m_lJobCnt )
        {
            bRet = TRUE;
            break;
        }
        Sleep( 10 );
        QueryPerformanceCounter( &liCurrTime );
    }
    return bRet;
}

UINT CALLBACK CThreadPoolIOCP::IOCPWorkerThread( VOID * aArgs )
{
    CThreadPoolIOCP * pThis = (CThreadPoolIOCP *)aArgs;
    UINT uRet = pThis->DoIOCPWorkerThread();
    InterlockedDecrement( &pThis->m_lThreadCnt );
    _endthreadex( uRet );
    return uRet;
}

UINT CThreadPoolIOCP::DoIOCPWorkerThread()
{
    OVERLAPPED_ENTRY entry = { 0 };
    while ( GetQueuedCompletionStatus( m_hPort , &entry.dwNumberOfBytesTransferred , &entry.lpCompletionKey , &entry.lpOverlapped , INFINITE ) )
    {
        if ( THREAD_POOL_IOCP_COMPLETION_KEY == entry.lpCompletionKey )
        {
            break;
        }
        else
        {
            this->m_pfnWorkerCbk( m_pWorkerArgs , entry.lpOverlapped );
            InterlockedDecrement( &m_lJobCnt );
        }
    }
    return GetLastError();
}

BOOL CThreadPoolIOCP::NotifyNewJob( OVERLAPPED * aOverlapped )
{
    _ASSERT( NULL != m_hPort );
    InterlockedIncrement( &m_lJobCnt );    
    return PostQueuedCompletionStatus( m_hPort , 0 , 0 , aOverlapped );
}

HANDLE CThreadPoolIOCP::GetCompletionPort()
{
    return m_hPort;
}

}