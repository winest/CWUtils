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

    CWUtils::CFixedSizeShmQueue<CWUtils::CSharedMemFileMapping> shmFixedSize;
    CWUtils::CFixedTypeShmQueue<CWUtils::CSharedMemFileMapping, Data> shmFixedType;
    do
    {
        if ( false == shmFixedSize.InitWriter( "TestShmFixedSize", sizeof( Data ), 10000 ) )
        {
            DbgOut( INFO, DBG_TEST, "TestShmFixedSize InitWriter() failed. Err=%!WINERROR!", GetLastError() );
            break;
        }
        if ( false == shmFixedType.InitWriter( "TestShmFixedType", 10000 ) )
        {
            DbgOut( INFO, DBG_TEST, "TestShmFixedType InitWriter() failed. Err=%!WINERROR!", GetLastError() );
            break;
        }

        int64_t nCnt = 0;
        int64_t nSum = 0;
        while ( nCnt < 10000 )
        {
            Data data;
            data.Num = (int32_t)'A' + rand() % 25;
            nSum += data.Num;
            shmFixedSize.PushBack( reinterpret_cast<uint8_t *>( &data ) );
            shmFixedType.PushBack( data );
            ++nCnt;
            Sleep( 1 );
        }
        std::cout << "nSum=" << nSum << std::endl;
    } while ( 0 );
    shmFixedSize.Close();
    shmFixedType.Close();

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );

    return 0;
}
