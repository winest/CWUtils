#include "stdafx.h"
#include <Windows.h>
#include <string>
using namespace std;

#include "CWTime.h"
#include "CWBuffer.h"

#include "_GenerateTmh.h"
#include "TestCWBuffer.tmh"

VOID TestAutoBuffer()
{
    wprintf_s( L"\n========== TestAutoBuffer() Enter ==========\n" );

    CWUtils::CAutoBuffer<CHAR> autoBuf(8);
    strncpy_s( autoBuf.GetBuf() , autoBuf.GetBufSize() , "123456789" , _TRUNCATE );    wprintf_s( L"%hs\n" , autoBuf.GetBuf() );
    autoBuf[3] = 'X';                                                                  wprintf_s( L"%hs\n" , &autoBuf[0] );

    wprintf_s( L"\n========== TestAutoBuffer() Leave ==========\n" );    
}

VOID TestDynamicBuffer()
{
    wprintf_s( L"\n========== TestDynamicBuffer() Enter ==========\n" );

    CWUtils::CDynamicBuffer buf;
    buf.Append( (CONST UCHAR *)"123" , 3 );         wprintf_s( L"%hs\n" , buf.GetBuf() );
    buf.Erase( 1 , 5 );                             wprintf_s( L"%hs\n" , buf.GetBuf() );
    buf.Insert( 0 , (CONST UCHAR *)"456" , 3 );     wprintf_s( L"%hs\n" , buf.GetBuf() );

    CWUtils::CDynamicBuffer buf2( buf );            wprintf_s( L"%hs\n" , buf2.GetBuf() );
    buf2 += buf;                                    wprintf_s( L"%hs\n" , &buf2[0] );

    wprintf_s( L"\n========== TestDynamicBuffer() Leave ==========\n" );    
}

VOID TestMemFileBuffer()
{
    wprintf_s( L"\n========== TestMemFileBuffer() Enter ==========\n" );

    BOOL bEnd = FALSE;
    CHAR szBuf[4096] = { 0 };
    string strBuf = "456";
    CWUtils::CMemFileBuffer memFileBuf( 1 , L"." );
    memFileBuf << "123";                                            wprintf_s( L"%c\n" , memFileBuf[1] );
    memFileBuf.Read( 0 , (UCHAR *)szBuf , sizeof(szBuf) , bEnd );   wprintf_s( L"%hs. bEnd=%d\n" , szBuf , bEnd );
    memFileBuf << strBuf;                                           wprintf_s( L"%hs\n" , strBuf.c_str() );
    memFileBuf >> strBuf;                                           wprintf_s( L"%hs\n" , strBuf.c_str() );

    wprintf_s( L"\n========== TestMemFileBuffer() Leave ==========\n" );    
}

INT wmain( INT aArgc , WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWBuffer" );
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
        TestAutoBuffer();
        TestDynamicBuffer();
        TestMemFileBuffer();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n" , stopWatch.GetIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO , DBG_TEST , "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}
