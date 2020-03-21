#include "stdafx.h"
#include <conio.h>
#include "CWWmiEventMonitor.h"

DWORD CALLBACK OnProcessCreateTerminate( BOOL aCreate, DWORD aPid, CONST WCHAR * aProcPath )
{
    wprintf_s( L"aCreate=%d, aPid=0x%04X, aProcPath=%ws\n", aCreate, aPid, aProcPath );
    return ERROR_SUCCESS;
}

INT wmain( INT aArgc, WCHAR * aArgv[] )
{
    UNREFERENCED_PARAMETER( aArgc );
    UNREFERENCED_PARAMETER( aArgv );

    HRESULT hResult = S_FALSE;
    CWUtils::CWmiEventMonitor monitor;

    //Initialize COM. Must be called for each thread that use COM objects
    hResult = CoInitializeEx( 0, COINIT_MULTITHREADED );
    if ( FAILED( hResult ) )
    {
        wprintf_s( L"CoInitializeEx() failed. hResult=0x%08X\n", hResult );
        goto exit;
    }

    //Set general COM security levels
    hResult = CoInitializeSecurity( NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
                                    EOAC_NONE, NULL );
    if ( FAILED( hResult ) )
    {
        wprintf_s( L"CoInitializeSecurity() failed. hResult=0x%08X\n", hResult );
        goto exit;
    }



    //Start to monitor
    monitor.Init();
    hResult = monitor.StartMonitorProcessEvt( OnProcessCreateTerminate );
    if ( FAILED( hResult ) )
    {
        wprintf_s( L"RegisterProcessEvt() failed. hResult=0x%08X\n", hResult );
        goto exit;
    }

    //Wait for the event
    do
    {
        wprintf_s( L"Process \"0\" to leave\n" );
    } while ( '0' != _getch() );

    monitor.StopMonitorProcessEvt();

exit:
    monitor.UnInit();
    CoUninitialize();

    wprintf_s( L"End of the program\n" );
    return 0;    // Program successfully completed.
}