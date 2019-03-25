#include "stdafx.h"
#include "resource.h"

#include "CWControl.h"
#include "CWLabel.h"
#include "CWEdit.h"
#include "CWButton.h"
#include "ButtonProcs.h"

#include "CWGeneralUtils.h"
#include "CWTime.h"
#include "CWVolume.h"
#include "CWFileMonitor.h"

using std::wstring;
using namespace CWUtils;
using namespace CWUi;

#include "_GenerateTmh.h"
#include "TestCWFileMonitor.tmh"

#pragma comment( \
    linker,      \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"" )

CControl * g_ctrlMain[CTRL_MAIN_COUNT];

ULONG CALLBACK FileChangedProc( DWORD aAction, CONST wstring & aSrcPath, CONST wstring & aDstPath )
{
    WCHAR wzBuf[1024] = { 0 };
    CEdit * edtShow = (CEdit *)g_ctrlMain[EDT_SHOW];

    //Don't care dummy messages
    if ( L'\0' == aSrcPath[0] )
        return TRUE;

    switch ( aAction )
    {
        case MY_FILE_MONITOR_EVT_LOCAL_ADD:
            _snwprintf_s( wzBuf, _TRUNCATE, L"File created: %ws\r\n", aSrcPath.c_str() );
            break;
        case MY_FILE_MONITOR_EVT_LOCAL_REMOVE:
            _snwprintf_s( wzBuf, _TRUNCATE, L"File deleted: %ws\r\n", aSrcPath.c_str() );
            break;
        case MY_FILE_MONITOR_EVT_LOCAL_RENAME:
            _snwprintf_s( wzBuf, _TRUNCATE, L"File renamed: %ws => %ws\r\n", aSrcPath.c_str(), aDstPath.c_str() );
            break;
        case MY_FILE_MONITOR_EVT_LOCAL_MODIFY:
            _snwprintf_s( wzBuf, _TRUNCATE, L"File changed: %ws\r\n", aSrcPath.c_str() );
            break;

        default:
            _snwprintf_s( wzBuf, _TRUNCATE, L"Not interesting. aAction=0x%X, aSrcPath=%ws, aDstPath=%ws\r\n", aAction,
                          aSrcPath.c_str(), aDstPath.c_str() );
            break;
    }

    edtShow->AddText( wzBuf );
    return TRUE;
}

