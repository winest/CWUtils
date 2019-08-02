#pragma once

/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their 
 * programming. It should be very easy to port them to other projects or 
 * learn how to implement things on different languages and platforms. 
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

#pragma warning( push, 0 )
#include <Windows.h>
#include <process.h>
#include <crtdbg.h>
#pragma warning( pop )

namespace CWUtils
{
typedef DWORD( CALLBACK * PFN_THREAD_POOL_IOCP_WORKER_THREAD )( VOID * aArgs, OVERLAPPED * aOverlapped );

class CThreadPoolIOCP
{
    public:
    CThreadPoolIOCP() :
        m_hPort( NULL ),
        m_lThreadCnt( 0 ),
        m_lJobCnt( 0 ),
        m_pfnWorkerCbk( NULL ),
        m_pWorkerArgs( NULL )
    {
    }
    ~CThreadPoolIOCP() { Stop(); }

    public:
    //If aWorkerCnt is 0, we will use processor's core count. Users should free aWorkerArgs themselves
    BOOL Start( UINT aWorkerCnt, PFN_THREAD_POOL_IOCP_WORKER_THREAD aWorkerCbk, VOID * aWorkerArgs );
    BOOL Stop();    //Stop will discard all incomplete jobs and close the completion port
    BOOL WaitAllJobs(
        DWORD
            aWaitMilliSec );    //If you want to make sure all jobs are done before leave, call WaitAllJobs() before Stop()

    BOOL NotifyNewJob( OVERLAPPED * aOverlapped = NULL );
    HANDLE GetCompletionPort();

    protected:
    static UINT CALLBACK IOCPWorkerThread( VOID * aArgs );
    UINT DoIOCPWorkerThread();

    private:
    HANDLE m_hPort;
    volatile LONG m_lThreadCnt, m_lJobCnt;
    PFN_THREAD_POOL_IOCP_WORKER_THREAD m_pfnWorkerCbk;
    VOID * m_pWorkerArgs;
};

}    // namespace CWUtils
