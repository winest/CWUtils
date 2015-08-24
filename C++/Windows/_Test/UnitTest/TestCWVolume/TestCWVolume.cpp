#include "stdafx.h"
#include "resource.h"

#include "CWGeneralUtils.h"
#include "CWTime.h"
#include "CWVolume.h"
using namespace CWUtils;

#include "CWButton.h"
#include "CWEdit.h"
using namespace CWUi;

#include "_GenerateTmh.h"
#include "TestCWVolume.tmh"

#pragma comment( linker , "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"" )


LRESULT CALLBACK WndProc( HWND aHWnd , UINT aMsg , WPARAM aWParam , LPARAM aLParam )
{
    static CButton btnVolume[12];
    static WCHAR szBtnVolume[12][50] = { L"GetVolumes" , L"GetVolumesPath" , L"GetVolumesGuidPath" ,
                                         L"GetVolumePathFromGuidPath" , L"GetGuidPathFromMountPath" ,
                                         L"GetMountPathFromFileOrDirPath" , L"GetMountedFoldersFromGuidPath" ,
                                         L"GetVolumeType" , L"GetVolumeInfo" ,  L"GetVolumeInfo by handle" ,
                                         L"GetVolumeSpace" , L"SetVolumeName" };
    static CButton btnClear;
    static CEdit edtShow;

    static HFONT hFont;

    int i;

    switch( aMsg )
    {
        case WM_CREATE :    //Receive when the window is created
        {
            HINSTANCE hInstance = ((LPCREATESTRUCT)aLParam)->hInstance;

            //Create button
            for ( i = 0 ; i < 12 ; i++ )
            {
                btnVolume[i].Init( IDC_BTN_GET_VOLUMES + i , aHWnd , hInstance , szBtnVolume[i] , 300 , 30 );
            }
            btnClear.Init( IDC_BTN_CLEAR , aHWnd , hInstance , L"Clear" , 100 , 30 );
            edtShow.Init( IDC_EDT_SHOW , aHWnd , hInstance , L"Result\r\n" , 0 , 0 , 0 , 0 ,
                          WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE |
                          WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL );


            //Set the font of edtShow
            HDC hdc = GetDC( edtShow.Self() );
            hFont = CreateFont(
                                  -MulDiv( 12 , GetDeviceCaps(hdc , LOGPIXELSY) , 72 ) , 0 ,
                                  0 , 0 , FW_BOLD , FALSE , FALSE , FALSE , DEFAULT_CHARSET ,
                                  OUT_DEFAULT_PRECIS , CLIP_DEFAULT_PRECIS , DEFAULT_QUALITY , FF_DONTCARE ,
                                  L"Courier New"
                              );
            
            edtShow.SetFont( hFont );

            ReleaseDC( edtShow.Self() , hdc );
            break;
        }
        case WM_COMMAND :    //Receive when some commands is sent
        {
            switch ( HIWORD (aWParam) )
            {
                case BN_CLICKED:
                {
                    switch ( LOWORD(aWParam) )
                    {
                        case IDC_BTN_GET_VOLUMES :
                        {
                            WCHAR buf[256] , bmp[256];
                            DWORD ret = CWUtils::GetVolumes();
                            _itow_s( ret , bmp , 2 );
                            swprintf_s( buf , L"Drives bitmap: %lS\r\n" , bmp );
                            edtShow.AddText( buf );
                            break;
                        }
                        case IDC_BTN_GET_VOLUMES_PATH :
                        {
                            wstring wstrDrivers;
                            int count = CWUtils::GetVolumesPath( wstrDrivers );
                            for ( i = 0 ; i < count ; i++ )
                            {
                                edtShow.AddText( &wstrDrivers[i * 4] );
                                edtShow.AddText( L" " );
                            }
                            edtShow.AddText( L"\r\n" );
                            break;
                        }
                        case IDC_BTN_GET_VOLUMES_GUID_PATH :
                        {
                            wstring wstrGuids;
                            int count = CWUtils::GetVolumesGuidPath( wstrGuids );
                            for ( i = 0 ; i < count ; i++ )
                            {
                                edtShow.AddText( &wstrGuids[i * 50] );
                                edtShow.AddText( L"\r\n" );
                            }
                            break;
                        }





                        case IDC_BTN_GET_VOLUME_PATH_FROM_GUID_PATH :
                        {
                            wstring wstrGuids, wstrVolPath;
                            WCHAR buf[256];
                            int count = CWUtils::GetVolumesGuidPath( wstrGuids );
                            for ( i = 0 ; i < count ; i++ )
                            {
                                CWUtils::GetVolumePathFromGuidPath( &wstrGuids[i*50] , wstrVolPath );
                                swprintf_s( buf , L"%lS - %lS\r\n" , &wstrGuids[i*50] , wstrVolPath.c_str() );
                                edtShow.AddText( buf );
                            }
                            break;
                        }
                        case IDC_BTN_GET_GUID_PATH_FROM_MOUNT_PATH :
                        {
                            WCHAR mountPath[] = L"C:\\";
                            wstring wstrGuid;
                            WCHAR buf[256];
                            CWUtils::GetGuidPathFromMountPath( mountPath , wstrGuid );
                            swprintf_s( buf , L"%lS is on %lS\r\n" , mountPath , wstrGuid.c_str() );
                            edtShow.AddText( buf );
                            break;
                        }                        
                        case IDC_BTN_GET_MOUNT_PATH_FROM_FILE_OR_DIR_PATH :
                        {
                            wstring wstrDrive;
                            WCHAR buf[256];
                            WCHAR filePath[] = L"..";
                            CWUtils::GetMountPathFromFileOrDirPath( filePath , wstrDrive );
                            swprintf_s( buf , L"%lS is at drive %lS\r\n" , filePath , wstrDrive.c_str() );
                            edtShow.AddText( buf );
                            break;
                        }
                        case IDC_BTN_GET_MOUNTED_FOLDERS_FROM_GUID_PATH :
                        {
                            wstring wstrGuids;
                            WCHAR paths[26][MAX_PATH+1] = { 0 };
                            WCHAR buf[256];
                            int guidCount = CWUtils::GetVolumesGuidPath( wstrGuids );
                            for ( i = 0 ; i < guidCount ; i++ )
                            {
                                int mountCount = CWUtils::GetMountedFoldersFromGuidPath( &wstrGuids[i*50] , (WCHAR **)paths );

                                edtShow.AddText( &wstrGuids[i*50] );
                                edtShow.AddText( L"\r\n" );
                                for ( int j = 0 ; j < mountCount ; j++ )
                                {
                                    swprintf_s( buf , L"\t%lS\r\n" , paths[j] );
                                    edtShow.AddText( buf );    
                                }                                                            
                            }
                            break;
                        }







                        case IDC_BTN_GET_VOLUME_TYPE :
                        {
                            wstring wstrDrives , wstrType;
                            WCHAR buf[256];
                            int count = CWUtils::GetVolumesPath( wstrDrives );
                            for ( i = 0 ; i < count ; i++ )
                            {
                                CWUtils::GetVolumeType( &wstrDrives[i*4] , wstrType );
                                swprintf_s( buf , L"%lS - %lS\r\n" , &wstrDrives[i*4] , wstrType.c_str() );
                                edtShow.AddText( buf );
                            }
                            break;
                        }
                        case IDC_BTN_GET_VOLUME_INFO :
                        {
                            wstring wstrDrives , wstrName , wstrFs;
                            DWORD flag = 123 , serial = 456 , maxFileLen = 789;
                            WCHAR buf[256];
                            int count = CWUtils::GetVolumesPath( wstrDrives );
                            for ( i = 0 ; i < count ; i++ )
                            {
                                BOOL ret = CWUtils::GetVolumeInfo( &wstrDrives[i*4] , wstrName , wstrFs , NULL , &serial , &maxFileLen );
                                if ( ret == TRUE )
                                {
                                    swprintf_s( buf , L"%lS - %12lS - %6lS - flag: %u - serial: %u - maxFileLen: %u\r\n" , &wstrDrives[i*4] , wstrName.c_str() , wstrFs.c_str() , flag , serial , maxFileLen );
                                }
                                else
                                {
                                    swprintf_s( buf , L"%lS - Failed\r\n" , &wstrDrives[i*4] );
                                }
                                edtShow.AddText( buf );                                
                            }
                            break;
                        }
                        case IDC_BTN_GET_VOLUME_INFO_BY_HANDLE :
                        {
                            MessageBox( aHWnd , L"Support from Windows Vista" , L"Woops" , MB_OK );
                            break;
                        }
                        case IDC_BTN_GET_VOLUME_SPACE :
                        {
                            wstring wstrDrives;
                            ULARGE_INTEGER free , total , currFree;
                            free.QuadPart = 123;
                            total.QuadPart = 456;
                            currFree.QuadPart = 789;
                            WCHAR buf[256];
                            int count = CWUtils::GetVolumesPath( wstrDrives );
                            for ( i = 0 ; i < count ; i++ )
                            {
                                BOOL ret = CWUtils::GetVolumeSpace( &wstrDrives[i*4] , &free , &total , &currFree );
                                DWORD dwMb = 1024 * 1024;
                                DWORD dwGb = 1024 * 1024 * 1024;
                                if ( ret == TRUE )
                                {
                                    swprintf_s( buf , L"%lS - %4I64uGb %4I64uMb - %4I64uGb %4I64uMb - %4I64uGb %4I64uMb\r\n" , &wstrDrives[i*4] ,
                                               free.QuadPart / dwGb , ( free.QuadPart % dwGb ) / dwMb ,
                                               total.QuadPart / dwGb , ( total.QuadPart % dwGb ) / dwMb ,
                                               currFree.QuadPart / dwGb , ( currFree.QuadPart % dwGb ) / dwMb );
                                }
                                else
                                {
                                    swprintf_s( buf , L"%lS - Failed\r\n" , &wstrDrives[i*4] );
                                }
                                edtShow.AddText( buf );
                            }
                            break;
                        }
                        case IDC_BTN_SET_VOLUME_NAME :
                        {
                            CWUtils::SetVolumeName( L"C:\\" , L"Testing" );
                            break;
                        }
                        case IDC_BTN_CLEAR :
                        {
                            edtShow.SetText( L"" );
                            break;
                        }
                    }
                    break;
                }
                default :
                    break;
            }
            break;
        }
        case WM_SIZE :    //Receive when windows size is changed
        {
            int nClientWidth = LOWORD( aLParam );
            int nClientHeight = HIWORD( aLParam );

            //Reposition button
            btnVolume[0].SetPos( ( nClientWidth - btnVolume[0].Width() ) / 2 , 10 );
            for ( i = 1 ; i < 12 ; i++ )
            {
                btnVolume[i].SetPos( btnVolume[i-1].PosX() , btnVolume[i-1].PosY() + btnVolume[i-1].Height() + 10 );
            }
            btnClear.SetPos( nClientWidth - btnClear.Width() - 10 , btnVolume[11].PosY() + 30 );
            edtShow.SetPos( 10 , btnClear.PosY() + btnClear.Height() + 10 );
            edtShow.SetWidthHeight( nClientWidth - 20 , nClientHeight - edtShow.PosY() - 10 );
            break;
        }
        case WM_PAINT :    //Receive when something need to be painted
        {
            break;
        }
        case WM_RBUTTONDOWN :    //Receive when user click right mouse
        {
            WCHAR szFileName[MAX_PATH];
            HINSTANCE hInstance = GetModuleHandle( NULL );

            GetModuleFileName( hInstance , szFileName , MAX_PATH );
            MessageBox( aHWnd , szFileName , L"This program is: " , MB_OK | MB_ICONINFORMATION );
            break;
        }            
        case WM_CLOSE :    //Receive when user try to close the window
        {
            DestroyWindow( aHWnd );
            break;
        }
        case WM_DESTROY :
        {
            PostQuitMessage( 0 );
            break;
        }
        default :
        {
            break;
        }
    }
    return DefWindowProc( aHWnd , aMsg , aWParam , aLParam );
}


