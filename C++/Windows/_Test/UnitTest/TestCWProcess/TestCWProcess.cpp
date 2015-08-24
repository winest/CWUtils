// TestCWProcess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "CWString.h"
#include "CWTime.h"
#include "CWProcess.h"

#include "_GenerateTmh.h"
#include "TestCWProcess.tmh"

VOID TestChangeDaclPermission()
{
    wprintf_s( L"\n========== TestChangeDaclPermission() Enter ==========\n" );

    wstring wstrPath = L"C:\\Test.txt";

    if ( FALSE == CWUtils::ChangeDaclPermission( wstrPath.c_str() , SE_FILE_OBJECT , GENERIC_ALL , GRANT_ACCESS , NO_INHERITANCE ,
                                                 L"winest" , TRUSTEE_IS_NAME , TRUSTEE_IS_USER ) )
    {
        wstring wstrErr;
        CWUtils::GetLastErrorStringW( wstrErr );
        wprintf_s( L"ChangeDaclPermission() failed. GetLastError()=%lu(%ws)\n" , GetLastError() , wstrErr.c_str());
    }
    else
    {
        wprintf_s( L"ChangeDaclPermission() succeed\n" );
    }


    SIZE_T sizeSid = SECURITY_MAX_SID_SIZE;
    PSID pSid = LocalAlloc( LMEM_FIXED , sizeSid );
    if ( NULL == pSid )
    {    
        wprintf_s( L"LocalAlloc() failed\n" );
    }
    else
    {
        if ( FALSE == CreateWellKnownSid( WinWorldSid , NULL , pSid , &sizeSid ) )
        {
            wprintf_s( L"CreateWellKnownSid() failed\n" );
        }
        else
        {
            if ( FALSE == CWUtils::ChangeDaclPermission( wstrPath.c_str() , SE_FILE_OBJECT , GENERIC_ALL , GRANT_ACCESS , NO_INHERITANCE ,
                                                         (CONST WCHAR *)pSid , TRUSTEE_IS_SID , TRUSTEE_IS_GROUP ) )
            {
                wstring wstrErr;
                CWUtils::GetLastErrorStringW( wstrErr );
                wprintf_s( L"ChangeDaclPermission() failed. GetLastError()=%lu(%ws)\n" , GetLastError() , wstrErr.c_str());
            }
            else
            {
                wprintf_s( L"ChangeDaclPermission() succeed\n" );
            }
        }
        LocalFree( pSid );
    }

    wprintf_s( L"\n========== TestChangeDaclPermission() Leave ==========\n" );
}


INT wmain( INT aArgc , WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWProcess" );
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
        TestChangeDaclPermission();
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n" , stopWatch.GetIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO , DBG_TEST , "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}

