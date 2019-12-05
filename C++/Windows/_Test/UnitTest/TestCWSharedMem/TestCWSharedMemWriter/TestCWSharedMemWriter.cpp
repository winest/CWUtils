#include "stdafx.h"

#include "../CommonDef.h"

#include "CWTime.h"
#include "CWSharedMem.h"

#include "_GenerateTmh.h"
#include "TestCWSharedMemWriter.tmh"

INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWSharedMemWriter" );
    DbgOut( INFO, DBG_TEST, "Enter" );
    for ( int i = 0; i < aArgc; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n", i, aArgv[i] );
    }
    wprintf_s( L"Start\n" );

    srand( (unsigned int)time( NULL ) );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();


    CWUtils::CFixedTypeShmQueue<CWUtils::CSharedMemFileMapping, Data> shm;
    do
    {
        if ( false == shm.InitWriter( "TestShm", 2000 ) )
        {
            DbgOut( INFO, DBG_TEST, "InitWriter() failed. Err=%!WINERROR!", GetLastError() );
            break;
        }

        int64_t nCnt = 0;
        int64_t nSum = 0;
        while ( nCnt < 10000 )
        {
            Data data;
            data.Num = (int32_t)'A' + rand() % 25;
            nSum += data.Num;
            shm.PushBack( data );
            ++nCnt;
            Sleep( 1 );
        }
        std::cout << "nSum=" << nSum << std::endl;
    } while ( 0 );
    shm.Close();

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );

    return 0;
}
