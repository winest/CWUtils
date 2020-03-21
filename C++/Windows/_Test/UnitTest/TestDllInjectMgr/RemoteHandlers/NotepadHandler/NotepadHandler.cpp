#include "stdafx.h"
#include <process.h>
#include <string>
#include <vector>
using namespace std;

#pragma warning( disable : 4127 )
#include "CWDllInjectCommonDef.h"
#include "CWDllInjectClient.h"

#include "CWGeneralUtils.h"
#include "CWString.h"
using namespace CWUtils;

#include "_GenerateTmh.h"
#include "NotepadHandler.tmh"

#define FIND_TOP_WINDOW_RETRY_TIMES    100
#define FIND_TOP_WINDOW_RETRY_DURATION ( 2 * 100 )    //In milli-seconds

#define SEND_BTN_MARGIN 10    //The outer margin of the "Send" button

#define KERNEL32_MODULE_NAME      L"kernel32.dll"
#define FREELIBRARY_FUNCTION_NAME "FreeLibrary"

CONST wstring g_TopWindowNames[] = { L"Notepad" };
CONST wstring g_EdtWindowNames[] = { L"Edit" };


volatile LONG g_lDllRefCnt = 0;


typedef struct _EnumWindowParam
{
    _EnumWindowParam() : dwMainTid( 0 ) {}
    DWORD dwMainTid;
} EnumWindowParam;



#ifdef _MANAGED
#    pragma managed( push, off )
#endif

DWORD CALLBACK FindWindowCallback( CONST WCHAR * aServerDirPath, CONST WCHAR * aClientCfgPath );
BOOL CALLBACK EnumWindowCallback( HWND hWnd, LPARAM pMainTid );
VOID UnInitialize();

LRESULT CALLBACK HookCallback( INT aCode, WPARAM aWParam, LPARAM aLParam );
BOOL CheckSendButtonDown( CONST POINT & aMousePos, CONST RECT & aChildRect, CONST RECT & aCurrentRect );
BOOL GetUserData( HWND aEditWnd, CHAR ** aUserData, UINT * aUserDataSize );
DWORD ScanUserData(
    CHAR * aUserData,
    UINT aUserDataSize );    //Return Win32 error code. Content will be blocked if return ERROR_CONTENT_BLOCKED



HMODULE g_hModule = NULL;
HANDLE g_hHookThread = NULL;
HHOOK g_hHook = NULL;

CDllInjectClient g_SmClient;

DWORD WINAPI RemoteHookHandler( VOID * pParam )
{
    UNREFERENCED_PARAMETER( pParam );

    LPTHREAD_START_ROUTINE pfnFreeLibrary = NULL;
    WCHAR wzSmName[MAX_PATH] = {};
    GenerateShareMemoryName( wzSmName, GetCurrentProcessId() );
    if ( g_SmClient.Connect( wzSmName, FindWindowCallback ) )
    {
        pfnFreeLibrary = g_SmClient.GetFreeLibraryAddr();

        BOOL bStop = FALSE;
        HANDLE hEvtWaitStop[] = { g_SmClient.GetServerQuitEvt(), g_SmClient.GetClientQuitEvt(),
                                  g_SmClient.GetServerAliveEvt() };

        while ( ! bStop )
        {
            DWORD dwWaitStop = WaitForMultipleObjects( _countof( hEvtWaitStop ), hEvtWaitStop, FALSE, INFINITE );
            switch ( dwWaitStop )
            {
                case WAIT_OBJECT_0:    //PER_SERVER_EVT_INDEX_STOP
                {
                    DbgOut( WARN, DBG_NOTEPAD_HANDLER, "PER_SERVER_EVT_INDEX_STOP event triggered" );
                    bStop = TRUE;
                    break;
                }
                case WAIT_OBJECT_0 + 1:    //PER_CLIENT_EVT_INDEX_STOP
                {
                    DbgOut( WARN, DBG_NOTEPAD_HANDLER, "PER_CLIENT_EVT_INDEX_STOP event triggered" );
                    bStop = TRUE;
                    break;
                }
                case WAIT_OBJECT_0 + 2:    //hDllInjectMgrAliveThread
                {
                    DbgOut( WARN, DBG_NOTEPAD_HANDLER, "CWDllInjectMgr leave event triggered" );
                    bStop = TRUE;
                    break;
                }
                default:
                {
                    DbgOut( ERRO, DBG_NOTEPAD_HANDLER,
                            "WaitForMultipleObjects() return unexpected value 0x%08X. GetLastError()=%!WINERROR!",
                            dwWaitStop, GetLastError() );
                    bStop = TRUE;
                    break;
                }
            }
        }

        UnhookWindowsHookEx( g_hHook );
        g_hHook = NULL;
        g_SmClient.Disconnect();
    }
    else
    {
        DbgOut( ERRO, DBG_NOTEPAD_HANDLER, "Connect() failed" );
    }

    //Be aware that some programs may hook on this function and therefore we may get an wrong address for FreeLibrary
    if ( NULL == pfnFreeLibrary )
    {
        DbgOut( ERRO, DBG_NOTEPAD_HANDLER, "Cannot get FreeLibrary's address from share memory. Try to use 0x%p",
                pfnFreeLibrary );
        pfnFreeLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress( GetModuleHandleW( KERNEL32_MODULE_NAME ),
                                                                 FREELIBRARY_FUNCTION_NAME );
    }

    //Create a thread to leave itself
    if ( 0 < ATOMIC_READ( g_lDllRefCnt ) && NULL != pfnFreeLibrary )
    {
        DWORD dwFreeTid = 0;
        HANDLE hFreeThread = CreateThread( NULL, 0, pfnFreeLibrary, g_hModule, 0, &dwFreeTid );
        DbgOut( WARN, DBG_NOTEPAD_HANDLER, "Create thread to FreeLibrary. dwFreeTid=0x%04X, hFreeThread=0x%p",
                dwFreeTid, hFreeThread );
        CloseHandle( hFreeThread );
    }

    DbgOut( INFO, DBG_NOTEPAD_HANDLER, "RemoteHookHandler() Leave" );
    return 0;
}


