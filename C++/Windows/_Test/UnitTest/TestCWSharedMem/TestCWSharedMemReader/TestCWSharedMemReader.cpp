#include "stdafx.h"
#include <csignal>

#include "../CommonDef.h"

#include "CWTime.h"
#include "CWSharedMem.h"

#include "_GenerateTmh.h"
#include "TestCWSharedMemReader.tmh"

CWUtils::CFixedTypeShmQueue<CWUtils::CSharedMemFileMapping, Data> g_Shm;
int64_t g_Cnt = 0;
int64_t g_Sum = 0;
bool g_Stop = false;
void SignalHandler( int aSignal )
{
    g_Stop = true;
    g_Shm.Close();
    wprintf_s( L"SignalHandler=%d\n", aSignal );
    wprintf_s( L"Cnt=%I64d, Sum=%I64d\n", g_Cnt, g_Sum );
    system( "pause" );
    exit( -1 );
}

INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWSharedMemReader" );
    DbgOut( INFO, DBG_TEST, "Enter" );
    for ( int i = 0; i < aArgc; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n", i, aArgv[i] );
    }
    wprintf_s( L"Start\n" );

    signal( SIGTERM, SignalHandler );
    signal( SIGINT, SignalHandler );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do
    {
        if ( false == g_Shm.InitReader( "TestShm", false ) )
        {
            DbgOut( INFO, DBG_TEST, "InitReader() failed. Err=%!WINERROR!", GetLastError() );
            break;
        }

        while ( g_Cnt < 10000 && !g_Stop )
        {
            Data * data = g_Shm.GetData();
            if ( data != nullptr )
            {
                g_Sum += data->Num;
                g_Shm.PopFront();
                ++g_Cnt;
            }
            else
            {
                Sleep( rand() % 5 );
            }
        }
        g_Shm.Close();

        wprintf_s( L"nSum=%I64d\n", g_Sum );
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );

    return 0;
}
