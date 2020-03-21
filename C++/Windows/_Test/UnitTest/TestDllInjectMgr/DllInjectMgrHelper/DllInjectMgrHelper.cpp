// DllInjectMgrHelper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "_GenerateTmh.h"
#include "DllInjectMgrHelper.tmh"

#define KERNEL32_MODULE_NAME L"kernel32.dll"



INT WINAPI WinMain( HINSTANCE aInstance, HINSTANCE aPrevInstance, LPSTR aCmdLine, INT aCmdShow )
{
    UNREFERENCED_PARAMETER( aInstance );
    UNREFERENCED_PARAMETER( aPrevInstance );
    UNREFERENCED_PARAMETER( aCmdShow );
    WPP_INIT_TRACING( L"DllInjectMgrHelper" );

    HMODULE hKernel32 = GetModuleHandleW( KERNEL32_MODULE_NAME );
    ULONG ulAddress = ( ULONG )( GetProcAddress( hKernel32, aCmdLine ) );
    DbgOut( INFO, DBG_DLL_INJECT_MGR_HELPER, "%lu = GetProcAddress(0x%p , %hs)", ulAddress, hKernel32, aCmdLine );
    if ( NULL == ulAddress )
    {
        DbgOut( ERRO, DBG_DLL_INJECT_MGR_HELPER, "GetProcAddress: %hs from %ws faild. GetLastError()=%!WINERROR!",
                aCmdLine, KERNEL32_MODULE_NAME, GetLastError() );
    }

    WPP_CLEANUP();
    return ulAddress;
}
