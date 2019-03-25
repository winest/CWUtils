#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <list>
#include <map>
using namespace std;

#include "CWTime.h"
#include "CWThreadPool.h"

#include "_GenerateTmh.h"
#include "TestCWThreadPool.tmh"



#define MAX_THREAD_ID 0x10000

#define MAX_JOB_CNT 100000
#define DATA_CNT 1000

typedef struct _WORKER_THREAD_ARGS
{
    UINT uDataCnt;
    UINT * pData;
} WORKER_THREAD_ARGS;

volatile LONGLONG g_llSum = 0;
UINT * g_pThreadStatistics = NULL;



DWORD CALLBACK CrazyAdd( VOID * aArgs, OVERLAPPED * aOverlapped );

VOID TestThreadPoolIOCP()
{
    wprintf_s( L"\n========== TestThreadPoolIOCP() Enter ==========\n" );

    CWUtils::CThreadPoolIOCP manager;
    UINT uJobCnt = 0;


    g_pThreadStatistics = new ( std::nothrow ) UINT[MAX_THREAD_ID];
    if ( NULL == g_pThreadStatistics )
    {
        wprintf_s( L"Failed to allocate g_pThreadStatistics\n" );
        goto exit;
    }
    ZeroMemory( g_pThreadStatistics, sizeof( UINT ) * MAX_THREAD_ID );



    //Input worker count
    INT nRet = 0;
    UINT uWorkerCnt = 0;
    wprintf_s( L"Please input worker count: " );
    wscanf_s( L"%d", &uWorkerCnt, sizeof( uWorkerCnt ) );

    //Prepare common arguments between threads
    WORKER_THREAD_ARGS * args = new ( std::nothrow ) WORKER_THREAD_ARGS[MAX_JOB_CNT];
    ZeroMemory( args, sizeof( WORKER_THREAD_ARGS ) * MAX_JOB_CNT );
    for ( UINT i = 0; i < MAX_JOB_CNT; i++ )
    {
        args[i].pData = new ( std::nothrow ) UINT[DATA_CNT];
        if ( NULL != args[i].pData )
        {
            args[i].uDataCnt = DATA_CNT;
            ZeroMemory( args[i].pData, sizeof( UINT ) * DATA_CNT );
            for ( UINT j = 0; j < DATA_CNT; j++ )
            {
                args[i].pData[j] += i * j;
            }
            uJobCnt++;
        }
        else
        {
            break;
        }
    }
    wprintf_s( L"Total job count=%u\n", uJobCnt );

    //Start thread pool
    wprintf_s( L"Start thread pool\n" );
    if ( FALSE == manager.Start( uWorkerCnt, CrazyAdd, args ) )
    {
        wprintf_s( L"Failed to start thread pool. GetLastError()=%lu\n", GetLastError() );
        goto exit;
    }

    //Give new jobs
    wprintf_s( L"Dispatching jobs\n" );
    for ( UINT i = 0; i < uJobCnt; i++ )
    {
        manager.NotifyNewJob( (OVERLAPPED *)i );
    }
    wprintf_s( L"All jobs are dispatched\n" );

    //Wait some time for jobs to be done, or jobs will be cancelled if we call Stop() immediately
    wprintf_s( L"WaitAllJobs() return %d\n", manager.WaitAllJobs( INFINITE ) );
    manager.Stop();

    //Print the result and statistics
    wprintf_s( L"\n" );
    UINT uThreadCnt = 0;
    UINT uJobSolved = 0;
    for ( UINT i = 0; i < MAX_THREAD_ID; i++ )
    {
        if ( 0 != g_pThreadStatistics[i] )
        {
            wprintf_s( L"%2u. Thread 0x%04X processed %u requests\n", ++uThreadCnt, i, g_pThreadStatistics[i] );
            uJobSolved += g_pThreadStatistics[i];
        }
    }
    wprintf_s( L"g_llSum=%I64u\n", g_llSum );
    wprintf_s( L"Solved job count / Total job count = %u / %u (%.1lf%%)\n\n", uJobSolved, uJobCnt,
               (DOUBLE)uJobSolved * 100 / (DOUBLE)uJobCnt );


exit:
    if ( NULL != args )
    {
        for ( UINT i = 0; i < MAX_JOB_CNT; i++ )
        {
            if ( NULL != args[i].pData )
            {
                delete[] args[i].pData;
            }
            else
            {
                break;
            }
        }
        delete[] args;
    }
    if ( NULL != g_pThreadStatistics )
    {
        delete[] g_pThreadStatistics;
    }

    wprintf_s( L"\n========== TestThreadPoolIOCP() Leave ==========\n" );
}


INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWThreadPool" );
    DbgOut( INFO, DBG_TEST, "Enter" );
    for ( int i = 0; i < aArgc; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n", i, aArgv[i] );
    }
    wprintf_s( L"Start\n" );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do
    {
        TestThreadPoolIOCP();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}


DWORD CALLBACK CrazyAdd( VOID * aArgs, OVERLAPPED * aOverlapped )
{
    WORKER_THREAD_ARGS * args = (WORKER_THREAD_ARGS *)aArgs;
    UINT uJobId = (UINT)aOverlapped;

    UINT64 uResult = 0;
    for ( UINT i = 0; i < args[uJobId].uDataCnt; i++ )
    {
        uResult += ( UINT64 )( (DOUBLE)args[uJobId].pData[i] * 1.5 );
    }

    //Users are responsible for freeing memory they allocated
    delete[] args[uJobId].pData;
    args[uJobId].pData = NULL;

    InterlockedExchangeAdd64( &g_llSum, (LONGLONG)uResult );
    InterlockedIncrement( (volatile LONG *)&g_pThreadStatistics[GetCurrentThreadId()] );

    return GetLastError();
}