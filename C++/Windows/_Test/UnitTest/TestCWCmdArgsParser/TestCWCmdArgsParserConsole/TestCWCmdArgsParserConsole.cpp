#include "stdafx.h"
#include <Windows.h>
#include <string>
#include <ctime>
using namespace std;

#include "CWTime.h"
#include "CWCmdArgsParser.h"

#include "_GenerateTmh.h"
#include "TestCWCmdArgsParserConsole.tmh"


VOID TestConsoleArgs()
{
    wprintf_s( L"\n========== TestConsoleArgs() Enter ==========\n" );

    do 
    {
        if ( ! CWUtils::CCmdArgsParser::GetInstance()->ParseArgs() )
        {
            wprintf_s( L"Error when parsing arguments\n" );
            CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
            break;
        }

        CWUtils::CCmdArgsParser::GetInstance()->DumpArgs();

        wprintf_s( L"ShowUsage():\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();

        wprintf_s( L"ShowUsage(\"data content2\"):\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage( L"data content2" );

        wstring wstrBuf;
        CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"Data content2" , wstrBuf , L"DefaultVal" );
        wprintf_s( L"wstrBuf=%ws\n" , wstrBuf.c_str() );
    } while ( 0 );

    wprintf_s( L"\n========== TestConsoleArgs() Leave ==========\n" );
}

VOID TestConsoleArgs( INT aArgc , WCHAR * aArgv[] )
{
    wprintf_s( L"\n========== TestConsoleArgs() Enter ==========\n" );

    do 
    {
        if ( ! CWUtils::CCmdArgsParser::GetInstance()->ParseArgs( aArgc , aArgv , TRUE ) )
        {
            wprintf_s( L"Error when parsing arguments\n" );
            CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
            break;
        }

        CWUtils::CCmdArgsParser::GetInstance()->DumpArgs();

        wprintf_s( L"ShowUsage():\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();

        wprintf_s( L"ShowUsage(\"data content2\"):\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage( L"data content2" );

        wstring wstrBuf;
        CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"data content2" , wstrBuf , L"DefaultVal" );
        wprintf_s( L"wstrBuf=%ws\n" , wstrBuf.c_str() );
    } while ( 0 );

    wprintf_s( L"\n========== TestConsoleArgs() Leave ==========\n" );
}


INT wmain( INT aArgc , WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWCmdArgsParserConsole" );
    DbgOut( INFO , DBG_TEST , "Enter" );

    //Be aware that what we get from wmain() and GetCommandLineW() are different which cause different results
    for ( int i = 0 ; i < aArgc ; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n" , i , aArgv[i] );
    }
    wprintf_s( L"GetCommandLineW(): %ws\n" , GetCommandLineW() );
    wprintf_s( L"Start\n" );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do 
    {
        //-data="111 222" -"data content2"="222 333" "-data content3=333 444" "\"-data content4=444 555\"" "-\"data content5\"=\"555 666\""
        CWUtils::CCmdArgsParser::GetInstance()->SetUsage( NULL , FALSE , L"\nTestCWCmdArgsParserConsole.exe test CCmdArgsParser\n" );
        CWUtils::CCmdArgsParser::GetInstance()->SetUsage( L"data" , FALSE , L"<Any value>" );
        CWUtils::CCmdArgsParser::GetInstance()->SetUsage( L"data content2" , TRUE , L"<Any value>" );

        TestConsoleArgs();
        TestConsoleArgs( aArgc , aArgv );
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n" , stopWatch.GetIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO , DBG_TEST , "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}