DWORD CALLBACK FindWindowCallback( CONST WCHAR * aServerDirPath, CONST WCHAR * aClientCfgPath )
{
    UNREFERENCED_PARAMETER( aServerDirPath );
    UNREFERENCED_PARAMETER( aClientCfgPath );

    DWORD dwRet = ERROR_NOT_READY;

    //Find Notepad's main thread
    EnumWindowParam enumWndParam;
    for ( UINT uRetryTimes = 0; uRetryTimes < FIND_TOP_WINDOW_RETRY_TIMES; uRetryTimes++ )
    {
        if ( FALSE == EnumWindows( EnumWindowCallback, (LPARAM)&enumWndParam ) && 0 != enumWndParam.dwMainTid )
        {
            DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Found Notepad window. TID=0x%04X ", enumWndParam.dwMainTid );
            break;
        }
        else
        {
            DbgOut( ERRO, DBG_NOTEPAD_HANDLER, "Failed to find Notepad window. dwMainTid=%lu", enumWndParam.dwMainTid );
        }

        Sleep( FIND_TOP_WINDOW_RETRY_DURATION );
    }
    if ( 0 == enumWndParam.dwMainTid )
    {
        DbgOut( WARN, DBG_NOTEPAD_HANDLER, "Stop finding Notepad thread and leave" );
        dwRet = ERROR_RESOURCE_ENUM_USER_STOP;
        goto exit;
    }

    //Hook on Notepad's thread
    g_hHook = SetWindowsHookEx( WH_GETMESSAGE, HookCallback, NULL, enumWndParam.dwMainTid );
    if ( NULL == g_hHook )
    {
        DbgOut( ERRO, DBG_NOTEPAD_HANDLER, "SetWindowsHookEx() failed. GetLastError()=%!WINERROR!", GetLastError() );
        dwRet = GetLastError();
        goto exit;
    }
    DbgOut( INFO, DBG_NOTEPAD_HANDLER, "SetWindowsHookEx() succeed. g_hHook=0x%p", g_hHook );

    dwRet = ERROR_SUCCESS;
exit:
    return dwRet;
}

BOOL CALLBACK EnumWindowCallback( HWND hWnd, LPARAM pEnumWndParam )
{
    _ASSERT( pEnumWndParam );
    EnumWindowParam * pParam = (EnumWindowParam *)pEnumWndParam;
    DWORD dwMainTid, dwMainPid;
    WCHAR wzClassName[MAX_PATH];
    dwMainTid = GetWindowThreadProcessId( hWnd, &dwMainPid );
    if ( GetCurrentProcessId() == dwMainPid && GetClassNameW( hWnd, wzClassName, _countof( wzClassName ) ) )
    {
        DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Checking window class name %ws", wzClassName );

        for ( size_t i = 0; i < _countof( g_TopWindowNames ); i++ )
        {
            if ( CWUtils::WildcardMatchW( wzClassName, g_TopWindowNames[i].c_str() ) )
            {
                pParam->dwMainTid = dwMainTid;
                return FALSE;    //Return FALSE will stop enumerating all windows and return FALSE to caller
            }
        }
    }
    return TRUE;
}