INT WINAPI WinMain( HINSTANCE aHInstance , HINSTANCE aHPrevInstance , LPSTR aCmdLine , INT aCmdShow )
{
    UNREFERENCED_PARAMETER( aHPrevInstance );
    UNREFERENCED_PARAMETER( aCmdLine );

    WPP_INIT_TRACING( L"TestCWVolume" );
    DbgOut( INFO , DBG_TEST , "Enter" );

    INT nRet = 0;
    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();

    const WCHAR szClassName[] = L"TestCWVolume";

    WNDCLASSEX wndClass;
    HWND hWnd;
    MSG msg;

    wndClass.cbSize        = sizeof( WNDCLASSEX );
    wndClass.style         = 0;    //用CS_HREDRAW | CS_VREDRAW的話，當視窗size改變時，整個視窗會重paint一次
    wndClass.lpfnWndProc   = WndProc;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = 0;
    wndClass.hInstance     = aHInstance;
    wndClass.hIcon         = LoadIcon( NULL , IDI_APPLICATION );
    wndClass.hCursor       = LoadCursor( NULL , IDC_ARROW );
    wndClass.hbrBackground = (HBRUSH) ( COLOR_WINDOW + 1 );
    wndClass.lpszMenuName  = NULL;
    wndClass.lpszClassName = szClassName;
    wndClass.hIconSm       = LoadIcon( NULL , IDI_APPLICATION );

    if ( ! RegisterClassEx( &wndClass ) )
    {
        CWUtils::ShowDebugMsg( L"Error: failed to register wndClass" );
        return -1;
    }
    else{}

/*
HWND CreateWindowEx(          
    DWORD dwExStyle ,
    LPCTSTR lpClassName, LPCTSTR lpWindowName ,
    DWORD dwStyle ,
    int x , int y , int nWidth , int nHeight ,
    HWND hWndParent , HMENU hMenu , HINSTANCE aHInstance , LPVOID lpParam
);
*/
    hWnd = CreateWindowExW
            (
                WS_EX_CLIENTEDGE ,
                szClassName , L"Title of window" ,
                WS_OVERLAPPEDWINDOW ,
                CW_USEDEFAULT , CW_USEDEFAULT , 800  , 600 ,
                NULL , NULL , aHInstance , NULL
            );
    if ( hWnd == NULL )
    {
        CWUtils::ShowDebugMsg( L"Error: failed to create hWnd" );
        return 0;
    }


    ShowWindow( hWnd, aCmdShow );
    UpdateWindow( hWnd );

/*
BOOL GetMessage( LPMSG lpMsg , HWND hWnd , UINT wMsgFilterMin , UINT wMsgFilterMax ); 

Return value:
If the function retrieves a message other than WM_QUIT, the return value is nonzero.
If the function retrieves the WM_QUIT message, the return value is zero. 
If there is an error, the return value is -1. To get extended error information, call GetLastError.
*/
    while( GetMessage( &msg , NULL , 0 , 0 ) > 0 )
    {
        if ( ! IsDialogMessage( hWnd , &msg ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }
    nRet = msg.wParam;
    
    stopWatch.Stop();
    wprintf_s( L"%I64u micro-sec\n" , stopWatch.GetIntervalInMicro() );
    wprintf_s( L"End of the program\n" );
    DbgOut( INFO , DBG_TEST , "Leave" );
    WPP_CLEANUP();
    return nRet;
}