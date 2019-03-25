#include "stdafx.h"
#include <Windows.h>
#include <string>
using namespace std;

#include "CWTime.h"
#include "CWCmdArgsParser.h"
#include "CWString.h"
#include "CWService.h"

#include "_GenerateTmh.h"
#include "TestCWService.tmh"


VOID TestInstallService( wstring & aInstallParams )
{
    wprintf_s( L"\n========== TestInstallService() Enter ==========\n" );

    wstring wstrErr, wstrSvcPath, wstrSvcName;
    vector<wstring> vecInstall;
    CWUtils::SplitStringW( aInstallParams, vecInstall, L"|" );
    DWORD dwSvcType = SERVICE_WIN32_OWN_PROCESS;
    DWORD dwStartType = SERVICE_DEMAND_START;

    for ( size_t i = 0; i < vecInstall.size(); i++ )
    {
        if ( 0 < vecInstall[i].length() )
        {
            switch ( i )
            {
                case 0:
                    wstrSvcPath = vecInstall[i];
                    break;
                case 1:
                    wstrSvcName = vecInstall[i];
                    break;
                case 2:
                    dwSvcType = wcstoul( vecInstall[i].c_str(), NULL, 10 );
                    break;
                case 3:
                    dwStartType = wcstoul( vecInstall[i].c_str(), NULL, 10 );
                    break;
                default:
                    break;
            }
        }
    }

    if ( 0 == wstrSvcPath.length() )
    {
        wprintf_s( L"wstrSvcPath is empty\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
        goto exit;
    }
    if ( 0 == wstrSvcName.length() )
    {
        size_t uBaseNamePos = wstrSvcPath.find_last_of( L'\\', wstring::npos );
        uBaseNamePos = ( wstring::npos == uBaseNamePos ) ? 0 : uBaseNamePos + 1;
        size_t uExtPos = wstrSvcPath.find_last_of( L'.', wstring::npos );
        wstrSvcName = wstrSvcPath.substr( uBaseNamePos, uExtPos - uBaseNamePos );
        if ( 0 == wstrSvcName.length() )
        {
            wprintf_s( L"wstrSvcName cannot be retrieved\n" );
            CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
            goto exit;
        }
    }

    wprintf_s( L"InstallService( %ws , %ws , %lu , %lu )\n", wstrSvcPath.c_str(), wstrSvcName.c_str(), dwSvcType,
               dwStartType );
    if ( FALSE == CWUtils::InstallService( wstrSvcPath.c_str(), wstrSvcName.c_str(), dwSvcType, dwStartType ) )
    {
        CWUtils::GetLastErrorStringW( wstrErr );
        wprintf_s( L"InstallService() failed. GetLastError()=%lu(%ws)\n", GetLastError(), wstrErr.c_str() );
    }
    else
    {
        wprintf_s( L"InstallService() succeed\n" );
    }

exit:
    wprintf_s( L"\n========== TestInstallService() Leave ==========\n" );
}

VOID TestUninstallService( wstring & aUninstallParams )
{
    wprintf_s( L"\n========== TestUninstallService() Enter ==========\n" );

    wstring wstrErr;
    wstring wstrSvcName = aUninstallParams;
    if ( 0 == wstrSvcName.length() )
    {
        wprintf_s( L"wstrSvcName is empty\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
        goto exit;
    }

    wprintf_s( L"UninstallService( %ws )\n", wstrSvcName.c_str() );
    if ( FALSE == CWUtils::UninstallService( wstrSvcName.c_str() ) )
    {
        CWUtils::GetLastErrorStringW( wstrErr );
        wprintf_s( L"UninstallService() failed. GetLastError()=%lu(%ws)\n", GetLastError(), wstrErr.c_str() );
    }
    else
    {
        wprintf_s( L"UninstallService() succeed\n" );
    }

exit:
    wprintf_s( L"\n========== TestUninstallService() Leave ==========\n" );
}

VOID TestStartService( wstring & aStartParams )
{
    wprintf_s( L"\n========== TestStartService() Enter ==========\n" );

    wstring wstrErr;
    wstring wstrSvcName = aStartParams;
    if ( 0 == wstrSvcName.length() )
    {
        wprintf_s( L"wstrSvcName is empty\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
        goto exit;
    }

    wprintf_s( L"StartServiceByName( %ws )\n", wstrSvcName.c_str() );
    if ( FALSE == CWUtils::StartServiceByName( wstrSvcName.c_str() ) )
    {
        CWUtils::GetLastErrorStringW( wstrErr );
        wprintf_s( L"StartServiceByName() failed. GetLastError()=%lu(%ws)\n", GetLastError(), wstrErr.c_str() );
    }
    else
    {
        wprintf_s( L"StartServiceByName() succeed\n" );
    }

exit:
    wprintf_s( L"\n========== TestStartService() Leave ==========\n" );
}


VOID TestStopService( wstring & aStopParams )
{
    wprintf_s( L"\n========== TestStopService() Enter ==========\n" );

    wstring wstrErr;
    wstring wstrSvcName = aStopParams;
    if ( 0 == wstrSvcName.length() )
    {
        wprintf_s( L"wstrSvcName is empty\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
        goto exit;
    }

    wprintf_s( L"StopServiceByName( %ws )\n", wstrSvcName.c_str() );
    if ( FALSE == CWUtils::StopServiceByName( wstrSvcName.c_str() ) )
    {
        CWUtils::GetLastErrorStringW( wstrErr );
        wprintf_s( L"StopServiceByName() failed. GetLastError()=%lu(%ws)\n", GetLastError(), wstrErr.c_str() );
    }
    else
    {
        wprintf_s( L"StopServiceByName() succeed\n" );
    }

exit:
    wprintf_s( L"\n========== TestStopService() Leave ==========\n" );
}


VOID TestChangeServiceStartType( wstring & aChangeParams )
{
    wprintf_s( L"\n========== TestChangeServiceStartType() Enter ==========\n" );

    vector<wstring> vecChangeStart;
    CWUtils::SplitStringW( aChangeParams, vecChangeStart, L"|" );
    DWORD dwNewStartType = SERVICE_DEMAND_START;

    wstring wstrErr, wstrSvcName;
    for ( size_t i = 0; i < vecChangeStart.size(); i++ )
    {
        if ( 0 < vecChangeStart[i].length() )
        {
            switch ( i )
            {
                case 0:
                    wstrSvcName = vecChangeStart[i];
                    break;
                case 1:
                    dwNewStartType = wcstoul( vecChangeStart[i].c_str(), NULL, 10 );
                    break;
                default:
                    break;
            }
        }
    }

    if ( 0 == wstrSvcName.length() )
    {
        wprintf_s( L"wstrSvcName is empty\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
        goto exit;
    }

    wprintf_s( L"ChangeServiceStartType( %ws , %lu )\n", wstrSvcName.c_str(), dwNewStartType );
    if ( FALSE == CWUtils::ChangeServiceStartType( wstrSvcName.c_str(), dwNewStartType ) )
    {
        CWUtils::GetLastErrorStringW( wstrErr );
        wprintf_s( L"ChangeServiceStartType() failed. GetLastError()=%lu(%ws)\n", GetLastError(), wstrErr.c_str() );
    }
    else
    {
        wprintf_s( L"ChangeServiceStartType() succeed\n" );
    }

exit:
    wprintf_s( L"\n========== TestChangeServiceStartType() Leave ==========\n" );
}

INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    WPP_INIT_TRACING( L"TestCWService" );
    DbgOut( INFO, DBG_TEST, "Enter" );
    for ( int i = 0; i < aArgc; i++ )
    {
        wprintf_s( L"aArgv[%d]=%ws\n", i, aArgv[i] );
    }
    wprintf_s( L"Start\n" );

    CWUtils::CCmdArgsParser::GetInstance()->SetUsage(
        NULL, FALSE,
        L"\n"
        L"Usage:\n"
        L"%ws [Options]\n"
        L"\n"
        L"Options:\n"
        L"/install=<ServicePath>|<ServiceName>|<ServiceType>|<StartType>\n"
        L"/uninstall=<ServiceName>\n"
        L"/start=<ServiceName>\n"
        L"/stop=<ServiceName>\n"
        L"/change_start=<ServiceName>|<NewStartType>"
        L"\n",
        CWUtils::GetPathBaseNameW( aArgv[0] ) );
    if ( !CWUtils::CCmdArgsParser::GetInstance()->ParseArgs( aArgc, aArgv ) ||
         0 == CWUtils::CCmdArgsParser::GetInstance()->GetTotalArgsCount() )
    {
        wprintf_s( L"Error when parsing arguments\n" );
        CWUtils::CCmdArgsParser::GetInstance()->ShowUsage();
        return -1;
    }

    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    do
    {
        wstring wstrInstall, wstrUninstall, wstrStart, wstrStop, wstrChangeStart;
        if ( CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"install", wstrInstall ) )
        {
            TestInstallService( wstrInstall );
        }
        if ( CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"uninstall", wstrUninstall ) )
        {
            TestUninstallService( wstrUninstall );
        }
        if ( CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"start", wstrStart ) )
        {
            TestStartService( wstrStart );
        }
        if ( CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"stop", wstrStop ) )
        {
            TestStopService( wstrStop );
        }
        if ( CWUtils::CCmdArgsParser::GetInstance()->GetArg( L"change_start", wstrChangeStart ) )
        {
            TestChangeServiceStartType( wstrChangeStart );
        }
    } while ( 0 );

    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    system( "pause" );
    return 0;
}