#include "stdafx.h"
#include <Windows.h>
#include <string>
using namespace std;

#include "CWTime.h"
#include "CWXml.h"

#include "_GenerateTmh.h"
#include "TestCWXml.tmh"

VOID TestXmlRead( CONST WCHAR * aXmlPath )
{
    wprintf_s( L"\n========== TestXmlRead() Enter ==========\n" );

    HRESULT hrRet = 0;
    CWUtils::CWXml xml;
    CWUtils::CWXmlElementNode root;

    do
    {
        hrRet = xml.ParserXmlFile( aXmlPath, &root );
        if ( FAILED( hrRet ) )
        {
            wprintf_s( L"Failed to parse %ws. HRESULT=0x%08X\n", aXmlPath, hrRet );
            wprintf_s(
                L"Error code list: http://msdn.microsoft.com/en-us/library/windows/desktop/ms753129(v=vs.85).aspx\n" );
            break;
        }

        xml.Print( &root );
    } while ( 0 );

    wprintf_s( L"\n========== TestXmlRead() Leave ==========\n" );
}



INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    setlocale( LC_ALL, "cht" );
    WPP_INIT_TRACING( L"TestCWXml" );
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
        if ( 1 >= aArgc )
        {
            wprintf_s( L"Usage: TestCWXml.exe <XmlFile>\n" );
            break;
        }
        TestXmlRead( aArgv[1] );
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}