void UnInitialize()
{
    if ( g_SmClient.IsConnected() )
    {
        g_SmClient.Disconnect();
    }
    DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Waiting for the hooking thread terminated" );
    if ( WAIT_TIMEOUT ==
         WaitForSingleObject( g_hHookThread,
                              0 ) )    //Just log whether it's alive to avoid loader lock issue in DllMain()
    {
        DbgOut( ERRO, DBG_NOTEPAD_HANDLER, "Failed to wait until the hooking thread terminated. Unload directly" );
    }
    else
    {
        DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Successfully wait until the hooking thread terminated" );
    }

    CloseHandle( g_hHookThread );
};


BOOL APIENTRY DllMain( HMODULE aModule, DWORD aReason, LPVOID aReserved )
{
    UNREFERENCED_PARAMETER( aReserved );

    BOOL bRet = TRUE;
    switch ( aReason )
    {
        case DLL_PROCESS_ATTACH:
        {
            WPP_INIT_TRACING( L"NotepadHandler" );
            DbgOut( VERB, DBG_NOTEPAD_HANDLER, "NotepadHandler.dll DLL_PROCESS_ATTACH" );

            DisableThreadLibraryCalls( aModule );
            g_hModule = aModule;

            //Create hook thread
            DWORD dwTid;
            g_hHookThread = CreateThread( NULL, 0, RemoteHookHandler, NULL, 0, &dwTid );
            if ( NULL == g_hHookThread )
            {
                DbgOut( ERRO, DBG_NOTEPAD_HANDLER, "Failed to create remote hook thread. GetLastError()=%!WINERROR!",
                        GetLastError() );
                bRet = FALSE;
                break;
            }

            DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Remote hook thread id 0x%04X created with handle 0x%p", dwTid,
                    g_hHookThread );
            ATOMIC_INC( g_lDllRefCnt );
            break;
        }
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
        {
            DbgOut( VERB, DBG_NOTEPAD_HANDLER, "NotepadHandler.dll DLL_PROCESS_DETACH" );
            ATOMIC_DEC( g_lDllRefCnt );

            UnInitialize();
            WPP_CLEANUP();
            break;
        }
    }
    return TRUE;
}

#ifdef _MANAGED
#    pragma managed( pop )
#endif


LRESULT CALLBACK HookCallback( INT aCode, WPARAM aWParam, LPARAM aLParam )
{
    DWORD dwScanResult = ERROR_SUCCESS;
    MSG * msg = (MSG *)aLParam;

    //     if ( WM_KEYDOWN == msg->message )
    //     {
    //         DbgOut( INFO , DBG_NOTEPAD_HANDLER , "message=0x%X, aCode=%d, aWParam=0x%X, aLParam=0x%X" , msg->message , aCode , aWParam , aLParam );
    //     }

    if ( ( WM_KEYDOWN == msg->message || WM_SYSKEYDOWN == msg->message ) && VK_RETURN == msg->wParam &&
         PM_REMOVE == aWParam )
    {
        //DbgOut( INFO , DBG_NOTEPAD_HANDLER , "Enter key message. aCode=%d, aWParam=%d, msg.wnd=%p, msg.aLParam=%d" , aCode , aWParam , msg->hwnd , msg->lParam );
        DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Get a return key Message" );

        CHAR * pUserData = NULL;
        UINT uUserDataSize = 0;
        if ( GetUserData( msg->hwnd, &pUserData, &uUserDataSize ) )
        {
            dwScanResult = ScanUserData( pUserData, uUserDataSize );
            DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Get scan result. dwScanResult=%lu", dwScanResult );
            delete[] pUserData;
        }
    }

    if ( ERROR_CONTENT_BLOCKED == dwScanResult )
    {
        DbgOut( WARN, DBG_NOTEPAD_HANDLER, "Block the message!" );
        MessageBoxW( msg->hwnd, L"Blocked!!!", L"Detect", MB_OK );
        msg->message = WM_USER;
        return 0;
    }

    return CallNextHookEx( NULL, aCode, aWParam, aLParam );
}

