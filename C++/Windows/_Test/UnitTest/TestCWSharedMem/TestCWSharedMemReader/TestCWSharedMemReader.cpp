#include "stdafx.h"
#include <csignal>

#include "../CommonDef.h"

#include "CWTime.h"
#include "CWSharedMem.h"

#include "_GenerateTmh.h"
#include "TestCWSharedMemReader.tmh"

CWUtils::CFixedSizeShmQueue<CWUtils::CSharedMemFileMapping> g_ShmFixedSize;
CWUtils::CFixedTypeShmQueue<CWUtils::CSharedMemFileMapping, Data> g_ShmFixedType;
int64_t g_CntFixedSize = 0, g_CntFixedType = 0;
int64_t g_SumFixedSize = 0, g_SumFixedType = 0;
bool g_Stop = false;
void SignalHandler( int aSignal )
{
    g_Stop = true;
    g_ShmFixedType.Close();
    wprintf_s( L"SignalHandler=%d\n", aSignal );
    wprintf_s( L"CntFixedSize=%I64d, SumFixedSize=%I64d\n", g_CntFixedSize, g_SumFixedSize );
    wprintf_s( L"CntFixedType=%I64d, SumFixedType=%I64d\n", g_CntFixedType, g_SumFixedType );
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
        if ( false == g_ShmFixedSize.InitReader( "TestShmFixedSize", sizeof( Data ), false ) )
        {
            DbgOut( INFO, DBG_TEST, "TestShmFixedSize InitReader() failed. Err=%!WINERROR!", GetLastError() );
            break;
        }
        if ( false == g_ShmFixedType.InitReader( "TestShmFixedType", false ) )
        {
            DbgOut( INFO, DBG_TEST, "TestShmFixedType InitReader() failed. Err=%!WINERROR!", GetLastError() );
            break;
        }

        while ( ( g_CntFixedSize < 10000 || g_CntFixedType < 10000 ) && !g_Stop )
        {
            bool bHasData = false;

            Data * dataFixedSize = reinterpret_cast<Data *>( g_ShmFixedSize.GetData() );
            if ( dataFixedSize != nullptr )
            {
                g_SumFixedSize += dataFixedSize->Num;
                g_ShmFixedSize.PopFront();
                ++g_CntFixedSize;
                bHasData = true;
            }

            Data * dataFixedType = g_ShmFixedType.GetData();
            if ( dataFixedType != nullptr )
            {
                g_SumFixedType += dataFixedType->Num;
                g_ShmFixedType.PopFront();
                ++g_CntFixedType;
                bHasData = true;
            }


            if ( !bHasData )
            {
                Sleep( rand() % 5 );
            }
        }
        g_ShmFixedSize.Close();
        g_ShmFixedType.Close();

        wprintf_s( L"CntFixedSize=%I64d, SumFixedSize=%I64d\n", g_CntFixedSize, g_SumFixedSize );
        wprintf_s( L"CntFixedType=%I64d, SumFixedType=%I64d\n", g_CntFixedType, g_SumFixedType );
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );

    return 0;
}