LRESULT CALLBACK WndProc( HWND aHWnd, UINT aMsg, WPARAM aWParam, LPARAM aLParam )
{
    static HBRUSH hBrush;

    switch ( aMsg )
    {
        case WM_CREATE:    //Receive when the window is created
        {
            HINSTANCE hInstance = ( (LPCREATESTRUCT)aLParam )->hInstance;
            int i;

            //Allocate memory for each control, don't initialize here since some messages will be sent
            for ( i = 0; i < CTRL_MAIN_COUNT; i++ )
            {
                if ( i < LAB_MAIN_END )
                {
                    g_ctrlMain[i] = new CLabel();
                }
                else if ( i < BTN_MAIN_END )
                {
                    g_ctrlMain[i] = new CButton();
                }
                else if ( i < EDT_MAIN_END )
                {
                    g_ctrlMain[i] = new CEdit();
                }
                else
                {
                    ShowDebugMsg( L"Forget to allocate memory for the control" );
                }
            }

            WCHAR szLab[LAB_MAIN_COUNT][100] = { L"Input:" };
            WCHAR szBtn[BTN_MAIN_COUNT][100] = { L"Start", L"Clean" };
            EventProc btnCommand[BTN_MAIN_COUNT] = { (EventProc)BtnStartCommand, (EventProc)BtnCleanCommand };
            WCHAR szEdt[EDT_MAIN_COUNT][100] = { L"(Something)", L"Result:\r\n" };

            //Initialize each control here
            for ( i = 0; i < LAB_MAIN_COUNT; i++ )
            {
                ( (CLabel *)g_ctrlMain[LAB_INPUT + i] )->Init( IDC_LAB_INPUT + i, aHWnd, hInstance, szLab[i] );
            }

            for ( i = 0; i < BTN_MAIN_COUNT; i++ )
            {
                ( (CButton *)g_ctrlMain[BTN_START + i] )->Init( IDC_BTN_START + i, aHWnd, hInstance, szBtn[i] );
                g_ctrlMain[BTN_START + i]->OnCommand = btnCommand[i];
            }

            for ( i = 0; i < EDT_MAIN_COUNT; i++ )
            {
                if ( EDT_INPUT + i != EDT_SHOW )
                {
                    ( (CEdit *)g_ctrlMain[EDT_INPUT + i] )->Init( IDC_EDT_INPUT + i, aHWnd, hInstance, szEdt[i] );
                }
                else
                {
                    ( (CEdit *)g_ctrlMain[EDT_INPUT + i] )
                        ->Init( IDC_EDT_INPUT + i, aHWnd, hInstance, szEdt[i], 0, 0, 0, 0,
                                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | WS_HSCROLL | WS_VSCROLL |
                                    ES_AUTOHSCROLL | ES_AUTOVSCROLL );
                    ( (CEdit *)g_ctrlMain[EDT_INPUT + i] )->SetMaxLength( -1 );
                }
            }

            hBrush = CreateSolidBrush( GetSysColor( COLOR_WINDOW ) );
            break;
        }
        case WM_COMMAND:    //Receive when some commands is sent
        {
            int i;
            WORD dstCtrl = LOWORD( aWParam );
            for ( i = 0; i < CTRL_MAIN_COUNT; i++ )
            {
                if ( ( g_ctrlMain[i]->OnCommand != NULL ) && ( dstCtrl == g_ctrlMain[i]->Id() ) )
                {
                    if ( g_ctrlMain[i]->OnCommand( aHWnd, aWParam, aLParam, g_ctrlMain ) )
                        break;
                }
            }

            break;
        }
        case WM_NOTIFY:    //Receive when user do something
        {
            int i;
            HWND hwndFrom = ( (NMHDR *)aLParam )->hwndFrom;
            for ( i = 0; i < CTRL_MAIN_COUNT; i++ )
            {
                if ( ( g_ctrlMain[i]->OnNotify != NULL ) && ( hwndFrom == g_ctrlMain[i]->Self() ) )
                {
                    if ( g_ctrlMain[i]->OnNotify( aHWnd, aWParam, aLParam, g_ctrlMain ) )
                        break;
                }
            }
            break;
        }
        case WM_SIZE:    //Receive when windows size is changed
        {
            int nWidth = LOWORD( aLParam );
            int nHeight = HIWORD( aLParam );

            int i;
            for ( i = 0; i < LAB_MAIN_COUNT; i++ )
            {
                g_ctrlMain[LAB_INPUT + i]->SetWidthHeight( nWidth / 6, 20 );
            }

            for ( i = 0; i < BTN_MAIN_COUNT; i++ )
            {
                g_ctrlMain[BTN_START + i]->SetWidthHeight( nWidth / 6, 20 );
            }

            for ( i = 0; i < EDT_MAIN_COUNT - 1; i++ )
            {
                g_ctrlMain[EDT_INPUT + i]->SetWidthHeight( nWidth / 6, 20 );
            }


            g_ctrlMain[LAB_INPUT]->SetPos(
                nWidth / 2 - g_ctrlMain[EDT_INPUT]->Width() - 10 - g_ctrlMain[LAB_INPUT]->Width(), 10 );
            g_ctrlMain[EDT_INPUT]->SetPos( g_ctrlMain[LAB_INPUT]->PosX() + g_ctrlMain[LAB_INPUT]->Width() + 10,
                                           g_ctrlMain[LAB_INPUT]->PosY() - 2 );
            g_ctrlMain[BTN_START]->SetPos( g_ctrlMain[EDT_INPUT]->PosX() + g_ctrlMain[EDT_INPUT]->Width() + 10,
                                           g_ctrlMain[LAB_INPUT]->PosY() - 2 );

            g_ctrlMain[BTN_CLEAN]->SetPos( nWidth - 10 - g_ctrlMain[BTN_CLEAN]->Width(),
                                           g_ctrlMain[LAB_INPUT]->PosY() + g_ctrlMain[LAB_INPUT]->Height() + 10 );

            g_ctrlMain[EDT_SHOW]->SetWidthHeight(
                nWidth - 20, nHeight - g_ctrlMain[BTN_CLEAN]->PosY() - g_ctrlMain[BTN_CLEAN]->Height() - 20 );
            g_ctrlMain[EDT_SHOW]->SetPos( 10, g_ctrlMain[BTN_CLEAN]->PosY() + g_ctrlMain[BTN_CLEAN]->Height() + 10 );

            break;
        }
        case WM_PAINT:    //Receive when something need to be painted
        {
            break;
        }
        case WM_CTLCOLORSTATIC:    //Receive when drawing static(label) or edit
        {
            //Paint background color to window's color
            LONG lId = GetWindowLong( (HWND)aLParam, GWL_ID );

            if ( g_ctrlMain[LAB_MAIN_START]->Id() <= lId && lId < g_ctrlMain[LAB_MAIN_END]->Id() )
            {
                SetBkColor( (HDC)aWParam, GetSysColor( COLOR_WINDOW ) );
                return (LRESULT)hBrush;
            }
            break;
        }
        case WM_SYSCOLORCHANGE:    //Receive when a change is made to a system color setting
        {
            DeleteObject( hBrush );
            hBrush = CreateSolidBrush( GetSysColor( COLOR_WINDOW ) );
            return 0;
        }
        case WM_RBUTTONDOWN:    //Receive when user click right mouse
        {
            WCHAR szFileName[MAX_PATH];
            HINSTANCE hInstance = GetModuleHandle( NULL );

            GetModuleFileName( hInstance, szFileName, MAX_PATH );
            MessageBoxW( aHWnd, szFileName, L"This program is: ", MB_OK | MB_ICONINFORMATION );
            break;
        }
        case WM_CLOSE:    //Receive when user try to close the window
        {
            DestroyWindow( aHWnd );
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage( 0 );
            break;
        }
        default:
        {
            break;
        }
    }
    return DefWindowProc( aHWnd, aMsg, aWParam, aLParam );
}




