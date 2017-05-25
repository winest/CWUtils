#include "stdafx.h"
#include <conio.h>
#pragma warning( disable : 4127 )
#include "CWDllInjectMgr.h"
#include "CWWmiEventMonitor.h"


#include "CWFile.h"

#include "_GenerateTmh.h"
#include "TestDllInjectMgr.tmh"


#define TEST_CFG_NAME           L"Rules.ini"
#define TEST_RULE_NAME_NOTEPAD  L"Notepad"
#define TEST_RULE_NAME_SKYPE    L"Skype"



CWUtils::CDllInjectMgr g_DllInjectMgr;
CWUtils::CWmiEventMonitor g_Monitor;



DWORD CALLBACK OnProcessCreateTerminateCbk( BOOL aCreate , DWORD aPid , CONST WCHAR * aProcPath )
{
    wprintf_s( L"aCreate=%d, aPid=0x%04X, aProcPath=%ws\n" , aCreate , aPid , aProcPath );
    g_DllInjectMgr.OnProcessCreateTerminate( aCreate , aPid , aProcPath );
    return ERROR_SUCCESS;
}


DWORD TestInjectedCbk( IN DWORD aPid , IN VOID * aUserCtx )
{
    wprintf_s( L"DllInjectServerInjectedCbk(). aPid=0x%04X, aUserCtx=0x%p\n" , aPid , aUserCtx );
    return ERROR_SUCCESS;
}


DWORD TestScanCbk( IN DWORD aPid , IN VOID * aUserCtx , CHAR * aBuf , DWORD aBufSize )
{
    wprintf_s( L"DllInjectServerScanCbk(). aPid=0x%04X, aUserCtx=0x%p, aBuf=%.*hs, aBufSize=%lu\n" ,
               aPid , aUserCtx , aBufSize , aBuf , aBufSize );
    if ( NULL != strstr( aBuf , "BLOCK" ) )
    {
        return ERROR_CONTENT_BLOCKED;   //SkypeHandler and LineHandler use this value to determine block or not
    }
    else
    {
        return ERROR_SUCCESS;
    }
}

DWORD TestUnInjectedCbk( IN DWORD aPid , IN VOID * aUserCtx )
{
    wprintf_s( L"DllInjectServerUnInjectedCbk(). aPid=0x%04X, aUserCtx=0x%p\n" , aPid , aUserCtx );
    return ERROR_SUCCESS;
}

INT wmain( INT aArgc , WCHAR * aArgv[] )
{
    UNREFERENCED_PARAMETER( aArgc );
    UNREFERENCED_PARAMETER( aArgv );

    wprintf_s( L"Start of the program\n" );
    WPP_INIT_TRACING( L"CWDllInjectMgr" );
    CoInitializeEx( 0 , COINIT_MULTITHREADED );

    HRESULT hResult = S_OK;

    wstring wstrModDir;
    CWUtils::GetModuleDir( GetModuleHandleW(NULL) , wstrModDir );

    CWUtils::DllInjectServerUserCfg cfgNotepad;
    cfgNotepad.wstrDllPath32 = wstrModDir + L"NotepadHandler32.dll";
    #ifdef _WIN64
        cfgNotepad.wstrDllPath64 = wstrModDir + L"NotepadHandler64.dll";
    #endif
    cfgNotepad.pUserCtx = (VOID *)0x123;
    cfgNotepad.InjectedCbk = TestInjectedCbk;
    cfgNotepad.ScanCbk = TestScanCbk;
    cfgNotepad.UnInjectedCbk = TestUnInjectedCbk;

    CWUtils::DllInjectServerUserCfg cfgSkype;
    cfgSkype.wstrDllPath32 = wstrModDir + L"SkypeHandler32.dll";
    #ifdef _WIN64
        cfgSkype.wstrDllPath64 = wstrModDir + L"SkypeHandler64.dll";
    #endif
    cfgSkype.pUserCtx = (VOID *)0x123;
    cfgSkype.InjectedCbk = TestInjectedCbk;
    cfgSkype.ScanCbk = TestScanCbk;
    cfgSkype.UnInjectedCbk = TestUnInjectedCbk;

    CWUtils::DllInjectServerUserCfg cfgLine;
    cfgLine.wstrDllPath32 = wstrModDir + L"LineHandler32.dll";
    #ifdef _WIN64
        cfgLine.wstrDllPath64 = wstrModDir + L"LineHandler64.dll";
    #endif
    cfgLine.pUserCtx = (VOID *)0x123;
    cfgLine.InjectedCbk = TestInjectedCbk;
    cfgLine.ScanCbk = TestScanCbk;
    cfgLine.UnInjectedCbk = TestUnInjectedCbk;

    if ( FALSE == g_DllInjectMgr.Init( (wstrModDir + TEST_CFG_NAME).c_str() ) )
    {
        wprintf_s( L"g_DllInjectMgr.Init() failed. GetLastError()=%lu\n" , GetLastError() );
        goto exit;
    }
    if ( FALSE == g_DllInjectMgr.RegisterDllInject( TEST_RULE_NAME_NOTEPAD , &cfgNotepad ) )
    {
        wprintf_s( L"g_DllInjectMgr.RegisterDllInject() Notepad failed. GetLastError()=%lu\n" , GetLastError() );
        goto exit;
    }
    if ( FALSE == g_DllInjectMgr.RegisterDllInject( TEST_RULE_NAME_SKYPE , &cfgSkype ) )
    {
        wprintf_s( L"g_DllInjectMgr.RegisterDllInject() Skype failed. GetLastError()=%lu\n" , GetLastError() );
        goto exit;
    }
    if ( FALSE == g_DllInjectMgr.StartMonitor( TEST_RULE_NAME_NOTEPAD , TRUE ) )
    {
        wprintf_s( L"g_DllInjectMgr.StartMonitor() Notepad failed. GetLastError()=%lu\n" , GetLastError() );
        goto exit;
    }
    if ( FALSE == g_DllInjectMgr.StartMonitor( TEST_RULE_NAME_SKYPE , TRUE ) )
    {
        wprintf_s( L"g_DllInjectMgr.StartMonitor() Skype failed. GetLastError()=%lu\n" , GetLastError() );
        goto exit;
    }

    hResult = g_Monitor.Init();
    if ( FAILED(hResult) )
    {
        wprintf_s( L"g_Monitor.Init() failed. hResult()=%lu\n" , hResult );
        goto exit;
    }
    hResult = g_Monitor.StartMonitorProcessEvt( OnProcessCreateTerminateCbk );
    if ( FAILED(hResult) )
    {
        wprintf_s( L"_Monitor.StartMonitorProcessEvt() failed. hResult()=%lu\n" , hResult );
        goto exit;
    }

    do 
    {
        wprintf_s( L"Press 0 to exit\n" );
    } while ( '0' != _getch() );

exit :
    g_DllInjectMgr.StopMonitor( TEST_RULE_NAME_NOTEPAD );
    g_DllInjectMgr.StopMonitor( TEST_RULE_NAME_SKYPE );
    g_DllInjectMgr.UnregisterDllInject( TEST_RULE_NAME_NOTEPAD );
    g_DllInjectMgr.UnregisterDllInject( TEST_RULE_NAME_SKYPE );
    g_DllInjectMgr.UnInit();
    g_Monitor.StopMonitorProcessEvt();
    g_Monitor.UnInit();

    CoUninitialize();
    WPP_CLEANUP();
    wprintf_s( L"End of the program\n" );
    _getch();
    return 0;
}
