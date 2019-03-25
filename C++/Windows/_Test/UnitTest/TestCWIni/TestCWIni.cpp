#include "stdafx.h"
#include <Windows.h>
#include <string>
using namespace std;

#include "CWTime.h"
#include "CWIni.h"

#include "_GenerateTmh.h"
#include "TestCWIni.tmh"

VOID TestIni()
{
    wprintf_s( L"\n========== TestIni() Enter ==========\n" );

    WCHAR wzPath[MAX_PATH] = { 0 };
    wprintf_s( L"Input ini \"full\" path: " );
    wscanf_s( L"%ws", wzPath, _countof( wzPath ) );

    list<wstring> lsNames;
    if ( FALSE == CWUtils::GetIniSectionNames( wzPath, lsNames ) )
    {
        wprintf_s( L"GetIniSectionNames() failed. GetLastError()=%lu\n", GetLastError() );
        goto exit;
    }

    for ( list<wstring>::iterator itName = lsNames.begin(); itName != lsNames.end(); itName++ )
    {
        wprintf_s( L"[%ws]\n", itName->c_str() );

        map<wstring, wstring> mapKeyVals;
        if ( FALSE == CWUtils::GetIniSectionValues( wzPath, itName->c_str(), mapKeyVals ) )
        {
            wprintf_s( L"GetIniSectionValues() failed. GetLastError()=%lu\n", GetLastError() );
        }
        else
        {
            for ( map<wstring, wstring>::iterator itKeyVal = mapKeyVals.begin(); itKeyVal != mapKeyVals.end();
                  itKeyVal++ )
            {
                wprintf_s( L"\t%ws=%ws\n", itKeyVal->first.c_str(), itKeyVal->second.c_str() );
            }
        }
    }

exit:
    wprintf_s( L"\n========== TestIni() Leave ==========\n" );
}


INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWIni" );
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
        TestIni();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}