BOOL CheckSendButtonDown( CONST POINT & aMousePos, CONST RECT & aChildRect, CONST RECT & aCurrentRect )
{
    //Currently use mouse position to check whether we press "Send" button since it doesn't have any class name
    DbgOut( VERB, DBG_NOTEPAD_HANDLER,
            "Mouse point x:%d y:%d, Child left:%d top:%d right:%d bottom:%d, Current right:%d", aMousePos.x,
            aMousePos.y, aChildRect.left, aChildRect.top, aChildRect.right, aChildRect.bottom, aCurrentRect.right );

    INT nMinY = 0;
    INT nMaxY = 0;
    INT nMinX = 0;
    INT nMaxX = 0;

    nMinY = aChildRect.top - SEND_BTN_MARGIN;
    nMaxY = aChildRect.bottom + SEND_BTN_MARGIN;
    nMinX = aChildRect.right;
    nMaxX = aCurrentRect.right;

    BOOL bRet = ( nMinX < aMousePos.x && aMousePos.x < nMaxX && nMinY < aMousePos.y && aMousePos.y < nMaxY );
    return bRet;
}

BOOL GetUserData( HWND aEditWnd, CHAR ** aUserData, UINT * aUserDataSize )
{
    _ASSERT( aUserData && aUserDataSize );

    WCHAR wzWindowClassName[MAX_PATH] = { 0 };

    DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Try to find the message input window" );
    BOOL bFoundTarget = FALSE;
    while ( aEditWnd && FALSE == bFoundTarget )
    {
        if ( 0 == GetClassNameW( aEditWnd, wzWindowClassName, _countof( wzWindowClassName ) - 1 ) )
        {
            DbgOut( INFO, DBG_NOTEPAD_HANDLER, "GetClassNameW() failed!" );
            break;
        }
        else
        {
            DbgOut( INFO, DBG_NOTEPAD_HANDLER, "GetClassNameW() get %ws", wzWindowClassName );
        }

        for ( size_t i = 0; i < _countof( g_EdtWindowNames ); i++ )
        {
            if ( CWUtils::WildcardMatchW( wzWindowClassName, g_EdtWindowNames[i].c_str() ) )
            {
                DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Class name %ws match %ws", wzWindowClassName,
                        g_EdtWindowNames[i].c_str() );
                bFoundTarget = TRUE;
                break;
            }
        }

        if ( FALSE == bFoundTarget )
        {
            aEditWnd = GetWindow( aEditWnd, GW_HWNDNEXT );
        }
    }

    if ( bFoundTarget )
    {
        INT nLen = GetWindowTextLengthA( aEditWnd );
        if ( nLen < 1 )
        {
            DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Message length < 1, Skip it" );
            bFoundTarget = FALSE;
        }
        else
        {
            *aUserData = new ( std::nothrow ) CHAR[nLen + 1];
            if ( NULL != *aUserData )
            {
                GetWindowTextA( aEditWnd, *aUserData, nLen + 1 );
                *aUserDataSize = nLen + 1;
                DbgOut( INFO, DBG_NOTEPAD_HANDLER, "Get input message=%s", *aUserData );
            }
        }
    }
    return bFoundTarget;
}


DWORD ScanUserData( CHAR * aUserData, UINT aUserDataSize )
{
    DbgOut( VERB, DBG_NOTEPAD_HANDLER, "Enter" );

    DWORD dwScanResult = ERROR_SUCCESS;
    DWORD dwSize = sizeof( dwScanResult );
    if ( g_SmClient.SendData( aUserData, aUserDataSize, (CHAR *)&dwScanResult, &dwSize ) )
    {
        if ( dwSize != sizeof( dwScanResult ) )
        {
            DbgOut( ERROR, DBG_NOTEPAD_HANDLER, "Output size unexpected. dwSize=%lu", dwSize );
            dwScanResult = ERROR_SUCCESS;
        }
    }
    else
    {
        DbgOut( ERROR, DBG_NOTEPAD_HANDLER, "SendData() failed. GetLastError()=%!WINERROR!", GetLastError() );
        dwScanResult = ERROR_SUCCESS;
    }

    DbgOut( VERB, DBG_NOTEPAD_HANDLER, "Leave. dwScanResult=0x%08X", dwScanResult );
    return dwScanResult;
}
