#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <process.h>
using namespace std;

#include "CWTime.h"
#include "CWQueue.h"

#include "_GenerateTmh.h"
#include "TestCWQueue.tmh"

HANDLE g_hEvtExit = NULL;
CWUtils::CQueueMRMW<int> g_queMRMW;

UINT CALLBACK TestQueueMRMWWriter( VOID * aArgs )
{
    wprintf_s( L"[%04X] Writer\n", GetCurrentThreadId() );
    for ( int i = 0; i < 10; i++ )
    {
        g_queMRMW.Push( GetCurrentThreadId() / 100 * 100 + i );
    }
    return ERROR_SUCCESS;
}

UINT CALLBACK TestQueueMRMWReader( VOID * aArgs )
{
    wprintf_s( L"[%04X] Reader\n", GetCurrentThreadId() );
    BOOL bLoop = TRUE;
    HANDLE hEvts[] = { g_hEvtExit, g_queMRMW.GetEvent() };

    while ( bLoop )
    {
        DWORD dwRet = WaitForMultipleObjects( _countof( hEvts ), hEvts, FALSE, INFINITE );
        switch ( dwRet )
        {
            case WAIT_OBJECT_0:
            {
                bLoop = FALSE;
                break;
            }
            case WAIT_OBJECT_0 + 1:
            {
                int nData = g_queMRMW.Pop();
                wprintf_s( L"[%04X] %d\n", GetCurrentThreadId(), nData );
                break;
            }
            default:
            {
                bLoop = FALSE;
                break;
            }
        }
    }
    return ERROR_SUCCESS;
}

VOID TestQueueMRMW()
{
    wprintf_s( L"\n========== TestQueueMRMW() Enter ==========\n" );
    wprintf_s( L"[%04X] Main\n", GetCurrentThreadId() );
    g_hEvtExit = CreateEventW( NULL, TRUE, FALSE, NULL );

    HANDLE hWriters[2] = {};
    for ( size_t i = 0; i < _countof( hWriters ); i++ )
    {
        hWriters[i] = (HANDLE)_beginthreadex( NULL, 0, TestQueueMRMWWriter, NULL, 0, NULL );
        if ( NULL == hWriters )
        {
            break;
        }
    }
    WaitForMultipleObjects( _countof( hWriters ), hWriters, TRUE, INFINITE );

    HANDLE hReaders[3] = {};
    for ( size_t i = 0; i < _countof( hReaders ); i++ )
    {
        hReaders[i] = (HANDLE)_beginthreadex( NULL, 0, TestQueueMRMWReader, NULL, 0, NULL );
        if ( NULL == hReaders )
        {
            break;
        }
    }

    wprintf_s( L"Sleep 3000ms\n" );
    Sleep( 3000 );
    SetEvent( g_hEvtExit );
    WaitForMultipleObjects( _countof( hReaders ), hReaders, TRUE, INFINITE );

    for ( size_t i = 0; i < _countof( hWriters ); i++ )
    {
        CloseHandle( hWriters[i] );
    }
    for ( size_t i = 0; i < _countof( hReaders ); i++ )
    {
        CloseHandle( hReaders[i] );
    }
    CloseHandle( g_hEvtExit );

    wprintf_s( L"\n========== TestQueueMRMW() Leave ==========\n" );
}


INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWQueue" );
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
        TestQueueMRMW();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}