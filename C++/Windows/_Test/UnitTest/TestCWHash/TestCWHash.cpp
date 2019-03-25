#include "stdafx.h"
#include <Windows.h>
#include <string>
using namespace std;

#include "CWTime.h"
#include "CWHash.h"

#include "_GenerateTmh.h"
#include "TestCWHash.tmh"

VOID TestCrc32()
{
    wprintf_s( L"\n========== TestCrc32() Enter ==========\n" );

    UINT32 uCrc32 = 0;
    CONST CHAR * pData = "ABCDEFGHJKLMNOPQRSTUVWXYZ";
    CWUtils::GetCrc32( (CONST UCHAR *)pData, strlen( pData ), uCrc32 );
    printf_s( "CRC32 of %s is 0x%04X\n", pData, uCrc32 );

    wprintf_s( L"\n========== TestCrc32() Leave ==========\n" );
}


INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWFile" );
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
        TestCrc32();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}
