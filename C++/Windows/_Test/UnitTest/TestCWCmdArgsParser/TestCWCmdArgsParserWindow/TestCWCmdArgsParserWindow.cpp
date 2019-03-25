#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <ctime>
using namespace std;

#include "CWTime.h"
#include "CWCmdArgsParser.h"

#include "_GenerateTmh.h"
#include "TestCWCmdArgsParserWindow.tmh"


VOID TestWindowArgs()
{
    if ( !CWUtils::CCmdArgsParser::GetInstance()->ParseArgs() )
    {
        MessageBoxW( NULL, L"Error when parsing arguments", NULL, MB_OK | MB_ICONSTOP );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
        return;
    }

    CWUtils::CCmdArgsParser::GetInstance()->DumpArgs();

    CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();

    CWUtils::CCmdArgsParser::GetInstance()->ShowUsage( L"data content2" );

    wstring wstrBuf;
    CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"data content2", wstrBuf, L"DefaultVal" );
    MessageBoxW( NULL, wstrBuf.c_str(), L"Key=data content2", MB_OK | MB_ICONQUESTION );
}

VOID TestWindowArgs( CONST WCHAR * aCmdLine )
{
    if ( !CWUtils::CCmdArgsParser::GetInstance()->ParseArgs( aCmdLine ) )
    {
        MessageBoxW( NULL, L"Error when parsing arguments", NULL, MB_OK | MB_ICONSTOP );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
        return;
    }

    CWUtils::CCmdArgsParser::GetInstance()->DumpArgs();

    CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();

    CWUtils::CCmdArgsParser::GetInstance()->ShowUsage( L"data content2" );

    wstring wstrBuf;
    CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"data content2", wstrBuf, L"DefaultVal" );
    MessageBoxW( NULL, wstrBuf.c_str(), L"Key=data content2", MB_OK | MB_ICONQUESTION );
}


INT WINAPI wWinMain( HINSTANCE aHInstance, HINSTANCE aHPrevInstance, LPWSTR aCmdLine, INT aCmdShow )
{
    UNREFERENCED_PARAMETER( aHInstance );
    UNREFERENCED_PARAMETER( aHPrevInstance );
    UNREFERENCED_PARAMETER( aCmdShow );

    WPP_INIT_TRACING( L"TestCWCmdArgsParserWindow" );
    DbgOut( INFO, DBG_TEST, "Enter" );

    //Be aware that what we get from wmain() and GetCommandLineW() are different which cause different results
    WCHAR wzBuf[1024] = {};
    _snwprintf_s( wzBuf, _TRUNCATE, L"aCmdLine:%ws\nGetCommandLineW(): %ws\n", aCmdLine, GetCommandLineW() );
    MessageBoxW( NULL, wzBuf, NULL, MB_OK | MB_ICONINFORMATION );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do
    {
        //-data="111 222" -"data content2"="222 333" "-data content3=333 444" "\"-data content4=444 555\"" "-\"data content5\"=\"555 666\""
        CWUtils::CCmdArgsParser::GetInstance()->SetUsage( NULL, FALSE,
                                                          L"\nTestCWCmdArgsParserWindow.exe test CCmdArgsParser\n" );
        CWUtils::CCmdArgsParser::GetInstance()->SetUsage( L"data", FALSE, L"<Any value>" );
        CWUtils::CCmdArgsParser::GetInstance()->SetUsage( L"data content2", TRUE, L"<Any value>" );


        TestWindowArgs();
        TestWindowArgs( aCmdLine );
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    return 0;
}