INT WINAPI WinMain( HINSTANCE aHInstance, HINSTANCE aHPrevInstance, LPSTR aCmdLine, INT aCmdShow )
{
    UNREFERENCED_PARAMETER( aHPrevInstance );
    UNREFERENCED_PARAMETER( aCmdLine );

    WPP_INIT_TRACING( L"TestCWFileMonitor" );
    DbgOut( INFO, DBG_TEST, "Enter" );

    INT nRet = 0;
    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    CONST WCHAR szClassName[] = L"TestCWFileMonitor";

    WNDCLASSEXW wndClass;
    HWND hWnd;
    MSG msg;

    wndClass.cbSize = sizeof( WNDCLASSEX );
    wndClass.style = 0;    //CS_HREDRAW | CS_VREDRAW
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = aHInstance;
    wndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
    wndClass.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = szClassName;
    wndClass.hIconSm = LoadIcon( NULL, IDI_APPLICATION );

    if ( !RegisterClassEx( &wndClass ) )
    {
        ShowDebugMsg( L"Error: failed to register wndClass" );
        nRet = -1;
        goto exit;
    }

    hWnd = CreateWindowExW( WS_EX_CLIENTEDGE, szClassName, L"Main Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                            CW_USEDEFAULT, 800, 600, NULL, NULL, aHInstance, NULL );
    if ( hWnd == NULL )
    {
        ShowDebugMsg( L"Error: failed to create hWnd" );
        nRet = -1;
        goto exit;
    }

    UpdateWindow( hWnd );
    ShowWindow( hWnd, aCmdShow );

    do
    {
        //const WCHAR * wzPath = L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}";
        //const WCHAR * wzPath = L"G:\\";
        CFileMonitor monitor;
        monitor.SetFileChangedProc( FileChangedProc );

        wstring wstrPath;
        INT nDriverCount = CWUtils::GetVolumesPath( wstrPath );
        for ( INT i = 0; i < nDriverCount; i++ )
        {
            if ( DRIVE_FIXED == GetDriveType( &wstrPath[i * 4] ) )
            {
                if ( FALSE == monitor.Start( &wstrPath[i * 4], TRUE ) )
                {
                    ShowDebugMsg();
                }
            }
        }

        while ( GetMessageW( &msg, NULL, 0, 0 ) > 0 )
        {
            if ( !IsDialogMessage( hWnd, &msg ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }

        nRet = (INT)msg.wParam;
    } while ( 0 );



exit:
    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n", stopWatch.GetTotalIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO, DBG_TEST, "Leave" );
    WPP_CLEANUP();
    return nRet;
}
