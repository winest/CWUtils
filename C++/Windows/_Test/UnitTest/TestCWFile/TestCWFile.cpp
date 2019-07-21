#include "stdafx.h"
#include <Windows.h>
#include <string>
using namespace std;

#include "CWTime.h"
#include "CWFile.h"

#include "_GenerateTmh.h"
#include "TestCWFile.tmh"

VOID TestSaveToFile()
{
    wprintf_s( L"\n========== TestSaveToFile() Enter ==========\n" );

    CONST CHAR * pData = "ABCDEFGHJKLMNOPQRSTUVWXYZ";
    CWUtils::SaveToFile( L"123.txt", TRUE, (CONST UCHAR *)pData, strlen( pData ) );

    wprintf_s( L"\n========== TestSaveToFile() Leave ==========\n" );
}

VOID TestFile()
{
    wprintf_s( L"\n========== TestFile() Enter ==========\n" );

    do
    {
        CWUtils::CFile file;
        if ( FALSE == file.Open( L"456.txt", CWUtils::FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST | CWUtils::FILE_OPEN_ATTR_WRITE, "\n" ) )
        {
            wprintf_s( L"Open() failed\n" );
            break;
        }

        CONST CHAR * pData = "Content of 456";
        file.Write( (CONST UCHAR *)pData, strlen( pData ) );
        file.Close();
    } while ( 0 );

    wprintf_s( L"\n========== TestFile() Leave ==========\n" );
}

VOID TestCsv()
{
    wprintf_s( L"\n========== TestCsv() Enter ==========\n" );

    do
    {
        CWUtils::CCsv file;
        if ( FALSE == file.Open( L"123.csv", CWUtils::FILE_OPEN_ATTR_CREATE_ALWAYS | CWUtils::FILE_OPEN_ATTR_WRITE, "\r\n" ) )
        {
            wprintf_s( L"Open() failed\n" );
            break;
        }

        vector<string> vecData;
        CONST CHAR * pData[] = { "Col1", "Col2", "3", "4.15", "6.0" };
        for ( size_t i = 0; i < _countof( pData ); i++ )
        {
            vecData.push_back( pData[i] );
        }
        file.WriteRow( vecData, TRUE );
        file.WriteRow( vecData, FALSE );

        vecData.push_back( "Col 7!!!" );
        file.WriteRow( vecData, TRUE );
        file.WriteRow( vecData, FALSE );
        file.Close();
    } while ( 0 );

    wprintf_s( L"\n========== TestCsv() Leave ==========\n" );
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
        TestSaveToFile();
        TestFile();
        TestCsv();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}
