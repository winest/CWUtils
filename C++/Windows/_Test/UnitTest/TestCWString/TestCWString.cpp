#include "stdafx.h"
#include <Windows.h>
#include <string>
using namespace std;

#include "CWTime.h"
#include "CWString.h"

#include "_GenerateTmh.h"
#include "TestCWString.tmh"

VOID TestGetStringType()
{
    wprintf_s( L"\n========== TestGetStringType() Enter ==========\n" );

    CONST CHAR * aryTest[] = { "123" , "abc" , "1A2b3C" , "1a=2b" , "1a2b3===" , "1a2b3c==" , "index_123.html" , "\t!@#$%^&*()_+" , "123\xFFisgood" };
    for ( SIZE_T i = 0 ; i < _countof(aryTest) ; i++ )
    {
        wprintf_s( L"%hs => %u\n" , aryTest[i] , CWUtils::GetStringTypeA(aryTest[i] , strlen(aryTest[i])) );
    }

    wprintf_s( L"\n========== TestGetStringType() Leave ==========\n" );
}

VOID TestFormatString()
{
    wprintf_s( L"\n========== TestFormatString() Enter ==========\n" );

    string strDetail;
    CWUtils::FormatStringA( strDetail , "\r\n [%04u] Url=%hs, Pcap=%hs" , 10 , "http" , "path" );
    wprintf_s( L"%hs\n" , strDetail.c_str() );

    wprintf_s( L"\n========== TestFormatString() Leave ==========\n" );
}


VOID TestToLower()
{
    wprintf_s( L"\n========== TestToLower() Enter ==========\n" );

    string strTest = "AbCdEFG";
    CWUtils::ToLower( strTest );
    wprintf_s( L"%hs\n" , strTest.c_str() );

    wprintf_s( L"\n========== TestToLower() Leave ==========\n" );
}


VOID TestUnEscapeString()
{
    wprintf_s( L"\n========== TestUnEscapeString() Enter ==========\n" );

    CONST WCHAR * pEscapedStr[] = {  L"" , L"12345" , L"abcde\\n" , L"\\nzzzzz" , L"QQ\\nQQ\\n" , L"WW\\n\\nWW" , L"\\\\123" , L"ha\\ha" , L"gg\\" }; 
    wstring wstrOutput;

    for ( size_t i = 0 ; i < _countof(pEscapedStr) ; i++ )
    {
        if ( FALSE == CWUtils::UnEscapeStringW( pEscapedStr[i] , wcslen(pEscapedStr[i]) , wstrOutput ) )
        {
            wprintf_s( L"UnEscapeStringW() failed. pEscapedStr=%ws\n" , pEscapedStr[i] );
        }
        else
        {
            wprintf_s( L"%ws ===> %ws\n" , pEscapedStr[i] , wstrOutput.c_str() );
        }
    }

    wprintf_s( L"\n========== TestUnEscapeString() Leave ==========\n" );
}

VOID TestUrlStringPtr()
{
    wprintf_s( L"\n========== TestUrlStringPtr() Enter ==========\n" );

    CHAR * pData[] = { "1" , "/" , "1.1.1.1" , "1.1.1.1:" , "1.1.1.1:/" , "1.1.1.1:80" , "1.1.1.1:80/test" ,
                       "http://1" , "http:///" , "http://1.1.1.1" , "http://1.1.1.1:" , "http://1.1.1.1:/" , "http://1.1.1.1:80" , "http://1.1.1.1:80/test" ,
                       "[1]" , "[1:1:1:1:]" , "[1:1:1:1:80]" , "[1:1:1:1]:80" , "[1:1:1:1]:80/" , "https://[1:1:1:1]:80/test" ,
                       "http://1.1.1.1:80/test/second/index.php?a=1&b=2;c=3#mark" };
    for ( size_t i = 0 ; i < _countof(pData) ; i++ )
    {
        string strScheme , strHost , strPort , strUri , strParams , strFragment;
        CWUtils::CUrlStringPtr url( pData[i] );
        
        url.GetScheme( strScheme );
        url.GetHost( strHost );
        url.GetPort( strPort );
        url.GetUri( strUri );
        url.GetParams( strParams );
        url.GetFragment( strFragment );
        wprintf_s( L"==========%02Iu==========\n" , i );
        wprintf_s( L"URL=%hs\n" , url.GetUrl() );

        wprintf_s( L"Scheme=%hs (%lu)\n" , strScheme.c_str() , url.GetScheme() );
        wprintf_s( L"Host=%hs\n" , strHost.c_str() );
        wprintf_s( L"Port=%hs (%lu)\n" , strPort.c_str() , url.GetPort() );
        wprintf_s( L"Uri=%hs\n" , strUri.c_str() );
        wprintf_s( L"Params=%hs\n" , strParams.c_str() );
        wprintf_s( L"Fragment=%hs\n\n" , strFragment.c_str() );
    }

    wprintf_s( L"\n========== TestUrlStringPtr() Leave ==========\n" );
}

INT wmain( INT aArgc , WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWString" );
    DbgOut( INFO , DBG_TEST , "Enter" );
    for ( int i = 0 ; i < aArgc ; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n" , i , aArgv[i] );
    }
    wprintf_s( L"Start\n" );

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do 
    {
        //TestGetStringType();
        //TestFormatString();
        TestToLower();
        //TestUnEscapeString();
        //TestUrlStringPtr();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n" , stopWatch.GetIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO , DBG_TEST , "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}
